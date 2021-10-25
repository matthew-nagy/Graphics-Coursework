#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <glm/glm.hpp>
#include <vector>

#define WIDTH 800
#define HEIGHT 600

glm::vec3 operator-(const glm::vec3& left, const glm::vec3& right){
	return {left.x - right.x, left.y - right.y, left.z - right.z};
}
glm::vec3 operator+(const glm::vec3& left, const glm::vec3& right){
	return {left.x + right.x, left.y + right.y, left.z + right.z};
}
glm::vec3 operator/(const glm::vec3& left, unsigned right){
	return {left.x / float(right), left.y / float(right), left.z / float(right)};
}

template<class T>
std::vector<T> interpolate(T from, T to, unsigned numberOfParameters){
	std::vector<T> floats;
	floats.reserve(numberOfParameters);
	T val = from;
	T step = (to - from) / (numberOfParameters - 1);

	for(size_t i = 0; i < numberOfParameters; i++){
		floats.emplace_back(val);
		val = val + step;
	}

	return floats;
}

uint32_t getColour(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255){
	return (int(a) << 24) + (int(r) << 16) + (int(g) << 8) + int(b);
}

void drawColours(DrawingWindow& window){
	glm::vec3 topLeft(255, 0, 0);        // red 
	glm::vec3 topRight(0, 0, 255);       // blue 
	glm::vec3 bottomRight(0, 255, 0);    // green 
	glm::vec3 bottomLeft(255, 255, 0);   // yellow

	auto colourLeftSide = interpolate<glm::vec3>(topLeft, bottomLeft, window.height);
	auto colourRightSide = interpolate<glm::vec3>(topRight, bottomRight, window.height);

	window.clearPixels();
	for (size_t y = 0; y < window.height; y++) {
		auto colRow = interpolate<glm::vec3>(colourLeftSide[y], colourRightSide[y], window.width);
		for (size_t x = 0; x < window.width; x++) {
			uint32_t colour = getColour(colRow[x].x, colRow[x].y, colRow[x].z);
			window.setPixelColour(x, y, colour);
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
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);


	SDL_Event event;
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		
		drawColours(window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
