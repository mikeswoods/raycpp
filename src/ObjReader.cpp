/******************************************************************************
 *
 * This file defines a class for reading a *.obj file and constructing a
 * geometric mesh object from the definition
 *
 * @file ObjReader.h
 * @author Michael Woods
 *
 *****************************************************************************/

#define _USE_MATH_DEFINES
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <string>
#include <vector>
#include <limits>
#include "ObjReader.h"
#include "Utils.h"

using namespace Utils;
using namespace std;
using namespace glm;

/*****************************************************************************/

MTL::MTL() : 
	Ka(Color::BLACK),
	Kd(Color::BLACK),
	Ks(Color::BLACK),
	d(0.0f),
	Ns(0.0f),
	illum(0.0f)
{

}

MTL::MTL(const MTL& other) : 
	Ka(other.Ka),
	Kd(other.Kd),
	Ks(other.Ks),
	d(other.d),
	Ns(other.Ns),
	illum(other.illum)
{

}

/*****************************************************************************/

MTLReader::MTLReader(const string& _mtlFile) :
	mtlFile(_mtlFile)
{


}

map<string, shared_ptr<MTL>>  MTLReader::read()
{
	map<string, shared_ptr<MTL>> mtlMap;
    ifstream is;

    is.open(this->mtlFile.c_str(), ifstream::in);

    if (!is.good()) {
        throw runtime_error(".mtl file '" + this->mtlFile + "' cannot be read or does not exist");
    }

    string line, lineType;
    shared_ptr<MTL> mtl(nullptr);

    while (getline(is, line)) {

        line = trim(line);

        if (line.length() == 0) {
            continue;
        }

        istringstream ss(line);
        ss >> lineType;

        if (lineType == "#") {
        	continue;
        } else if (lineType == "newmtl") {
        	{
        		string mtlName;
        		ss >> mtlName;
        		mtl = shared_ptr<MTL>(make_shared<MTL>());
        	}
        	break;
        } else if (lineType == "Ka") {
        	continue;
        } else if (lineType == "Kd") {
        	continue;
        } else if (lineType == "Ks") {
        	continue;
        } else if (lineType == "Tf") {
        	continue;
        } else if (lineType == "d") {
        	continue;
        } else if (lineType == "Ns") {
        	continue;
        } else if (lineType == "Ni") {
        	continue;
        } else if (lineType == "illum") {
        	continue;
        } else if (lineType == "sharpness") {
        	continue;
        } else if (lineType == "map_Ka") {
        	continue;
        } else if (lineType == "map_Kd") {
        	continue;
        } else if (lineType == "map_Ks") {
        	continue;
        } else if (lineType == "map_Ns") {
        	continue;
        } else if (lineType == "map_d") {
        	continue;
        } else if (lineType == "disp") {
        	continue;
        } else if (lineType == "decal") {
        	continue;
        } else if (lineType == "bump") {
        	continue;
        } else if (lineType == "refl") {
        	continue;
        } else {
        	continue;
        }
    }

	return mtlMap;
}

/*****************************************************************************/

ObjReader::ObjReader(const string& _objFile) :
    objFile(_objFile),
    vertices(unique_ptr<vector<vec3>>(new vector<vec3>())),
    normals(unique_ptr<vector<vec3>>(new vector<vec3>())),
    textures(unique_ptr<vector<vec3>>(new vector<vec3>())),
    faces(unique_ptr<vector<Face>>(new vector<Face>())),
    maxVComp(-numeric_limits<float>::infinity())
{

}

ObjReader::~ObjReader()
{

}

void ObjReader::reset()
{
    this->vertices->clear();
    this->normals->clear();
    this->faces->clear();
}

int ObjReader::normalizeIndex(int i) const
{
    return (i > 0) ? i : (this->vertices->size() + i) + 1;
}

