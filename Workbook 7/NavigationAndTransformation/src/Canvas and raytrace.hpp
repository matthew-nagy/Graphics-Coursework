#include "Parser.hpp"
#include "Workers.hpp"
#include "../libs/sdw/RayTriangleIntersection.h"

#define PI 3.141592

//###########################################
//###########################################
//PARSER OVER
//###########################################
//###########################################


CanvasPoint getCanvasIntersectionPoint(Camera& camera, glm::vec3 vertexPos, float focalLength = 2.0f){
    //Scale the model to draw it nicer

    //vertexPos.z *= -1;
	//vertexPos.y *= -1;
	vertexPos.x *= -1;
    vertexPos.x -= camera.pos.x; vertexPos.y -= camera.pos.y; vertexPos.z -= camera.pos.z;


	vertexPos = camera.getAdjuastedVertex(vertexPos);



    CanvasPoint cp;

	glm::vec3 towards = camera.pos - vertexPos;

    cp.depth = 1 / towards.z * -1;

    cp.x = focalLength * (vertexPos.x / vertexPos.z); 
    cp.y = focalLength * (vertexPos.y / vertexPos.z); 

    cp.x *= 50;
    cp.y *= 50;

    cp.x += (WIDTH / 2);
    cp.y += (HEIGHT / 2);



    return cp;

}


/*CanvasPoint getCanvasIntersectionPoint(const glm::vec3& cameraPos, glm::vec3 vertexPos, float focalLength = 2.0f){
	//Scale the model to draw it nicer
	vertexPos.x *= -55;
	vertexPos.y *= 55;
	vertexPos.z *= -1;
	vertexPos.x -= cameraPos.x; vertexPos.y -= cameraPos.y; vertexPos.z -= cameraPos.z;

	CanvasPoint cp;
	cp.depth = 1 / vertexPos.z;
	cp.x = focalLength * (vertexPos.x / vertexPos.z) + (WIDTH / 2);
	cp.y = focalLength * (vertexPos.y / vertexPos.z) + (HEIGHT / 2);

	return cp;
}*/

CanvasTriangle getCanvasTri(Camera& camera, const ModelTriangle& mTri){
	CanvasTriangle cTri;
	for(size_t i = 0; i < 3; i++){
		cTri[i] = getCanvasIntersectionPoint(camera, mTri.vertices[i], camera.focalLength);
        cTri[i].texturePoint = mTri.texturePoints[i];
    }
	return cTri;
}


glm::vec3 ray_getPossibleSolution(const ModelTriangle& triangle, const glm::vec3& cameraPosition, const glm::vec3& rayDirection){
	glm::vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
	glm::vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
	glm::vec3 SPVector = cameraPosition - triangle.vertices[0];
	glm::mat3 DEMatrix(-rayDirection, e0, e1);
	glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
	return possibleSolution;
}

glm::vec3 ray_getIntersectionPoint(ModelTriangle& tri, glm::vec3& solution){
	float u = solution.y;
	float v = solution.z;
	auto intersect = tri.vertices[0] + 
					u*(tri.vertices[1] - tri.vertices[0]) + 
					v*(tri.vertices[2] - tri.vertices[0]);
	return intersect;
}

bool inBounds(float value, float lower, float heigher){
	return (value >= lower) && (value <= heigher);
}

bool ray_solutionValid(const glm::vec3& solution){
	bool isInBounds = 	inBounds(solution.y, 0.0, 1.0) && 
						inBounds(solution.z, 0.0, 1.0) && 
						inBounds(solution.y + solution.z, 0.0, 1.0);
	bool isValid =  isInBounds && 
					solution.x >= 0.0; 

	return isValid;
}


struct RowTrace{
	std::array<RayTriangleIntersection, WIDTH>* intoRow;
	float rayY;
	Model* model;
	Camera* camera;
	Semaphore* finishSemaphore;
};

struct Ray_Lighting_Info{
	float proximityPiFactor;
	float proximityNumerator;
	float specularGeneralN;
	void load(){
		proximityPiFactor = __config["plight_pi_factor"];
		proximityNumerator = __config["plight_numerator"];
		specularGeneralN = __config["spec_general_n"];
	}
}__rayLightInfo;

RayTriangleIntersection ray_getCell(Model& model, const glm::vec3& rayDirection, glm::vec3& shooterPosition, int ignoredIndex = -1){
	RayTriangleIntersection rti;
	rti.distanceFromCamera = INFINITY;
	rti.hasIntersection = false;

	size_t index = 0;
	for(auto& tri : model.triangles){
		if(index != ignoredIndex){
			glm::vec3 possibleSolution = ray_getPossibleSolution(tri, shooterPosition, rayDirection);
			if(ray_solutionValid(possibleSolution)){
				if(possibleSolution.x < rti.distanceFromCamera){
					rti.distanceFromCamera = possibleSolution.x;
					rti.hasIntersection = true;
					rti.intersectedTriangle = tri;
					rti.intersectionPoint = ray_getIntersectionPoint(tri, possibleSolution);
					rti.u = possibleSolution.y;
					rti.v = possibleSolution.z;
					rti.triangleIndex = index;
				}
			}
		}
		index += 1;
	}
	return rti;
}
std::array<float, 3> ray_barycentricProportions(const RayTriangleIntersection& rti){
	std::array<float, 3> props;

	float u = rti.u;
	float v = rti.v;
	float w = 1- (u + v);

	props[0] = u;
	props[1] = v;
	props[2] = w;

	return props;
}

