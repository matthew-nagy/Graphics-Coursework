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

template<size_t RowWidth>
struct RowTrace{
	std::array<uint32_t, RowWidth>* intoRow;
	float rayY;
	Model* model;
	Camera* camera;
	Semaphore* finishSemaphore;
};

struct Ray_Lighting_Info{
	float proximityPiFactor;
	float proximityNumerator;
	float specularGeneralN;
	std::array<std::pair<float, float>, 3> cellBoundries;
	void load(){
		/*
cell_low_thresh:0.3
cell_low_value:0.2
cell_mid_thresh:0.8
cell_mid_value:0.7
cell_high_thresh:1.0
celll_high_value:1.0
		*/
		proximityPiFactor = __config["plight_pi_factor"];
		proximityNumerator = __config["plight_numerator"];
		specularGeneralN = __config["spec_general_n"];

		cellBoundries[0].first = __config["cell_low_thresh"];
		cellBoundries[0].second = __config["cell_low_value"];
		cellBoundries[1].first = __config["cell_mid_thresh"];
		cellBoundries[1].second = __config["cell_mid_value"];
		cellBoundries[2].first = __config["cell_high_thresh"];
		cellBoundries[2].second = __config["cell_high_value"];
	}
}__rayLightInfo;

void ray_cellShadeBrightness(float& brightness){
	float b = int(brightness * 10.0);
	brightness = b / 10.0;
	return;
	for(size_t i = 0; i < __rayLightInfo.cellBoundries.size();i++)
		if(brightness <= __rayLightInfo.cellBoundries[i].first){

			brightness = __rayLightInfo.cellBoundries[i].second;
			return;
		}
}

enum TextureMode{tm_Normal, tm_Bump};
Colour ray_getTexColour(const RayTriangleIntersection& rti, TextureMode texMode = tm_Normal){
	auto& tri = rti.intersectedTriangle;

	TextureMap* tex = texMode == tm_Normal ? tri.texture : tri.bumpmap;

	auto bary = ray_barycentricProportions(rti);//2 0 1

	TexturePoint tp = (tri.texturePoints[0] * bary[2]) + (tri.texturePoints[1] * bary[0]) + (tri.texturePoints[2] * bary[1]);

	return Colour(getColourData(tp.x, tp.y, tex, rti.distanceFromCamera));
}

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

	if(rti.intersectedTriangle.texture != nullptr){
		rti.colour = ray_getTexColour(rti);
	}
	else
		rti.colour = rti.intersectedTriangle.colour;

	return rti;
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

glm::mat3 Identity3x3 = glm::mat3(1,0,0,  0,1,0,  0,0,1);

/*def rotation_matrix_from_vectors(vec1, vec2):
    """ Find the rotation matrix that aligns vec1 to vec2
    :param vec1: A 3d "source" vector
    :param vec2: A 3d "destination" vector
    :return mat: A transform matrix (3x3) which when applied to vec1, aligns it with vec2.
    """
    a, b = (vec1 / np.linalg.norm(vec1)).reshape(3), (vec2 / np.linalg.norm(vec2)).reshape(3)
    v = np.cross(a, b)
    c = np.dot(a, b)
    s = np.linalg.norm(v)
    kmat = np.array([[0, -v[2], v[1]], [v[2], 0, -v[0]], [-v[1], v[0], 0]])
    rotation_matrix = np.eye(3) + kmat + kmat.dot(kmat) * ((1 - c) / (s ** 2))
    return rotation_matrix*/

glm::mat3 operator*(glm::mat3 mat, float right){
	for(size_t i = 0; i < 3; i++)
		for(size_t j = 0; j < 3; j++)
			mat[i][j] *= right;
	return mat;
}

glm::mat3 ray_getRotationBetweenVectors(const glm::vec3& start, const glm::vec3& target){
	glm::vec3 ns = glm::normalize(start);
	glm::vec3 ne = glm::normalize(target);
	auto cross = glm::cross(ns, ne);
	auto dot = glm::dot(ns, ne);
	auto sine = glm::length(cross);
	
	glm::mat3 midMatrix(0,cross.z,cross.y*-1,  cross.z*-1,0,cross.x,   cross.y,cross.x*-1,0);
	glm::mat3 rotation =Identity3x3 + midMatrix + (midMatrix * midMatrix) * ( (1 - dot) / (pow(sine, 2)) );
	return rotation;
}

glm::vec3 ray_getBumpedNormal(const glm::vec3& oldNormal, const RayTriangleIntersection& rti){
	Colour normCol = ray_getTexColour(rti, tm_Bump);
	glm::vec3 fullNormVec(normCol.red, normCol.green, normCol.blue);
	glm::vec3 adjustedNormal = fullNormVec - glm::vec3(128);

	glm::vec3 supposedNormal = glm::vec3(0, 0, 1);
	glm::mat3 transformMatrix = ray_getRotationBetweenVectors(supposedNormal, rti.intersectedTriangle.normal);

	//Not right! rotate first
	return adjustedNormal * transformMatrix;
}

