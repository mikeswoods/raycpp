

/******************************************************************************
 *
 * This file defines a basic type for representing ray-object intersections
 *
 * @file Intersection.h
 * @author Michael Woods
 *
 ******************************************************************************/

#include "Intersection.h"

/******************************************************************************/

using namespace std;

/******************************************************************************/

// Miss constructor by default:
Intersection::Intersection() : 
    t(-1.0f),
    density(-1.0f),
    node(nullptr),
    inside(false),
    correctNormal(true)
{ 
    
}

Intersection::Intersection(float _t, V _normal) : 
    t(_t), 
    density(1.0f),
    node(nullptr),
    normal(_normal),
    inside(false),
    correctNormal(true)
{ 
    
}

Intersection::Intersection(float _t, float _density, V _normal) : 
    t(_t), 
    density(_density),
    node(nullptr),
    normal(_normal),
    inside(false),
    correctNormal(true)
{ 

}

/*
Intersection::Intersection(const Intersection& other) : 
    t(other.t), 
    density(other.density),
    normal(other.normal),
    node(other.node),
    inside(other.inside),
    hitWorld(other.hitWorld),
    hitLocal(other.hitLocal),
    correctNormal(other.correctNormal)
{ 

}
*/

bool Intersection::isMiss() const 
{
    return this->t < 0.0f; 
}

bool Intersection::isHit() const 
{
    return !this->isMiss(); 
}

bool Intersection::isCloser(const Intersection& other) const
{
    if (this->isMiss()) {
        return false;
    } else if (other.isMiss()) {
        return true;
    } else {
        return this->t < other.t;
    }
}

/******************************************************************************/