glm::vec3 ray_getPhongNormal(const RayTriangleIntersection& rti, Model& model){
	auto bcc = ray_barycentricProportions(rti);
	auto& normals = model.triangles[rti.triangleIndex].vertexNormals;

	auto normal = (normals[0]*bcc[2]) + (normals[1]*bcc[0]) + (normals[2]*bcc[1]);
	// char a;
	// printf("Normals are:\n");
	// unsigned prop[] = {2, 0, 1};
	// for(size_t i = 0; i < 3; i++){
	// 	printf("\t%f of %f %f %f\n", bcc[prop[i]], normals[i].x, normals[i].y, normals[i].z);
	// }
	// printf("\nFor a total of %f %f %f\n", normal.x, normal.y, normal.z);
	// std::cin>>a;
	return normal;
}

glm::vec3 ray_getNormalOf(const RayTriangleIntersection& rti, Model& model){
	switch(rti.intersectedTriangle.normalFinder){
		case Regular:
		case Gouraud:
			return rti.intersectedTriangle.normal;
		case Phong:
			return ray_getPhongNormal(rti, model);
	}
	return glm::vec3(1);
}

float ray_getDiffuse(float distanceToLight, float insidentIntensity, const glm::vec3& u_surfaceToLight){
	float radiusSqr = pow(std::abs(distanceToLight), 2);
	float proximIntensity = __rayLightInfo.proximityNumerator/ (__rayLightInfo.proximityPiFactor * PI * radiusSqr);
	clamp<float>(proximIntensity, 0.0, 1.0);

	float diffuseBrightness = (proximIntensity + insidentIntensity) / 2.0;
	return diffuseBrightness;
}

float ray_getSpecular(const glm::vec3& u_surfaceToLight, const glm::vec3& u_surfaceNormal, const glm::vec3& cameraPos, const glm::vec3& surfacePos, float insidentIntensity){
	glm::vec3 vectorOfReflection = u_surfaceToLight - (2.0f * u_surfaceNormal * insidentIntensity);
	glm::vec3 rayDirection = surfacePos - cameraPos;
	glm::vec3 intersectToView = glm::vec3(0) - glm::normalize(rayDirection);
	float specularBrightness = pow(glm::dot(intersectToView, vectorOfReflection), __rayLightInfo.specularGeneralN);
	return specularBrightness;
}

const float ignorDistThreshold = 0.05;

