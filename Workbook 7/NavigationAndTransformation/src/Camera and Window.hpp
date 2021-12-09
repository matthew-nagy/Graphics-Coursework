#pragma once
#include <libs/sdw/CanvasTriangle.h>
#include <libs/sdw/DrawingWindow.h>
#include "libs/sdw/Colour.h"
#include "libs/sdw/CanvasPoint.h"
#include "libs/sdw/TextureMap.h"
#include "libs/sdw/ModelTriangle.h"
#include <libs/sdw/Utils.h>
#include "libs/glm-0.9.7.2/glm/glm.hpp"
#include <fstream>
#include <vector>
#include <algorithm>
#include <array>
#include <unordered_map>
#include <cstring>

#define WIDTH 640
#define HEIGHT 480
// #define WIDTH 100
// #define HEIGHT 80

float c_totXRot = 0.0f;
float c_totYRot = 0.0f;

std::vector<std::string> splitStringOn(const std::string& in, char splitOn){
    std::vector<std::string> splits;
    splits.reserve(8);

    size_t start = 0;
    for(size_t i = 0; i < in.size(); i++){
        if(in[i] == splitOn){
            splits.emplace_back(in.substr(start, i - start));
            start = i + 1;
        }
    }

    if(start != in.size())
        splits.emplace_back(in.substr(start));

    return splits;
}
/*
###################################################
###################################################

	Generalised class for drawing

###################################################
###################################################
*/

void rotateCamera(char dim, float degrees, glm::vec3& camera){
	float radians = degrees * 3.141592 / 180;
	float s = sin(radians);
	float c = cos(radians);

	glm::mat3 m;

	switch(dim){
		case 'x':
			m = glm::mat3(1, 0, 0,    0, c, s,    0, s*-1, c);
			c_totXRot += degrees;
			break;
		case 'y':
			m = glm::mat3(c, 0, s * -1,    0, 1, 0,    s, 0, c);
			c_totYRot -= degrees;
			break;
		default:
			printf("Invalid rotation dimention\n");
			return;
	}

	camera = m * camera;
}

glm::mat3 getXwardMatrix(float degrees){
	float rad = degrees * 3.141592 / 180;
	float c = cos(rad);
	float s = sin(rad);

	return glm::mat3(
		1, 0, 0,
		0, c, s * -1,
		0, s, c
	);
}
glm::mat3 getYwardMatrix(float degrees){
	float rad = degrees * 3.141592 / 180;
	float c = cos(rad);
	float s = sin(rad);

	return glm::mat3(
		c, 0, s * -1,
		0, 1, 0,
		s, 0, c
	);
}

struct Camera{
	//Position for raster
	glm::vec3 pos;
	//Orientation for raster
	glm::mat3 orientation;

	//Position for raytrace
	glm::vec3 rayPos;
	//Orientation for raytrace
	glm::mat3 rayOrientation;

	float xLook = 0;
	float yLook = 0;

	//Used for animation
	void setXY(float XL, float YL){
		xLook = XL;
		yLook = YL;
		rotateViewY(0);
	}

	void rotateViewX(float degrees){
		xLook += degrees;

		glm::vec3 direction;
		direction.x = cos(glm::radians(xLook)) * cos(glm::radians(yLook));
		direction.y = sin(glm::radians(yLook));
		direction.z = sin(glm::radians(xLook)) * cos(glm::radians(yLook));
		
		glm::vec3 front = glm::normalize(direction);
		orientation = getMat3FromForwards(front);
		rayOrientation = glm::inverse(getMat3FromForwards(front));

		//printf("%f %f %f\n", rayOrientation[0].x, rayOrientation[0].y, rayOrientation[0].z);


	}
	void rotateViewY(float degrees){
		yLook += degrees;

		glm::vec3 direction;
		direction.x = cos(glm::radians(xLook)) * cos(glm::radians(yLook));
		direction.y = sin(glm::radians(yLook));
		direction.z = sin(glm::radians(xLook)) * cos(glm::radians(yLook));
		glm::vec3 front = glm::normalize(direction);
		orientation = getMat3FromForwards(front);
		rayOrientation = glm::inverse(getMat3FromForwards(front));
		//printf("%f %f %f\n", rayOrientation[0].x, rayOrientation[0].y, rayOrientation[0].z);
		
	}

