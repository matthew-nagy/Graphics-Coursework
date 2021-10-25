#include <libs/sdw/CanvasTriangle.h>
#include <libs/sdw/DrawingWindow.h>
#include "libs/sdw/Colour.h"
#include "libs/sdw/CanvasPoint.h"
#include "libs/sdw/TextureMap.h"
#include "libs/sdw/ModelTriangle.h"
#include <libs/sdw/Utils.h>
#include "libs/glm-0.9.7.2/glm/glm.hpp"
#include <fstream>
#include <vector>
#include <algorithm>
#include <array>
#include <unordered_map>
#include <cstring>

#define WIDTH 600
#define HEIGHT 550
/*
###################################################
###################################################

	Generalised class for drawing

###################################################
###################################################
*/

class Window{
public:
	virtual void setPixelColour(unsigned x, unsigned y, uint32_t colour, float depth = 0.0f) = 0;

	virtual void clearPixels() = 0;

	virtual ~Window(){}
};

class SDL_Window : public Window{
public:
	void setPixelColour(unsigned x, unsigned y, uint32_t colour, float depth = 0.0f){
		t_window.setPixelColour(x, y, colour);
	}

	void clearPixels(){
		t_window.clearPixels();
	}

	SDL_Window(DrawingWindow& targetWindow):
		t_window(targetWindow)
	{}
private:
	DrawingWindow& t_window;
};

class Depth_Window : public Window{
public:
	void setPixelColour(unsigned x, unsigned y, uint32_t colour, float drawDepth){
		if(x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)return;
		if(drawDepth > depth[y][x]){
			t_window.setPixelColour(x, y, colour);
			depth[y][x] = drawDepth;
		}
	}

	void clearPixels(){
		t_window.clearPixels();
		memcpy(depth, clearDepth, sizeof(float) * HEIGHT * WIDTH);
	}

	Depth_Window(DrawingWindow& targetWindow):
		t_window(targetWindow)
	{
		for(size_t i = 0; i < HEIGHT; i++)
			for(size_t j = 0; j < WIDTH; j++)
				clearDepth[i][j] = depth[i][j] = -9999999.0f;
	}
private:
	DrawingWindow& t_window;
	float depth[HEIGHT][WIDTH];
	float clearDepth[HEIGHT][WIDTH];
};

/*
###################################################
###################################################

	Generalised functions for drawing triangles and
	lines to the screen

###################################################
###################################################
*/


uint32_t getColourData(const Colour& col, uint8_t a = 255){
	uint32_t ret = (a << 24) + (uint8_t(col.red) << 16) + (uint8_t(col.green) << 8) + uint8_t(col.blue);
	return ret;
}
uint32_t getColourData(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255){
	uint32_t ret = (a << 24) + (r << 16) + (g << 8) + b;
	return ret;
}
uint32_t getColourData(float x, float y, TextureMap const*const map){
	size_t index = std::round(x) + (std::round(y) * map->width);
	return map->pixels[index];
}

template<class T, size_t length>
using array_vector = std::vector<std::array<T, length>>;

//Returns a vector of arrays, with the given type and dimensionality. This lets you interpolate positions of any dimensions
template<class Type, size_t length>
array_vector<Type, length> interpolate(const std::array<Type, length>& from, const std::array<Type, length>& to, size_t count){
	array_vector<Type, length> toret;
	toret.reserve(count);

	std::array<Type, length> current = from;
	std::array<float, length> step;
	for(size_t i = 0; i < length; i++)
		step[i] = (to[i] - from[i]) / (count - 1);

	for(size_t i = 0; i < count; i++){

		toret.emplace_back(current);
		for(size_t j = 0; j < length; j++)
			current[j] += step[j];
	}
	
	return toret;
}

//Given some xs and ys, returns the length of the ionterpolated line that will be between them
template<class T>
unsigned getLen(T x1, T y1, T x2, T y2){
	return std::max( std::abs(x1 - x2), std::abs(y1 - y2) );
}

//Draws a line between two positions of T data type and dataDimentions dimentionality
//@param getColour- a lambda function that given a dimentional position and funcData will return the colour at this point. This lets you draw textured lines
//@param funcData- the external value given to getColour. Can be a Colour struct, a TextureMap, or something else to help the function run
template<class T, size_t dataDimentions>
void drawLine(const std::array<T, dataDimentions>& from, const std::array<T, dataDimentions>& to, Window& window, uint32_t(*getColour)(const std::array<T, dataDimentions>& pos, void* data), void* funcData){
	auto drawers = interpolate(from, to, getLen(from[0], from[1], to[0], to[1]) + 5);
	for(auto& p : drawers)
		window.setPixelColour(std::round(p[0]), std::round(p[1]), getColour(p, funcData), p[2]);
}

