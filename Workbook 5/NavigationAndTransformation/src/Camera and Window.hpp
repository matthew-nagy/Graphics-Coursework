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

#define WIDTH 600
#define HEIGHT 550
float c_totXRot = 0.0f;
float c_totYRot = 0.0f;
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


struct Camera{
	glm::vec3 pos;
	glm::mat3 orientation;

    glm::mat3 totalXRot;
    glm::mat3 totalYRot;
    glm::mat3 basic;

	void move(float x, float y, float z){
		pos.x += x;
		pos.y += y;
		pos.z += z;
	}

	void rotate(float degrees, char dim, glm::vec3 center = glm::vec3(0, 0, 0)){
		pos = pos - center;

		rotateCamera(dim, degrees, pos);

		pos = pos + center;
	}

	void adjustOrientation(glm::mat3 by){
		orientation = by * orientation;
	}

	void adjustOrientation(float degrees, char dim){
		float rad = degrees * 3.141592 / 180;
		glm::mat3 by;
		float c = cos(rad);
		float s = sin(rad);
		
		switch(dim){
		case 'x':
			by = glm::mat3(
				1, 0, 0,
				0, c, s * -1,
				0, s, c
			);
            totalXRot = totalXRot * by;
			break;
		case 'y':
			by = glm::mat3(
				c, 0, s * -1,
				0, 1, 0,
				s, 0, c
			);
            totalYRot = totalYRot * by;
			break;
		default:
			printf("LMAO in defauilt\n");
			char nameb[2];
			nameb[0] = dim;
			nameb[1] = '\0';
			printf("Dim given was '%s'\n", nameb);
			return;
		}

        orientation = basic * totalYRot * (totalXRot * totalYRot);
	}

	void lookAt(const glm::vec3& position){
		glm::vec3 forwards = position - pos;
		glm::vec3 right = glm::cross(forwards, glm::vec3(0, 1, 0));
		glm::vec3 up = glm::cross(forwards, right);

		forwards = glm::normalize(forwards);
		right = glm::normalize(right);
		up = glm::normalize(up);

		right.x *= -1; right.y *= -1; right.z *= -1;

		orientation = glm::mat3(right, up, forwards);
	}

	glm::vec3 getAdjuastedVertex(glm::vec3 vertex){
		glm::vec3 cameraToVertex = pos - vertex;
		return cameraToVertex * orientation;
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