shared_ptr<Mesh> ObjReader::read()
{
    this->reset();

    ifstream is;
    is.open(this->objFile.c_str(), ifstream::in);

    if (!is.good()) {
        throw runtime_error(".obj file '" + this->objFile + "' cannot be read or does not exist");
    }

    int lineCount = 0, nextFaceId = 1;
    string line, lineType;

    while (getline(is, line)) {

        line = trim(line);
        lineCount++;

        if (line.length() == 0) { // Skip blank lines
            continue;
        }

        istringstream ss(line);
        ss >> lineType;

        // See http://www.martinreddy.net/gfx/3d/OBJ.spec for reference

        if (lineType == "#") { // Comment
        
            this->parseComment(lineCount, line, ss);
        
        } else if (lineType == "g") { // Group
        
            // Ignore for now
        
        } else if (lineType == "sg") { // Smoothing Group
        
            // Ignore for now

        } else if (lineType == "mg") { // Merging Group
        
            // Ignore for now

        } else if (lineType == "o") { // Object name
        
            // Ignore for now

        } else if (lineType == "mtllib") { // Material library include

            this->parseMtllib(lineCount, line, ss);

        } else if (lineType == "usemtl") { // Use material directive
        
            // Ignore for now

        } else if (lineType == "p") { // Point
        
            // Ignore for now

        } else if (lineType == "l") { // Line
        
            // Ignore for now
        
        } else if (lineType == "v") { // Vertex

            this->parseV(lineCount, line, ss);

        } else if (lineType == "vt") { // Vertex texture

            this->parseVT(lineCount, line, ss);

        } else if (lineType == "f") { // Face

            this->parseF(lineCount, nextFaceId++, line, ss);

        } else if (lineType == "vn") { // Vertex normal

            this->parseVN(lineCount, line, ss);

        } else {

            //cerr << "[!] ObjReader: skipping unrecognized type (" << lineCount << "): " << lineType;
        }
    }

    is.close();

    // Finally, normalize the size of each vertex component to the range [0,1] if the vertex
    // components are sufficiently large: e.g. greater than 10.
    if (this->maxVComp > 10.0f) {

        float scale = 0.5f * this->maxVComp;

        for (auto i=this->vertices->begin(); i != this->vertices->end(); i++) {
            i->x /= scale; 
            i->y /= scale; 
            i->z /= scale; 
        }
    }

    cout << this->getVertices().size() << ", "
         << this->getNormals().size()  << ", "
         << this->getTextures().size() << ", "
         << this->getFaces().size()    <<
         endl;

    return make_shared<Mesh>(this->getVertices()
                            ,this->getNormals()
                            ,this->getTextures()
                            ,this->getFaces());
}

/** 
 * Parse a comment from the string stream
 */
void ObjReader::parseComment(int lineNum, const string& line, istringstream& ss)
{
    #ifdef DEBUG
    clog << "called ObjReader::parseComment" << endl;
    #endif
}

/** 
 * Parse a material file include
 */
void ObjReader::parseMtllib(int lineNum, const std::string& line, std::istringstream& ss)
{
    #ifdef DEBUG
    clog << "called ObjReader::parseMtllib" << endl;
    #endif	

    string mtlFile;
    ss >> mtlFile;

    string mtlPath = resolvePath(mtlFile, baseName(this->objFile));

    MTLReader mtlReader(mtlPath);
    mtlReader.read();
}

/** 
 * Parse a vertex from the string stream
 */
void ObjReader::parseV(int lineNum, const std::string& line, istringstream& ss)
{
    #ifdef DEBUG
    clog << "called ObjReader::parseV" << endl;
    #endif

    float vx, vy, vz;
    ss >> vx >> vy >> vz;

    if (abs(vx) > this->maxVComp) {
        this->maxVComp = abs(vx);
    }
    if (abs(vy) > this->maxVComp) {
        this->maxVComp = abs(vy);
    }
    if (abs(vz) > this->maxVComp) {
        this->maxVComp = abs(vz);
    }

    this->vertices->push_back(vec3(vx, vy, vz));
}

/**
 * Parse a vertex texture from the string stream
 */
void ObjReader::parseVT(int lineNum, const std::string& line, istringstream& ss)
{
    #ifdef DEBUG
    clog << "called ObjReader::parseVT" << endl;
    #endif

    float vtu, vtv, vtw;
    //ss >> vtu >> vtv;
    ss >> vtu >> vtv >> vtw;

    this->textures->push_back(vec3(vtu, vtv, 1.0f));    
}

/** 
 * Parse a vertex normal from the string stream
 */
void ObjReader::parseVN(int lineNum, const std::string& line, istringstream& ss)
{
    #ifdef DEBUG
    clog << "called ObjReader::parseVN" << endl;
    #endif

    float vnx, vny, vnz;
    ss >> vnx >> vny >> vnz;

    this->normals->push_back(V(vnx, vny, vnz));
}

/**
 * Classifies a face chunk into one of four categories defined by
 * ObjReader::FaceType
 */
