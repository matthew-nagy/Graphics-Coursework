#include "TexturePoint.h"

TexturePoint::TexturePoint() = default;
TexturePoint::TexturePoint(float xPos, float yPos) : x(xPos), y(yPos) {}

std::ostream &operator<<(std::ostream &os, const TexturePoint &point) {
	os << "x: " << point.x << " y: " << point.y;
	return os;
}

TexturePoint operator+(const TexturePoint& left, const TexturePoint& right){
	TexturePoint ret;
	ret.x = left.x + right.x;
	ret.y = left.y + right.y;
	return ret;
}
TexturePoint operator-(const TexturePoint& left, const TexturePoint& right){
	TexturePoint ret;
	ret.x = left.x - right.x;
	ret.y = left.y - right.y;
	return ret;
}
TexturePoint operator*(const TexturePoint& left, float scale){
	TexturePoint tp;
	tp.x = left.x * scale;
	tp.y = left.y * scale;
	return tp;
}
TexturePoint operator*(float scale, const TexturePoint& right){
	return operator*(right, scale);
}