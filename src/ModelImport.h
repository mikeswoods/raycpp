/*******************************************************************************
 *
 * @file ModelImport.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef MODEL_IMPORTER_H
#define MODEL_IMPORTER_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

/******************************************************************************/

void import(const std::string& model);

/******************************************************************************/

#endif
