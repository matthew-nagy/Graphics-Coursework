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
