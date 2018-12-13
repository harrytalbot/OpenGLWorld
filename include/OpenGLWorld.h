#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include <iostream>
#include <string.h>
#include <stddef.h> // must include for the offsetof macro 
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Primitives\ShapeGenerator.h>
#include <Primitives\Vertex.h>

#include "OpenGLWorld.h"
#include "Camera.h"
#include "utils.h"
#include "World.h"
#include "PhysicsEngine.h"


class OpenGLWorld
{
	// intial scene
	int scene;

	// whether the mouse is in the window;
	bool mouseInWindow;
	const int KEYS;
	const int WIDTH;
	const int HEIGHT;

						// reference to pi
	const float pi = glm::pi<float>();

	glm::vec3 lightPos;
	// purple
	glm::vec3 color;

	// SHADERS
	GLuint lightSourceShader;
	GLuint wireFrameShader;
	GLuint textureShader;

	// VERTEX BUFFERS AND OBJECTS
	GLuint sphereVertexBuffer;
	GLuint sphereVertexIndexBuffer;
	GLuint sphereArrayObject;

	GLuint planeVertexBuffer;
	GLuint planeVertexIndexBuffer;
	GLuint planeArrayObject;

	GLuint lightVertexBuffer;
	GLuint lightVertexIndexBuffer;
	GLuint lightArrayObject;

	GLuint textureID;
	GLuint texture;
	const char * image = "./Images/space.bmp";

	Camera camera;
	ShapeData sphere, plane;
	PhysicsEngine physicsEngine;

	// struct for a co-ord
	struct XYZ {
		float x, y, z;
	};
public:

};