ObjReader::FaceType ObjReader::classifyFaceChunk(std::string chunk)
{
    if (chunk.find("//") != string::npos) { // Case (3)
        return FACE_VN;
    } else if (chunk.find("/") != string::npos) { // Case (2) or (4)
        size_t sz = split(chunk, "/").size();
        if (sz == 2) {
            return FACE_VT; // Case (2)
        } else if (sz == 3) {
            return FACE_VTN; // Case (3)
        } else {
            return FACE_BAD;
        }
    }

    return FACE_V; // otherwise case (1)
}

/**
 * Parse a face definition from the string stream
 *
 * Variations:
 * (1) Face with vertices only:  "f v1 v1 v2 ... vn"
 * (2) Face with texture coords: "f v1/t1 v2/t2 .... vn/tn"
 * (3) Face with vertex normals: "f v1//n1 v2//n2 .... vn//nn"
 * (4) Face with txt and norms:  "f v1/t1/n1 v2/t2/n2 .... vn/tn/nn"
 */
void ObjReader::parseF(int lineNum, int nextFaceId
                      ,const std::string& line, istringstream& ss)
{
    #ifdef DEBUG
    clog << "called ObjReader::parseF" << endl;
    #endif

    // Detect which variation about we have:
    vector<string> raw_chunks = split(string(line), " ");

    // Copy from then 2nd element to the end into CHUNKS
    vector<string> CHUNKS;
    copy(raw_chunks.begin() + 1, raw_chunks.end(), back_inserter(CHUNKS));

    // 1 chunk = 1 vertex; n chunks = n - 2 triangles
    for (unsigned int i=1; i<CHUNKS.size()-1; i++) {

        FaceType ft[3] = { 
             ObjReader::classifyFaceChunk(CHUNKS[0])
            ,ObjReader::classifyFaceChunk(CHUNKS[i])
            ,ObjReader::classifyFaceChunk(CHUNKS[i+1])
        };

        if ((ft[0] == ft[1]) && (ft[1] == ft[2])) {
            // Do nothing
        } else {
            throw runtime_error("at line " + S(lineNum) + ": Bad face type definition \"" + line + "\"");
        }

        Face face;
        auto faceType = ft[0];

        switch (faceType) {
            case FACE_V:
                face = this->parseFV(lineNum, nextFaceId, line, CHUNKS[0], CHUNKS[i], CHUNKS[i+1]);
                break;
            case FACE_VT:
                face = this->parseFVT(lineNum, nextFaceId, line, CHUNKS[0], CHUNKS[i], CHUNKS[i+1]);
                break;
            case FACE_VN:
                face = this->parseFVN(lineNum, nextFaceId, line, CHUNKS[0], CHUNKS[i], CHUNKS[i+1]);
                break;
            case FACE_VTN:
                face = this->parseFVTN(lineNum, nextFaceId, line, CHUNKS[0], CHUNKS[i], CHUNKS[i+1]);
                break;
            default:
                throw runtime_error("at line " + S(lineNum) + ": Malformed face chunk <"+ S(ft[0]) + "> \"" + line + "\"");
        }

        this->faces->push_back(face);
    }
}

/**
 * Parse a face definiton: variant 1
 * 
 * Format: "f v1 v2 ... vn"
 */
Face ObjReader::parseFV(int lineNum, int nextFaceId, const std::string& line
                       ,const string& chunk1, const string& chunk2, const string& chunk3) const 
{
    int v[3] = {-1, -1, -1};
    int t[3] = {-1, -1, -1};
    int n[3] = {-1, -1, -1};

    // Assume each chunk is a single number:
    v[0] = this->normalizeIndex(stoi(chunk1));
    v[1] = this->normalizeIndex(stoi(chunk2));
    v[2] = this->normalizeIndex(stoi(chunk3));

    return Face(nextFaceId, v, t, n);
}

/**
 * Parse a face definiton: variant 2
 * 
 * Format: "f v1/t1 v2/t2 .... vn/tn"
 */
