#include "Colour.h"
#include <utility>

Colour::Colour() = default;
Colour::Colour(int r, int g, int b) : red(r), green(g), blue(b) {}
Colour::Colour(std::array<int, 3> vals){
	for(size_t i = 0; i < 3; i++){
		if(vals[i] < 0)
			vals[i] = 0;
		else if(vals[i] > 255)
			vals[i] = 255;
	}
	red = vals[0];
	green = vals[1];
	blue = vals[2];
}
Colour::Colour(std::string n, int r, int g, int b) :
		name(std::move(n)),
		red(r), green(g), blue(b) {}
		
Colour::Colour(uint32_t c){
	red = (c & 0x00FF0000) >> 16;
	green = (c & 0x0000FF00) >> 8;
	blue = (c & 0x000000FF);
}

std::ostream &operator<<(std::ostream &os, const Colour &colour) {
	os << colour.name << " ["
	   << colour.red << ", "
	   << colour.green << ", "
	   << colour.blue << "]";
	return os;
}



Colour operator+(const Colour& left, const Colour& right){
	std::array<int, 3> l = {left.red, left.green, left.blue};
	std::array<int, 3> r = {right.red, right.green, right.blue};

	std::array<int, 3> res;
	for(size_t i = 0; i < 3; i++)
		res[i] = l[i] + r[i];
	
	return Colour(res);
}
Colour operator-(const Colour& left, const Colour& right){
	std::array<int, 3> l = {left.red, left.green, left.blue};
	std::array<int, 3> r = {right.red, right.green, right.blue};

	std::array<int, 3> res;
	for(size_t i = 0; i < 3; i++)
		res[i] = l[i] - r[i];
	
	return Colour(res);
}
Colour operator*(const Colour& left, float right){
	std::array<int, 3> l = {left.red, left.green, left.blue};

	std::array<int, 3> res;
	for(size_t i = 0; i < 3; i++)
		res[i] = l[i] * right;
	
	return Colour(res);
}
Colour operator*(float left, const Colour& right){
	return right * left;
}
Colour operator/(float left, const Colour& right){
	return right * (1/left);
}
Colour operator/(const Colour& left, float right){
	return (1/right) * left;
}
