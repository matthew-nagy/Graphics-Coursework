#pragma once

#include <iostream>
#include <array>

struct Colour {
	std::string name;
	int red{};
	int green{};
	int blue{};
	Colour();
	Colour(int r, int g, int b);
	Colour(std::array<int, 3> vals);
	Colour(std::string n, int r, int g, int b);
	Colour(uint32_t c);
};

std::ostream &operator<<(std::ostream &os, const Colour &colour);

Colour operator+(const Colour& left, const Colour& right);
Colour operator-(const Colour& left, const Colour& right);
Colour operator*(const Colour& left, float right);
Colour operator*(float left, const Colour& right);
Colour operator/(float left, const Colour& right);
Colour operator/(const Colour& left, float right);