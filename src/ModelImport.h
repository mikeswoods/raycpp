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

std::vector<std::shared_ptr<aiMesh>> import(const std::string& model);

/******************************************************************************/

#endif
