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

	unsigned int flags = aiProcess_Triangulate 
	                   | aiProcess_FindDegenerates
		               | aiProcess_FindInvalidData
		               | aiProcess_CalcTangentSpace
		               | aiProcess_Triangulate
		               | aiProcess_JoinIdenticalVertices
		               | aiProcess_GenSmoothNormals
		               | aiProcess_GenUVCoords
//		               | aiProcess_TransformUVCoords
		               | aiProcess_FixInfacingNormals
		               | aiProcess_FindInstances
		               | aiProcess_ImproveCacheLocality
		               | aiProcess_SplitLargeMeshes
		               | aiProcess_SortByPType
//		               | aiProcess_RemoveRedundantMaterials
		               | aiProcess_ValidateDataStructure;

	if (!importer.ReadFile(model, flags)) {
		LOG(ERROR) << importer.GetErrorString();
		return meshes;
	}

	// Detach the scene from the importer, otherwise when the import goes out
	// of scope, the scene and all accompanying meshes will be freed:

	auto scene = importer.GetOrphanedScene();

	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		meshes.push_back(make_shared<aiMesh>(*scene->mMeshes[i]));
	}

	return meshes;
}

/******************************************************************************/