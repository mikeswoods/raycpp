/******************************************************************************
 *
 * This file defines basic configuration file reader according to the text
 * format commonly used in assignment for CIS560
 *
 * @file Config.h
 * @author Michael Woods
 *
 *****************************************************************************/

#include <cctype>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <utility>
#include <ctime>
#include <easylogging++.h>

#include "ModelImport.h"
#include "Utils.h"
#include "Config.h"
#include "Sphere.h"
#include "Cube.h"
#include "Cylinder.h"
#include "Mesh.h"
#include "GLGeometry.h"
#include "PointLight.h"
#include "AreaLight.h"
#include "SurfaceMap.h"

/******************************************************************************/

using namespace std;

/******************************************************************************/

#if defined(_WIN32) || defined(_WIN64)
	#define DirSep "\\"
#else
	#define DirSep "/"
#endif

/******************************************************************************/

Configuration::Configuration(const string& _filename) :
	filename(_filename),
	RESO{0,0},
	EYEP{0.0f, 0.0f, 0.0f},
	VDIR{0.0f, 0.0f, 0.0f},
	UVEC{0.0f, 0.0f, 0.0f},
	FOVY(0.0f),
	envMap(shared_ptr<EnvironmentMap>(nullptr)),
	materials(shared_ptr<MATERIALS>(make_shared<MATERIALS>())),
	lights(shared_ptr<LIGHTS>(make_shared<LIGHTS>()))
{

}

Configuration::~Configuration() 
{ 

}

ostream& operator<<(ostream& os, const Configuration& c)
{
    os << "Configuration {"  << endl
	   << "  filename  = "   << c.filename << endl
       << "  RESO      = <"  << c.RESO[0]  << "," << c.RESO[1] << ">" << endl
       << "  EYEP      = <"  << c.EYEP[0]  << "," << c.EYEP[1] << "," << c.EYEP[2] << ">" << endl
       << "  VDIR      = <"  << c.VDIR[0]  << "," << c.VDIR[1] << "," << c.VDIR[2] << ">" << endl
       << "  UVEC      = <"  << c.UVEC[0]  << "," << c.UVEC[1] << "," << c.UVEC[2] << ">" << endl
       << "  FOVY      = "   << c.FOVY            << endl
       << "  |light|   = "   << c.lights->size()   << endl
       << "}"
	   << endl 
	   << endl;

	os << "Materials { " << endl;
	for (auto i=c.materials->begin(); i != c.materials->end(); i++) {
		os << "  \"" << i->first << "\": " << *(i->second) << endl;
	}
	os << "}"
	   << endl
	   << endl;

	os << "Graph {"                 << endl 
	   << "  " << c.graph << endl
	   << "}"                       << endl;

	return os;
}

void Configuration::registerMaterial(shared_ptr<Material> material)
{
	(*this->materials)[material->getName()] = material;
}

shared_ptr<Material> Configuration::getMaterial(const string& name) const
{
	return this->materials->at(name);
}

bool Configuration::materialExists(const string& name) const
{
	return this->materials->find(name) != this->materials->end();
}

void Configuration::registerLight(shared_ptr<Light> light)
{
	this->lights->push_back(light);
}

/**
 * Reads values from a "CAMERA" section using the following format:
 *
 * RESO <width:int> <height:int>
 * EYEP <x:float> <y:float> <z:float>
 * VDIR <x:float> <y:float> <z:float>
 * UVEC <x:float> <y:float> <z:float>
 * FOVY <angle:float>
 */