Face ObjReader::parseFVT(int lineNum, int nextFaceId, const std::string& line
                        ,const std::string& chunk1, const std::string& chunk2, const std::string& chunk3) const
{
    int v[3] = {-1, -1, -1};
    int t[3] = {-1, -1, -1};
    int n[3] = {-1, -1, -1};

    vector<string> parts[3] = {
         split(chunk1, "/")
        ,split(chunk2, "/")
        ,split(chunk3, "/")
    };

    if (parts[0].size() != 2) {
        throw runtime_error("at line " + S(lineNum) + " in parseFVT: bad chunk(1) \"" + line + "\"");
    } else if (parts[1].size() != 2) {
        throw runtime_error("at line " + S(lineNum) + " in parseFVT: bad chunk(2) \"" + line + "\"");
    } else if (parts[2].size() != 2) {
        throw runtime_error("at line " + S(lineNum) + " in parseFVT: bad chunk(3) \"" + line + "\"");
    }
     
    v[0] = this->normalizeIndex(stoi(trim(parts[0][0])));
    v[1] = this->normalizeIndex(stoi(trim(parts[1][0])));
    v[2] = this->normalizeIndex(stoi(trim(parts[2][0])));

    t[0] = this->normalizeIndex(stoi(trim(parts[0][1])));
    t[1] = this->normalizeIndex(stoi(trim(parts[1][1])));
    t[2] = this->normalizeIndex(stoi(trim(parts[2][1])));

    return Face(nextFaceId, v, t, n);
}

/**
 * Parse a face definiton: variant 3
 * 
 * Format: "f v1//n1 v2//n2 .... vn//nn"
 */
Face ObjReader::parseFVN(int lineNum, int nextFaceId, const std::string& line
                        ,const std::string& chunk1, const std::string& chunk2, const std::string& chunk3) const
{
    int v[3] = {-1, -1, -1};
    int t[3] = {-1, -1, -1};
    int n[3] = {-1, -1, -1};

    vector<string> parts[3] = {
         split(chunk1, "//")
        ,split(chunk2, "//")
        ,split(chunk3, "//")
    };

    if (parts[0].size() != 2) {
        throw runtime_error("at line " + S(lineNum) + " in parseFVN: bad chunk(1) \"" + line + "\"");
    } else if (parts[1].size() != 2) {
        throw runtime_error("at line " + S(lineNum) + " in parseFVN: bad chunk(2) \"" + line + "\"");
    } else if (parts[2].size() != 2) {
        throw runtime_error("at line " + S(lineNum) + " in parseFVN: bad chunk(3) \"" + line + "\"");
    }
     
    v[0] = this->normalizeIndex(stoi(trim(parts[0][0])));
    v[1] = this->normalizeIndex(stoi(trim(parts[1][0])));
    v[2] = this->normalizeIndex(stoi(trim(parts[2][0])));

    n[0] = this->normalizeIndex(stoi(trim(parts[0][1])));
    n[1] = this->normalizeIndex(stoi(trim(parts[1][1])));
    n[2] = this->normalizeIndex(stoi(trim(parts[2][1])));

    return Face(nextFaceId, v, t, n);
}

/**
 * Parse a face definiton: variant 4
 * 
 * Format: "f v1/t1/n1 v2/t2/n2 .... vn/tn/nn"
 */
Face ObjReader::parseFVTN(int lineNum, int nextFaceId, const std::string& line
                         ,const std::string& chunk1, const std::string& chunk2, const std::string& chunk3) const
{
    int v[3] = {-1, -1, -1};
    int t[3] = {-1, -1, -1};
    int n[3] = {-1, -1, -1};

    vector<string> parts[3] = {
         split(chunk1, "/")
        ,split(chunk2, "/")
        ,split(chunk3, "/")
    };

    if (parts[0].size() != 3) {
        throw runtime_error("at line " + S(lineNum) + " in parseFVTN: bad chunk(1) \"" + line + "\"");
    } else if (parts[1].size() != 3) {
        throw runtime_error("at line " + S(lineNum) + " in parseFVTN: bad chunk(2) \"" + line + "\"");
    } else if (parts[2].size() != 3) {
        throw runtime_error("at line " + S(lineNum) + " in parseFVTN: bad chunk(3) \"" + line + "\"");
    }
     
    v[0] = this->normalizeIndex(stoi(trim(parts[0][0])));
    v[1] = this->normalizeIndex(stoi(trim(parts[1][0])));
    v[2] = this->normalizeIndex(stoi(trim(parts[2][0])));

    t[0] = this->normalizeIndex(stoi(trim(parts[0][1])));
    t[1] = this->normalizeIndex(stoi(trim(parts[1][1])));
    t[2] = this->normalizeIndex(stoi(trim(parts[2][1])));

    n[0] = this->normalizeIndex(stoi(trim(parts[0][2])));
    n[1] = this->normalizeIndex(stoi(trim(parts[1][2])));
    n[2] = this->normalizeIndex(stoi(trim(parts[2][2])));

    return Face(nextFaceId, v, t, n);
}

/*****************************************************************************/
