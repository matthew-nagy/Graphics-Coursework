#include "TextureMap.h"


MipData __mData;

void loadTexture(const std::string& filename, size_t& width, size_t& height, std::vector<uint32_t>& pixels){
	std::ifstream inputStream(filename, std::ifstream::binary);

	if(inputStream.fail() || !inputStream.is_open())
		printf("Failed to open %s\n", filename.c_str());

	std::string nextLine;
	// Get the "P6" magic number
	std::getline(inputStream, nextLine);
	// Read the width and height line
	std::getline(inputStream, nextLine);
	// Skip over any comment lines !
	while (nextLine.at(0) == '#') std::getline(inputStream, nextLine);
	auto widthAndHeight = split(nextLine, ' ');
	if (widthAndHeight.size() != 2)
		throw std::invalid_argument("Failed to parse width and height line, line was `" + nextLine + "`");

	width = std::stoi(widthAndHeight[0]);
	height = std::stoi(widthAndHeight[1]);
	// Read the max value (which we assume is 255)
	std::getline(inputStream, nextLine);

	pixels.resize(width * height);
	for (size_t i = 0; i < width * height; i++) {
		int red = inputStream.get();
		int green = inputStream.get();
		int blue = inputStream.get();
		pixels[i] = ((255 << 24) + (red << 16) + (green << 8) + (blue));
	}
	inputStream.close();
	printf("Managed to open texture '%s'\n", filename.c_str());
}

TextureMap::TextureMap() = default;
TextureMap::TextureMap(const std::string &filename, MipData md) {
	loadTexture(filename, width, height, pixels);
	if(md.mipMapped){
		mip = md;
		loadTexture(filename + "-high-mip.ppm", width, height, mip.high_mip_pixels);
	}
}

std::ostream &operator<<(std::ostream &os, const TextureMap &map) {
	os << "(" << map.width << " x " << map.height << ")";
	return os;
}
