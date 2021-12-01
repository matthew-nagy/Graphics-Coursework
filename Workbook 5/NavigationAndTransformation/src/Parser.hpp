#include "Drawers.hpp"

/*
###################################################
###################################################

	Parser for obj files

###################################################
###################################################
*/

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

struct Material{
	bool isColour = true;
	Colour col;
    TextureMap* tMap;
	std::string name;
};
struct Vert{
	glm::vec3 pos;
	TexturePoint uvCoords;
	Material* material;
};

struct ObjFile{
	std::vector<glm::vec3> positions;
	std::vector<TexturePoint> texturePoints;
	std::unordered_map<std::string, Material> materials;

	std::vector< std::array<Vert, 3>> faces;


	void readInObj(std::ifstream& inFile);
	void readInMaterials(std::ifstream& inFile);
	std::vector<ModelTriangle> getModelTriangles(float scaleFactor = 1.0f);
};

size_t atou(const char* data){
	return size_t(std::atoi(data));
}

#define GSF( arr, in ) std::atof(arr[in].c_str())

void ObjFile::readInObj(std::ifstream& inFile){
	std::string currentLine = "";
	Material* currentMaterial = nullptr;
	while(std::getline(inFile, currentLine)){
		auto splits = splitStringOn(currentLine, ' ');
		if(splits[0] == "mtllib"){
			std::ifstream matFile(splits[1], std::ios::in);
			readInMaterials(matFile);
			matFile.close();
		}
		else if(splits[0] == "usemtl"){
			currentMaterial = &(materials[splits[1]]);
		}
		else if(splits[0] == "v"){
			positions.emplace_back( GSF(splits, 1), GSF(splits, 2), GSF(splits, 3) );
		}
		else if(splits[0] == "vt"){
			texturePoints.emplace_back(  GSF(splits, 1), GSF(splits, 2)  );
		}
		else if(splits[0] == "f"){
			std::array<Vert, 3> face;
			for(size_t i = 0; i < 3; i++){
				auto faceSplit = splitStringOn(splits[i + 1], '/');
				face[i].material = currentMaterial;
				face[i].pos = positions[atou(faceSplit[0].c_str()) - 1];
				if(faceSplit.size() > 1)
					face[i].uvCoords = texturePoints[atou(faceSplit[1].c_str()) - 1];
			}
			faces.emplace_back(face);
		}
	}
}

void ObjFile::readInMaterials(std::ifstream& inFile){
	std::string currentLine = "";
	bool onMtl = false;
	std::string mtlName = "";
	Material currentMtl;
	while(std::getline(inFile, currentLine)){
		auto splits = splitStringOn(currentLine, ' ');
		//finished, do the thing
		if(currentLine == ""){
			if(onMtl){
				onMtl = false;
				materials[mtlName] = currentMtl;
				currentMtl.tMap = nullptr;
			}
		}
		else if(splits[0] == "newmtl"){
			onMtl = true;
			mtlName = splits[1];
			currentMtl.name = splits[1];
		}
		else if(splits[0] == "Kd"){
			currentMtl.isColour = true;
			currentMtl.col = Colour( GSF(splits, 1) * 255, GSF(splits, 2) * 255, GSF(splits, 3) * 255 );
		}
		else if(splits[0] == "map_Kd"){
			currentMtl.isColour = false;
			currentMtl.tMap = new TextureMap(splits[1]);
		}
	}

	if(onMtl){
		materials[mtlName] = currentMtl;
	}
}


std::vector<ModelTriangle> ObjFile::getModelTriangles(float scaleFactor){
	std::vector<ModelTriangle> tris;

	for(auto& f : faces){
		ModelTriangle newTri;
		//If these vertices have different materials then by fuck I will flip
		Material* m = f[0].material;
		for(size_t i = 0; i < 3; i ++){
			newTri.vertices[i] = f[i].pos;
			if(!m->isColour){
				printf("Tex point was %f, %f\t is now ", f[i].uvCoords.x, f[i].uvCoords.y);
				newTri.texturePoints[i] = f[i].uvCoords;
				newTri.texturePoints[i].x *= m->tMap->width;
				newTri.texturePoints[i].y *= m->tMap->height;
				printf("%f, %f\n", newTri.texturePoints[i].x, newTri.texturePoints[i].y);
			}
		}

		if(m->isColour){
			newTri.texture = nullptr;
			newTri.colour = m->col;
		}
		else{
			newTri.texture = m->tMap;
		}

		tris.emplace_back(newTri);
	}

	printf("The texture points are:\n");
	for(size_t i = 0; i < texturePoints.size(); i++)
		printf("\t%f, %f\n", texturePoints[i].x, texturePoints[i].y);

	return tris;
}