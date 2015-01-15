/*******************************************************************************
 *
 * @file ModelImport.h
 * @author Michael Woods
 *
 ******************************************************************************/

#include <easylogging++.h>
#include "ModelImport.h"

using namespace std;

/******************************************************************************/

void import(const string& model)
{
	Assimp::Importer importer;

	auto scene = importer.ReadFile(model
		                          , aiProcess_FindDegenerates
		                          | aiProcess_FindInvalidData
		                          | aiProcess_CalcTangentSpace
		                          | aiProcess_JoinIdenticalVertices
		                          | aiProcess_SortByPType);

	if (!scene) {
		LOG(ERROR) << importer.GetErrorString();
	}


	cout << "mNumMaterials: " << scene->mNumMaterials << endl;
}