#include "include/voxel_data.hpp"
#include "include/drawers.hpp"
#include "include/voxel traversal.hpp"


TextureMap tm;
void draw(DrawingWindow &window) {
	window.clearPixels();
}

enum DrawMode{Wire, Raster, Ray};
DrawMode drawmode = Wire;

bool orbit = false;

void handleEvent(SDL_Event event, Window &window, std::vector<ModelTriangle>& tris) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_z)drawmode = Wire;
		if (event.key.keysym.sym == SDLK_x)drawmode = Raster;
		if (event.key.keysym.sym == SDLK_c)drawmode = Ray;
		else if (event.key.keysym.sym == SDLK_o) orbit = !orbit;
		else if (event.key.keysym.sym == SDLK_k)toDraw += 1;
		else if (event.key.keysym.sym == SDLK_l)toDraw = 1;
		else if (event.key.keysym.sym == SDLK_r)toDraw = 1000;
		else if (event.key.keysym.sym == SDLK_m)focal += 0.5;
		else if (event.key.keysym.sym == SDLK_n)focal -= 0.5;
	}
}


void handleCamera(SDL_Event event, Camera& camera){
	int speed = 1;
	float pr = 1;
	float mr = pr * -1;
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_a) camera.moveRel(speed * -1, 0, 0);
		else if (event.key.keysym.sym == SDLK_d) camera.moveRel(speed, 0, 0);
		else if (event.key.keysym.sym == SDLK_SPACE) camera.move(0, speed * -1, 0);
		else if (event.key.keysym.sym == SDLK_LSHIFT) camera.move(0, speed, 0);
		else if (event.key.keysym.sym == SDLK_w) camera.moveRel(0, 0, 1);
		else if (event.key.keysym.sym == SDLK_s) camera.moveRel(0, 0, -1);

		// if (event.key.keysym.sym == SDLK_a) {camera.rotate(pr, 'y'); camera.adjustOrientation(pr, 'y');}
		// else if (event.key.keysym.sym == SDLK_d) {camera.rotate(mr, 'y'); camera.adjustOrientation(mr, 'y');}
		// else if (event.key.keysym.sym == SDLK_w) {camera.rotate(mr, 'x'); camera.adjustOrientation(pr, 'x');}
		// else if (event.key.keysym.sym == SDLK_s) {camera.rotate(pr, 'x'); camera.adjustOrientation(mr, 'x');}

		if (event.key.keysym.sym == SDLK_q) printf("Rotation is \tx: %f \ty: %f\n", c_totXRot, c_totYRot);
	}

	auto md = getMouseChange();
	while(md.x < 0){
		camera.orientation = mouseLR_rot * camera.orientation;
		md.x += 1;
	}
	while(md.x > 0){
		camera.orientation = mouseLR_rotInv * camera.orientation;
		md.x -= 1;
	}
}


/*
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

	auto ddares = getDDACoords3D(glm::vec3(2, 2, 2), glm::vec3(9, 2, 2), 64, 64, 64);
	printf("DDA res is:\n");
	for(auto a : ddares)
		printf("\t%f %f %f\n", a.x, a.y, a.z);


	RayResult rayRes = getClosestIntersection(camera.pos, glm::vec3(-0.1, -0.1, -2.0), tris);
	printf("Distance is %f\n", rayRes.beam.t);
	printf("Colour is apparently %d %d %d\n", rayRes.tri->colour.red, rayRes.tri->colour.green, rayRes.tri->colour.blue);

	while (true) {
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		dp.clearPixels();
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)){
			handleEvent(event, dp, tris);
			handleCamera(event, camera);
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
			case Ray:break;
		}
		window.renderFrame();
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		deltaT = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

		if(orbit){
			camera.rotate(orbitSpeed * deltaT / 1000, 'y');
			camera.lookAt(glm::vec3(0, 0, 0));
		}
	}
}*/


int main(){
	initMouse();
	Camera camera;
	camera.pos = glm::vec3(32, 16.5, 0);
	camera.orientation = glm::mat3(
		1, 0, 0,
		0, 1, 0,
		0, 0, 1
	);

	//std::cout<<o;
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	//SDL_Window dp(window);
	SDL_Window dp(window);

	SDL_Event event;

	vx::Material_Voxel_Chunk mvc(7, 4);
	//vx::Material_Voxel_Chunk mvc; for(size_t i = 0; i < 40; i++) for(size_t j = 0; j < 40; j++) mvc.getIndex(i, j, 40) = (i + j) % 2 == 0 ? 5 : 3;
	
	mvc.getIndex(20, 20, 39) = 7;
	mvc.getIndex(30, 20, 37) = 7;
	mvc.getIndex(30, 21, 37) = 7;
	mvc.getIndex(31, 20, 37) = 7;
	mvc.getIndex(31, 21, 37) = 7;
	std::vector<vx::Material> mat;
	mat.emplace_back();
	mat.emplace_back(255, 255, 255);
	mat.emplace_back(255, 0, 0);
	mat.emplace_back(0, 255, 0);
	mat.emplace_back(0, 0, 255);
	mat.emplace_back(200, 0, 200);
	mat.emplace_back(0, 150, 255);
	mat.emplace_back(240, 100, 30, 50);

	std::vector<ModelTriangle> etri;

	while (true) {
		
		dp.clearPixels();
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)){
			handleEvent(event, dp, etri);
			handleCamera(event, camera);
		}
		
		//vd(camera, mvc, dp, mat);
		vdWithTransparency(camera, mvc, dp, mat);

		window.renderFrame();
	}
}


