#include "Util.hpp"

//Draws a line between two positions of T data type and dataDimentions dimentionality
//@param getColour- a lambda function that given a dimentional position and funcData will return the colour at this point. This lets you draw textured lines
//@param funcData- the external value given to getColour. Can be a Colour struct, a TextureMap, or something else to help the function run
template<class T, size_t dataDimentions>
void drawLine(const std::array<T, dataDimentions>& from, const std::array<T, dataDimentions>& to, Window& window, uint32_t(*getColour)(const std::array<T, dataDimentions>& pos, void* data), void* funcData){
	auto drawers = interpolate(from, to, getLen(from[0], from[1], to[0], to[1]) + 5);
	for(auto& p : drawers){
		window.setPixelColour(std::round(p[0]), std::round(p[1]), getColour(p, funcData), p[2]);
		window.setPixelColour(std::round(p[0] + 1), std::round(p[1]), getColour(p, funcData), p[2]);
		window.setPixelColour(std::round(p[0]), std::round(p[1])+1, getColour(p, funcData), p[2]);
		window.setPixelColour(std::round(p[0] + 1), std::round(p[1])+1, getColour(p, funcData), p[2]);
	}
}

//Draws a wireframe between 3 n dimentional points. Assumes [0] is x, [1] is y and [2] is depth.
template<class T, size_t dataDimentions>
void drawWireframe(const std::array<std::array<T, dataDimentions>, 3>& tri, Window& window, Colour frameColour = Colour(255, 255, 255)){
	uint32_t(*colFunc)(const std::array<float, 3>&, void*) = [](const std::array<float, 3>&, void* colP)->uint32_t{return getColourData(*(Colour*)colP);};
	drawLine(std::array<float, 3>{tri[0][0], tri[0][1], tri[0][2]}, std::array<float, 3>{tri[1][0], tri[1][1], tri[1][2]}, window, colFunc, (void*)&frameColour);
	drawLine(std::array<float, 3>{tri[2][0], tri[2][1], tri[2][2]}, std::array<float, 3>{tri[1][0], tri[1][1], tri[1][2]}, window, colFunc, (void*)&frameColour);
	drawLine(std::array<float, 3>{tri[0][0], tri[0][1], tri[0][2]}, std::array<float, 3>{tri[2][0], tri[2][1], tri[2][2]}, window, colFunc, (void*)&frameColour);
}

//A specialized version of drawWireframe<float, 2> that draws off a CanvasTriangle rather than a gathering of n dimensional points
void drawWireframe(const CanvasTriangle& tri, Window& window, Colour frameColour = Colour(255, 255, 255)){
	drawWireframe(std::array<std::array<float, 3>, 3>
		{ std::array<float, 3>{tri[0].x, tri[0].y, tri[0].depth}, std::array<float, 3>{tri[1].x, tri[1].y, tri[1].depth}, 
	std::array<float, 3>{tri[2].x, tri[2].y, tri[2].depth} }, window, frameColour);
}

template<class T, size_t dataDimentions>
void drawRaster(std::array<std::array<T, dataDimentions>, 3> tri, Window& window, uint32_t(*getColour)(const std::array<T, dataDimentions>& pos, void* data), void* funcData){
	std::sort(tri.begin(), tri.end(), [](std::array<T, dataDimentions>& l, std::array<T, dataDimentions>& r)->bool{return l[1] < r[1];});
	auto ends = interpolate(tri[0], tri[2], (tri[2][1] - tri[0][1]) + 4);
	auto starts = interpolate(tri[0], tri[1], (tri[1][1] - tri[0][1]) + 2);
	auto addOn = interpolate(tri[1], tri[2], (tri[2][1] - tri[1][1]) + 2);
	starts.insert(starts.end(), addOn.begin(), addOn.end());

	for (size_t i = 0; i < ends.size() && i < starts.size(); i++)
		drawLine(starts[i], ends[i], window, getColour, funcData);
}

void drawRaster(const CanvasTriangle& tri, Window& window, Colour col){
	uint32_t(*colFunc)(const std::array<float, 3>&, void*) = [](const std::array<float, 3>&, void* colP)->uint32_t{return getColourData(*(Colour*)colP);};
	drawRaster(std::array<std::array<float, 3>, 3>
		{ std::array<float, 3>{tri[0].x, tri[0].y, tri[0].depth}, std::array<float, 3>{tri[1].x, tri[1].y, tri[1].depth}, 
	std::array<float, 3>{tri[2].x, tri[2].y, tri[2].depth} }, window,colFunc, (void*)&col  );
}



 void drawRaster(const CanvasTriangle& tri, Window& window, TextureMap* map){
 	uint32_t(*colFunc)(const std::array<float, 5>&, void*) = [](const std::array<float, 5>& pos, void* texturPoint)->uint32_t{
 		TextureMap* tex = (TextureMap*)texturPoint;
 		return getColourData(pos[3], pos[4], tex);
 	};
	
 	drawRaster(std::array<std::array<float, 5>, 3>
 		{ std::array<float, 5>{tri[0].x, tri[0].y, tri[0].depth, tri[0].texturePoint.x, tri[0].texturePoint.y}, 
 		std::array<float, 5>{tri[1].x, tri[1].y, tri[1].depth, tri[1].texturePoint.x, tri[1].texturePoint.y}, 
 		std::array<float, 5>{tri[2].x, tri[2].y, tri[2].depth, tri[2].texturePoint.x, tri[2].texturePoint.y} }, 
 	window, colFunc, (void*)map  );
}
