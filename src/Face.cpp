/******************************************************************************
 *
 * *.obj format geometric face data type
 *
 * @file Face.h
 * @author Michael Woods
 *
 ******************************************************************************/

#include "Face.h"

using namespace std;

/******************************************************************************/

ostream& operator<<(ostream& s, const Face& face)
{
    s << "Face {v=[" << face.v[0] << "," << face.v[1] << "," << face.v[2] << "}";
    return s;
}
