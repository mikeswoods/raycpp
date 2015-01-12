/******************************************************************************
 *
 * This file defines a world state, which as the name implies is a type that
 * contains quanities, flags, and other pieces of data used to define the 
 * current state of the rendering world at a given point in time
 *
 * @file WorldState.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef GL_WORLD_STATE_H
#define GL_WORLD_STATE_H

#include <list>
#include <ostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Config.h"
#include "PointLight.h"

#define TRANSLATE_BY_UNIT 0.5f
//#define ROTATE_BY_UNIT 10.0f

#define ROTATE_BY_UNIT 0.174532925f

#define SCALE_BY_UNIT 0.5f

#define HUE_UNIT 2.0f

#define SATURATION_UNIT 0.05f

#define BRIGHTNESS_UNIT 0.05f

/*******************************************************************************
 * Data structures used to track the current world GL state
 ******************************************************************************/
class GLWorldState
{
	protected:
		Graph graph;
		std::unique_ptr<Graph::iterator> iterator;

		// Various flags and states:
		bool flagRotateScene;
		bool flagCycleLightHue;

		int polyModeIndex;

		// Single point light for preview only:
		PointLight previewLight;

		// Global light hue
		float globalLightHue;

	public:
		GLWorldState(const Graph& graph);
		~GLWorldState();

		static const glm::vec3 DEFAULT_LIGHT_POSITION;

		// Preview lighting
		const PointLight& getPreviewLight()                  { return this->previewLight; }
		void setPreviewLight(const PointLight& previewLight) { this->previewLight = previewLight; }

		// Node traversal operations:
		std::shared_ptr<GraphNode> gotoRoot();
		std::shared_ptr<GraphNode> getCurrentNode();
		std::shared_ptr<GraphNode> getNextNode();

		// Highlight next node:
		void highlightNextNode();
		
		// Switch between polygon drawing modes:
		void switchPolygonMode();

		// Delete the selected node from the scene graph
		bool deleteSelectedNode();

		// Translation operations:
		void translateSelectedXPos();
		void translateSelectedXNeg();
		void translateSelectedYPos();
		void translateSelectedYNeg();
		void translateSelectedZPos();
		void translateSelectedZNeg();

		// Rotation operations:
		void toggleRotateScene();
		bool doRotateScene();
		void rotateSelectedPosX();
		void rotateSelectedNegX();
		void rotateSelectedPosY();
		void rotateSelectedNegY();
		void rotateSelectedPosZ();
		void rotateSelectedNegZ();

		// Scaling operations:
		void scaleIncreaseSelectedX();
		void scaleDecreaseSelectedX();
		void scaleIncreaseSelectedY();
		void scaleDecreaseSelectedY();
		void scaleIncreaseSelectedZ();
		void scaleDecreaseSelectedZ();

		// Light operations
		void toggleLightHueChange();
		bool doLightHueChange();
		void translateLightPosX();
		void translateLightNegX();
		void translateLightPosY();
		void translateLightNegY();
		void translateLightPosZ();
		void translateLightNegZ();

		// Shift the global lighting hue continuously
		void shiftGlobalLightHue();
};

/******************************************************************************/

#endif