void Configuration::parseCameraSection(istream& is, const string& beginToken)
{
	string line, attribute;
	bool readNonEmptyLine = false;

	#ifdef ENABLE_DEBUG
	LOG(DEBUG) << "<<parseCameraSection>>";
	#endif

	while (getline(is, line)) {

		line = Utils::trim(line);
		if (line.length() == 0) {
			if (readNonEmptyLine) {
				break;
			} else {
				continue;
			}
		}

		istringstream ss(line);
		ss >> attribute;

		#ifdef ENABLE_DEBUG
		LOG(DEBUG) << "ATTRIBUTE<parseCameraSection>: "<< attribute;
		#endif

		if (attribute == "RESO") {
			ss >> this->RESO[0] >> this->RESO[1];
			readNonEmptyLine = true;
		} else if (attribute == "EYEP") {
			ss >> this->EYEP[0] >> this->EYEP[1] >> this->EYEP[2];
			readNonEmptyLine = true;
		} else if (attribute == "VDIR") {
			ss >> this->VDIR[0] >> this->VDIR[1] >> this->VDIR[2];
			readNonEmptyLine = true;
		} else if (attribute == "UVEC") {
			ss >> this->UVEC[0] >> this->UVEC[1] >> this->UVEC[2];
			readNonEmptyLine = true;
		} else if (attribute == "FOVY") {
			ss >> this-> FOVY;
			readNonEmptyLine = true;
		} else {
			LOG(WARNING) << "<parseCameraSection> Ignoring extra attribute: " 
			             << attribute;
		}
	}
}

/**
 * Reads values from a "ENVIRONMENT" section using the following format:
 *
 * FILE "filename.bmp"               -- Environment mapping file
 * SHAPE <SPHERE|CUBE|WILD1|WILD2>   -- Mapping shape: sphere or cube
 */
void Configuration::parseEnvironmentSection(istream& is, const string& beginToken)
{
	string line, attribute;
	bool readNonEmptyLine = false;

	string SHAPE           = "";
	string envMapFile      = "";
	string basePath        = Utils::baseName(Utils::realPath(this->filename));

	#ifdef ENABLE_DEBUG
	LOG(DEBUG) << "<<parseEnvironmentSection>>" << endl;
	#endif

	while (getline(is, line)) {

		line = Utils::trim(line);
		if (line.length() == 0) {
			if (readNonEmptyLine) {
				break;
			} else {
				continue;
			}
		}

		istringstream ss(line);
		ss >> attribute;

		#ifdef ENABLE_DEBUG
		LOG(DEBUG) << "ATTRIBUTE<parseEnvironmentSection>: " << attribute;
		#endif

		if (attribute == "FILE") {
			ss >> envMapFile;
			// Get the basepath from the filename for tetxure map file lookup:
			envMapFile = basePath + DirSep + "environments" + DirSep + envMapFile;
			readNonEmptyLine = true;
		} else if (attribute == "SHAPE") {
			ss >> SHAPE;
			readNonEmptyLine = true;
		} else {
			LOG(WARNING) << "<parseEnvironmentSection> Ignoring extra attribute: " 
			             << attribute ;
		}
	}

	this->envMap = shared_ptr<EnvironmentMap>(make_shared<TextureEnvironmentMap>(envMapFile, Utils::uppercase(SHAPE)));
}

/**
 * Reads values from a "MAT" (material) section using the following format:
 *
 * MAT
 * DIFF <r:float> <g:float> <b:float> -- Diffuse color
 * REFL <r:float> <g:float> <b:float> -- Reflection color
 * EXPO <value:float>                 -- Specular exponent
 * IOR <value:float>                  -- Index of refraction
 * MIRR <0|1>                         -- Mirror surface
 * TRAN <0|1>                         -- Transparent surface
 * EMIT <0|1>                         -- Emissive surface (area light)
 * TEXTURE "filename.bmp"             -- Texture mapping file
 * BUMP "filename.bmp"                -- Bump mapping file
 */
