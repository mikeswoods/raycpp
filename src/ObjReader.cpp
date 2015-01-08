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
#include <string>
#include <vector>
#include <limits>
#include "ObjReader.h"
#include "Utils.h"

using namespace Utils;
using namespace std;

/*****************************************************************************/

ObjReader::ObjReader(const string& _objFile) :
	objFile(_objFile),
	vertices(unique_ptr<vector<glm::vec3>>(new vector<glm::vec3>())),
	normals(unique_ptr<vector<glm::vec3>>(new vector<glm::vec3>())),
	uv(unique_ptr<vector<glm::vec2>>(new vector<glm::vec2>())),
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

Mesh* ObjReader::parse()
{
	this->reset();

	ifstream is;
	is.open(this->objFile.c_str(), ifstream::in);

	if (!is.good()) {
		throw runtime_error("ObjReader::readFromFile: " + this->objFile + " cannot be read");
	}

	int lineCount = 0, nextFaceId = 1;
	string line, lineType;

	while (getline(is, line)) {

		line = Utils::trim(line);
		lineCount++;

		if (line.length() == 0) { // Skip blank lines
			continue;
		}

		istringstream ss(line);
		ss >> lineType;

		if (lineType == "#") { // Comment
		
			this->parseComment(lineCount, line, ss);
		
		} else if (lineType == "g") { // Group
		
			// Ignore for now
		
		} else if (lineType == "mtllib") { // Material library include
		
			// Ignore for now

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

			//cerr << "[!] ObjReader: unsupported type (" << lineCount << "): " << lineType;

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

	return new Mesh(this->getVertices(), this->getNormals(), this->getUVs(), this->getFaces());
}

// Parse a comment from the string stream
void ObjReader::parseComment(int lineNum, const std::string& line, istringstream& ss)
{
	#ifdef DEBUG
	clog << "called ObjReader::parseComment" << endl;
	#endif
}

// Parse a vertex from the string stream
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

	this->vertices->push_back(glm::vec3(vx, vy, vz));
}

// Parse a vertex texture from the string stream
void ObjReader::parseVT(int lineNum, const std::string& line, istringstream& ss)
{
	#ifdef DEBUG
	clog << "called ObjReader::parseVT" << endl;
	#endif
}

// Parse a vertex normal from the string stream
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
		size_t sz = Utils::split(chunk, "/").size();
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
	vector<string> chunks = Utils::split(string(line), " ");

	if (chunks.size() > 4) {
		cerr << "parseF:<Warning> Only 3 vertices are supported for faces; discarding extra vertices..." << endl;
	}

	FaceType ft[3] = { 
		 ObjReader::classifyFaceChunk(chunks[1])
		,ObjReader::classifyFaceChunk(chunks[2])
		,ObjReader::classifyFaceChunk(chunks[3])
	};

	if ((ft[0] == ft[1]) && (ft[1] == ft[2])) {
		// Do nothing
	} else {
		throw runtime_error("at line " + S(lineNum) + ": Bad face type definition \"" + line + "\"");
	}

	switch (ft[0]) {
		case FACE_V:
			this->faces->push_back(this->parseFV(lineNum, nextFaceId, line, chunks[1], chunks[2], chunks[3]));
			break;
		case FACE_VT:
			this->faces->push_back(this->parseFVT(lineNum, nextFaceId, line, chunks[1], chunks[2], chunks[3]));
			break;
		case FACE_VN:
			this->faces->push_back(this->parseFVN(lineNum, nextFaceId, line, chunks[1], chunks[2], chunks[3]));
			break;
		case FACE_VTN:
			this->faces->push_back(this->parseFVTN(lineNum, nextFaceId, line, chunks[1], chunks[2], chunks[3]));
			break;
		default:
			throw runtime_error("at line " + S(lineNum) + ": Malformed face chunk <"+ S(ft[0]) + "> \"" + line + "\"");
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
		 Utils::split(chunk1, "/")
		,Utils::split(chunk2, "/")
		,Utils::split(chunk3, "/")
	};

	if (parts[0].size() != 2) {
		throw runtime_error("at line " + S(lineNum) + " in parseFVT: bad chunk(1) \"" + line + "\"");
	} else if (parts[1].size() != 2) {
		throw runtime_error("at line " + S(lineNum) + " in parseFVT: bad chunk(2) \"" + line + "\"");
	} else if (parts[2].size() != 2) {
		throw runtime_error("at line " + S(lineNum) + " in parseFVT: bad chunk(3) \"" + line + "\"");
	}
	 
	v[0] = this->normalizeIndex(stoi(Utils::trim(parts[0][0])));
	v[1] = this->normalizeIndex(stoi(Utils::trim(parts[1][0])));
	v[2] = this->normalizeIndex(stoi(Utils::trim(parts[2][0])));

	t[0] = this->normalizeIndex(stoi(Utils::trim(parts[0][1])));
	t[1] = this->normalizeIndex(stoi(Utils::trim(parts[1][1])));
	t[2] = this->normalizeIndex(stoi(Utils::trim(parts[2][1])));

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
		 Utils::split(chunk1, "//")
		,Utils::split(chunk2, "//")
		,Utils::split(chunk3, "//")
	};

	if (parts[0].size() != 2) {
		throw runtime_error("at line " + S(lineNum) + " in parseFVN: bad chunk(1) \"" + line + "\"");
	} else if (parts[1].size() != 2) {
		throw runtime_error("at line " + S(lineNum) + " in parseFVN: bad chunk(2) \"" + line + "\"");
	} else if (parts[2].size() != 2) {
		throw runtime_error("at line " + S(lineNum) + " in parseFVN: bad chunk(3) \"" + line + "\"");
	}
	 
	v[0] = this->normalizeIndex(stoi(Utils::trim(parts[0][0])));
	v[1] = this->normalizeIndex(stoi(Utils::trim(parts[1][0])));
	v[2] = this->normalizeIndex(stoi(Utils::trim(parts[2][0])));

	n[0] = this->normalizeIndex(stoi(Utils::trim(parts[0][1])));
	n[1] = this->normalizeIndex(stoi(Utils::trim(parts[1][1])));
	n[2] = this->normalizeIndex(stoi(Utils::trim(parts[2][1])));

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
		 Utils::split(chunk1, "/")
		,Utils::split(chunk2, "/")
		,Utils::split(chunk3, "/")
	};

	if (parts[0].size() != 3) {
		throw runtime_error("at line " + S(lineNum) + " in parseFVTN: bad chunk(1) \"" + line + "\"");
	} else if (parts[1].size() != 3) {
		throw runtime_error("at line " + S(lineNum) + " in parseFVTN: bad chunk(2) \"" + line + "\"");
	} else if (parts[2].size() != 3) {
		throw runtime_error("at line " + S(lineNum) + " in parseFVTN: bad chunk(3) \"" + line + "\"");
	}
	 
	v[0] = this->normalizeIndex(stoi(Utils::trim(parts[0][0])));
	v[1] = this->normalizeIndex(stoi(Utils::trim(parts[1][0])));
	v[2] = this->normalizeIndex(stoi(Utils::trim(parts[2][0])));

	t[0] = this->normalizeIndex(stoi(Utils::trim(parts[0][1])));
	t[1] = this->normalizeIndex(stoi(Utils::trim(parts[1][1])));
	t[2] = this->normalizeIndex(stoi(Utils::trim(parts[2][1])));

	n[0] = this->normalizeIndex(stoi(Utils::trim(parts[0][2])));
	n[1] = this->normalizeIndex(stoi(Utils::trim(parts[1][2])));
	n[2] = this->normalizeIndex(stoi(Utils::trim(parts[2][2])));

	return Face(nextFaceId, v, t, n);
}

/*****************************************************************************/
