#include "stdafx.h"
#include "ShapeGenerator.h"
#include "utils.h"
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <Primitives\Vertex.h>
#define NUM_ARRAY_ELEMENTS(a) sizeof(a) / sizeof(*a)

const float pi = glm::pi<float>();
bool randFirstTime = false;

glm::vec3 randomColor()
{
	glm::vec3 ret;
	ret.x = rand() / (float)RAND_MAX;
	ret.y = rand() / (float)RAND_MAX;
	ret.z = rand() / (float)RAND_MAX;
	return ret;
}

float randomNumber() {

	return rand() / (float)RAND_MAX;
}

glm::vec3 getNormalOfFace(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
	
	glm::vec3 U = p2 - p1;
	glm::vec3 V = p3 - p1;

	glm::vec3 N;

	N.x = U.y * V.z - U.z * V.y;
	N.y = U.z * V.x - U.x * V.z;
	N.z = U.x * V.y - U.y * V.x;
	return normalize(N);
}


// Method to normalize a vector
glm::vec3 normalize(glm::vec3 v) {
	GLfloat d = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	v.x /= d; v.y /= d; v.z /= d;
	return v;
}

glm::vec2 ShapeGenerator::getSphereUV(glm::vec3 point, glm::vec3 normal) {

	//u = ((-point.z / abs(point.x) + 1) / 2); v = ((-point.y / abs(point.x) + 1) / 2);
	float u = 0.5f + (atan2(normal.z, normal.x) / (2*pi));
	float v = 0.5f + (asin(normal.y) / pi);

	return glm::vec2(u, v);
}

ShapeData ShapeGenerator::makeSphere(int dtheta, int dphi)
{
	ShapeData ret;

	Vertex vertices[2556];
	int indiceCount = 0;
	int vertexCount = 0;

	GLuint sphereIndices[30000] = {};
	int theta, phi;
	float DegToRad = pi / 180;
	float u, v;

	glm::vec3 p[4];
	glm::vec3 normal;

	for (theta = -90;theta <= 90 - dtheta;theta += dtheta) {
		for (phi = 0;phi <= 360 - dphi;phi += dphi) {
			int n = 0;

			// add each new point to the list of points, and keep a reference to which position it is at (pIndex)
			// that we add to the indeces array after all the points have been added
			//	vertices[vertexCount] = { {vX, vY, vZ}, color, {nX, nY, nZ}};	

			// Point 0
			p[n].x = cos(DegToRad*theta) * cos(DegToRad*phi);
			p[n].y = cos(DegToRad*theta) * sin(DegToRad*phi);
			p[n].z = sin(DegToRad*theta);
			normal = normalize(p[n]);

			vertices[vertexCount] = { p[n], randomColor(), normal, getSphereUV(p[n], normal) };

			int vec1Index = vertexCount;
			vertexCount++; n++;

			// Point 1
			p[n].x = cos(DegToRad*(theta + dtheta)) * cos(DegToRad*phi);
			p[n].y = cos(DegToRad*(theta + dtheta)) * sin(DegToRad*phi);
			p[n].z = sin(DegToRad*(theta + dtheta));

			normal = normalize(p[n]);
			vertices[vertexCount] = { p[n], randomColor(), normal, getSphereUV(p[n], normal) };
			int vec2Index = vertexCount;
			vertexCount++; n++;

			//Point 2
			p[n].x = cos(DegToRad*(theta + dtheta)) * cos(DegToRad*(phi + dphi));
			p[n].y = cos(DegToRad*(theta + dtheta)) * sin(DegToRad*(phi + dphi));
			p[n].z = sin(DegToRad*(theta + dtheta));

			normal = normalize(p[n]);

			vertices[vertexCount] = { p[n], randomColor(), normal, getSphereUV(p[n], normal) };

			int vec3Index = vertexCount;
			vertexCount++; n++;

			// Add indices for first triangle
			sphereIndices[indiceCount] = vec1Index; 
			indiceCount++;
			sphereIndices[indiceCount] = vec2Index; 
			indiceCount++;
			sphereIndices[indiceCount] = vec3Index;
			indiceCount++;

			// if square face there will be two triangles to draw the section
			if (theta > -90 && theta < 90) {
				p[n].x = cos(DegToRad*theta) * cos(DegToRad*(phi + dphi));
				p[n].y = cos(DegToRad*theta) * sin(DegToRad*(phi + dphi));
				p[n].z = sin(DegToRad*theta);

				normal = normalize(p[n]);

				vertices[vertexCount] = { p[n], randomColor(), normal, getSphereUV(p[n], normal) };
				int vec4Index = vertexCount; vertexCount++;

				// Add indices for right triangle:  vector 2 to 23 to 12

				sphereIndices[indiceCount] = vec1Index;
				indiceCount++;
				sphereIndices[indiceCount] = vec3Index;
				indiceCount++;
				sphereIndices[indiceCount] = vec4Index;
				indiceCount++;
			}
		}

	}

	ret.numVertices = vertexCount;
	ret.vertices = new Vertex[vertexCount];
	memcpy(ret.vertices, vertices, sizeof(vertices));
	(sizeof(vertices));
	ret.numIndices = indiceCount;
	ret.indices = new GLuint[indiceCount];
	memcpy(ret.indices, sphereIndices, sizeof(sphereIndices[0])*indiceCount);

	return ret;

}