void Configuration::parseMaterialSection(istream& is, const string& beginToken)
{
	string line, attribute;
	bool readNonEmptyLine = false;

	string name   = "";
	float DIFF[3] = { 0.0f, 0.0f, 0.0f };
	float REFL[3] = { 0.0f, 0.0f, 0.0f };
	float EXPO    = 0.0f;
	float IOR     = 0.0f;
	int   MIRR    = 0;
	int   TRAN    = 0;
	int   EMIT    = 0;
	float AMBIENT = Material::DEFAULT_AMBIENT_COEFF;
	string textureMapFile = "", bumpMapFile = "";
	string basePath = Utils::baseName(Utils::realPath(this->filename));

	#ifdef ENABLE_DEBUG
	LOG(DEBUG) << "<<parseMaterialSection>>";
	#endif

	while (getline(is, line)) {

		line = Utils::trim(line);
		if (line.length() == 0) {
			if (readNonEmptyLine) {
				break;
			} else {
				continue;
			}
		}

		istringstream ss(line);
		ss >> attribute;

		#ifdef ENABLE_DEBUG
		LOG(DEBUG) << "ATTRIBUTE<parseMaterialSection>: "<< attribute;
		#endif

		if (attribute == "DIFF") {
			ss >> DIFF[0] >> DIFF[1] >> DIFF[2];
			readNonEmptyLine = true;
		} else if (attribute == "REFL") {
			ss >> REFL[0] >> REFL[1] >> REFL[2];
			readNonEmptyLine = true;
		} else if (attribute == "EXPO") {
			ss >> EXPO;
			readNonEmptyLine = true;
		} else if (attribute == "IOR") {
			ss >> IOR;
			readNonEmptyLine = true;
		} else if (attribute == "MIRR") {
			ss >> MIRR;
			readNonEmptyLine = true;
		} else if (attribute == "TRAN") {
			ss >> TRAN;
			readNonEmptyLine = true;
		} else if (attribute == "EMIT") {
			ss >> EMIT;
			readNonEmptyLine = true;
		} else if (attribute == "AMBIENT") {
			ss >> AMBIENT;
			readNonEmptyLine = true;
		} else if (attribute == "TEXTURE") {
			ss >> textureMapFile;
			// Get the basepath from the filename for tetxure map file lookup:
			textureMapFile = basePath + DirSep + "textures" + DirSep + textureMapFile;
			readNonEmptyLine = true;
		} else if (attribute == "BUMP") {
			ss >> bumpMapFile;
			// Get the basepath from the filename for normal map file lookup:
			bumpMapFile = basePath + DirSep + "textures" + DirSep + bumpMapFile;
			readNonEmptyLine = true;
		} else {
			if (beginToken == "MAT") {
				name = attribute;
			} else {
				LOG(WARNING) << "<parseMaterialSection> Ignoring extra attribute: " 
				             << attribute 
				             << endl;
			}
		}
	}

	// Sanity check
	if (Utils::trim(name) == "") {
		throw runtime_error("Material name cannot be empty");
	}

	shared_ptr<TextureMap> textureMap(nullptr);
	if (textureMapFile != "") {
		textureMap = shared_ptr<TextureMap>(make_shared<TextureMap>(textureMapFile));
	}

	shared_ptr<BumpMap> bumpMap(nullptr);
	if (bumpMapFile != "") {
		bumpMap = shared_ptr<BumpMap>(make_shared<BumpMap>(bumpMapFile));
	}

	shared_ptr<Material> 
		material(make_shared<Material>(
			 name
			,Color(DIFF), Color(REFL), EXPO, IOR 
			,MIRR != 0, TRAN != 0, EMIT != 0, AMBIENT, textureMap, bumpMap));

	this->registerMaterial(material);
}

/**
 * Reads values from a "LIGHT" (point light) section using the following format:
 *
 * LIGHT
 * LPOS <x:float> <y:float> <z:float>
 * LCOL <r:float> <g:float> <b:float>
 */
void Configuration::parsePointLightSection(istream& is, const string& beginToken)
{
	string line, attribute;
	bool readNonEmptyLine = false;

	float LPOS[3] = { 0.0f, 0.0f, 0.0f };
	float LCOL[3] = { 0.0f, 0.0f, 0.0f };

	#ifdef ENABLE_DEBUG
	LOG(DEBUG) << "<<parsePointLightSection>>" << endl;
	#endif

	while (getline(is, line)) {

		line = Utils::trim(line);
		if (line.length() == 0) {
			if (readNonEmptyLine) {
				break;
			} else {
				continue;
			}
		}

		istringstream ss(line);
		ss >> attribute;

		#ifdef ENABLE_DEBUG
		LOG(DEBUG) << "ATTRIBUTE<parsePointLightSection>: " << attribute;
		#endif

		if (attribute == "LPOS") {
			ss >> LPOS[0] >> LPOS[1] >> LPOS[2];
			readNonEmptyLine = true;
		} else if (attribute == "LCOL") {
			ss >> LCOL[0] >> LCOL[1] >> LCOL[2];
			readNonEmptyLine = true;
		} else {
			LOG(WARNING) << "<parsePointLightSection> Ignoring extra attribute: " 
			             << attribute;
		}
	}

	auto light = make_shared<PointLight>(glm::vec3(LPOS[0], LPOS[1], LPOS[2])
		                                ,Color(LCOL[0], LCOL[1], LCOL[2]));
	this->registerLight(light);
}

