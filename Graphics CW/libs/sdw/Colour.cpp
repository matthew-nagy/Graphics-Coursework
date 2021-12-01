#include "Colour.h"
#include <utility>

Colour::Colour(const std::string& red, const std::string& green, const std::string& blue):
	r(std::atof(red.c_str()) * 255.0),
	g(std::atof(green.c_str()) * 255.0),
	b(std::atof(blue.c_str()) * 255.0)
{}
Colour::Colour(uint8_t r, uint8_t g, uint8_t b, uint8_t a):
	r(r),
	g(g),
	b(b),
	a(a)
{}