#include "Camera and Window.hpp"

/*
###################################################
###################################################

	Generalised functions for drawing triangles and
	lines to the screen

###################################################
###################################################
*/


uint32_t getColourData(const Colour& col, uint8_t a = 255){
	uint32_t ret = (a << 24) + (uint8_t(col.red) << 16) + (uint8_t(col.green) << 8) + uint8_t(col.blue);
	return ret;
}
uint32_t getColourData(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255){
	uint32_t ret = (a << 24) + (r << 16) + (g << 8) + b;
	return ret;
}
uint32_t getColourData(float x, float y, TextureMap const*const map){
	size_t index = std::round(x) + (std::round(y) * map->width);
	return map->pixels[index];
}

template<class T, size_t length>
using array_vector = std::vector<std::array<T, length>>;

//Returns a vector of arrays, with the given type and dimensionality. This lets you interpolate positions of any dimensions
template<class Type, size_t length>
array_vector<Type, length> interpolate(const std::array<Type, length>& from, const std::array<Type, length>& to, size_t count){
	array_vector<Type, length> toret;
	toret.reserve(count);

	std::array<Type, length> current = from;
	std::array<float, length> step;
	for(size_t i = 0; i < length; i++)
		step[i] = (to[i] - from[i]) / (count - 1);

	for(size_t i = 0; i < count; i++){

		toret.emplace_back(current);
		for(size_t j = 0; j < length; j++)
			current[j] += step[j];
	}
	
	return toret;
}

//Given some xs and ys, returns the length of the ionterpolated line that will be between them
template<class T>
unsigned getLen(T x1, T y1, T x2, T y2){
	return std::max( std::abs(x1 - x2), std::abs(y1 - y2) );
}