//Draws a wireframe between 3 n dimentional points. Assumes [0] is x, [1] is y and [2] is depth.
template<class T, size_t dataDimentions>
void drawWireframe(const std::array<std::array<T, dataDimentions>, 3>& tri, Window& window, Colour frameColour = Colour(255, 255, 255)){
	uint32_t(*colFunc)(const std::array<float, 3>&, void*) = [](const std::array<float, 3>&, void* colP)->uint32_t{return getColourData(*(Colour*)colP);};
	drawLine(std::array<float, 3>{tri[0][0], tri[0][1], tri[0][2]}, std::array<float, 3>{tri[1][0], tri[1][1], tri[1][2]}, window, colFunc, (void*)&frameColour);
	drawLine(std::array<float, 3>{tri[2][0], tri[2][1], tri[2][2]}, std::array<float, 3>{tri[1][0], tri[1][1], tri[1][2]}, window, colFunc, (void*)&frameColour);
	drawLine(std::array<float, 3>{tri[0][0], tri[0][1], tri[0][2]}, std::array<float, 3>{tri[2][0], tri[2][1], tri[2][2]}, window, colFunc, (void*)&frameColour);
}

//A specialized version of drawWireframe<float, 2> that draws off a CanvasTriangle rather than a gathering of n dimensional points
void drawWireframe(const CanvasTriangle& tri, Window& window, Colour frameColour = Colour(255, 255, 255)){
	drawWireframe(std::array<std::array<float, 3>, 3>
		{ std::array<float, 3>{tri[0].x, tri[0].y, tri[0].depth}, std::array<float, 3>{tri[1].x, tri[1].y, tri[1].depth}, 
	std::array<float, 3>{tri[2].x, tri[2].y, tri[2].depth} }, window, frameColour);
}

template<class T, size_t dataDimentions>
void drawRaster(std::array<std::array<T, dataDimentions>, 3> tri, Window& window, uint32_t(*getColour)(const std::array<T, dataDimentions>& pos, void* data), void* funcData){
	std::sort(tri.begin(), tri.end(), [](std::array<T, dataDimentions>& l, std::array<T, dataDimentions>& r)->bool{return l[1] < r[1];});
	auto ends = interpolate(tri[0], tri[2], (tri[2][1] - tri[0][1]) + 4);
	auto starts = interpolate(tri[0], tri[1], (tri[1][1] - tri[0][1]) + 2);
	auto addOn = interpolate(tri[1], tri[2], (tri[2][1] - tri[1][1]) + 2);
	starts.insert(starts.end(), addOn.begin(), addOn.end());

	for (size_t i = 0; i < ends.size() && i < starts.size(); i++)
		drawLine(starts[i], ends[i], window, getColour, funcData);
}

void drawRaster(const CanvasTriangle& tri, Window& window, Colour col){
	uint32_t(*colFunc)(const std::array<float, 3>&, void*) = [](const std::array<float, 3>&, void* colP)->uint32_t{return getColourData(*(Colour*)colP);};
	drawRaster(std::array<std::array<float, 3>, 3>
		{ std::array<float, 3>{tri[0].x, tri[0].y, tri[0].depth}, std::array<float, 3>{tri[1].x, tri[1].y, tri[1].depth}, 
	std::array<float, 3>{tri[2].x, tri[2].y, tri[2].depth} }, window,colFunc, (void*)&col  );
}



 void drawRaster(const CanvasTriangle& tri, Window& window, TextureMap* map){
 	uint32_t(*colFunc)(const std::array<float, 5>&, void*) = [](const std::array<float, 5>& pos, void* texturPoint)->uint32_t{
 		TextureMap* tex = (TextureMap*)texturPoint;
 		return getColourData(pos[3], pos[4], tex);
 	};
	
 	drawRaster(std::array<std::array<float, 5>, 3>
 		{ std::array<float, 5>{tri[0].x, tri[0].y, tri[0].depth, tri[0].texturePoint.x, tri[0].texturePoint.y}, 
 		std::array<float, 5>{tri[1].x, tri[1].y, tri[1].depth, tri[1].texturePoint.x, tri[1].texturePoint.y}, 
 		std::array<float, 5>{tri[2].x, tri[2].y, tri[2].depth, tri[2].texturePoint.x, tri[2].texturePoint.y} }, 
 	window, colFunc, (void*)map  );
}



/*
###################################################
###################################################

	Parser for obj files

###################################################
###################################################
*/

