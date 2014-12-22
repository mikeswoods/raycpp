#include "Ray.h"
#include "Utils.h"

/*****************************************************************************/

Ray Ray::normalize() const
{
	return Ray(this->orig, glm::normalize(this->dir));
}

// Given a magnitude t, this function projects along ray with magnitude t
// to produce a new point
P Ray::project(float t) const
{
	return P(this->orig + (glm::normalize(this->dir) * t));
}

void Ray::nudge(float epsilon)
{
	this->orig += (this->dir * epsilon);
}

std::ostream& operator<<(std::ostream& s, const Ray& ray)
{
	s << "Ray<orig= " << ray.orig << ", dir=" << ray.dir << ">";
	return s;
}

/*****************************************************************************/
