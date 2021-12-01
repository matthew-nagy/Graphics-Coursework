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
float c_totXRot = 0.0f;
float c_totYRot = 0.0f;
/*
###################################################
###################################################

	Generalised class for drawing

###################################################
###################################################
*/

void rotateCamera(char dim, float degrees, glm::vec3& camera){
	float radians = degrees * 3.141592 / 180;
	float s = sin(radians);
	float c = cos(radians);

	glm::mat3 m;

	switch(dim){
		case 'x':
			m = glm::mat3(1, 0, 0,    0, c, s,    0, s*-1, c);
			c_totXRot += degrees;
			break;
		case 'y':
			m = glm::mat3(c, 0, s * -1,    0, 1, 0,    s, 0, c);
			c_totYRot -= degrees;
			break;
		default:
			printf("Invalid rotation dimention\n");
			return;
	}

	camera = m * camera;
}


struct Camera{
	glm::vec3 pos;
	glm::mat3 orientation;

	void move(float x, float y, float z){
		pos.x += x;
		pos.y += y;
		pos.z += z;
	}

	void rotate(float degrees, char dim, glm::vec3 center = glm::vec3(0, 0, 0)){
		pos = pos - center;

		rotateCamera(dim, degrees, pos);

		pos = pos + center;
	}

	void adjustOrientation(glm::mat3 by){
		orientation = by * orientation;
	}

	void adjustOrientation(float degrees, char dim){
		float rad = degrees * 3.141592 / 180;
		glm::mat3 by;
		float c = cos(rad);
		float s = sin(rad);
		
		switch(dim){
		case 'x':
			by = glm::mat3(
				1, 0, 0,
				0, c, s * -1,
				0, s, c
			);
			break;
		case 'y':
			by = glm::mat3(
				c, 0, s * -1,
				0, 1, 0,
				s, 0, c
			);
			break;
		default:
			printf("LMAO in defauilt\n");
			char nameb[2];
			nameb[0] = dim;
			nameb[1] = '\0';
			printf("Dim given was '%s'\n", nameb);
			return;
		}

		adjustOrientation(by);
	}

	void lookAt(const glm::vec3& position){
		glm::vec3 forwards = position - pos;
		glm::vec3 right = glm::cross(forwards, glm::vec3(0, 1, 0));
		glm::vec3 up = glm::cross(forwards, right);

		forwards = glm::normalize(forwards);
		right = glm::normalize(right);
		up = glm::normalize(up);

		right.x *= -1; right.y *= -1; right.z *= -1;

		orientation = glm::mat3(right, up, forwards);
	}

	glm::vec3 getAdjuastedVertex(glm::vec3 vertex){
		glm::vec3 cameraToVertex = pos - vertex;
		return cameraToVertex * orientation;
	}
};

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
    TextureMap* tMap;
	std::string name;
};
struct Vert{
	glm::vec3 pos;
	TexturePoint uvCoords;
	Material* material;
};

struct ObjFile{
	std::vector<glm::vec3> positions;
	std::vector<TexturePoint> texturePoints;
	std::unordered_map<std::string, Material> materials;

	std::vector< std::array<Vert, 3>> faces;


	void readInObj(std::ifstream& inFile);
	void readInMaterials(std::ifstream& inFile);
	std::vector<ModelTriangle> getModelTriangles(float scaleFactor = 1.0f);
};

size_t atou(const char* data){
	return size_t(std::atoi(data));
}

#define GSF( arr, in ) std::atof(arr[in].c_str())

void ObjFile::readInObj(std::ifstream& inFile){
	std::string currentLine = "";
	Material* currentMaterial = nullptr;
	while(std::getline(inFile, currentLine)){
		auto splits = splitStringOn(currentLine, ' ');
		if(splits[0] == "mtllib"){
			std::ifstream matFile(splits[1], std::ios::in);
			readInMaterials(matFile);
			matFile.close();
		}
		else if(splits[0] == "usemtl"){
			currentMaterial = &(materials[splits[1]]);
		}
		else if(splits[0] == "v"){
			positions.emplace_back( GSF(splits, 1), GSF(splits, 2), GSF(splits, 3) );
		}
		else if(splits[0] == "vt"){
			texturePoints.emplace_back(  GSF(splits, 1), GSF(splits, 2)  );
		}
		else if(splits[0] == "f"){
			std::array<Vert, 3> face;
			for(size_t i = 0; i < 3; i++){
				auto faceSplit = splitStringOn(splits[i + 1], '/');
				face[i].material = currentMaterial;
				face[i].pos = positions[atou(faceSplit[0].c_str()) - 1];
				if(faceSplit.size() > 1)
					face[i].uvCoords = texturePoints[atou(faceSplit[1].c_str()) - 1];
			}
			faces.emplace_back(face);
		}
	}
}

