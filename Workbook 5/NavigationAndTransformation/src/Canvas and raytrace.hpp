#include "Parser.hpp"

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

    cp.depth = 1 / (std::abs(camera.pos.z - vertexPos.z));

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

CanvasTriangle getCanvasTri(Camera& camera, const ModelTriangle& mTri, float focalLength = 2.0f){
	CanvasTriangle cTri;
	for(size_t i = 0; i < 3; i++){
		cTri[i] = getCanvasIntersectionPoint(camera, mTri.vertices[i], focalLength);
        cTri[i].texturePoint = mTri.texturePoints[i];
    }
	return cTri;
}