/**
 * Parses a node definition of the following format, adding
 * it to the node map maintained by this class
 *
 * NODE <name:string>
 * TRANSLATION <x:float> <y:float> <z:float>
 * ROTATION <x:float> <y:float> <z:float>
 * SCALE <x:float> <y:float> <z:float>
 * CENTER <x:float> <y:float> <z:float>
 * PARENT <parent-name:string>
 * SHAPE <type-name:string>
 * MAT <name:string>
 */
void Configuration::parseNodeDefinition(istream& is, const string& beginToken)
{
	string line, attribute;
	string objFileName    = "";
	bool isMesh           = false;
	bool readNonEmptyLine = false;
	bool firstLine        = true;

	std::shared_ptr<GraphNode> node(nullptr);
	shared_ptr<Geometry> geometry(nullptr);

	#ifdef ENABLE_DEBUG
	LOG(DEBUG) << "<<parseNodeDefinition>>" << endl;
	#endif

	while (getline(is, line)) {

		if (firstLine) {
			line = Utils::trim(beginToken + " " + line);
			firstLine = false;
		} else {
			line = Utils::trim(line);
		}

		if (line.length() == 0) {
			if (readNonEmptyLine) {
				break;
			} else {
				continue;
			}
		}

		istringstream ss(line);
		ss >> attribute;

		#ifdef ENABLE_DEBUG
		LOG(DEBUG) << "ATTRIBUTE<parseNodeDefinition>: " 
		           << attribute 
		           << endl;
		#endif

		if (attribute == "NODE") {

			string name;
			ss >> name;

			node = make_shared<GraphNode>(name);
			readNonEmptyLine = true;

		} else {

			if (node == nullptr) {
				throw runtime_error(attribute + ": NODE attribute must be defined first");
			}

			if (attribute == "TRANSLATION") {
				float T[3] = { 0.0f, 0.0f, 0.0f };
				ss >> T[0] >> T[1] >> T[2];
				node->setTranslate(glm::vec3(T[0], T[1], T[2]));
				readNonEmptyLine = true;
			} else if (attribute == "ROTATION") {
				float R[3] = { 0.0f, 0.0f, 0.0f };
				ss >> R[0] >> R[1] >> R[2];
				#ifdef GLM_FORCE_RADIANS
					node->setRotate(glm::vec3(glm::radians(R[0])
											 ,glm::radians(R[1])
											 ,glm::radians(R[2])));
				#else
					node->setRotate(glm::vec3(rx, ry, rz));
				#endif
				readNonEmptyLine = true;
			} else if (attribute == "SCALE") {
				float S[3] = { 0.0f, 0.0f, 0.0f };
				ss >> S[0] >> S[1] >> S[2];
				node->setScale(glm::vec3(S[0], S[1], S[2]));
				readNonEmptyLine = true;
			} else if (attribute == "CENTER") {
				float C[3] = { 0.0f, 0.0f, 0.0f };
				ss >> C[0] >> C[1] >> C[2];
				node->setCenter(glm::vec3(C[0], C[1], C[2]));
				readNonEmptyLine = true;
			} else if (attribute == "PARENT") {
				string parentName;
				ss >> parentName;
				 // if parentName is null, then node is the scene graph's root
				if (parentName == "null") {
					node->setParent(nullptr);
					this->graphBuilder.setRoot(node);
				} else {
					// Otherwise, see if named parent exists, and if so link it to the child:
					this->graphBuilder.linkNodes(parentName, node);
				}
				readNonEmptyLine = true;
			} else if (attribute == "SHAPE") {
				string shapeType;
				ss >> shapeType;
				if (shapeType == "null") {
					// Do nothing
				} else if (shapeType == "sphere") {
					geometry = shared_ptr<Geometry>(make_shared<Sphere>());
				} else if (shapeType == "cylinder") {
					geometry = shared_ptr<Geometry>(make_shared<Cylinder>());
				} else if (shapeType == "cube") {
					geometry = shared_ptr<Geometry>(make_shared<Cube>());
				} else if (shapeType == "mesh") {
					isMesh = true;
				} else {
					throw runtime_error("parseNodeDefinition: Unsupported geometry type: " + shapeType);
				}
				readNonEmptyLine = true;
			} else if (attribute == "FILE") {
				ss >> objFileName;
				// Get the basepath from the filename for obj file lookup:
				string path = Utils::baseName(Utils::realPath(this->filename));
				objFileName = path + DirSep + "models" + DirSep + objFileName;
				readNonEmptyLine = true;
			} else if (attribute == "MAT") {
				string matName;
				ss >> matName;
				if (matName != "null") {
					if (!this->materialExists(matName)) {
						throw runtime_error("parseNodeDefinition: Material not defined: " + matName);
					}
					shared_ptr<Material> material = this->getMaterial(matName);
					if (!material) {
						// This should never happen:
						throw runtime_error("No material instance found for name: " + matName);
					}
					node->setMaterial(material);
				}
				readNonEmptyLine = true;
			} else {
				LOG(WARNING) << "<parseNodeDefinition> Ignoring extra attribute: " 
				             << attribute;
			}
		}
	}

	// This should never happen
	if (node == nullptr) {
		throw runtime_error("Node must exist by now!");
	}

	// Did we get a mesh definition?
	if (isMesh) {
		
		objFileName = Utils::trim(objFileName);

		if (objFileName == "") {
			throw runtime_error("No object filename given for mesh object!");
		}

		vector<shared_ptr<aiMesh>> meshData = Model::importMeshes(objFileName);

		if (meshData.size() > 0) {
			geometry = shared_ptr<Geometry>(make_shared<Mesh>(meshData[0]));
		} else {
			geometry = shared_ptr<Geometry>(nullptr);
		}
	}

	// We have a non-null object:
	if (geometry) {

		// Associate the geometric object definition with the actual node:
		node->setGeometry(geometry);

		std::shared_ptr<GLGeometry> instance(make_shared<GLGeometry>(geometry));
		node->setInstance(instance);

		// Set the color of the geometry contained in the node based
		// on the diffuse color of the node's material:
		instance->setColor(node->getMaterial()->getDiffuseColor());
	}

	// Test if the map contains the node already. If so
	// signal an error, since a duplicate has been defined
	if (this->graphBuilder.nodeExists(node->getName())) {
		throw runtime_error("Duplicate node found: " + node->getName());
	} else {
		this->graphBuilder.registerNode(node);
	}
}

