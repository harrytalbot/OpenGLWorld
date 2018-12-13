#pragma once
#include <Primitives\ShapeData.h>
#include "Texture.h"

typedef unsigned int uint;

class ShapeGenerator
{
	static ShapeData makePlaneVerts(uint dimensions, Texture terrain);
	static ShapeData makePlaneIndices(uint dimensions);
	static glm::vec2 getSphereUV(glm::vec3 point, glm::vec3 normal);
public:
	static ShapeData makeSphere(int theta, int dphi);
	static ShapeData makePlane(uint dimensions, Texture terrain);
	static ShapeData makeSphereNew(uint tesselation);
	static ShapeData makeCylinder(float radius, float height);
};
#pragma once