ShapeData ShapeGenerator::makePlaneVerts(uint dimensions, Texture terrain)
{

	ShapeData ret;
	ret.numVertices = dimensions * dimensions;
	int half = dimensions / 2;
	ret.vertices = new Vertex[ret.numVertices];
	for (int row = 0; row < dimensions; row++)
	{
		for (int col = 0; col < dimensions; col++)
		{
			Vertex& thisVert = ret.vertices[row * dimensions + col];

		
			thisVert.position.x = col - half;
			thisVert.position.z = row - half;
			unsigned char color = (unsigned char) terrain.data[3 * (col * terrain.width + row)];
			float h = terrain.height * ((color / 255.0f) - 0.5f);
			//scaling
			h = h / 5.0f;
			
			thisVert.position.y = h;


			float u = (thisVert.position.x + half) * (float)(1.0f / dimensions);
			float v = (thisVert.position.z + half) * (float)(1.0f / dimensions);

			thisVert.uv = glm::vec2(u, v);
			thisVert.color = randomColor();

		}
	}
	// do normals
	for (int row = 0; row < dimensions-1; row++)
	{
		for (int col = 0; col < dimensions-1; col++)
		{
			glm::vec3 p1 = ret.vertices[dimensions * row + col].position;
			glm::vec3 p2 = ret.vertices[dimensions * row + col + dimensions].position;
			glm::vec3 p3 = ret.vertices[dimensions * row + col + dimensions + 1].position;

			Vertex& thisVert = ret.vertices[row * dimensions + col];
			thisVert.normal = getNormalOfFace(p1, p2, p3);

			p1 = ret.vertices[dimensions * row + col].position;
			p2 = ret.vertices[dimensions * row + col + dimensions + 1].position;
			p3 = ret.vertices[dimensions * row + col + 1].position;

			thisVert = ret.vertices[row * dimensions + col];
			thisVert.normal = getNormalOfFace(p1, p2, p3);

		}
	}

	return ret;
}