/**
 * Reads a configuration from the given stream, updating the corresponding 
 * member values of this instance
 */
unique_ptr<SceneContext> Configuration::read()
{
	ifstream is;
	is.open(filename.c_str(), ifstream::in);

	if (!is.good()) {
		throw runtime_error("read: " + filename + " cannot be read");
	}

	string readToken;

	while (is >> readToken) {
		if (readToken == "CAMERA") { // Begin CAMERA section
			this->parseCameraSection(is, readToken);
		} else if (readToken == "ENVIRONMENT") { // Begin ENVIRONMENT section
			this->parseEnvironmentSection(is, readToken);
		} else if (readToken == "LIGHT") { // Begin LIGHT section
			this->parsePointLightSection(is, readToken);
		} else if (readToken == "MAT") { // Begin material definition
			this->parseMaterialSection(is, readToken);
		} else { // Begin graph node definition
			this->parseNodeDefinition(is, readToken);
		}
	}

	// Finally, set the scene graph's root node:
	this->graph = this->graphBuilder.build();

	is.close();

	return unique_ptr<SceneContext>(
		new SceneContext(glm::vec2(this->RESO[0], this->RESO[1])
                        ,glm::vec3(this->EYEP[0], this->EYEP[1], this->EYEP[2])
                        ,glm::vec3(this->VDIR[0], this->VDIR[1], this->VDIR[2])
                        ,glm::vec3(this->UVEC[0], this->UVEC[1], this->UVEC[2])
                        ,this->FOVY
                        ,this->graph
                        ,this->envMap
                        ,this->materials
                        ,this->lights));
}

/******************************************************************************/
