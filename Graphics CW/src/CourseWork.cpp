#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include "DrawingLines.hpp"
#include "Interpolate.hpp"
#include "Window.hpp"
#include "Parser.hpp"
#include "DrawingTriangles.hpp"

#define WIDTH 320
#define HEIGHT 240

void draw(Window &window) {
	window.clearPixels();
	for (size_t y = 0; y < window.height(); y++) {
		for (size_t x = 0; x < window.width(); x++) {
			float red = rand() % 256;
			float green = 0.0;
			float blue = 0.0;
			Colour c(red, blue, green);
			window.setPixel(x, y, c, 200.0);
		}
	}
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[]) {

	Window window(1);
	window.setViewTopLeft(-100, 100);

	auto damnHackspace = getObjFromFile("Cornell-box-upd.obj");
	Camera c;
	c.orientation = glm::mat3(1,0,0,  0,1,0   ,0,0,1);
	c.position = glm::vec3(0, 0, -4);
	c.focalLength = 10.0f;
	c.viewDimensions = glm::vec2(WIDTH, HEIGHT);

	printf("After camera\n");

	SDL_Event event;
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.sdlData().pollForInputEvents(event)) handleEvent(event, window.sdlData());
		//draw(window);
		//drawSceneRaster(damnHackspace, c, window);
		drawSceneWireframe(damnHackspace, c, window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}

}
