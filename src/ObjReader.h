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
#include <tuple>
#include <map>
#include <memory>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "Face.h"
#include "Mesh.h"
#include "Color.h"

/*****************************************************************************/

class MTL
{
	protected:
		Color Ka, Kd, Ks;
		float d, Ns, illum;
		std::string mapKdFilename;

	public:
		MTL();
		MTL(const MTL& other);
		~MTL() { };

	const Color& getKa() const  { return this->Ka; }
	void setKa(const Color& Ka) { this->Ka = Ka; }

	const Color& getKd() const  { return this->Kd; }
	void setKd(const Color& Kd) { this->Kd = Kd; }

	const Color& getKs() const  { return this->Ks; }
	void setKs(const Color& Ks) { this->Ks = Ks; }

	float getD() const { return this->d; }
	void setD(float d) { this->d = d; }

	float getNs() const { return this->Ns; }
	void setNs(float Ns) { this->Ns = Ns; }

	float getIllum() const { return this->illum; }
	void setIllum(float illum) { this->illum = illum; }

	const std::string& getMapKdFilename() const { return this->mapKdFilename; }
	void setMapKdFilename(const std::string& mapKdFilename) { this->mapKdFilename = mapKdFilename; }
};

/*****************************************************************************/

class MTLReader
{
	protected:
		std::string mtlFile;

	public:
		MTLReader(const std::string& mtlFile);
		~MTLReader() { };

		std::map<std::string, std::shared_ptr<MTL>> read();
};

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

		std::unique_ptr<std::vector<glm::vec3>> vertices;
		std::unique_ptr<std::vector<glm::vec3>> normals;
		std::unique_ptr<std::vector<glm::vec3>> textures;
		std::unique_ptr<std::vector<Face>> faces;

		// Maximum vertex component in either X, Y, or Z
		float maxVComp;

		static FaceType classifyFaceChunk(std::string chunk);

		inline int normalizeIndex(int i) const;

		void reset();
		void parseComment(int lineNum, const std::string& line, std::istringstream& ss);
		void parseMtllib(int lineNum, const std::string& line, std::istringstream& ss);
		void parseV(int lineNum, const std::string& line, std::istringstream& ss);
		void parseVT(int lineNum, const std::string& line, std::istringstream& ss);
		void parseVN(int lineNum, const std::string& line, std::istringstream& ss);

		void parseF(int lineNum, int nextFaceId, const std::string& line, std::istringstream& ss);

		Face parseFV(int lineNum, int nextFaceId, const std::string& line, const std::string& chunk1, const std::string& chunk2, const std::string& chunk3) const;
		Face parseFVT(int lineNum, int nextFaceId, const std::string& line, const std::string& chunk1, const std::string& chunk2, const std::string& chunk3) const;
		Face parseFVN(int lineNum, int nextFaceId, const std::string& line, const std::string& chunk1, const std::string& chunk2, const std::string& chunk3) const;
		Face parseFVTN(int lineNum, int nextFaceId, const std::string& line, const std::string& chunk1, const std::string& chunk2, const std::string& chunk3) const;

	public:
		ObjReader(const std::string& objFile);
		~ObjReader();

		std::shared_ptr<Mesh> read();

		const std::vector<glm::vec3>& getVertices() const { return *this->vertices; }
		const std::vector<glm::vec3>& getNormals() const  { return *this->normals; }
		const std::vector<glm::vec3>& getTextures() const { return *this->textures; }
		const std::vector<Face>& getFaces() const         { return *this->faces; }
};

/*****************************************************************************/

#endif
