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

vector<shared_ptr<aiMesh>> Model::importMeshes(const string& model)
{
	Assimp::Importer importer;
	vector<shared_ptr<aiMesh>> meshes;

	unsigned int flags = aiProcess_FindDegenerates
		               | aiProcess_FindInvalidData
		               | aiProcess_CalcTangentSpace
		               | aiProcess_Triangulate
		               | aiProcess_JoinIdenticalVertices
		               | aiProcess_GenNormals
		               | aiProcess_ImproveCacheLocality
		               | aiProcess_SortByPType;

	importer.ReadFile(model, flags);

	// Detach the scene from the importer, otherwise when the import goes out
	// of scope, the scene and all accompanying meshes will be freed:

	auto scene = importer.GetOrphanedScene();

	if (!scene) {
		LOG(ERROR) << importer.GetErrorString();
	}

	for (unsigned int i=0; i<scene->mNumMeshes; i++) {
		meshes.push_back(make_shared<aiMesh>(*scene->mMeshes[i]));
	}

	return meshes;
}

/******************************************************************************/