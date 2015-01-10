/******************************************************************************
 *
 * This file defines basic configuration file reader according to the text
 * format commonly used in assignment for CIS560
 *
 * @file Config.h
 * @author Michael Woods
 *
 *****************************************************************************/

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <memory>
#include <iostream>
#include <map>
#include <list>
#include <string>
#include "Graph.h"
#include "GraphBuilder.h"
#include "ObjReader.h"
#include "Light.h"
#include "Material.h"
#include "EnvironmentMap.h"
#include "SceneContext.h"

/******************************************************************************/

class Configuration
{
	private:
		bool materialExists(const std::string& name) const;
		std::shared_ptr<Material> getMaterial(const std::string& name) const;
		void registerMaterial(std::shared_ptr<Material> material);
		void registerLight(std::shared_ptr<Light> light);

    protected:
        GraphBuilder graphBuilder;

		std::string filename;
		std::shared_ptr<EnvironmentMap> envMap;
		Graph graph;
		std::shared_ptr<MATERIALS> materials;
        std::shared_ptr<LIGHTS> lights;

		void parseCameraSection(std::istream& is, const std::string& beginToken);
		void parseEnvironmentSection(std::istream& is, const std::string& beginToken);
		void parsePointLightSection(std::istream& is, const std::string& beginToken);
		void parseMaterialSection(std::istream& is, const std::string& beginToken);
		void parseNodeDefinition(std::istream& is, const std::string& beginToken);

    public:
        // RESO: two integers specifying the width and height (in pixels) of 
        // the ray casting that should be performed.
        int RESO[2];

        // EYEP: the x, y, z position of the eye/camera in world-space.
        float EYEP[3];

		// VDIR: the viewing direction of the eye/camera towards the box, 
        // in world-space.
        float VDIR[3];
        
		// UVEC: the up-vector in world-space.
        float UVEC[3];
        
		// FOVY: the half-angle field of view in the Y-direction in degrees.
        float FOVY;

        Configuration(const std::string& filename);
		virtual ~Configuration();

		const std::string& getFileName() { return this->filename; }

        std::unique_ptr<SceneContext> read();

        friend std::ostream& operator<<(std::ostream& os, const Configuration& c);
};

/*****************************************************************************/

#endif
