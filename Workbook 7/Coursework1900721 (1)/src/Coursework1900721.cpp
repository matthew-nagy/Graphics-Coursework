#include "Scripting.hpp"

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
		if(event.key.keysym.sym == SDLK_UP)
			camera.rotateViewY(mr);
		if(event.key.keysym.sym == SDLK_DOWN)
			camera.rotateViewY(pr);
		if(event.key.keysym.sym == SDLK_LEFT)
			camera.rotateViewX(mr);
		if(event.key.keysym.sym == SDLK_RIGHT)
			camera.rotateViewX(pr);

		if(event.key.keysym.sym == SDLK_RCTRL){
			CameraPosFrame(camera).printOut();
			CameraRotFrame(camera).printout();
		}
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


void handleFlags(SDL_Event event){
	if(event.key.keysym.sym == SDLK_f)
		mode::setter = true;
	if(event.key.keysym.sym == SDLK_g)
		mode::setter = false;
	if(event.key.keysym.sym == SDLK_e)
		mode::shadows = mode::setter;
	if(event.key.keysym.sym == SDLK_r)
		mode::reflections = mode::setter;
	if(event.key.keysym.sym == SDLK_t)
		mode::shadings = mode::setter;
	if(event.key.keysym.sym == SDLK_y)
		mode::quality = mode::setter;
	if(event.key.keysym.sym == SDLK_u)
		mode::cellShading = mode::setter;
	if(event.key.keysym.sym == SDLK_TAB){
		mode::superAntiAliasing = mode::setter;
		if(mode::setter)
			mode::fastAproxAntiAliasing = false;
	}
	if(event.key.keysym.sym == SDLK_q){
		mode::fastAproxAntiAliasing = mode::setter;
		if(mode::setter)
			mode::superAntiAliasing = false;
	}

	if(event.key.keysym.sym == SDLK_RSHIFT)
		FlagPack(true).printOut();

}

int main(int argc, char *argv[]) {

	auto x = interpolateWeighted<float, 1>(std::array<float, 1>{0}, std::array<float, 1>{1}, 10, [](float prop){return float(sin(prop * 3.141592 / 2.0));});
	for(size_t i = 0; i < 10; i++)
		printf("%f\n", x[i][0]);


	loadConfig();
	__rayLightInfo.load();
	

	pool::init();

	//std::ifstream f("RayTest.obj", std::ios::in);
	//std::ifstream f("Cornell-box-upd.obj", std::ios::in);
	//std::ifstream f("Sphere-low.obj", std::ios::in);
	std::string models[] = {"RayTest.obj", "Cornell-box-upd.obj", "Gouraud-Sphere-low.obj","Phong-Sphere-low.obj", "cornell-spicy.obj", "WaterScene.obj"};
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

	float totalT = 0.0;
	unsigned fCount = 0;

	int fsInChat = 0;

	//LMAO render the scene
	if(__config["render"] == 1.0){
		ScriptedAnimation myAnimation(camera, model, &dp, &window, "Main-scene");
		printf("About to load the scene\n");
		myAnimation.loadProgram("MainCornell.anim");
		printf("Scene loaded\n");
			myAnimation.render();
		printf("Scene has been rendered\n");
	}

	printf("Regular while loop\n");
	bool ron = false;
	while (window.isOpen()) {
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		for(size_t i = 0; i < __DFX.size() ;i++)
					__DFX[i]->frameStep();

		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)){
			handleEvent(event, dp, camera);
			handleCamera(event, camera, deltaT);
			handleFlags(event);
			if(model.lights.size() > 0)
				for(size_t i = 0; i < model.lights.size(); i++)
					handleLight(event, model.lights[i]);

			if(event.key.keysym.sym == SDLK_RALT){
				if(!ron){
				std::string fsInString = std::to_string(fsInChat);
				while(fsInString.size() < 4)
					fsInString = "0" + fsInString;
					window.savePPM(std::string("Frames/Final-" + std::string(fsInString) + ".ppm"));
				fsInChat += 1;
				printf("Saved image '%s'\n", std::string("Frames/Final-" + std::string(fsInString) + ".ppm").c_str());
				
				ron = true;
				}
			}
			else ron = false;
		}
		dp.clearPixels();
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
				if(mode::superAntiAliasing){
					ray_raytraceInto<WIDTH*mode::superScalerQuality, HEIGHT*mode::superScalerQuality>(*superScalerRaytraceInfo, model, camera);
					ray_drawResult<WIDTH*mode::superScalerQuality, HEIGHT*mode::superScalerQuality>(*superScalerRaytraceInfo, dp);
				}
				else{
					ray_raytraceInto<WIDTH, HEIGHT>(*raytraceInfo, model, camera);
					ray_drawResult<WIDTH, HEIGHT>(*raytraceInfo, dp);
				}
				break;
		}
		window.renderFrame();
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		deltaT = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
		totalT += deltaT;
		fCount++;
		if(totalT >= 1000.0){
			totalT -= 1000.0;
			//printf("> %u fps\n", fCount);
			fCount = 0;
		}
	}
	printf("About to shutdown\n");
	pool::shutdown();
}
