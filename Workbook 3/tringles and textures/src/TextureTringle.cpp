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

std::vector<CanvasTriangle> cts;
TextureMap* ctst;

template<size_t len>
std::vector< std::array<int, len> > interpolate(const std::array<int, len>& from, const std::array<int, len>& to, int size){
    std::vector<std::array<int, len>> ret;
    ret.reserve(size);

    std::array<float, len> val;
    for(size_t i = 0; i < len; i++)val[i] = from[i];
    std::array<float, len> step;
    for(size_t i = 0; i < len; i++)step[i] = float((to[i] - from[i])) / float((size - 1));

    for(size_t i = 0; i < size; i++){
        std::array<int, len> nVal;
        for(size_t j = 0; j < len; j++){
            nVal[j] = std::round(val[j]);
            val[j] += step[j];
        }
        ret.emplace_back(std::move(nVal));
    }

    return ret;
}

std::array<int, 4> mA(float x, float y, float u, float v){
    std::array<int, 4> ret;
    ret[0] = std::round(x);
    ret[1] = std::round(y);
    ret[2] = std::round(u);
    ret[3] = std::round(v);
    return ret;
}
std::array<int, 4> mA(const CanvasPoint& cp){
    return mA(cp.x, cp.y, cp.texturePoint.x, cp.texturePoint.y);
}

std::array<CanvasPoint, 3> orderPoints(const std::array<CanvasPoint, 3>& p){
	std::array<CanvasPoint, 3> ret = p;
	
	std::sort(ret.begin(), ret.end(), [](CanvasPoint& left, CanvasPoint& right)->bool{return left.y < right.y;});

	return ret;
}

uint32_t getColour(const TextureMap& tex, int x, int y){
    return tex.pixels[x + (y * tex.width)];
}

void drawTextureLine(const std::array<int, 4>& from, const std::array<int, 4>& to, const TextureMap& texture, DrawingWindow& window){
    unsigned length = std::max( std::abs(from[0] - to[0]), std::abs(from[1] - to[1]) );
    //+ 2 for luck!
    auto points = interpolate(from, to, length + 2);

    for(auto& p : points)
        window.setPixelColour(p[0], p[1], getColour(texture, p[2], p[3]));
}

void drawTexTriangle(const CanvasTriangle& tri, const TextureMap& tex, DrawingWindow& window){
    auto points = orderPoints(tri.vertices);
    const float magic = 1.1;
    auto ends = interpolate(mA(points[0]), mA(points[2]), (points[2].y - points[0].y) * magic);
    std::vector<std::array<int, 4>> starts;
    {
        auto s1 = interpolate(mA(points[0]), mA(points[1]), (points[1].y - points[0].y)* magic);
        auto s2 = interpolate(mA(points[1]), mA(points[2]), (points[2].y - points[1].y)* magic);
        starts.insert(starts.end(), s1.begin(), s1.end());
        starts.insert(starts.end(), s2.begin(), s2.end());
    }

    for(size_t i = 0; i < starts.size(); i++)
        drawTextureLine(starts[i], ends[i], tex, window);
}

void draw(DrawingWindow &window) {
	window.clearPixels();
    for(auto& t : cts)
        drawTexTriangle(t, *ctst, window);
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

	TextureMap tex("texture.ppm");
    ctst = &tex;
	CanvasTriangle testTexTriangle(
		CanvasPoint(160, 10), CanvasPoint(300, 230), CanvasPoint(10, 150)
	);
	testTexTriangle[0].texturePoint = TexturePoint(195, 5);
	testTexTriangle[1].texturePoint = TexturePoint(395, 380);
	testTexTriangle[2].texturePoint = TexturePoint(65, 330);
    cts.emplace_back(std::move(testTexTriangle));

    std::array<int, 4> from = {10, 10, 50, 50};
    std::array<int, 4> to = {400, 400, 200, 100};
    std::array<int, 4> from2 = {60, 10, 50, 50};
    std::array<int, 4> to2 = {450, 400, 200, 200};

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		draw(window);

		//for(size_t y = 0; y < tex.height; y++)
		//for(size_t x = 0; x < tex.width; x++)
		//window.setPixelColour(x, y, getColour(tex, TexturePoint(x, y)));

		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
