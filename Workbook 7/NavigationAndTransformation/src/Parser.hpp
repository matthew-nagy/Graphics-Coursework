#include "Drawers.hpp"

namespace mode{
	bool setter = false;
	bool shadows = true;
	bool reflections = true;
	bool shadings = true;
	bool quality = false;
	bool cellShading = false;
	float qualityOffLevel = 5;
}

/*
###################################################
###################################################

	Parser for obj files

###################################################
###################################################
*/

struct Material{
	bool isColour = true;
	Colour col;
    TextureMap* tMap;
	TextureMap* bumpMap = nullptr;
	std::string name;
	float specN = -1;
	float reflectivity = 0.0;
};
struct Vert{
	glm::vec3 pos;
	glm::vec3 vertNormal = glm::vec3(0);
	TexturePoint uvCoords;
	Material* material;
};

struct Light{
	glm::vec3 position;

	Light(float x, float y, float z):
		position(x, y, z)
	{}
};

struct Model{
	std::vector<ModelTriangle> triangles;
	std::vector<Light> lights;
};

struct ObjFile{
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> vertNormals;
	std::vector<TexturePoint> texturePoints;
	std::unordered_map<std::string, Material> materials;

	std::vector<Light> lights;

	std::vector< std::array<Vert, 3>> faces;
	std::vector<NormalType> faceNormalTypes;
	std::vector<float> specNValues;
	std::vector<glm::vec3> normals;


	void readInObj(std::ifstream& inFile);
	void readInMaterials(std::ifstream& inFile);
	Model getModel(float scaleFactor = 1.0f);
};

size_t atou(const char* data){
	return size_t(std::atoi(data));
}

glm::vec3 getNormals(const std::array<Vert, 3>& face){
	auto a = face[1].pos - face[0].pos;
	auto b = face[2].pos - face[0].pos;

	auto norm = glm::normalize(glm::cross(a, b));

	return norm;
}

#define GSF( arr, in ) std::atof(arr[in].c_str())

void ObjFile::readInObj(std::ifstream& inFile){
	std::string currentLine = "";
	Material* currentMaterial = nullptr;
	NormalType currentNormalType = Regular;
	float globalSpecN = __config["spec_general_n"];

	bool ignoring = false;

	while(std::getline(inFile, currentLine)){
		if(ignoring){
			if(currentLine == "focus")
				ignoring = false;
			continue;
		}
		else if(currentLine == "ignore"){
			ignoring = true;
			continue;
		}

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
		else if(splits[0] == "vn"){
			vertNormals.emplace_back( GSF(splits, 1), GSF(splits, 2), GSF(splits, 3) );
		}
		else if(splits[0] == "l"){
			lights.emplace_back(std::atof(splits[1].c_str()), std::atof(splits[2].c_str()), std::atof(splits[3].c_str()));
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
				if(faceSplit.size() > 1){
					if(faceSplit[1] != "")
						face[i].uvCoords = texturePoints[atou(faceSplit[1].c_str()) - 1];
					if(faceSplit.size() > 2)
						if(faceSplit[2] != "")
							face[i].vertNormal = vertNormals[atou(faceSplit[2].c_str()) - 1];
				}
			}
			faces.emplace_back(face);
			faceNormalTypes.emplace_back(currentNormalType);
			normals.emplace_back(getNormals(face));
			
			if(currentMaterial->specN == -1){
				specNValues.emplace_back(globalSpecN);
				printf("Using global spec\n");
			}
			else{
				specNValues.emplace_back(currentMaterial->specN);
				printf("Using a spec value\n");
			}
		}
		else if(splits[0] == "RegularNormal")
			currentNormalType = Regular;
		else if(splits[0] == "Gouraud")
			currentNormalType = Gouraud;
		else if(splits[0] == "Phong")
			currentNormalType = Phong;
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
				currentMtl.tMap = currentMtl.bumpMap = nullptr;
				currentMtl.specN = -1;
				currentMtl.reflectivity = 0.0;
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
		else if(splits[0] == "map_Kb"){
			currentMtl.bumpMap = new TextureMap(splits[1]);
		}
		else if(splits[0] == "specN"){
			currentMtl.specN = std::atof(splits[1].c_str());
		}
		else if(splits[0] == "reflective"){
			currentMtl.reflectivity = std::atof(splits[1].c_str());
		}
	}

	if(onMtl){
		materials[mtlName] = currentMtl;
	}
}


Model ObjFile::getModel(float scaleFactor){
	std::vector<ModelTriangle> tris;

	for(size_t i = 0; i < faces.size(); i++){
		auto& f = faces[i];
		ModelTriangle newTri;
		newTri.normalFinder = faceNormalTypes[i];
		newTri.normal = normals[i];
		newTri.specN = specNValues[i];

		//If these vertices have different materials then by fuck I will flip
		Material* m = f[0].material;
		newTri.reflectivity = m->reflectivity;
		for(size_t j = 0; j < 3; j ++){
			newTri.vertices[j] = f[j].pos;
			newTri.vertexNormals[j] = f[j].vertNormal;
			if(!m->isColour){
				newTri.texturePoints[j] = f[j].uvCoords;
				newTri.texturePoints[j].x *= m->tMap->width;
				newTri.texturePoints[j].y *= m->tMap->height;
			}
		}

		if(m->isColour){
			newTri.texture = nullptr;
			newTri.colour = m->col;
		}
		else{
			newTri.texture = m->tMap;
		}
		newTri.bumpmap = m->bumpMap;

		glm::vec3 a = newTri.vertices[1] - newTri.vertices[0];
		glm::vec3 b = newTri.vertices[2] - newTri.vertices[0];
		glm::vec3 c = newTri.vertices[2] - newTri.vertices[1];
		newTri.area = 0.5 * (glm::length(a) + glm::length(b) + glm::length(c));

		tris.emplace_back(newTri);
	}

	Model m;
	m.triangles = tris;
	m.lights = this->lights;

	return m;
}