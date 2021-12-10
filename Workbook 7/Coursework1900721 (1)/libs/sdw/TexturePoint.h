#pragma once

#include <iostream>

struct TexturePoint {
	float x{};
	float y{};

	TexturePoint();
	TexturePoint(float xPos, float yPos);
	friend std::ostream &operator<<(std::ostream &os, const TexturePoint &point);
};

TexturePoint operator+(const TexturePoint& left, const TexturePoint& right);
TexturePoint operator-(const TexturePoint& left, const TexturePoint& right);
TexturePoint operator*(const TexturePoint& left, float scale);
TexturePoint operator*(float scale, const TexturePoint& right);