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
#include <memory>
#include <glm/glm.hpp>
#include <map>
#include <list>
#include <string>
#include "Graph.h"
#include "Light.h"
#include "Material.h"
#include "EnvironmentMap.h"

/******************************************************************************/

// Shorthand type declarations
typedef std::list<std::shared_ptr<Light>> LIGHTS;
typedef std::map<std::string, std::shared_ptr<Material>> MATERIALS;

/******************************************************************************/

class SceneContext
{
    protected:
        glm::vec2 resolution;
        glm::vec3 eyePosition;
        glm::vec3 viewDir;
        glm::vec3 lookAtPosition;
        glm::vec3 upDir;
        float yFOV;
        Graph graph;
        std::shared_ptr<EnvironmentMap> envMap;
        std::shared_ptr<std::map<std::string,std::shared_ptr<Material>>> materials;
        std::shared_ptr<std::list<std::shared_ptr<Light>>> lights;
    public:

        SceneContext(const glm::vec2& resolution
                    ,const glm::vec3& eyePosition
                    ,const glm::vec3& viewDir
                    ,const glm::vec3& upDir
                    ,float yFOV
                    ,const Graph& graph
                    ,std::shared_ptr<EnvironmentMap> envMap
                    ,std::shared_ptr<std::map<std::string, std::shared_ptr<Material>>> materials
                    ,std::shared_ptr<std::list<std::shared_ptr<Light>>> lights);
        ~SceneContext();

        const glm::vec2& getResolution() const     { return this->resolution; }
        const glm::vec3& getEyePosition() const    { return this->eyePosition; }
        const glm::vec3& getLookAtPosition() const { return this->lookAtPosition; }
        const glm::vec3& getViewDir() const        { return this->viewDir; }
        const glm::vec3& getUpDir() const          { return this->upDir; }
        float getFOVAngle() const                  { return static_cast<float>(this->yFOV); }
        float getAspectRatio() const               { return static_cast<float>(this->resolution.x) / static_cast<float>(this->resolution.y); }
        float getZNear() const                     { return 0.1f; }
        float getZFar() const                      { return 100.0f; }

        const Graph& getSceneGraph() const { return graph; }
        std::shared_ptr<EnvironmentMap> getEnvironmentMap() const { return this->envMap; }
        void setEnvironmentMap(std::shared_ptr<EnvironmentMap> envMap) { this->envMap = envMap ; }
        std::shared_ptr<MATERIALS> getMaterials() const { return this->materials; }
        std::shared_ptr<LIGHTS> getLights() const { return this->lights; }
};

/******************************************************************************/

#endif
