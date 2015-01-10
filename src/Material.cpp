/******************************************************************************
 *
 * This class defines the qualities of an object's surface material
 *
 * @file Material.h
 * @author Michael Woods
 *
 ******************************************************************************/

#include "Material.h"

using namespace std;

/******************************************************************************/

const float Material::DEFAULT_AMBIENT_COEFF = 0.15f;

Material::Material() :
	name(""),
	diff(Color::BLACK),
	refl(Color::BLACK),
	expo(0.0f),
	ior(0.0f),
	mirr(false),
	tran(false),
	emit(false),
	ambient(DEFAULT_AMBIENT_COEFF),
	textureMap(nullptr),
	bumpMap(nullptr)
{ 

}

Material::Material(string _name
	              ,Color _diff
				  ,Color _refl
				  ,float _expo
				  ,float _ior
				  ,bool _mirr
				  ,bool _tran
				  ,bool _emit
				  ,float _ambient
				  ,shared_ptr<TextureMap> _textureMap
				  ,shared_ptr<BumpMap> _bumpMap) :
	name(_name),
	diff(_diff),
	refl(_refl),
	expo(_expo),
	ior(_ior),
	mirr(_mirr),
	tran(_tran),
	emit(_emit),
	ambient(_ambient),
	textureMap(_textureMap),
	bumpMap(_bumpMap)
{ }

Material::Material(const Material& other) :
	name(other.name),
	diff(other.diff),
	refl(other.refl),
	expo(other.expo),
	ior(other.ior),
	mirr(other.mirr),
	tran(other.tran),
	emit(other.emit),
	ambient(other.ambient),
	textureMap(other.textureMap),
	bumpMap(other.bumpMap)
{ }

ostream& operator<<(ostream& s, const Material& mat)
{
	s << "Material { name: " << mat.name 
	   << ", diff: " << mat.diff
	   << ", refl: " << mat.refl
	   << ", expo: " << mat.expo
	   << ", ior: "  << mat.ior
	   << ", mirr: " << mat.mirr
	   << ", tran: " << mat.tran
	   << ", emit: " << mat.emit
	   << ", ambient: " << mat.ambient;
	if (mat.hasTextureMap()) {
		s << ", texture: " << *mat.getTextureMap();
	}
	if (mat.hasBumpMap()) {
		s << ", bump: " << *mat.getBumpMap();
	}
	s  << " }";
	return s;
}

/** 
 * "Smart" color function will return the correct color based on assigned 
 * material attributes:
 */
Color Material::getColor() const
{
	if (this->isMirror()) {
		return this->getReflectColor();
	}

	return this->getDiffuseColor();
}

/**
 * Color function that takes a position in R^3 and a geometric object
 * and maps a color based on the given information
 */
Color Material::getColor(const glm::vec3& d, Geometry const * geometry) const
{
	if (this->hasTextureMap()) {

		// Based on the type of geometry presented, choose the most appropriate 
		// mapping type:
		glm::vec2 uv;

		switch (geometry->getGeometryType()) {
			case Geometry::SPHERE:
			case Geometry::CYLINDER:
			case Geometry::MESH:
				{
					uv = SurfaceMap::mapToSphere(d);
				}
				break;
			case Geometry::CUBE:
			default:
				{
					uv = SurfaceMap::mapToCube(d);
				}
				break;
		}

		return this->textureMap->getColor(uv[0], uv[1]);
	}

	return this->getColor();
}

/**
 * Given a position in R^3 and a geometric object, this function returns
 * the normal intensity at the given position
 */
float Material::getIntensity(const glm::vec3& d, Geometry const * geometry) const
{
	if (this->hasBumpMap()) {

		// Based on the type of geometry presented, choose the most appropriate 
		// mapping type:
		glm::vec2 uv;

		switch (geometry->getGeometryType()) {
			case Geometry::SPHERE:
			case Geometry::CYLINDER:
			case Geometry::MESH:
				{
					uv = SurfaceMap::mapToSphere(d);
				}
				break;
			case Geometry::CUBE:
			default:
				{
					uv = SurfaceMap::mapToCube(d);
				}
				break;
		}

		return this->bumpMap->getIntensity(uv[0], uv[1]);
	}

	return 1.0f;
}

/** 
 * Given a position in R^3 and a geometric object, this function returns
 * the surface bump normal the given position
 */
glm::vec3 Material::getNormal(const glm::vec3& d, Geometry const * geometry) const
{
	if (this->hasBumpMap()) {

		glm::vec2 uv = this->bumpMap->mapToSphere(d);

		return this->bumpMap->getNormal(uv[0], uv[1]);
	}

	return glm::vec3(); // zero vector
}

/******************************************************************************/
