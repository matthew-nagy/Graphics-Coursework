#include "Canvas and raytrace.hpp"
#include "Workers.hpp"
#include <chrono>

float focal = 2.0f;

void drawPointCloud(Camera& camera, Window& window, const std::vector<ModelTriangle>& tris){
	for(auto& M : tris){
		CanvasTriangle cTri = getCanvasTri(camera, M);
		for(auto& p : cTri.vertices)
			window.setPixelColour(p.x, p.y, getColourData(Colour(255, 255, 255)));	
	}
}

void drawWireframe(Camera& camera, Window& window, const std::vector<ModelTriangle>& tris){
	for(auto& M : tris){
		CanvasTriangle cTri = getCanvasTri(camera, M);
		Colour c = M.colour;
		if(M.texture != nullptr) c = Colour(200, 200, 200);
		drawWireframe(cTri, window, c);	
	}
}

int toDraw = 100;
int tDraw = 0;

void drawRasterisedView(Camera& camera, Window& window, const std::vector<ModelTriangle>& tris){
	int count = 0;
	for(auto& M : tris){
		CanvasTriangle cTri = getCanvasTri(camera, M);
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

enum DrawMode{
	PointCloud, Wireframe, Raster, Raytrace
};
DrawMode drawmode = Raster;

bool orbit = false;

void handleEvent(SDL_Event event, Window &window, Camera& camera) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_z)drawmode = PointCloud;
		if (event.key.keysym.sym == SDLK_x)drawmode = Wireframe;
		if (event.key.keysym.sym == SDLK_c)drawmode = Raster;
		if (event.key.keysym.sym == SDLK_v)drawmode = Raytrace;

		
		if (event.key.keysym.sym == SDLK_o)orbit = !orbit;
		
		else if (event.key.keysym.sym == SDLK_m)camera.focalLength += 0.5;
		else if (event.key.keysym.sym == SDLK_n)camera.focalLength -= 0.5;
	}
}


void handleCamera(SDL_Event event, Camera& camera, int64_t deltaT){
	int speed = 1;
	float pr = 1;
	float mr = pr * -1;
	if (event.type == SDL_KEYDOWN) {
		if(event.key.keysym.sym == SDLK_w)
			camera.move(0, 0, -speed);
		if(event.key.keysym.sym == SDLK_s)
			camera.move(0, 0, speed);
		if(event.key.keysym.sym == SDLK_d)
			camera.move(-speed, 0 , 0);
		if(event.key.keysym.sym == SDLK_a)
			camera.move(speed, 0 , 0);
		if(event.key.keysym.sym == SDLK_1)
			camera.move(0, speed, 0);
		if(event.key.keysym.sym == SDLK_2)
			camera.move(0, -speed, 0);


		if(event.key.keysym.sym == SDLK_0)
			camera.lookAt(glm::vec3(0, 0, 0));
		if(event.key.keysym.sym == SDLK_LEFT)
			camera.rotatePositionAround('y', mr);
		if(event.key.keysym.sym == SDLK_RIGHT)
			camera.rotatePositionAround('y', pr);


		if (event.key.keysym.sym == SDLK_q) printf("Rotation is \tx: %f \ty: %f\n", c_totXRot, c_totYRot);
	}
}

void handleLight(SDL_Event event, Light& l){
	float lspeed = 0.1;
	if (event.type == SDL_KEYDOWN) {
		if(event.key.keysym.sym == SDLK_i)
			l.position.z -= lspeed;
		if(event.key.keysym.sym == SDLK_k)
			l.position.z += lspeed;
		if(event.key.keysym.sym == SDLK_j)
			l.position.x -= lspeed;
		if(event.key.keysym.sym == SDLK_l)
			l.position.x += lspeed;
		if(event.key.keysym.sym == SDLK_8)
			l.position.y -= lspeed;
		if(event.key.keysym.sym == SDLK_9)
			l.position.y += lspeed;
		
		if(event.key.keysym.sym == SDLK_RETURN){
			printf("%f %f %f\n", l.position.x, l.position.y, l.position.z);
		}
	}
}

int main(int argc, char *argv[]) {
	loadConfig();
	__rayLightInfo.load();
	

	pool::init();

	//std::ifstream f("RayTest.obj", std::ios::in);
	//std::ifstream f("Cornell-box-upd.obj", std::ios::in);
	//std::ifstream f("Sphere-low.obj", std::ios::in);
	std::string models[] = {"RayTest.obj", "Cornell-box-upd.obj", "Gouraud-Sphere-low.obj","Phong-Sphere-low.obj", "cornell-spicy.obj"};
	std::ifstream f(models[int(__config["model"])], std::ios::in);
	if(f.bad() || f.eof()){
		throw(0);
	}
	printf("j\n");
	ObjFile objf;
	objf.readInObj(f);
	f.close();

	auto model = objf.getModel(0.17f);
	Camera camera;
	camera.pos = camera.rayPos = glm::vec3(0.0, 0.0, 8.0);
	camera.orientation = camera.rayOrientation = glm::mat3(
		1, 0, 0,
		0, 1, 0,
		0, 0, 1
	);
	camera.focalLength = 2.0f;
	int64_t deltaT = 0.0f;

	//std::cout<<o;
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	//SDL_Window dp(window);
	Depth_Window dp(window);
	SDL_Event event;
	float orbitSpeed = 0.1;

	std::array<std::array<RayTriangleIntersection, WIDTH>, HEIGHT>* raytraceInfo = new std::array<std::array<RayTriangleIntersection, WIDTH>, HEIGHT>;

	float totalT = 0.0;
	unsigned fCount = 0;
	while (window.isOpen()) {
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		for(auto& t : model.triangles)
			t.foundGouraudThisFrame = false;

		dp.clearPixels();
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)){
			handleEvent(event, dp, camera);
			handleCamera(event, camera, deltaT);
			if(model.lights.size() > 0)
				handleLight(event, model.lights[0]);
		}
		if(orbit){
			camera.rotatePositionAround('y', orbitSpeed);
			camera.lookAt(glm::vec3(0,0,0));
		}
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		
		switch(drawmode){
			case PointCloud:
				drawPointCloud(camera, dp, model.triangles);
				break;
			case Wireframe:
				drawWireframe(camera, dp, model.triangles);
				break;
			case Raster:
				drawRasterisedView(camera, dp, model.triangles);
				break;
			case Raytrace:
				ray_raytraceInto(*raytraceInfo, model, camera);
				ray_drawResult(*raytraceInfo, dp);
				break;
		}
		for(auto& l : model.lights){
			glm::mat3 oo = camera.orientation;
			glm::vec3 op = camera.pos;
			camera.pos = camera.rayPos;
			camera.orientation = camera.rayOrientation;
			auto cip = getCanvasIntersectionPoint(camera, l.position, camera.focalLength);
			for(int i = -1; i < 2; i++)
				for(int j = -1; j < 2; j++)
					dp.setPixelColour(cip.x + j, cip.y + i, 0xFFFFFFFF, cip.depth);
			camera.pos = op;
			camera.orientation = oo;
		}
		window.renderFrame();
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		deltaT = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
		totalT += deltaT;
		fCount++;
		if(totalT >= 1000.0){
			totalT -= 1000.0;
			printf("> %u fps\n", fCount);
			fCount = 0;
		}
	}
	printf("About to shutdown\n");
	pool::shutdown();
}