	float focalLength;

	void move(float x, float y, float z){
		move(glm::vec3(x, y, z));
	}
	void move(glm::vec3 m){
		pos += (m * orientation);
		m.x *= -1;
		rayPos += (m * rayOrientation);
	}

	void moveRealWorldBased(float x, float y, float z){
		moveRealWorldBased(glm::vec3(x, y, z));
	}
	void moveRealWorldBased(glm::vec3 m){
		pos += m;
		m.x *= -1;
		rayPos += m;
	}

	void rotatePositionAround(char dim, float degrees, glm::vec3 center = glm::vec3(0, 0, 0)){
		pos = pos - center;
		rayPos = rayPos - center;


		switch(dim){
			case 'x':
				pos = pos * getXwardMatrix(degrees);
				rayPos = rayPos * getXwardMatrix(degrees * -1);
				break;
			case 'y':
				pos = pos * getYwardMatrix(degrees);
				rayPos = rayPos * getYwardMatrix(degrees * -1);
				break;
		}


		pos = pos + center;
		rayPos = rayPos + center;
	}

	void lookAt(const glm::vec3& position){
		orientation = getLookAt(position, pos);
		rayOrientation = glm::inverse(getLookAt(position, rayPos));
	}

	//For canvas points
	glm::vec3 getAdjuastedVertex(glm::vec3 vertex){
		glm::vec3 cameraToVertex = pos - vertex;
		return cameraToVertex * orientation;
	}

private:

	glm::mat3 getMat3FromForwards(glm::vec3 forwards){
		glm::vec3 right = glm::cross(forwards, glm::vec3(0, 1, 0));
		glm::vec3 up = glm::cross(forwards, right);

		forwards = glm::normalize(forwards);
		right = glm::normalize(right);
		up = glm::normalize(up);

		right.x *= -1; right.y *= -1; right.z *= -1;
		up.x *= -1; up.y *= -1; up.z *= -1;
		return glm::mat3(right, up, forwards);

	}

	glm::mat3 getLookAt(const glm::vec3& lookPosition, const glm::vec3& currentPosition){
		
		glm::vec3 forwards = currentPosition - lookPosition;
		return getMat3FromForwards(forwards);
	}
};

class Window{
public:
	virtual void setPixelColour(unsigned x, unsigned y, uint32_t colour, float depth = 0.0f) = 0;

	virtual void clearPixels() = 0;

	virtual ~Window(){}
};

class SDL_Window : public Window{
public:
	void setPixelColour(unsigned x, unsigned y, uint32_t colour, float depth = 0.0f){
		t_window.setPixelColour(x, y, colour);
	}


	void clearPixels(){
		t_window.clearPixels();
	}

	SDL_Window(DrawingWindow& targetWindow):
		t_window(targetWindow)
	{}
private:
	DrawingWindow& t_window;
};

class Depth_Window : public Window{
public:
	void setPixelColour(unsigned x, unsigned y, uint32_t colour, float drawDepth){
		if(x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)return;
		if(drawDepth < 0)return;
		if(drawDepth > depth[y][x]){
			t_window.setPixelColour(x, y, colour);
			depth[y][x] = drawDepth;
		}
	}

	void clearPixels(){
		t_window.clearPixels();
		memcpy(depth, clearDepth, sizeof(float) * HEIGHT * WIDTH);
	}

	Depth_Window(DrawingWindow& targetWindow):
		t_window(targetWindow)
	{
		for(size_t i = 0; i < HEIGHT; i++)
			for(size_t j = 0; j < WIDTH; j++)
				clearDepth[i][j] = depth[i][j] = -9999999.0f;
	}
private:
	DrawingWindow& t_window;
	float depth[HEIGHT][WIDTH];
	float clearDepth[HEIGHT][WIDTH];
};


std::unordered_map<std::string, float> __config;
void loadConfig(){
	std::ifstream file("config.txt", std::ios::in);
	std::string line;
	while(std::getline(file, line)){
		auto s = splitStringOn(line, ':');
		__config[s[0]] = std::atof(s[1].c_str());
	}
}