std::vector<std::string> splitStringOn(const std::string& in, char splitOn){
    std::vector<std::string> splits;
    splits.reserve(8);

    size_t start = 0;
    for(size_t i = 0; i < in.size(); i++){
        if(in[i] == splitOn){
            splits.emplace_back(in.substr(start, i - start));
            start = i + 1;
        }
    }

    if(start != in.size())
        splits.emplace_back(in.substr(start));

    return splits;
}

struct Material{
	bool isColour = true;
	Colour col;
};
struct Model{
	//Indexes into an ObjFile's faces vector
	std::vector<size_t> faces;
	std::string material;
};
struct ObjFile{
	std::vector<glm::vec3> vertices;
	std::vector<std::array<size_t, 3>> faces;
	std::unordered_map<std::string, Material> materials;
	//Stores each model as a vector of the faces it uses
	std::unordered_map<std::string, Model> models;

	void readInObj(std::ifstream& inFile);
	void readInMaterials(std::ifstream& inFile);
	std::vector<ModelTriangle> getModelTriangles(float scaleFactor = 1.0f);
};

size_t atou(const char* data){
	return size_t(std::atoi(data));
}

void ObjFile::readInObj(std::ifstream& inFile){
	std::string line;
	bool onObject = false;
	std::string currentObjName = "";
	std::string currentMaterialName = "";
	std::vector<size_t> currentModelFaces;


	while(std::getline(inFile, line)){
		auto splits = splitStringOn(line, ' ');
		if(splits[0] == "mtllib"){
			std::ifstream materialFile(splits[1].c_str(), std::ios::in);
			readInMaterials(materialFile);
			materialFile.close();
		}
		else if(splits[0] == ""){
			if(onObject){
				onObject = false;
				models[currentObjName] = {currentModelFaces, currentMaterialName};
			}
		}
		else if(splits[0] == "o"){
			onObject = true;
			currentObjName = splits[1];
			currentModelFaces.clear();
		}
		else if(splits[0] == "usemtl"){
			currentMaterialName = splits[1];
		}
		else if(splits[0] == "v"){
			vertices.emplace_back(std::atof(splits[1].c_str()), std::atof(splits[2].c_str()), std::atof(splits[3].c_str()));
		}
		else if(splits[0] == "f"){
			for(size_t i = 1; i < 4; i++)splits[i].pop_back();
			currentModelFaces.emplace_back(faces.size());
			faces.emplace_back(	std::array<size_t, 3>{atou(splits[1].c_str()) - 1, 
								atou(splits[2].c_str()) - 1, 
								atou(splits[3].c_str()) - 1}
			);

		}
	}
	if(onObject){
		onObject = false;
		models[currentObjName] = {currentModelFaces, currentMaterialName};
	}
}

void ObjFile::readInMaterials(std::ifstream& inFile){
	bool onMtl = false;
	std::string line;
	std::string currentMtlName = "";
	Material currentMtl;
	while(std::getline(inFile, line)){
		auto splits = splitStringOn(line, ' ');
		if(splits[0] == ""){
			if(onMtl){
				onMtl = false;
				materials[currentMtlName] = currentMtl;
			}
		}
		else if(splits[0] == "newmtl"){
			currentMtlName = splits[1];
			onMtl = true;
		}
		else if(splits[0] == "Kd"){
			currentMtl.isColour = true;
			currentMtl.col = Colour(
				std::atof(splits[1].c_str()) * 255,
				std::atof(splits[2].c_str()) * 255,
				std::atof(splits[3].c_str()) * 255
			);
		}
	}
	if(onMtl){
		onMtl = false;
		materials[currentMtlName] = currentMtl;
	}
}


std::vector<ModelTriangle> ObjFile::getModelTriangles(float scaleFactor){
	std::vector<ModelTriangle> tris;
	printf("There are %lu veriteces\n", this->vertices.size());
	tris.reserve(faces.size());
	for(auto& p : this->models){
		for(auto& faceIndices : p.second.faces){
			ModelTriangle tri;
			tri.colour = materials[p.second.material].col;
			for(size_t i = 0; i < 3; i++){
				printf("Requesting vertex %lu\n", faces[faceIndices][i]);
				tri.vertices[i] = vertices[faces[faceIndices][i]];
				tri.vertices[i].x *= scaleFactor;tri.vertices[i].y *= scaleFactor;tri.vertices[i].z *= scaleFactor;
			}
			tris.emplace_back(tri);
		}
	}


	return tris;
}
//###########################################
//###########################################
//PARSER OVER
//###########################################
//###########################################


