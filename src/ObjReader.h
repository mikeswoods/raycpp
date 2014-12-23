/******************************************************************************
 *
 * This file defines a class for reading a *.obj file and constructing a
 * geometric mesh object from the definition
 *
 * @file ObjReader.h
 * @author Michael Woods
 *
 *****************************************************************************/

#ifndef OBJ_READER_H
#define OBJ_READER_H

#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
 
#include "Face.h"
#include "Mesh.h"

/*****************************************************************************/

class ObjReader
{
	public:
		enum FaceType {
			 FACE_BAD = -99 // Malformed face
			,FACE_V   = 1   // (1) Face with vertices only:  "f v1 v1 v2 ... vn"
			,FACE_VT  = 2   // (2) Face with texture coords: "f v1/t1 v2/t2 .... vn/tn"
			,FACE_VN  = 3   // (3) Face with vertex normals: "f v1//n1 v2//n2 .... vn//nn"
			,FACE_VTN = 4   // (4) Face with txt and norms:  "f v1/t1/n1 v2/t2/n2 .... vn/tn/nn"
		};

	protected:
		std::string objFile;
		std::vector<glm::vec3>* vertices;
		std::vector<glm::vec3>* normals;
		std::vector<glm::vec2>* uv;
		std::vector<Face>* faces;

		// Maximum vertex component in either X, Y, or Z
		float maxVComp;

		static FaceType classifyFaceChunk(std::string chunk);

		inline int normalizeIndex(int i) const;

		void reset();
		void parseComment(int lineNum, const std::string& line, std::istringstream& ss);
		void parseV(int lineNum, const std::string& line, std::istringstream& ss);
		void parseVT(int lineNum, const std::string& line, std::istringstream& ss);
		void parseVN(int lineNum, const std::string& line, std::istringstream& ss);

		void parseF(int lineNum, const std::string& line, std::istringstream& ss);

		Face parseFV(int lineNum, const std::string& line, const std::string& chunk1, const std::string& chunk2, const std::string& chunk3) const;
		Face parseFVT(int lineNum, const std::string& line, const std::string& chunk1, const std::string& chunk2, const std::string& chunk3) const;
		Face parseFVN(int lineNum, const std::string& line, const std::string& chunk1, const std::string& chunk2, const std::string& chunk3) const;
		Face parseFVTN(int lineNum, const std::string& line, const std::string& chunk1, const std::string& chunk2, const std::string& chunk3) const;

	public:
		ObjReader(const std::string& objFile);
		~ObjReader();

		Mesh* parse();

		const std::vector<glm::vec3>& getVertices() const { return *this->vertices; }
		const std::vector<glm::vec3>& getNormals() const  { return *this->normals; }
		const std::vector<glm::vec2>& getUVs() const      { return *this->uv; }
		const std::vector<Face>& getFaces() const         { return *this->faces; }
};

/*****************************************************************************/

#endif