glm::vec3 ray_getNormalOf(const RayTriangleIntersection& rti, Model& model){
	glm::vec3 normal(1);
	switch(rti.intersectedTriangle.normalFinder){
		case Regular:
		case Gouraud:
			normal = rti.intersectedTriangle.normal;
		case Phong:
			if(mode::shadings)
				normal = ray_getPhongNormal(rti, model);
			else
				normal = rti.intersectedTriangle.normal;
	}

	if(rti.intersectedTriangle.bumpmap != nullptr)
		normal= ray_getBumpedNormal(normal, rti);

	return normal;
}

float ray_getDiffuse(float distanceToLight, float insidentIntensity, const glm::vec3& u_surfaceToLight){
	float radiusSqr = pow(std::abs(distanceToLight), 2);
	float proximIntensity = __rayLightInfo.proximityNumerator/ (__rayLightInfo.proximityPiFactor * PI * radiusSqr);
	clamp<float>(proximIntensity, 0.0, 1.0);

	float diffuseBrightness = (proximIntensity + insidentIntensity) / 2.0;
	return diffuseBrightness;
}

float ray_getSpecular(const glm::vec3& u_surfaceToLight, const glm::vec3& u_surfaceNormal, const glm::vec3& cameraPos, const glm::vec3& surfacePos, float insidentIntensity, float specN){
	glm::vec3 vectorOfReflection = u_surfaceToLight - (2.0f * u_surfaceNormal * insidentIntensity);
	glm::vec3 rayDirection = surfacePos - cameraPos;
	glm::vec3 intersectToView = glm::vec3(0) - glm::normalize(rayDirection);
	float specularBrightness = pow(glm::dot(intersectToView, vectorOfReflection), specN);
	return specularBrightness;
}

const float ignorDistThreshold = 0.05;

float ray_setRTI(RayTriangleIntersection& rti, Model& model, Camera& camera, glm::vec3 rayDirection, bool ignoreGouraud = false){
	if(!mode::shadows)
		return 0.9;

	//Now handle lighting
			// auto intersectNormal = ray_getNormalOf(rti, model);

			// intersectNormal = glm::normalize(intersectNormal);
			// rti.intersectedTriangle.colour = Colour(128.0 * (intersectNormal.x+1), 128.0 * (intersectNormal.y+1), 128.0);
			// return 0.5;

	glm::vec3 intersectPosition = rti.intersectionPoint;

	float totBrightness = 0.0;

	for(size_t i = 0; i < model.lights.size(); i++){

	float brightness = 0.2;
	glm::vec3 intersectionToLight = model.lights[i].position - intersectPosition;
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
		if(rti.intersectedTriangle.normalFinder != Gouraud || ignoreGouraud || !mode::shadings){
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
			float specularBrightness = ray_getSpecular(u_intersectionToLight, intersectNormal, camera.rayPos, rti.intersectionPoint, insidentIntensity, rti.intersectedTriangle.specN);


			brightness = diffuseBrightness + specularBrightness;
		}
		else{
			std::array<float, 3> rayBright;
			auto& thisTri = model.triangles[rti.triangleIndex];
			for(size_t i = 0; i <3;i++){
				glm::vec3& surfaceNormal = thisTri.vertexNormals[i];
				auto vertPos = rti.intersectedTriangle.vertices[i];
				glm::vec3 vertexToLight = model.lights[i].position - rti.intersectedTriangle.vertices[i];
				glm::vec3 u_vertexToLight = glm::normalize(vertexToLight);
				float insidentIntensity = glm::dot(u_vertexToLight, surfaceNormal);
				clamp<float>(insidentIntensity, 0.0, 1.0);

				float diffuse = ray_getDiffuse(glm::length(vertexToLight), insidentIntensity, u_vertexToLight);
				float specular = ray_getSpecular(u_vertexToLight, glm::normalize(surfaceNormal), camera.rayPos, vertPos, insidentIntensity, thisTri.specN);

				rayBright[i] = diffuse + specular;
			}
			auto bcc = ray_barycentricProportions(rti);

			brightness = rayBright[0]*bcc[2] + rayBright[1]*bcc[0] + rayBright[2]*bcc[1];
		}
	}
	clamp<float>(brightness, 0.2, 1.0);
	totBrightness += brightness;

	}

	totBrightness = totBrightness / float(model.lights.size());

	if(mode::cellShading){
		printf("REEEE\n");
		ray_cellShadeBrightness(totBrightness);
	}

	rti.colour = rti.colour * totBrightness;

	//auto bcc = ray_barycentricProportions(model.triangles[rti.triangleIndex], rti.intersectionPoint);
	//rti.intersectedTriangle.colour = Colour(bcc[0] * 200, bcc[1] * 200, bcc[2] * 200);

	return totBrightness;

}

const int MaxReflectionNum = 5;