float ray_setRTI(RayTriangleIntersection& rti, Model& model, Camera& camera, glm::vec3 rayDirection, bool ignoreGouraud = false){
	//Now handle lighting
	float brightness = 0.2;
	if(rti.hasIntersection){
				// auto intersectNormal = ray_getNormalOf(rti, model);

				// intersectNormal = glm::normalize(intersectNormal);
				// rti.intersectedTriangle.colour = Colour(128.0 * (intersectNormal.x+1), 128.0 * (intersectNormal.y+1), 128.0);
				// return 0.5;

		glm::vec3 intersectPosition = rti.intersectionPoint;
		glm::vec3 intersectionToLight = model.lights[0].position - intersectPosition;
		float distanceToLight = glm::length(intersectionToLight);
		glm::vec3 u_intersectionToLight = glm::normalize(intersectionToLight);
		auto lightIntersectInfo = ray_getCell(model, u_intersectionToLight, intersectPosition, rti.triangleIndex);
		bool illuminated = false;

		if(lightIntersectInfo.hasIntersection == false){
			illuminated = true;
		}
		else{
			if((lightIntersectInfo.distanceFromCamera > distanceToLight) && (lightIntersectInfo.distanceFromCamera > ignorDistThreshold)){
				illuminated = true;
			}
		}

		if(illuminated){
			if(rti.intersectedTriangle.normalFinder != Gouraud || ignoreGouraud){
				//Hecc it, keep this code
				auto intersectNormal = ray_getNormalOf(rti, model);

				intersectNormal = glm::normalize(intersectNormal);
				float insidentIntensity = glm::dot(u_intersectionToLight, intersectNormal);
				clamp<float>(insidentIntensity, 0.0, 1.0);

				// float diffuseBrightness = (proximIntensity + insidentIntensity) / 2.0;
				float diffuseBrightness = ray_getDiffuse(distanceToLight, insidentIntensity, u_intersectionToLight);
			
				// glm::vec3 vectorOfReflection = u_intersectionToLight - (2.0f * intersectNormal * insidentIntensity);
				// glm::vec3 intersectToView = glm::vec3(0) - glm::normalize(rayDirection);
				// float specularBrightness = pow(glm::dot(intersectToView, vectorOfReflection), __rayLightInfo.specularGeneralN);
				float specularBrightness = ray_getSpecular(u_intersectionToLight, intersectNormal, camera.rayPos, rti.intersectionPoint, insidentIntensity);


				brightness = diffuseBrightness + specularBrightness;
			}
			else{
				std::array<float, 3> rayBright;
				for(size_t i = 0; i <3;i++){
					glm::vec3& surfaceNormal = model.triangles[rti.triangleIndex].vertexNormals[i];
					auto vertPos = rti.intersectedTriangle.vertices[i];
					glm::vec3 vertexToLight = model.lights[0].position - rti.intersectedTriangle.vertices[i];
					glm::vec3 u_vertexToLight = glm::normalize(vertexToLight);
					float insidentIntensity = glm::dot(u_vertexToLight, surfaceNormal);
					clamp<float>(insidentIntensity, 0.0, 1.0);

					float diffuse = ray_getDiffuse(glm::length(vertexToLight), insidentIntensity, u_vertexToLight);
					float specular = ray_getSpecular(u_vertexToLight, glm::normalize(surfaceNormal), camera.rayPos, vertPos, insidentIntensity);

					rayBright[i] = diffuse + specular;
				}
				auto bcc = ray_barycentricProportions(rti);

				brightness = rayBright[0]*bcc[2] + rayBright[1]*bcc[0] + rayBright[2]*bcc[1];
			}
		}
		
		clamp<float>(brightness, 0.2, 1.0);
		rti.intersectedTriangle.colour = rti.intersectedTriangle.colour * brightness;
	}

		//auto bcc = ray_barycentricProportions(model.triangles[rti.triangleIndex], rti.intersectionPoint);
		//rti.intersectedTriangle.colour = Colour(bcc[0] * 200, bcc[1] * 200, bcc[2] * 200);

	return brightness;

}

void ray_traceRow(void* voidRowTracePtr){
	RowTrace* rowTrace = (RowTrace*)voidRowTracePtr;
	int arrayX = 0;
	Camera& camera = *rowTrace->camera;
	Model& model = *rowTrace->model;
	for(float x = WIDTH / -2; x < WIDTH / 2 && arrayX < WIDTH; x++){
		glm::vec3 rayDirection(x / 50, rowTrace->rayY / 50 * -1, camera.focalLength * -1);
		//rayDirection = rayDirection * inverse;
		rayDirection = rayDirection * camera.rayOrientation;

		RayTriangleIntersection& rti = rowTrace->intoRow->operator[](arrayX);
		rti = ray_getCell(model, rayDirection, camera.rayPos);
		ray_setRTI(rti, model, camera, rayDirection);

		arrayX++;
	}
	rowTrace->finishSemaphore->incriment();
	delete rowTrace;
}

void ray_raytraceInto(std::array<std::array<RayTriangleIntersection, WIDTH>, HEIGHT>& into, Model& model, Camera& camera){
	int arrayX, arrayY;
	arrayX = arrayY = 0;
	Semaphore finish(0);
	for(float y = HEIGHT / -2.0; y < HEIGHT / 2.0 && arrayY < HEIGHT; y++){
		RowTrace* thisRow = new RowTrace();
		thisRow->camera = &camera;
		thisRow->model = &model;
		thisRow->intoRow = &into[arrayY];
		thisRow->rayY = y;
		thisRow->finishSemaphore = &finish;

		pool::addWork(ray_traceRow, (void*)thisRow);

		arrayY++;
	}
	for(size_t i = 0; i < arrayY; i++)
		finish.decriment();
}

uint32_t ray_getColourOf(RayTriangleIntersection& rti, Window& window){
	if(rti.intersectedTriangle.texture == nullptr)
		return getColourData(rti.intersectedTriangle.colour);
	else{
		auto points = rti.intersectedTriangle.texturePoints;
		TexturePoint tp = points[0];
		tp = tp + ((points[1] - points[0]) * rti.u) + ((points[2] - points[0]) * rti.v);
		uint32_t cDat = getColourData(tp.x, tp.y, rti.intersectedTriangle.texture);
		return cDat;
	}
}

void ray_drawResult(std::array<std::array<RayTriangleIntersection, WIDTH>, HEIGHT>& info, Window& window){
	for(size_t y = 0; y < HEIGHT; y++){
		for(size_t x = 0; x < WIDTH; x++){
			if(info[y][x].hasIntersection){
				window.setPixelColour(x, y, ray_getColourOf(info[y][x], window), info[y][x].distanceFromCamera);
			}
		}
	}
}