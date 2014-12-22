#ifndef SQUARE_H
#define SQUARE_H

#include "Geometry.h"

class Square : public Geometry
{
public:
    Square();
    virtual ~Square();

    virtual void buildGeometry();
};

#endif
