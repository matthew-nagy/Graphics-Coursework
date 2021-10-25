#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <CanvasPoint.h>
#include <TextureMap.h>
#include <CanvasTriangle.h>
#include <Colour.h>
#include <Utils.h>
#include <fstream>
#include <algorithm>
#include <vector>
#include <glm/glm.hpp>

#define WIDTH 800
#define HEIGHT 600

glm::vec2 tptov2(const TexturePoint& tp){
	return glm::vec2(tp.x, tp.y);
}
TexturePoint v2totp(const glm::vec2& v2){
	return TexturePoint(v2.x, v2.y);
}

glm::vec3 operator-(const glm::vec3& left, const glm::vec3& right){
	return {left.x - right.x, left.y - right.y, left.z - right.z};
}
glm::vec3 operator+(const glm::vec3& left, const glm::vec3& right){
	return {left.x + right.x, left.y + right.y, left.z + right.z};
}
glm::vec3 operator/(const glm::vec3& left, unsigned right){
	return {left.x / float(right), left.y / float(right), left.z / float(right)};
}
glm::vec2 operator-(const glm::vec2& left, const glm::vec2& right){
	return {left.x - right.x, left.y - right.y};
}
glm::vec2 operator+(const glm::vec2& left, const glm::vec2& right){
	return {left.x + right.x, left.y + right.y};
}
glm::vec2 operator/(const glm::vec2& left, unsigned right){
	return {left.x / float(right), left.y / float(right)};
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
uint32_t getColour(const Colour& col, uint8_t a = 255){
	return (int(a) << 24) + (int(col.red) << 16) + (int(col.green) << 8) + int(col.blue);
}
uint32_t getColour(const TextureMap& tex, const TexturePoint& point){
	size_t index = point.x;
	index += tex.width * point.y;
	return tex.pixels[index];
}

std::array<CanvasPoint, 3> orderPoints(const std::array<CanvasPoint, 3>& p){
	std::array<CanvasPoint, 3> ret = p;
	
	std::sort(ret.begin(), ret.end(), [](CanvasPoint& left, CanvasPoint& right)->bool{return left.y < right.y;});

	return ret;
}

std::vector<glm::vec2> interpolateCanvasPoints(const CanvasPoint& from, const CanvasPoint& to, unsigned distMax = 0){
	//Get maximum distance, + 1 for rounding error
	if(distMax == 0) distMax = std::max( std::abs(from.x - to.x), std::abs(from.y - to.y) ) + 1;
	return interpolate<glm::vec2>(glm::vec2(from.x, from.y), glm::vec2(to.x, to.y), distMax);
}

void drawLine(const CanvasPoint& from, const CanvasPoint& to, DrawingWindow& window, const Colour& col){
	//Get maximum distance, + 1 for rounding error
	auto positions = interpolateCanvasPoints(from, to);

	uint32_t slockCol = getColour(col);
	for(auto p : positions)
		window.setPixelColour(p.x, p.y, slockCol);
}
void drawLine(const glm::vec2& from, const glm::vec2& to, DrawingWindow& window, const Colour& col){
	//Get maximum distance, + 1 for rounding error
	auto positions = interpolate(from, to, std::max( std::abs(to.x - from.x), std::abs(to.y - from.y)) + 10);

	uint32_t slockCol = getColour(col);
	for(auto p : positions)
		window.setPixelColour(p.x, p.y, slockCol);
}

void drawTextureLine(const glm::vec2& from, const glm::vec2& to, const TexturePoint& fromT, const TexturePoint& endT, const TextureMap& tex, DrawingWindow& window){
	//Get maximum distance, + 1 for rounding error
	auto positions = interpolate(from, to, std::max( std::abs(to.x - from.x), std::abs(to.y - from.y)) + 10);
	auto texturePositions = interpolate(tptov2(fromT), tptov2(endT), std::max( std::abs(to.x - from.x), std::abs(to.y - from.y)) + 10);

	for(size_t i=0; i < positions.size(); i++)
		window.setPixelColour(positions[i].x, positions[i].y, getColour(tex, v2totp(texturePositions[i])));
}

struct Request{
	CanvasTriangle t;
	Colour c;
	bool rasterised;

	Request(CanvasTriangle t, Colour c, bool r):
		t(t),
		c(c),
		rasterised(r)
	{}
};
void drawStrokedTriangle(const CanvasTriangle& tri, DrawingWindow& window, const Colour& col){
	drawLine(tri[0], tri[1], window, col);
	drawLine(tri[1], tri[2], window, col);
	drawLine(tri[2], tri[0], window, col);
}

glm::vec2 cptov(const CanvasPoint& cp){
	return glm::vec2(cp.x, cp.y);
}

float pythagoras(glm::vec2 l, glm::vec2 r){
	float xdif = std::abs(l.x - r.x);
	float ydif = std::abs(l.y - r.y);
	return sqrt((xdif * xdif) + (ydif * ydif));
}

void drawRasterTriangle(Request t, DrawingWindow& window){
	std::array<CanvasPoint, 3> orderedPoints = orderPoints(t.t.vertices);

	auto ys = interpolate<int>(std::round(orderedPoints[0].y), std::round(orderedPoints[2].y),( std::round(orderedPoints[2].y) - std::round(orderedPoints[0].y)));
	auto endXs = interpolate(orderedPoints[0].x, orderedPoints[2].x, ys.size());
	auto startX = interpolate(orderedPoints[0].x, orderedPoints[1].x, std::abs(  std::round(orderedPoints[0].y) -  std::round(orderedPoints[1].y)  ));
	auto startPartX = interpolate(orderedPoints[1].x, orderedPoints[2].x, endXs.size() - startX.size());
	startX.insert(startX.end(), startPartX.begin(), startPartX.end());

	for(size_t i =0; i <ys.size(); i++){
		drawLine(glm::vec2(startX[i], int(ys[i])), glm::vec2(endXs[i], int(ys[i])), window, t.c);
	}
}

void drawTextureTriangle(const CanvasTriangle& triangle, const TextureMap& tex, DrawingWindow& window){
	std::array<CanvasPoint, 3> orderedPoints = orderPoints(triangle.vertices);

	auto ys = interpolate<int>(std::round(orderedPoints[0].y), std::round(orderedPoints[2].y),( std::round(orderedPoints[2].y) - std::round(orderedPoints[0].y)));
	
	auto endXs = interpolate(orderedPoints[0].x, orderedPoints[2].x, ys.size());
	auto startX = interpolate(orderedPoints[0].x, orderedPoints[1].x, std::abs(  std::round(orderedPoints[0].y) -  std::round(orderedPoints[1].y)  ));
	auto startPartX = interpolate(orderedPoints[1].x, orderedPoints[2].x, endXs.size() - startX.size());
	startX.insert(startX.end(), startPartX.begin(), startPartX.end());

	auto fullLineTP = interpolate(tptov2(orderedPoints[0].texturePoint), tptov2(orderedPoints[2].texturePoint), ys.size());
	auto startTP = interpolate(tptov2(orderedPoints[0].texturePoint), tptov2(orderedPoints[1].texturePoint), std::abs(  std::round(orderedPoints[0].y) -  std::round(orderedPoints[1].y)  ));
	auto lastPartStartTP = interpolate(tptov2(orderedPoints[1].texturePoint), tptov2(orderedPoints[2].texturePoint), fullLineTP.size() - startTP.size());
	startTP.insert(startTP.end(), lastPartStartTP.begin(), lastPartStartTP.end());

	for(size_t i =0; i <ys.size(); i++){
		drawTextureLine(glm::vec2(startX[i], ys[i]), glm::vec2(endXs[i], ys[i]), v2totp(startTP[i]), v2totp(fullLineTP[i]), tex, window);
	}

	drawStrokedTriangle(triangle, window, Colour(255, 255, 255));
}

std::vector<Request> tringls;
void generateTriangle(bool raster = false){
	CanvasTriangle t;
	for(size_t i = 0; i < 3; i++){
		t[i].x = rand() % WIDTH;
		t[i].y = rand() % HEIGHT;
	}
	Colour c(rand() % 256, rand() % 256, rand() % 256);

	tringls.emplace_back(t, c, raster);
}

void draw(DrawingWindow &window) {
	window.clearPixels();
	for(auto t : tringls){
		if(t.rasterised){
			drawRasterTriangle(t, window);
			drawStrokedTriangle(t.t, window, Colour(255, 255, 255));
		}
		else drawStrokedTriangle(t.t, window, t.c);
	}
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
		else if (event.key.keysym.sym == SDLK_u) generateTriangle();
		else if (event.key.keysym.sym == SDLK_f) generateTriangle(true);
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;

	TextureMap tex("texture.ppm");
	CanvasTriangle testTexTriangle(
		CanvasPoint(160, 10), CanvasPoint(300, 230), CanvasPoint(10, 150)
	);
	testTexTriangle[0].texturePoint = TexturePoint(195, 5);
	testTexTriangle[1].texturePoint = TexturePoint(395, 380);
	testTexTriangle[2].texturePoint = TexturePoint(65, 330);

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		draw(window);
		drawTextureTriangle(testTexTriangle, tex, window);

		//for(size_t y = 0; y < tex.height; y++)
		//for(size_t x = 0; x < tex.width; x++)
		//window.setPixelColour(x, y, getColour(tex, TexturePoint(x, y)));

		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
