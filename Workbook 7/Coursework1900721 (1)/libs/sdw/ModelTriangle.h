#pragma once

#include <glm/glm.hpp>
#include <string>
#include <array>
#include "Colour.h"
#include "TexturePoint.h"
#include "TextureMap.h"

enum NormalType {Regular, Gouraud, Phong};

struct ModelTriangle {
	std::array<glm::vec3, 3> vertices{};
	std::array<glm::vec3, 3> vertexNormals{};
	std::array<TexturePoint, 3> texturePoints{};
	NormalType normalFinder = Regular; 
	Colour colour{};
	glm::vec3 normal{};
	TextureMap* texture = nullptr;
	TextureMap* bumpmap = nullptr;

	bool hoidLighting = false;

	float specN;
	float reflectivity;

	bool refractive = false;
	float roi;

	float area;

	ModelTriangle();
	ModelTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, Colour trigColour);
	friend std::ostream &operator<<(std::ostream &os, const ModelTriangle &triangle);
};
