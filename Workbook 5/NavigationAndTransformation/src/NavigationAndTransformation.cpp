#include "Wireframe.hpp"


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
		Colour c = M.colour;
		if(M.texture != nullptr) c = Colour(200, 200, 200);
		drawWireframe(cTri, window, c);	
	}
}

int toDraw = 1;
int tDraw = 0;

void drawRasterisedView(const glm::vec3& camera, Window& window, const std::vector<ModelTriangle>& tris){
	int count = 0;
	for(auto& M : tris){
		CanvasTriangle cTri = getCanvasTri(camera, M, 2.5f);
		if(M.texture != nullptr){
			drawRaster(cTri, window, M.texture);
			if(tDraw < 1){
				printf("Texture with ps,  %f,%f   %f,%f   %f,%f\n", cTri[0].texturePoint.x, cTri[0].texturePoint.y,
				cTri[1].texturePoint.x, cTri[1].texturePoint.y, cTri[2].texturePoint.x, cTri[2].texturePoint.y);
				tDraw += 1;
			}
		}
		else
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
	std::ifstream f("Cornell-box-upd.obj", std::ios::in);
	if(f.bad() || f.eof()){
		throw(0);
	}
	ObjFile objf;
	objf.readInObj(f);
	f.close();
	printf("There are %lu materials.\n", objf.materials.size());
//	printf("There are %lu vertices\n", objf.vertices.size());
	printf("There are %lu faces\n", objf.faces.size());

	auto tris = objf.getModelTriangles(1.0f);
	std::reverse(tris.begin(), tris.end());
	printf("There are %lu model triangles\n", tris.size());
	glm::vec3 camera(0.0, 0.0, -4.0);

	//std::cout<<o;
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
