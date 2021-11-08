#include "Wireframe.hpp"
#include <chrono>

float focal = 2.0f;

void drawPointCloud(const glm::vec3& camera, Window& window, const std::vector<ModelTriangle>& tris){
	for(auto& M : tris){
		CanvasTriangle cTri = getCanvasTri(camera, M, focal);
		for(auto& p : cTri.vertices)
			window.setPixelColour(p.x, p.y, getColourData(Colour(255, 255, 255)));	
	}
}

void drawWireframe(const glm::vec3& camera, Window& window, const std::vector<ModelTriangle>& tris){
	for(auto& M : tris){
		CanvasTriangle cTri = getCanvasTri(camera, M, focal);
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
		CanvasTriangle cTri = getCanvasTri(camera, M, focal);
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
		if (event.key.keysym.sym == SDLK_u)drawmode = !drawmode;
		else if (event.key.keysym.sym == SDLK_o)window.clearPixels();
		else if (event.key.keysym.sym == SDLK_k)toDraw += 1;
		else if (event.key.keysym.sym == SDLK_l)toDraw = 1;
		else if (event.key.keysym.sym == SDLK_r)toDraw = 1000;
		else if (event.key.keysym.sym == SDLK_m)focal += 0.5;
		else if (event.key.keysym.sym == SDLK_n)focal -= 0.5;
	}
}

float c_totXRot = 0.0f;
float c_totYRot = 0.0f;

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

void handleCamera(SDL_Event event, glm::vec3& camera, int64_t deltaT){
	int speed = 1;
	float pr = 0.3;
	float mr = pr * -1;
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) camera.x -= speed;
		else if (event.key.keysym.sym == SDLK_RIGHT) camera.x += speed;
		else if (event.key.keysym.sym == SDLK_UP) camera.y -= speed;
		else if (event.key.keysym.sym == SDLK_DOWN) camera.y += speed;

		if (event.key.keysym.sym == SDLK_a) rotateCamera('y', pr, camera);
		else if (event.key.keysym.sym == SDLK_d) rotateCamera('y', mr, camera);
		else if (event.key.keysym.sym == SDLK_w) rotateCamera('x', mr, camera);
		else if (event.key.keysym.sym == SDLK_s) rotateCamera('x', pr, camera);

		if (event.key.keysym.sym == SDLK_q) printf("Rotation is \tx: %f \ty: %f\n", c_totXRot, c_totYRot);
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

	auto tris = objf.getModelTriangles(0.17f);
	std::reverse(tris.begin(), tris.end());
	printf("There are %lu model triangles\n", tris.size());
	glm::vec3 camera(0.0, 0.0, 8.0);
	int64_t deltaT = 0.0f;

	//std::cout<<o;
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	//SDL_Window dp(window);
	Depth_Window dp(window);
	SDL_Event event;
	while (true) {
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		dp.clearPixels();
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)){
			handleEvent(event, dp, camera, tris);
			handleCamera(event, camera, deltaT);
		}
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		
		//drawPointCloud(camera, window, tris);
		//drawWireframe(camera, window, tris);
		if(drawmode) drawRasterisedView(camera, dp, tris);
		else drawWireframe(camera, dp, tris);
		window.renderFrame();
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		deltaT = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
	}
}
