#pragma once

#include <iostream>
#include <fstream>
#include <stdexcept>
#include "Utils.h"
#include "TexturePoint.h"
#include "Colour.h"

struct MipData{
	bool mipMapped = false;
	float lowMipThreashold, highMipThreashold;
	std::vector<uint32_t> high_mip_pixels;
};


extern MipData __mData;

class Displacement_Effect;

class TextureMap {
public:
	size_t width;
	size_t height;
	std::vector<uint32_t> pixels;

	MipData mip;

	Displacement_Effect* dpe = nullptr;

	TextureMap();
	TextureMap(const std::string &filename, MipData md = __mData);
	friend std::ostream &operator<<(std::ostream &os, const TextureMap &point);
};

//Leak all the memory, its 2:30Am and the deadline is at 1PM, who cares anymore

class Displacement_Effect{
public:
	virtual uint32_t getColourFrom(const TextureMap* texMap, float origX, float origY) = 0;
	virtual void frameStep() = 0;
};

#include<cmath>

extern std::vector<Displacement_Effect*> __DFX;

class Floor_Displacement_Effect : public Displacement_Effect{
public:

	Floor_Displacement_Effect(TexturePoint off1, TexturePoint off2, TexturePoint v1, TexturePoint v2, float strength, float topVal, float lowVal);

	void frameStep()override;

	uint32_t getColourFrom(const TextureMap* texMap, float origX, float origY)override;

private:
	TextureMap* displacer;

	float strength = 10.0;
	float topVal;
	float lowVal;

	TexturePoint vel1, vel2, offset1, offset2;

	uint32_t __getColourData(float x, float y, TextureMap const*const map);
};


class Wall_Displacement_Effect : public Displacement_Effect{
public:

	Wall_Displacement_Effect(TexturePoint v1, TexturePoint v2, float strength, float r, float g, float b);

	void frameStep()override;

	uint32_t getColourFrom(const TextureMap* texMap, float origX, float origY)override;

private:
	TextureMap* displacer;
	TextureMap* ripple;

	float r, g, b;
	uint32_t rippleCol;

	float strength = 10.0;

	TexturePoint vel1, vel2, offset1, offset2;

	uint32_t __getColourData(float x, float y, TextureMap const*const map);
};
