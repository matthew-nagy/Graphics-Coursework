#include "Wireframe.hpp"
#include <chrono>
#include <thread>
#include <queue>
#include <mutex>
#include <RayTriangleIntersection.h>

float focal = 2.0f;

void drawPointCloud(Camera& camera, Window& window, const std::vector<ModelTriangle>& tris){
	for(auto& M : tris){
		CanvasTriangle cTri = getCanvasTri(camera, M, focal);
		for(auto& p : cTri.vertices)
			window.setPixelColour(p.x, p.y, getColourData(Colour(255, 255, 255)));	
	}
}

void drawWireframe(Camera& camera, Window& window, const std::vector<ModelTriangle>& tris){
	for(auto& M : tris){
		CanvasTriangle cTri = getCanvasTri(camera, M, focal);
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


void drawRayTraceWorker(Camera& camera, const std::vector<ModelTriangle>& tris, Window& window, std::mutex& drawm, std::mutex& workm, std::queue<unsigned>& ys){
	bool working = true;
	Colour row[WIDTH];
	while(working){
		unsigned y = 0;
		workm.lock();
		if(ys.size() > 0){
			y = ys.front();
			ys.pop();
		}
		else working = false;
		workm.unlock();
		if(!working)continue;

		for(unsigned x = 0; x < WIDTH; x++){
			RayResult res = getClosestIntersection(camera.pos, 
				glm::vec3((WIDTH / -2) + x, (HEIGHT / -2) + y, focal * -1), tris);
			if(res.intersection)
				row[x] = res.tri->colour;
			else
				row[x] = Colour(0, 0, 0);
		}

		drawm.lock();
		for(unsigned x = 0; x < WIDTH; x++)
			window.setPixelColour(x, y, getColourData(row[x]));
		drawm.unlock();
	}
}

void drawRayTaceView(Camera& camera, Window& window, const std::vector<ModelTriangle>& tris, int coreCount){
	std::mutex drawm, workm;
	std::queue<unsigned> ys;
	for(unsigned i = 0; i < HEIGHT; i++)ys.emplace(i);

	drawRayTraceWorker(camera, tris, window, drawm, workm, ys);
}

void TomCheatDraw(Camera& camera, Window& window, const std::vector<ModelTriangle>& tris){
	// RAY TRACE SCENE
		// For each pixel on the image plane
		for (auto i = 0; i < WIDTH; i++)
		{
			for (auto j = 0; j < HEIGHT; j++)
			{
				// Cast a ray from camera position, through pixel and into scene
				float dz = -focal;
				float dy = (HEIGHT / 2) - j;
				float dx = (WIDTH / 2) - i;

				glm::vec3 dir(dx, dy, dz);
				glm::vec3 normalised_dir = glm::normalize(dir) * glm::inverse(camera.orientation);

				// RayTriangleIntersection intersection = camera.getClosestIntersection(normalised_dir, model.triangles);
				RayResult intersection = getClosestIntersection(camera.pos, normalised_dir, tris);

				// IS THE POINT IN SHADOW?
				// Convert to world-space point
			/*	glm::vec3 intersection_worldspace = camera.pos + normalised_dir * intersection.beam.x;
				RayTriangleIntersection intersection_point_light = canSeeLight(intersection_worldspace, model.lightSource, model.triangles);

				// std::cout << "AAA: " << intersection_point_light.distanceFromCamera << std::endl;

				// If in shadow, draw black
				if (intersection_point_light.distanceFromCamera != 0)
				{
					window.setPixelColour(i, j, colourToUint32(Colour(0, 0, 0)));
				}
				else
				{*/
				if(intersection.intersection)
					window.setPixelColour(i, j, getColourData(intersection.tri->colour));
				//}
			}
		}
}

TextureMap tm;
void draw(DrawingWindow &window) {
	window.clearPixels();
}

enum DrawMode{Wire, Raster, Ray, Tom};
DrawMode drawmode = Wire;

bool orbit = false;

void handleEvent(SDL_Event event, Window &window, std::vector<ModelTriangle>& tris) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_z)drawmode = Wire;
		if (event.key.keysym.sym == SDLK_x)drawmode = Raster;
		if (event.key.keysym.sym == SDLK_c)drawmode = Ray;
		if (event.key.keysym.sym == SDLK_v)drawmode = Tom;
		else if (event.key.keysym.sym == SDLK_o) orbit = !orbit;
		else if (event.key.keysym.sym == SDLK_k)toDraw += 1;
		else if (event.key.keysym.sym == SDLK_l)toDraw = 1;
		else if (event.key.keysym.sym == SDLK_r)toDraw = 1000;
		else if (event.key.keysym.sym == SDLK_m)focal += 0.5;
		else if (event.key.keysym.sym == SDLK_n)focal -= 0.5;
	}
}


void handleCamera(SDL_Event event, Camera& camera, int64_t deltaT){
	int speed = 1;
	float pr = 1;
	float mr = pr * -1;
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) camera.move(speed * -1, 0, 0);
		else if (event.key.keysym.sym == SDLK_RIGHT) camera.move(speed, 0, 0);
		else if (event.key.keysym.sym == SDLK_UP) camera.move(0, speed * -1, 0);
		else if (event.key.keysym.sym == SDLK_DOWN) camera.move(0, speed, 0);

		if (event.key.keysym.sym == SDLK_a) {camera.rotate(pr, 'y'); camera.adjustOrientation(pr, 'y');}
		else if (event.key.keysym.sym == SDLK_d) {camera.rotate(mr, 'y'); camera.adjustOrientation(mr, 'y');}
		else if (event.key.keysym.sym == SDLK_w) {camera.rotate(mr, 'x'); camera.adjustOrientation(pr, 'x');}
		else if (event.key.keysym.sym == SDLK_s) {camera.rotate(pr, 'x'); camera.adjustOrientation(mr, 'x');}

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

	auto tris = objf.getModelTriangles(0.17);
	std::reverse(tris.begin(), tris.end());
	printf("There are %lu model triangles\n", tris.size());
	Camera camera;
	camera.pos = glm::vec3(0.0, 0.0, 3.0);
	camera.orientation = glm::mat3(
		1, 0, 0,
		0, 1, 0,
		0, 0, 1
	);
	int64_t deltaT = 0.0f;

	//std::cout<<o;
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	//SDL_Window dp(window);
	Depth_Window dp(window);

	SDL_Event event;
	float orbitSpeed = 0.1;


	/*RayResult rayRes = getClosestIntersection(camera.pos, glm::vec3(-0.1, -0.1, -2.0), tris);
	printf("Distance is %f\n", rayRes.beam.t);
	printf("Colour is apparently %d %d %d\n", rayRes.tri->colour.red, rayRes.tri->colour.green, rayRes.tri->colour.blue);*/

	while (true) {
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		dp.clearPixels();
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)){
			handleEvent(event, dp, tris);
			handleCamera(event, camera, deltaT);
		}
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		
		//drawPointCloud(camera, window, tris);
		//drawWireframe(camera, window, tris);
		switch(drawmode){
			case Wire:
				drawWireframe(camera, dp, tris);
				break;
			case Raster:
				drawRasterisedView(camera, dp, tris);
				break;
			case Ray:
				drawRayTaceView(camera, dp, tris, 12);
				break;
			case Tom:
				TomCheatDraw(camera, dp, tris);
		}
		window.renderFrame();
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		deltaT = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

		if(orbit){
			camera.rotate(orbitSpeed * deltaT / 1000, 'y');
			camera.lookAt(glm::vec3(0, 0, 0));
		}
	}
}