RayTriangleIntersection ray_getRay(glm::vec3 rayDirection, Camera& camera, glm::vec3 shootPos, Model& model, int reflections = 0, int ignoredIndex = -1){
	//rayDirection = rayDirection * inverse;
	rayDirection = rayDirection * camera.rayOrientation;

	RayTriangleIntersection rti = ray_getCell(model, rayDirection, shootPos, ignoredIndex);
	if(rti.hasIntersection){
		float reflectivity = rti.intersectedTriangle.reflectivity;
		if(reflectivity > 0 && reflections < MaxReflectionNum && mode::reflections){
			auto normal = ray_getNormalOf(rti, model);
			glm::vec3 reflectiveRay = rayDirection - (2 *  normal * glm::dot(rayDirection, normal));
			RayTriangleIntersection reflection = ray_getRay(reflectiveRay, camera, rti.intersectionPoint, model, reflections+1, rti.triangleIndex);
			
			rti.colour = (rti.colour * (1-reflectivity)) + (reflection.colour * reflectivity);
		}

		ray_setRTI(rti, model, camera, rayDirection);
	}
	return rti;
}

template<int RTWidth>
void ray_traceRow(void* voidRowTracePtr){
	RowTrace<RTWidth>* rowTrace = (RowTrace<RTWidth>*)voidRowTracePtr;
	int arrayX = 0;
	Camera& camera = *rowTrace->camera;
	Model& model = *rowTrace->model;
	float focalDivider = 300.0;
	float incr = mode::quality ? float(WIDTH) / float(RTWidth) : mode::qualityOffLevel;
	for(float x = WIDTH / -2.0; x < WIDTH / 2.0; x += incr){
		glm::vec3 rayDirection(x / focalDivider, rowTrace->rayY / focalDivider * -1, camera.focalLength * -1);
		RayTriangleIntersection ray = ray_getRay(rayDirection, camera, camera.rayPos, model);
		if(ray.hasIntersection)
			rowTrace->intoRow->operator[](arrayX) = getColourData(ray.colour);
		else
			rowTrace->intoRow->operator[](arrayX) = 0;
		

		arrayX++;
	}
	rowTrace->finishSemaphore->incriment();
	delete rowTrace;
}

template<size_t RWidth, int RHeight>
void ray_raytraceInto(std::array<std::array<uint32_t, RWidth>, RHeight>& into, Model& model, Camera& camera){
	int arrayX, arrayY;
	arrayX = arrayY = 0;
	Semaphore finish(0);
	float incr = mode::quality ? float(HEIGHT) / float(RHeight) : mode::qualityOffLevel;
	for(float y = HEIGHT / -2.0; y < HEIGHT / 2.0; y += incr){
		RowTrace<RWidth>* thisRow = new RowTrace<RWidth>();
		thisRow->camera = &camera;
		thisRow->model = &model;
		thisRow->intoRow = &into[arrayY];
		thisRow->rayY = y;
		thisRow->finishSemaphore = &finish;

		pool::addWork(ray_traceRow<RWidth>, (void*)thisRow);

		arrayY++;
	}
	for(size_t i = 0; i < arrayY; i++)
		finish.decriment();
}

template<int RW, int RH>
void ray_drawResult(std::array<std::array<uint32_t, RW>, RH>& info, Window& window){
	float incr = mode::quality ? 1 : mode::qualityOffLevel;
	for(size_t y = 0; y < HEIGHT / incr; y++){
		for(size_t x = 0; x < WIDTH / incr; x++){
			if(incr > 1)
				for(size_t i = 0; i < incr; i++)
					for(size_t j = 0; j < incr; j++)
						window.setPixelColour((x*incr)+j, (y*incr)+i, info[y][x], 0);
			else{
				if(mode::fastAproxAntiAliasing){
					//fast approximate
					Colour finalColour = Colour(info[y][x]) * 0.7;
					float sideProp = 0.3 / 8;
					for(int i = -1; i <= 1; i++)
						for(int j = -1; j <=1; j++)
							if(i == j && i == 0)
								continue;
							else
								finalColour = finalColour + (Colour(info[(y+i)%RH][(x+j)%RW]) * sideProp);
					window.setPixelColour(x, y, getColourData(finalColour), 0);
				}
				else if(mode::superAntiAliasing){
					size_t sy = y * mode::superScalerQuality;
					size_t sx = x * mode::superScalerQuality;
					float downscaleLevel = 1.0 / (float(mode::superScalerQuality) * float(mode::superScalerQuality));
					Colour finalColour(0,0,0);
					for(size_t i = 0; i < mode::superScalerQuality;i++)
						for(size_t j = 0; j < mode::superScalerQuality;j++)
							finalColour = finalColour + (Colour(info[sy + i][sx + j]) * downscaleLevel);
					window.setPixelColour(x, y, getColourData(finalColour));
				}
				else{
					window.setPixelColour(x, y, info[y][x], 0);
				}
			}
		}
	}
}