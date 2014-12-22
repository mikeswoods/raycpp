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

#include <iostream>
#include <map>
#include <list>
#include <string>
#include "Graph.h"
#include "ObjReader.h"
#include "Light.h"
#include "Material.h"
#include "EnvironmentMap.h"

/*****************************************************************************/

class Configuration
{
	private:
		bool nodeExists(const std::string& name) const;
		void registerNode(GraphNode* node);
		GraphNode* getNode(const std::string& name) const;
		void linkNodes(const std::string& parentName, GraphNode* child);
		void linkNodes(GraphNode* parent, GraphNode* child);

		bool materialExists(const std::string& name) const;
		Material* getMaterial(const std::string& name) const;
		void registerMaterial(Material* material);
		void registerEnvironmentMap(EnvironmentMap* envMap);
		void registerLight(Light* light);

    protected:
		std::string filename;
		GraphNode* root;
		EnvironmentMap* environmentMap;
		Graph graph;
		std::map<std::string,GraphNode*>* nodeMap;
		std::map<std::string,Material*>* materialMap;
        std::list<Light*> lights;

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
		Configuration(const Configuration& other);
		~Configuration();

		const std::string& getFileName() { return this->filename; }

        void read();

		bool hasEnvironmentMap() const { return this->environmentMap != NULL; }

		EnvironmentMap const * getEnvironmentMap() const { return this->environmentMap; }

        const Graph& getSceneGraph() const { return this->graph; };

        const std::list<Light*>& getLights() const { return this->lights; };

        friend std::ostream& operator<<(std::ostream& os, const Configuration& c);
};

/*****************************************************************************/

#endif
