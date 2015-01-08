/*******************************************************************************
 *
 * This class defines all contextual data needed to render a scene
 *
 * @file SceneContext.h
 * @author Michael Woods
 *
 ******************************************************************************/

#include <utility>
#include "SceneContext.h"
#include "Utils.h"

using namespace std;

SceneContext::SceneContext(const glm::vec2& _resolution
                          ,const glm::vec3& _eyePosition
                          ,const glm::vec3& _viewDir
                          ,const glm::vec3& _upDir
                          ,float _yFOV
                          ,unique_ptr<EnvironmentMap> _envMap
                          ,const Graph& _graph
                          ,const map<string,Material*>& _materials
                          ,const list<Light*>& _lights) :
    resolution(_resolution),
    eyePosition(_eyePosition),
    viewDir(_viewDir),
    yFOV(_yFOV),
    envMap(move(_envMap)),
    graph(_graph),
    materials(_materials),
    lights(_lights)
{

  this->upDir          = Utils::fixUpVector(_viewDir, _upDir);
  this->lookAtPosition = _eyePosition + _viewDir;
}

SceneContext::~SceneContext()
{

}
