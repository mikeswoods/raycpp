/*******************************************************************************
 *
 * This class defines all contextual data needed to render a scene
 *
 * @file SceneContext.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef SCENE_CONTEXT_H
#define SCENE_CONTEXT_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <map>
#include <list>
#include <string>
#include "Graph.h"
#include "Light.h"
#include "Material.h"
#include "EnvironmentMap.h"

/******************************************************************************/

class SceneContext
{
    protected:
        glm::vec2 resolution;
        glm::vec3 eyePosition;
        glm::vec3 viewDir;
        glm::vec3 upDir;
        float yFOV;
        EnvironmentMap const * envMap;
        Graph graph;
        std::map<std::string,Material*> materials;
        std::list<Light*> lights;
    public:

        SceneContext(const glm::vec2& resolution
                    ,const glm::vec3& eyePosition
                    ,const glm::vec3& viewDir
                    ,const glm::vec3& upDir
                    ,float yFOV
                    ,EnvironmentMap const * envMap
                    ,const Graph& graph
                    ,const std::map<std::string,Material*>& materials
                    ,const std::list<Light*>& lights);
        SceneContext(const SceneContext& other);
        ~SceneContext();

        const glm::vec2& getResolution() const  { return this->resolution; }
        const glm::vec3& getEyePosition() const { return this->eyePosition; }
        const glm::vec3& getViewDir() const     { return this->viewDir; }
        const glm::vec3& getUpDir() const       { return this->upDir; }
        float getFOV() const                    { return this->yFOV; }
        EnvironmentMap const * getEnvironmentMap() const { return this->envMap; }
        const Graph& getSceneGraph() const      { return graph; }
        const std::map<std::string,Material*>& getMaterials() const { return this->materials; }
        const std::list<Light*>& getLights() const { return this->lights; }
};

/******************************************************************************/

#endif
