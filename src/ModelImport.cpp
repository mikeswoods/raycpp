/*******************************************************************************
 *
 * @file ModelImport.h
 * @author Michael Woods
 *
 ******************************************************************************/

#include <easylogging++.h>
#include "ModelImport.h"

/******************************************************************************/

using namespace std;

/******************************************************************************/

vector<shared_ptr<aiMesh>> import(const string& model)
{
	Assimp::Importer importer;
	vector<shared_ptr<aiMesh>> meshes;

	auto scene = importer.ReadFile(model
		                          , aiProcess_FindDegenerates
		                          | aiProcess_FindInvalidData
		                          | aiProcess_CalcTangentSpace
		                          | aiProcess_JoinIdenticalVertices
		                          | aiProcess_SortByPType);

	if (!scene) {
		LOG(ERROR) << importer.GetErrorString();
	}

	for (unsigned int i=0; i<scene->mNumMeshes; i++) {
		meshes.push_back(make_shared<aiMesh>(*scene->mMeshes[i]));
	}

	return meshes;
}

/******************************************************************************/