CanvasPoint getCanvasIntersectionPoint(const glm::vec3& cameraPos, glm::vec3 vertexPos, float focalLength = 2.0f){
	focalLength = 6;
    //Scale the model to draw it nicer
    vertexPos.z *= -1;
	vertexPos.y *= -1;
    vertexPos.x -= cameraPos.x; vertexPos.y -= cameraPos.y; vertexPos.z -= cameraPos.z;





    CanvasPoint cp;

    cp.depth = 1 / vertexPos.z;

    cp.x = focalLength * (vertexPos.x / vertexPos.z); 

    cp.y = focalLength * (vertexPos.y / vertexPos.z); 



    cp.x *= 17;

    cp.y *= 17;



    cp.x += (WIDTH / 2);

    cp.y += (HEIGHT / 2);



    return cp;

}


/*CanvasPoint getCanvasIntersectionPoint(const glm::vec3& cameraPos, glm::vec3 vertexPos, float focalLength = 2.0f){
	//Scale the model to draw it nicer
	vertexPos.x *= -55;
	vertexPos.y *= 55;
	vertexPos.z *= -1;
	vertexPos.x -= cameraPos.x; vertexPos.y -= cameraPos.y; vertexPos.z -= cameraPos.z;

	CanvasPoint cp;
	cp.depth = 1 / vertexPos.z;
	cp.x = focalLength * (vertexPos.x / vertexPos.z) + (WIDTH / 2);
	cp.y = focalLength * (vertexPos.y / vertexPos.z) + (HEIGHT / 2);

	return cp;
}*/

CanvasTriangle getCanvasTri(const glm::vec3& cameraPos, const ModelTriangle& mTri, float focalLength = 2.0f){
	CanvasTriangle cTri;
	for(size_t i = 0; i < 3; i++)
		cTri[i] = getCanvasIntersectionPoint(cameraPos, mTri.vertices[i], focalLength);
	return cTri;
}

void drawPointCloud(const glm::vec3& camera, Window& window, const std::vector<ModelTriangle>& tris){
	for(auto& M : tris){
		CanvasTriangle cTri = getCanvasTri(camera, M);
		for(auto& p : cTri.vertices)
			window.setPixelColour(p.x, p.y, getColourData(Colour(255, 255, 255)));	
	}
}

void drawWireframe(const glm::vec3& camera, Window& window, const std::vector<ModelTriangle>& tris){
	for(auto& M : tris){
		CanvasTriangle cTri = getCanvasTri(camera, M);
		drawWireframe(cTri, window, M.colour);	
	}
}

int toDraw = 1;

void drawRasterisedView(const glm::vec3& camera, Window& window, const std::vector<ModelTriangle>& tris){
	int count = 0;
	for(auto& M : tris){
		CanvasTriangle cTri = getCanvasTri(camera, M);
		drawRaster(cTri, window, M.colour);	
		count ++;
		if(count > toDraw)
			return;
	}
}



TextureMap tm;
void draw(DrawingWindow &window) {
	window.clearPixels();
}

bool drawmode = true;

void handleEvent(SDL_Event event, Window &window, glm::vec3 camera, std::vector<ModelTriangle>& tris) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
		else if (event.key.keysym.sym == SDLK_u)drawmode = !drawmode;
		else if (event.key.keysym.sym == SDLK_o)window.clearPixels();
		else if (event.key.keysym.sym == SDLK_k)toDraw += 1;
		else if (event.key.keysym.sym == SDLK_l)toDraw = 1;
	}
}

int main(int argc, char *argv[]) {
	//char a;
	//std::cin>>a;
	std::ifstream f("Cornell-box.obj", std::ios::in);
	if(f.bad() || f.eof()){
		throw(0);
	}
	ObjFile objf;
	objf.readInObj(f);
	f.close();
	printf("There are %lu materials.\n", objf.materials.size());
	printf("There are %lu vertices\n", objf.vertices.size());
	printf("There are %lu faces\n", objf.faces.size());

	auto tris = objf.getModelTriangles(1.0f);
	std::reverse(tris.begin(), tris.end());
	printf("There are %lu model triangles\n", tris.size());
	glm::vec3 camera(0.0, 0.0, -4.0);

	//std::cout<<o;
	tm = TextureMap("texture.ppm");
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	//SDL_Window dp(window);
	Depth_Window dp(window);
	SDL_Event event;
	while (true) {
		dp.clearPixels();
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, dp, camera, tris);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		
		//drawPointCloud(camera, window, tris);
		//drawWireframe(camera, window, tris);
		if(drawmode) drawRasterisedView(camera, dp, tris);
		else drawWireframe(camera, dp, tris);
		window.renderFrame();
	}
}