void ObjFile::readInMaterials(std::ifstream& inFile){
	std::string currentLine = "";
	bool onMtl = false;
	std::string mtlName = "";
	Material currentMtl;
	while(std::getline(inFile, currentLine)){
		auto splits = splitStringOn(currentLine, ' ');
		//finished, do the thing
		if(currentLine == ""){
			if(onMtl){
				onMtl = false;
				materials[mtlName] = currentMtl;
				currentMtl.tMap = nullptr;
			}
		}
		else if(splits[0] == "newmtl"){
			onMtl = true;
			mtlName = splits[1];
			currentMtl.name = splits[1];
		}
		else if(splits[0] == "Kd"){
			currentMtl.isColour = true;
			currentMtl.col = Colour( GSF(splits, 1) * 255, GSF(splits, 2) * 255, GSF(splits, 3) * 255 );
		}
		else if(splits[0] == "map_Kd"){
			currentMtl.isColour = false;
			currentMtl.tMap = new TextureMap(splits[1]);
		}
	}

	if(onMtl){
		materials[mtlName] = currentMtl;
	}
}


std::vector<ModelTriangle> ObjFile::getModelTriangles(float scaleFactor){
	std::vector<ModelTriangle> tris;

	for(auto& f : faces){
		ModelTriangle newTri;
		//If these vertices have different materials then by fuck I will flip
		Material* m = f[0].material;
		for(size_t i = 0; i < 3; i ++){
			newTri.vertices[i] = f[i].pos;
			if(!m->isColour){
				printf("Tex point was %f, %f\t is now ", f[i].uvCoords.x, f[i].uvCoords.y);
				newTri.texturePoints[i] = f[i].uvCoords;
				newTri.texturePoints[i].x *= m->tMap->width;
				newTri.texturePoints[i].y *= m->tMap->height;
				printf("%f, %f\n", newTri.texturePoints[i].x, newTri.texturePoints[i].y);
			}
		}

		if(m->isColour){
			newTri.texture = nullptr;
			newTri.colour = m->col;
		}
		else{
			newTri.texture = m->tMap;
		}

		tris.emplace_back(newTri);
	}

	printf("The texture points are:\n");
	for(size_t i = 0; i < texturePoints.size(); i++)
		printf("\t%f, %f\n", texturePoints[i].x, texturePoints[i].y);

	return tris;
}
//###########################################
//###########################################
//PARSER OVER
//###########################################
//###########################################


CanvasPoint getCanvasIntersectionPoint(Camera& camera, glm::vec3 vertexPos, float focalLength = 2.0f){
    //Scale the model to draw it nicer

    //vertexPos.z *= -1;
	//vertexPos.y *= -1;
	vertexPos.x *= -1;
    vertexPos.x -= camera.pos.x; vertexPos.y -= camera.pos.y; vertexPos.z -= camera.pos.z;


	vertexPos = camera.getAdjuastedVertex(vertexPos);



    CanvasPoint cp;

    cp.depth = 1 / (std::abs(camera.pos.z - vertexPos.z));

    cp.x = focalLength * (vertexPos.x / vertexPos.z); 
    cp.y = focalLength * (vertexPos.y / vertexPos.z); 

    cp.x *= 50.0;
    cp.y *= 50.0;

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

CanvasTriangle getCanvasTri(Camera& camera, const ModelTriangle& mTri, float focalLength = 2.0f){
	CanvasTriangle cTri;
	for(size_t i = 0; i < 3; i++){
		cTri[i] = getCanvasIntersectionPoint(camera, mTri.vertices[i], focalLength);
        cTri[i].texturePoint = mTri.texturePoints[i];
    }
	return cTri;
}


/*######################################
##	Welcome to raytracing hell!
########################################*/

const unsigned c_maxRayDistance = 20000;


struct RayResult{
	bool intersection = false;
	glm::vec3 beam;
	ModelTriangle const* tri = nullptr;
};


RayResult getClosestIntersection(const glm::vec3& cameraPosition, const glm::vec3& rayDirection, const std::vector<ModelTriangle>& tris){
	
	RayResult res;
	res.beam = glm::vec3(c_maxRayDistance);

	for(auto& triangle : tris){
		glm::vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
		glm::vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
		glm::vec3 SPVector = cameraPosition - triangle.vertices[0];
		glm::mat3 DEMatrix(-rayDirection, e0, e1);
		glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;

		if(possibleSolution.x > 0 && possibleSolution.y >= 0.0 && possibleSolution.y <= 1.0
			&& possibleSolution.z >= 0.0 && possibleSolution.z <= 1.0
			&& (possibleSolution.y + possibleSolution.z) <= 1.0){

			if(possibleSolution.x < res.beam.x){
				res.beam = possibleSolution;
				res.intersection = true;
				res.tri = &triangle;
			}
		}
	}

	return res;
}




std::array<float, 3> vec3ToArr(glm::vec3 v){
	std::array<float, 3> arr;
	arr[0] = v.x;
	arr[1] = v.y;
	arr[2] = v.z;
	return arr;
}

glm::vec3 arrToVec3(const std::array<float, 3>& arr){
	glm::vec3 v;
	v.x = arr[0];
	v.y = arr[1];
	v.z = arr[2];
	return v;
}