ShapeData ShapeGenerator::makePlaneIndices(uint dimensions)
{
	ShapeData ret;
	ret.numIndices = (dimensions - 1) * (dimensions - 1) * 2 * 3; // 2 triangles per square, 3 indices per triangle
	ret.indices = new GLuint[ret.numIndices];
	int runner = 0;
	glm::vec3 normal;
	for (int row = 0; row < dimensions - 1; row++)
	{
		for (int col = 0; col < dimensions - 1; col++)
		{
			ret.indices[runner++] = dimensions * row + col;
			ret.indices[runner++] = dimensions * row + col + dimensions;
			ret.indices[runner++] = dimensions * row + col + dimensions + 1;

			ret.indices[runner++] = dimensions * row + col;
			ret.indices[runner++] = dimensions * row + col + dimensions + 1;
			ret.indices[runner++] = dimensions * row + col + 1;
		}
	}
	assert(runner = ret.numIndices);
	return ret;
}

ShapeData ShapeGenerator::makePlane(uint dimensions, Texture terrain)
{
	ShapeData ret = makePlaneVerts(dimensions, terrain);
	ShapeData ret2 = makePlaneIndices(dimensions);
	ret.numIndices = ret2.numIndices;
	ret.indices = ret2.indices;
	return ret;
}

ShapeData ShapeGenerator::makeSphereNew(uint tesselation)
{
	ShapeData ret; // = makePlaneVerts(tesselation);
	ShapeData ret2 = makePlaneIndices(tesselation);
	ret.indices = ret2.indices;
	ret.numIndices = ret2.numIndices;

	uint dimensions = tesselation;
	const float RADIUS = 1.0f;
	const double CIRCLE = pi * 2;
	const double SLICE_ANGLE = CIRCLE / (dimensions - 1);
	for (size_t col = 0; col < dimensions; col++)
	{
		double phi = -SLICE_ANGLE * col;
		for (size_t row = 0; row < dimensions; row++)
		{
			double theta = -(SLICE_ANGLE / 2.0) * row;
			size_t vertIndex = col * dimensions + row;
			Vertex& v = ret.vertices[vertIndex];
			v.position.x = RADIUS * cos(phi) * sin(theta);
			v.position.y = RADIUS * sin(phi) * sin(theta);
			v.position.z = RADIUS * cos(theta);
			v.normal = glm::normalize(v.position);
		}
	}
	return ret;
}

ShapeData ShapeGenerator::makeCylinder(float radius, float height) {

	ShapeData ret;

	// 36 points round bottom + declare end and start twice + top point = 38
	const int numOfVertices = 74;
	Vertex vertices[numOfVertices];
	glm::vec3 point, secondPoint, normal;
	int offset = 37;

	float DegToRad = glm::pi<float>() / 180;
	int counter = 0;

	for (int theta = 0; theta <= 360; theta += 10) {

		point.x = radius * sin(DegToRad*theta);
		point.y = 0.0f;
		point.z = radius * cos(DegToRad*theta);
		normal = normalize(point);
		vertices[counter] = { point,randomColor(), normalize(point), getSphereUV(secondPoint, normal) };

		secondPoint = point;
		secondPoint.y = height;
		normal = normalize(secondPoint);
		vertices[counter + offset] = { secondPoint,  randomColor(), normalize(point), getSphereUV(secondPoint, normal) };
		counter++;
	}
	GLuint indices[numOfVertices * 3];
	int j = 0;
	int indiceCount = 0;
	for (int indice = 0; indice < numOfVertices; indice += 2) {
		indices[indiceCount] = j + offset;
		indiceCount++;
		indices[indiceCount] = indice / 2;
		indiceCount++;
		indices[indiceCount] = (indice / 2) + 1;
		indiceCount++;
		indices[indiceCount] = (indice / 2) + 1;
		indiceCount++;
		indices[indiceCount] = j + offset + 1;
		indiceCount++;
		indices[indiceCount] = j + offset;
		indiceCount++;
		j++;
	}


	ret.numVertices = numOfVertices;
	ret.vertices = new Vertex[ret.numVertices];
	memcpy(ret.vertices, vertices, sizeof(vertices));
	ret.numIndices = numOfVertices * 3;
	ret.indices = new GLuint[ret.numIndices];
	memcpy(ret.indices, indices, sizeof(indices[0])*ret.numIndices);

	return ret;
}

