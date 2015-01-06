/*******************************************************************************
 *
 * This class defines all contextual data needed to render a scene
 *
 * @file SceneContext.h
 * @author Michael Woods
 *
 ******************************************************************************/

#include "SceneContext.h"

SceneContext::SceneContext(const glm::vec2& _resolution
                          ,const glm::vec3& _eyePosition
                          ,const glm::vec3& _viewDir
                          ,const glm::vec3& _upDir
                          ,float _yFOV
                          ,EnvironmentMap const * _envMap
                          ,const Graph& _graph
                          ,const std::map<std::string,Material*>& _materials
                          ,const std::list<Light*>& _lights) :
    resolution(_resolution),
    eyePosition(_eyePosition),
    viewDir(_viewDir),
    upDir(_upDir),
    yFOV(_yFOV),
    envMap(_envMap),
    graph(_graph),
    materials(_materials),
    lights(_lights)
{

}

SceneContext::SceneContext(const SceneContext& other) :
    resolution(other.resolution),
    eyePosition(other.eyePosition),
    viewDir(other.viewDir),
    upDir(other.upDir),
    yFOV(other.yFOV),
    envMap(other.envMap),
    graph(other.graph),
    materials(other.materials),
    lights(other.lights)
{

}

SceneContext::~SceneContext()
{

}
