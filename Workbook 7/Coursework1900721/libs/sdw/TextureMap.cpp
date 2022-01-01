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
		printf("Mip mapping in %s\n", filename.c_str());
		mip = md;
		loadTexture(filename + "-high-mip.ppm", width, height, mip.high_mip_pixels);
	}
}

std::ostream &operator<<(std::ostream &os, const TextureMap &map) {
	os << "(" << map.width << " x " << map.height << ")";
	return os;
}

std::vector<Displacement_Effect*> __DFX;



	Floor_Displacement_Effect::Floor_Displacement_Effect(TexturePoint off1, TexturePoint off2, TexturePoint v1, TexturePoint v2, float strength, float topVal, float lowVal):
		offset1(off1),
		offset2(off2),
		vel2(v2),
		vel1(v1),
		strength(strength),
		topVal(topVal),
		lowVal(lowVal)
	{
		displacer = new TextureMap("displace.ppm");
		__DFX.emplace_back(this);
	}

	void Floor_Displacement_Effect::frameStep(){
		offset1 = offset1 + vel1;
		offset2 = offset2 + vel2;
	}

	uint32_t Floor_Displacement_Effect::getColourFrom(const TextureMap* texMap, float origX, float origY){
		uint8_t d1 = __getColourData(origX + offset1.x, origY + offset1.y, displacer) & 0x000000FF;
		uint8_t d2 = __getColourData(origX + offset2.x, origY + offset2.y, displacer) & 0x000000FF;
		
		float fd1 = d1;
		fd1 /= 255.00;
		float fd2 = d2;
		fd2 /= 255.0;

		float combined = (fd1 + fd2) / 2.0;
		uint32_t e = uint32_t(combined * 255.0);

		if(combined > topVal){
			return 0xFF000000 + (e << 16) + (e << 8) + e;
		}
		
		float disp = (strength * 2 * fd1) - strength;
		return __getColourData(origX + disp, origY + disp, texMap);

	}



	uint32_t Floor_Displacement_Effect::__getColourData(float x, float y, TextureMap const*const map){
		x = std::round(std::abs(x));
		while(x >= map->width)
			x -= map->width;
		y = std::round(std::abs(y));
		while(y >= map->height)
			y -= map->height;

		size_t index = std::round(x) + (std::round(y) * map->width);

		return map->pixels[index];
	}


	Wall_Displacement_Effect::Wall_Displacement_Effect(TexturePoint v1, TexturePoint v2, float strength, float r, float g, float b):
		vel1(v1),
		vel2(v2),
		strength(strength),
		offset1(0,0),
		offset2(0,0),
		r(r),
		g(g),
		b(b)
	{
		displacer = new TextureMap("displace.ppm");
		
		ripple = new TextureMap("waterfall.ppm");

		rippleCol = 0xFF000000 + (uint32_t(r * 255.0) << 16) + (uint32_t(g * 255.0) << 8) + uint32_t(b * 255.0);

		__DFX.emplace_back(this);
	}

	void Wall_Displacement_Effect::frameStep(){
		offset1 = offset1 + vel1;
		offset2 = offset2 + vel2;
	}

	uint32_t Wall_Displacement_Effect::getColourFrom(const TextureMap* texMap, float origX, float origY){

		uint8_t d11 = __getColourData(origX + offset1.x, origY + offset1.y, displacer) & 0x000000FF;
		float fd1 = d11;
		fd1 /= 255.00;
		
		float disp = (strength * 2 * fd1) - strength;
		Colour origColour = Colour(__getColourData(origX + disp, origY + disp, texMap));
		Colour rippleC = Colour(rippleCol);

		uint8_t ripProportion = __getColourData(origX + offset2.x, origY + offset2.y, ripple) & 0x000000FF;
		float rp = (float(ripProportion) / 255.0);

		Colour finalCol = (origColour * (1.0-rp)) + (rippleC * rp);

		return 0xFF000000 + (uint32_t(finalCol.red) << 16) + (uint32_t(finalCol.green) << 8) + uint32_t(finalCol.blue);
	}

	uint32_t Wall_Displacement_Effect::__getColourData(float x, float y, TextureMap const*const map){
		x = std::round(std::abs(x));
		while(x >= map->width)
			x -= map->width;
		y = std::round(std::abs(y));
		while(y >= map->height)
			y -= map->height;

		size_t index = std::round(x) + (std::round(y) * map->width);

		return map->pixels[index];
	}