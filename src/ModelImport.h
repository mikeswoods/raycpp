/*******************************************************************************
 *
 * @file ModelImport.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef MODEL_IMPORTER_H
#define MODEL_IMPORTER_H

#include <memory>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

/******************************************************************************/

namespace Model
{
	std::vector<std::shared_ptr<aiMesh>> importMeshes(const std::string& model);
};

/******************************************************************************/

#endif
