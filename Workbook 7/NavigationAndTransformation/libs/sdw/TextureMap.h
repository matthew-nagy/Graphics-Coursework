#pragma once

#include <iostream>
#include <fstream>
#include <stdexcept>
#include "Utils.h"

struct MipData{
	bool mipMapped = false;
	float lowMipThreashold, highMipThreashold;
	std::vector<uint32_t> high_mip_pixels;
};

extern MipData __mData;

class TextureMap {
public:
	size_t width;
	size_t height;
	std::vector<uint32_t> pixels;

	MipData mip;

	TextureMap();
	TextureMap(const std::string &filename, MipData md = __mData);
	friend std::ostream &operator<<(std::ostream &os, const TextureMap &point);
};
