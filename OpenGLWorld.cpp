
#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include <string.h>
#include <stddef.h> // must include for the offsetof macro 
#include <unordered_map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL

#include <Primitives/ShapeGenerator.h>
#include <Primitives/Vertex.h>

#include "OpenGLWorld.h"
#include "Camera.h"
#include "utils.h"
#include "World.h"
#include "PhysicsEngine.h"
#include "Texture.h"

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.position) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.uv) << 1);
		}
	};
}

// 0 = not touring, 1 = touring, 2 = paused, 3 = at end of stage (need to reset tourCounter for next spline interpolation);
int touring = 0;
float tourCounter = 0;

// intial scene
int scene = 2;
float oldTime;

// whether the mouse is in the window;
bool useMouse = false;
bool mouseInWindow;
const int KEYS = GLFW_KEY_MENU;
bool pressed[KEYS];
const int WIDTH = 1200;
const int HEIGHT = 800;

// reference to pi
const float PI = 3.141592653589793;
const float PIo2 = PI / 2.;
const float PIo4 = PI / 4.;
const float PI2 = PI * 2.;
const float lod = PI / 32.;

const float FAR_CLIPPING_PLANE = 1000.0f;
const float NEAR_CLIPPING_PLANE = 0.1f;

glm::vec3 lightPos(0.0f, 80.0f, 0.0f);
const glm::vec3 COLOR_PURPLE(0.8f, 0.3f, 0.8f);

// SHADERS
GLuint lightSourceShader;
GLuint wireFrameShader;
GLuint textureShader;
GLuint textureWaterShader;
GLuint textureWithLightShader;
GLuint textureWithLightShaderTerrain;

// VERTEX BUFFERS AND OBJECTS

GLuint surfaceVertexBuffer;
GLuint surfaceVertexIndexBuffer;
GLuint surfaceArrayObject;

GLuint liquidVertexBuffer;
GLuint liquidVertexIndexBuffer;
GLuint liquidArrayObject;

GLuint sphereVertexBuffer;
GLuint sphereVertexIndexBuffer;
GLuint sphereArrayObject;

GLuint cylinderVertexBuffer;
GLuint cylinderVertexIndexBuffer;
GLuint cylinderArrayObject;

GLuint falconVertexBuffer;
GLuint falconVertexIndexBuffer;
GLuint falconArrayObject;

GLuint arcVertexBuffer;
GLuint arcVertexIndexBuffer;
GLuint arcArrayObject;

// TEXTURES
const char * textureNames[] = { 
	///// TERRAIN TEXTURES /////
	"./Images/heightmap.bmp", "./Images/flatmap.bmp", "./Images/terrain_rock.bmp", "./Images/terrain_grass.bmp", "./Images/terrain_slope.bmp",
	///// PLANET TEXTURES /////
	"./Images/surface.bmp", "./Images/sun.bmp", "./Images/neptune.bmp", "./Images/mercury.bmp",
	"./Images/sand.bmp", "./Images/saturn.bmp", "./Images/space.bmp", "./Images/moon.bmp",
	"./Images/stone.bmp", "./Images./concrete.bmp", "./Images./snow.bmp",
	///// MISC /////
	"./Images/ship.bmp" , "./Images/window.bmp", "./Images/Cloud.bmp", 
	///// OBJECTS /////
	"./Images/falcon.bmp", "./Images/arc170.bmp"
};
enum textureIndexes {
	///// TERRAIN TEXTURES /////
	TERRIAN_HEIGHTMAP, TERRAIN_FLATMAP, TEXTURE_GREYROCK, TEXTURE_GRASS, TEXTURE_SLOPE,
	///// PLANET TEXTURES /////
	TEXTURE_SURFACE, TEXTURE_SUN, TEXTURE_NEPTUNE, TEXTURE_MERCURY, 
	TEXTURE_SAND, TEXTURE_SATURN, TEXTURE_SPACE, TEXTURE_MOON, 
	TEXTURE_STONE, TEXTURE_CONCRETE, TEXTURE_SNOW,
	///// MISC /////
	TEXTURE_SHIP, TEXTURE_WINDOW, TEXTURE_CLOUD, 
	///// OBJECTS /////
	TEXTURE_FALCON, TEXTURE_ARC170
};

GLuint * textures = new GLuint[sizeof(textureNames) / sizeof(textureNames[0])];
Texture * textureList = new Texture[sizeof(textureNames) / sizeof(textureNames[0])];

// Objects
const char * objectNames[] = {  "./Objects/millenium-falcon/millenium-falcon.obj", "./Objects./arc170./obj2./Arc170.obj" };
enum objectIndexes { OBJECT_FALCON, OBJECT_ARC170 };



Camera camera; // handles camera movement
ShapeData terrain, liquid, sphere, cylinder, falcon, arc; // holds vertices, normals, indexes etc
PhysicsEngine physicsEngine; // handles all the bullet implementation




///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Method to Compile a shader
static unsigned int CompileShader(GLuint type, const char* source)
{

	GLuint id = glCreateShader(type);
	const char* src = filetobuf((char*)source);
	//specify source code
	glShaderSource(id, 1, (const GLchar**)&src, nullptr);

	glCompileShader(id);

	// error handling 
	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		// Shader did not compile successfully 
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << "Failed to compile " <<
			(type == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader!" << std::endl;
		std::cout << message << std::endl;

		glDeleteShader(id);
		return 0;

	}

	return(id);
}
// Method to create a shader
static unsigned int CreateShader(const char* vertexShader, const char* fragmentShader)
{
	unsigned int program = glCreateProgram();
	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

	// Attach our shaders to our program
	glAttachShader(program, vs);
	glAttachShader(program, fs);

	// Link our program, and set it as being actively used 
	glLinkProgram(program);

	glValidateProgram(program);

	return program;

}
// Method to build all the shaders
void setupShaders()
{
	glEnable(GL_DEPTH_TEST);

	//setup shaders
	lightSourceShader = CreateShader("./Shaders/lightsource.vert", "./Shaders/lightsource.frag");
	wireFrameShader = CreateShader("./Shaders/wireframe.vert", "./Shaders/wireframe.frag");
	textureShader = CreateShader("./Shaders/textured.vert", "./Shaders/textured.frag");
	textureWaterShader = CreateShader("./Shaders/texturedWater.vert", "./Shaders/texturedWater.frag");
	textureWithLightShader = CreateShader("./Shaders/texturedWithLight.vert", "./Shaders/texturedWithLight.frag");
	textureWithLightShaderTerrain = CreateShader("./Shaders/texturedWithLightTerrain.vert", "./Shaders/texturedWithLightTerrain.frag");

}

// Method to load a texture
bool LoadTextures() {

	// Data read from the header of the BMP file
	unsigned char header[54]; // Each BMP file begins by a 54-bytes header
	unsigned int dataPos;     // Position in the file where the actual data begins

	Texture t;

	print("Loading Textures...");

	for (int i = 0; i < sizeof(textureNames) / sizeof(textureNames[0]); i++)
	{
		// Open the file, read binary
		FILE * file;
		file = fopen(textureNames[i], "rb");

		if (!file) { printf("Image could not be opened\n"); return 0; }
		// Check it's a BMP
		if (fread(header, 1, 54, file) != 54) { // If not 54 bytes read : problem
			printf("Not a correct BMP file\n");
			return false;
		}
		if (header[0] != 'B' || header[1] != 'M') {
			printf("Not a correct BMP file\n");
			return 0;
		}

		// Read ints from the byte array
		dataPos = *(int*)&(header[0x0A]); //10
		t.imageSize = *(int*)&(header[0x22]); //34
		t.width = *(int*)&(header[0x12]); //18
		t.height = *(int*)&(header[0x16]); //22

		// Create a buffer
		t.data = new unsigned char[t.imageSize];

		// Read the actual data from the file into the buffer
		fread(t.data, sizeof(unsigned char), t.imageSize, file);

		//Everything is in memory now, the file can be closed
		fclose(file);

		// Create one OpenGL texture
		glGenTextures(i, &textures[i]);

		// "Bind" the newly created texture : all future texture functions will modify this texture
		glBindTexture(GL_TEXTURE_2D, textures[i]);

		// Give the image to OpenGL
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t.width, t.height, 0, GL_BGR, GL_UNSIGNED_BYTE, t.data);


		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// When MAGnifying the image (no bigger mipmap available), use LINEAR filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// When MINifying the image, use a LINEAR blend of two mipmaps, each filtered LINEARLY too
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		// Generate mipmaps
		glGenerateMipmap(GL_TEXTURE_2D);

		textureList[i] = t;

		print(textureNames[i]);
	}

	print("...done.");
}

// Method to generate a Sphere object
void CreateSphere(ShapeData &sphere, int dtheta, int dphi)
{
	//sphere = ShapeGenerator::makeSphereNew(20);
	sphere = ShapeGenerator::makeSphere(10, 10);

	// Allocate and assign a vertex Array Object to our handle
	glGenVertexArrays(1, &sphereArrayObject);
	// Bind our vertex array as the current used object
	glBindVertexArray(sphereArrayObject);
	// Allocate and assign One Vertex Buffer Object to our handle 
	glGenBuffers(1, &sphereVertexBuffer);
	// Bind our VBO as being the active buffer and storing vertex attributes
	glBindBuffer(GL_ARRAY_BUFFER, sphereVertexBuffer);
	// Copy the vertex data from positions to our buffer 
	glBufferData(GL_ARRAY_BUFFER, sphere.numVertices * sizeof(struct Vertex), sphere.vertices, GL_STATIC_DRAW);

	// Specify that vertex data is going into attribute 0, 3 floats per vertex
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), 0);
	// Enable array 0 as used
	glEnableVertexAttribArray(0);
	// Specify that colour data is going into attribute 1, 3 floats per vertex
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (const GLvoid*)offsetof(struct Vertex, color));
	// Enable array 1 as used
	glEnableVertexAttribArray(1);
	// Specify that normal data is going into attribute 2, 2 floats per vertex
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (const GLvoid*)offsetof(struct Vertex, normal));
	// Enable array 2 as used
	glEnableVertexAttribArray(2);
	// Specify that texture data is going into attribute 3, 2 floats per vertex
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (const GLvoid*)offsetof(struct Vertex, uv));
	// Enable array 3 as used
	glEnableVertexAttribArray(3);

	// Allocate and assign index Vertex Buffer Object to our handle
	glGenBuffers(1, &sphereVertexIndexBuffer);
	// Bind our VBO as being the active buffer and storing vertex attributes (coordinates + colors)
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereVertexIndexBuffer);
	// Copy the vertex data from indecies to our buffer - 3 vertex per triangle = sphereIndiceCount
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (sphere.numIndices) * sizeof(GLuint), sphere.indices, GL_STATIC_DRAW);
	print("Sphere Created.");
}
// Method to generate a plane object
void CreatePlane(ShapeData &surface, ShapeData &liquid, int dimension)
{
	// do surface heightmap first
	Texture t = textureList[TERRIAN_HEIGHTMAP];
	surface = ShapeGenerator::makePlane(dimension, t);

	camera.loadHeightData(dimension, t);

	// Allocate and assign a vertex Array Object to our handle
	glGenVertexArrays(1, &surfaceArrayObject);
	// Bind our vertex array as the current used object
	glBindVertexArray(surfaceArrayObject);
	// Allocate and assign One Vertex Buffer Object to our handle 
	glGenBuffers(1, &surfaceVertexBuffer);
	// Bind our VBO as being the active buffer and storing vertex attributes
	glBindBuffer(GL_ARRAY_BUFFER, surfaceVertexBuffer);
	// Copy the vertex data from positions to our buffer 
	glBufferData(GL_ARRAY_BUFFER, surface.numVertices * sizeof(struct Vertex), surface.vertices, GL_STATIC_DRAW);

	// Specify that vertex data is going into attribute 0, 3 floats per vertex
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), 0);
	// Enable array 0 as used
	glEnableVertexAttribArray(0);
	// Specify that colour data is going into attribute 1, 3 floats per vertex
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (const GLvoid*)offsetof(struct Vertex, color));
	// Enable array 1 as used
	glEnableVertexAttribArray(1);
	// Specify that normal data is going into attribute 2, 3 floats per vertex
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (const GLvoid*)offsetof(struct Vertex, normal));
	// Enable array 2 as used
	glEnableVertexAttribArray(2);
	// Specify that texture data is going into attribute 3, 2 floats per vertex
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (const GLvoid*)offsetof(struct Vertex, uv));
	// Enable array 3 as used
	glEnableVertexAttribArray(3);

	// Allocate and assign index Vertex Buffer Object to our handle
	glGenBuffers(1, &surfaceVertexIndexBuffer);
	// Bind our VBO as being the active buffer and storing vertex attributes (coordinates + colors)
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surfaceVertexIndexBuffer);
	// Copy the vertex data from indecies to our buffer - 3 vertex per triangle = sphereIndiceCount
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (surface.numIndices) * sizeof(GLuint), surface.indices, GL_STATIC_DRAW);
	print("Surface Plane Created.");

	// do surface heightmap first
	t = textureList[TERRAIN_FLATMAP];
	liquid = ShapeGenerator::makePlane(dimension, t);
	// Allocate and assign a vertex Array Object to our handle
	glGenVertexArrays(1, &liquidArrayObject);
	// Bind our vertex array as the current used object
	glBindVertexArray(liquidArrayObject);
	// Allocate and assign One Vertex Buffer Object to our handle 
	glGenBuffers(1, &liquidVertexBuffer);
	// Bind our VBO as being the active buffer and storing vertex attributes
	glBindBuffer(GL_ARRAY_BUFFER, liquidVertexBuffer);
	// Copy the vertex data from positions to our buffer 
	glBufferData(GL_ARRAY_BUFFER, liquid.numVertices * sizeof(struct Vertex), liquid.vertices, GL_STATIC_DRAW);

	// Specify that vertex data is going into attribute 0, 3 floats per vertex
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), 0);
	// Enable array 0 as used
	glEnableVertexAttribArray(0);
	// Specify that colour data is going into attribute 1, 3 floats per vertex
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (const GLvoid*)offsetof(struct Vertex, color));
	// Enable array 1 as used
	glEnableVertexAttribArray(1);
	// Specify that normal data is going into attribute 2, 3 floats per vertex
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (const GLvoid*)offsetof(struct Vertex, normal));
	// Enable array 2 as used
	glEnableVertexAttribArray(2);
	// Specify that texture data is going into attribute 3, 2 floats per vertex
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (const GLvoid*)offsetof(struct Vertex, uv));
	// Enable array 3 as used
	glEnableVertexAttribArray(3);

	// Allocate and assign index Vertex Buffer Object to our handle
	glGenBuffers(1, &liquidVertexIndexBuffer);
	// Bind our VBO as being the active buffer and storing vertex attributes (coordinates + colors)
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, liquidVertexIndexBuffer);
	// Copy the vertex data from indecies to our buffer - 3 vertex per triangle = sphereIndiceCount
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (liquid.numIndices) * sizeof(GLuint), liquid.indices, GL_STATIC_DRAW);
	print("Liquid Plane Created.");

}
// Method to generate a cylinder object
void CreateCylinder(ShapeData &cylinder, float radius, float height)
{
	cylinder = ShapeGenerator::makeCylinder(radius, height);

	// Allocate and assign a vertex Array Object to our handle
	glGenVertexArrays(1, &cylinderArrayObject);
	// Bind our vertex array as the current used object
	glBindVertexArray(cylinderArrayObject);
	// Allocate and assign One Vertex Buffer Object to our handle 
	glGenBuffers(1, &cylinderVertexBuffer);
	// Bind our VBO as being the active buffer and storing vertex attributes
	glBindBuffer(GL_ARRAY_BUFFER, cylinderVertexBuffer);
	// Copy the vertex data from positions to our buffer 
	glBufferData(GL_ARRAY_BUFFER, cylinder.numVertices * sizeof(struct Vertex), cylinder.vertices, GL_STATIC_DRAW);

	// Specify that vertex data is going into attribute 0, 3 floats per vertex
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), 0);
	// Enable array 0 as used
	glEnableVertexAttribArray(0);
	// Specify that colour data is going into attribute 1, 3 floats per vertex
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (const GLvoid*)offsetof(struct Vertex, color));
	// Enable array 1 as used
	glEnableVertexAttribArray(1);
	// Specify that normal data is going into attribute 2, 2 floats per vertex
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (const GLvoid*)offsetof(struct Vertex, normal));
	// Enable array 2 as used
	glEnableVertexAttribArray(2);
	// Specify that texture data is going into attribute 3, 2 floats per vertex
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (const GLvoid*)offsetof(struct Vertex, uv));
	// Enable array 3 as used
	glEnableVertexAttribArray(3);

	// Allocate and assign index Vertex Buffer Object to our handle
	glGenBuffers(1, &cylinderVertexIndexBuffer);
	// Bind our VBO as being the active buffer and storing vertex attributes (coordinates + colors)
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylinderVertexIndexBuffer);
	// Copy the vertex data from indecies to our buffer - 3 vertex per triangle = sphereIndiceCount
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (cylinder.numIndices) * sizeof(GLuint), cylinder.indices, GL_STATIC_DRAW);
	print("Cylinder Created.");
}

// method to load in an object
void LoadObject(ShapeData &object, int objectIndex, GLuint &vertexBuffer, GLuint &vertexIndexBuffer, GLuint &arrayObject) {

		// input file
		std::string inputfile = objectNames[objectIndex];
		// tiny object attributes
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;

		// load the .obj
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, objectNames[objectIndex]);

		if (!err.empty()) { // `err` may contain warning message.
			std::cerr << err << std::endl; 
		}

		if (!ret) {
			exit(1);
		}


		object.numVertices = 0;

		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces(polygon)
			size_t index_offset = 0;

			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {

				int fv = shapes[s].mesh.num_face_vertices[f];

				object.numVertices += shapes[s].mesh.num_face_vertices[f];

			}

		}

		object.vertices = new Vertex[object.numVertices];
		object.numIndices = object.numVertices;
		object.indices = new GLuint[object.numIndices];
		int n = 0;

		std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

		int vertexCounter = 0;
		int indiceCounter = 0;
		
		float t = glfwGetTime();

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex = {};

				vertex.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};

				if (attrib.texcoords.size() > 0) {
					vertex.uv = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
					};
				}

				vertex.color = { 1.0f, 1.0f, 1.0f };

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertexCounter);
					object.vertices[vertexCounter] = vertex;
					vertexCounter++;
				}

				object.indices[indiceCounter] = uniqueVertices[vertex];
				indiceCounter++;
			}
		}

		

		float tPassed = glfwGetTime() - t;


		printf("Parsing time: %d [ms]\n", (int) tPassed);

		printf("# of vertices  = %d\n", (int)(attrib.vertices.size()) / 3);
		printf("# of normals   = %d\n", (int)(attrib.normals.size()) / 3);
		printf("# of texcoords = %d\n", (int)(attrib.texcoords.size()) / 2);
		printf("# of materials = %d\n", (int)materials.size());
		printf("# of shapes    = %d\n", (int)shapes.size());

		// Allocate and assign a vertex Array Object to our handle
		glGenVertexArrays(1, &arrayObject);
		// Bind our vertex array as the current used object
		glBindVertexArray(arrayObject);
		// Allocate and assign One Vertex Buffer Object to our handle 
		glGenBuffers(1, &vertexBuffer);
		// Bind our VBO as being the active buffer and storing vertex attributes
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		// Copy the vertex data from positions to our buffer 
		glBufferData(GL_ARRAY_BUFFER, object.numVertices * sizeof(struct Vertex), object.vertices, GL_STATIC_DRAW);

		// Specify that vertex data is going into attribute 0, 3 floats per vertex
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), 0);
		// Enable array 0 as used
		glEnableVertexAttribArray(0);
		// Specify that colour data is going into attribute 1, 3 floats per vertex
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (const GLvoid*)offsetof(struct Vertex, color));
		// Enable array 1 as used
		glEnableVertexAttribArray(1);
		// Specify that normal data is going into attribute 2, 3 floats per vertex
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (const GLvoid*)offsetof(struct Vertex, normal));
		// Enable array 2 as used
		glEnableVertexAttribArray(2);
		// Specify that texture data is going into attribute 3, 2 floats per vertex
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (const GLvoid*)offsetof(struct Vertex, uv));
		// Enable array 3 as used
		glEnableVertexAttribArray(3);

		// Allocate and assign index Vertex Buffer Object to our handle
		glGenBuffers(1, &vertexIndexBuffer);
		// Bind our VBO as being the active buffer and storing vertex attributes (coordinates + colors)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndexBuffer);
		// Copy the vertex data from indecies to our buffer - 3 vertex per triangle = sphereIndiceCount
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (object.numIndices) * sizeof(GLuint), object.indices, GL_STATIC_DRAW);

		print(objectNames[objectIndex]);

}

// method to update camera movement
void moveCamera() {

	for (int i = 0; i < KEYS; i++) {
		if (!pressed[i]) continue;  // Skip if not pressed
		switch (i) {

		case GLFW_KEY_UP:
			camera.moveForwards();
			break;
		case GLFW_KEY_DOWN:
			camera.moveBackwards();
			break;
		case GLFW_KEY_LEFT:
		{
			if (!useMouse) {
				camera.lookLeft();
			}
			else
			{
				camera.strafeLeft();
			}
			break;
		}
		break;
		case GLFW_KEY_RIGHT:
		{
			if (!useMouse) {
				camera.lookRight();
			}
			else
			{
				camera.strafeRight();
			}
			break;
		}
		break;
		case GLFW_KEY_PAGE_UP:
			camera.lookUp();
			break;
		case GLFW_KEY_PAGE_DOWN:
			camera.lookDown();
			break;
		case GLFW_KEY_BACKSPACE:
			camera.moveUp();
			break;
		case GLFW_KEY_ENTER:
			break;
		default:
			break;
		}
	}
}
// method to check when mouse is in window
static void cursor_callback(GLFWwindow* window, double xPos, double yPos) {

	if (mouseInWindow & useMouse) {
		camera.mouseUpdate(glm::vec2(xPos / 10, yPos / 10));
	}

}
// method to check when the cursor is inside the window
void cursor_enter_callback(GLFWwindow* window, int entered)
{

	mouseInWindow = (entered) ? true : false;

}
// method for setting the scene flag on key press /  for things that aren't continous
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	else if (key == GLFW_KEY_T && action == GLFW_PRESS) {
		//not touring
		if (touring == 0) {
			useMouse = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			oldTime = clock();
			tourCounter = 0;
			//start touring
			touring = 1;
		}
		//already touring
		else if (touring == 1) {
			//pause
			touring = 2;
		}
		//already paused
		else if (touring == 2) {
			//resume
			touring = 1;
		}
	}
	// touring or paused touring
	else if ((key == GLFW_KEY_E && action == GLFW_PRESS) && (touring > 0)) {
		
		//stop touring
		touring = 0;
	}
	//only respond to this if not on a tour
	else if (touring == 0) {
		if ((key == GLFW_KEY_L || key == GLFW_KEY_O) && action == GLFW_PRESS)
			camera.goToNextPos();
		else if (key == GLFW_KEY_P && action == GLFW_PRESS)
			camera.goToScreenShotPos();
		else if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
			print(lightPos);
		else if (key == GLFW_KEY_M && action == GLFW_PRESS)
			camera.returnToUserControl();
		else if (key == GLFW_KEY_V && action == GLFW_PRESS)
		{
			if (!useMouse) {
				// use the mouse to look around
				useMouse = true;
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			else
			{
				useMouse = false;
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
		}
		else if (key == GLFW_KEY_R && action == GLFW_PRESS)
			oldTime = clock();
		else if (action == GLFW_PRESS)
			pressed[key] = true;
		else if (action == GLFW_RELEASE)
			pressed[key] = false;
	}

}

// Method to change to wireframe shader and bind uniforms
void useWireframeRendering(glm::mat4 Model, glm::mat4 View, glm::mat4 Projection) {

	glm::mat4 MVP = Projection * View * Model;

	// use the wireframe shader
	glUseProgram(wireFrameShader);

	glUniform4f(glGetUniformLocation(wireFrameShader, "u_Color"), COLOR_PURPLE.x, COLOR_PURPLE.y, COLOR_PURPLE.z, 1.0f);

	glUniformMatrix4fv(glGetUniformLocation(wireFrameShader, "mvpmatrix"), 1, GL_FALSE, glm::value_ptr(MVP));

}
// Method to change to lighting shader and bind uniforms
void useShadedRendering(glm::mat4 Model, glm::mat4 View, glm::mat4 Projection) {

	glm::mat4 MVP = Projection * View * Model;

	glUseProgram(lightSourceShader);

	glUniform3f(glGetUniformLocation(lightSourceShader, "objectColor"), COLOR_PURPLE.x, COLOR_PURPLE.y, COLOR_PURPLE.z);

	glUniform3f(glGetUniformLocation(lightSourceShader, "lightColor"), 1.0f, 1.0f, 1.0f);

	glUniform3f(glGetUniformLocation(lightSourceShader, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

	glUniform3f(glGetUniformLocation(lightSourceShader, "viewPos"), 45.0f, 1.0f, 0.1f);

	// Bind our model variable to be a uniform called mvpmatrix in our shaderprogram 
	glUniformMatrix4fv(glGetUniformLocation(lightSourceShader, "model"), 1, GL_FALSE, glm::value_ptr(Model));
	// Bind our view variable to be a uniform called mvpmatrix in our shaderprogram 
	glUniformMatrix4fv(glGetUniformLocation(lightSourceShader, "view"), 1, GL_FALSE, glm::value_ptr(View));
	// Bind our projection variable to be a uniform called mvpmatrix in our shaderprogram 
	glUniformMatrix4fv(glGetUniformLocation(lightSourceShader, "projection"), 1, GL_FALSE, glm::value_ptr(Projection));
	// Bind our modelmatrix variable to be a uniform called mvpmatrix in our shaderprogram 
	glUniformMatrix4fv(glGetUniformLocation(lightSourceShader, "mvpmatrix"), 1, GL_FALSE, glm::value_ptr(MVP));

}
// Method to change to use textured shader and bind uniforms
void useTexturedRendering(glm::mat4 Model, glm::mat4 View, glm::mat4 Projection, int textureIndex, float sf) {

	glUseProgram(textureShader);

	glm::mat4 MVP = Projection * View * Model;
	// Bind our modelmatrix variable to be a uniform called mvpmatrix in our shaderprogram 
	glUniformMatrix4fv(glGetUniformLocation(textureShader, "mvpmatrix"), 1, GL_FALSE, glm::value_ptr(MVP));

	// the scale factor indicates how many times to tile the texture. by default all UV mappings go from 0-1
	// so that the texture will fill the entire object
	glUniform1f(glGetUniformLocation(textureShader, "textureScaleFactor"), sf);
	glActiveTexture(GL_TEXTURE0); // Texture unit 0
	// bind the texture to use
	glBindTexture(GL_TEXTURE_2D, textures[textureIndex]);


}
// Method to change to use textured WATER shader and bind uniforms
void useTexturedWaterRendering(glm::mat4 Model, glm::mat4 View, glm::mat4 Projection, int textureIndex, float sf, float time) {

	glUseProgram(textureWaterShader);

	glm::mat4 MVP = Projection * View * Model;
	// Bind our modelmatrix variable to be a uniform called mvpmatrix in our shaderprogram 
	glUniformMatrix4fv(glGetUniformLocation(textureWaterShader, "mvpmatrix"), 1, GL_FALSE, glm::value_ptr(MVP));

	// the scale factor indicates how many times to tile the texture. by default all UV mappings go from 0-1
	// so that the texture will fill the entire object
	glUniform1f(glGetUniformLocation(textureWaterShader, "textureScaleFactor"), sf);

	// movement properties
	glUniform1f(glGetUniformLocation(textureWaterShader, "waveTime"), time);


	glActiveTexture(GL_TEXTURE0); // Texture unit 0
								  // bind the texture to use
	glBindTexture(GL_TEXTURE_2D, textures[textureIndex]);


}
// Method to change to textured shader that uses light and bind uniforms
void useTexturedRenderingWithLight(glm::mat4 Model, glm::mat4 View, glm::mat4 Projection, int textureIndex, float sf) {

	glUseProgram(textureWithLightShader);

	glm::mat4 MVP = Projection * View * Model;

	glUniform3f(glGetUniformLocation(textureWithLightShader, "lightColor"), 1.0f, 1.0f, 1.0f);

	glUniform3f(glGetUniformLocation(textureWithLightShader, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

	glUniform3f(glGetUniformLocation(textureWithLightShader, "viewPos"), 45.0f, 1.0f, 0.1f);

	// Bind our model variable to be a uniform called mvpmatrix in our shaderprogram 
	glUniformMatrix4fv(glGetUniformLocation(textureWithLightShader, "model"), 1, GL_FALSE, glm::value_ptr(Model));
	// Bind our view variable to be a uniform called mvpmatrix in our shaderprogram 
	glUniformMatrix4fv(glGetUniformLocation(textureWithLightShader, "view"), 1, GL_FALSE, glm::value_ptr(View));
	// Bind our projection variable to be a uniform called mvpmatrix in our shaderprogram 
	glUniformMatrix4fv(glGetUniformLocation(textureWithLightShader, "projection"), 1, GL_FALSE, glm::value_ptr(Projection));
	// Bind our modelmatrix variable to be a uniform called mvpmatrix in our shaderprogram 
	glUniformMatrix4fv(glGetUniformLocation(textureWithLightShader, "mvpmatrix"), 1, GL_FALSE, glm::value_ptr(MVP));
	// the scale factor indicates how many times to tile the texture. by default all UV mappings go from 0-1
	// so that the texture will fill the entire object
	glUniform1f(glGetUniformLocation(textureWithLightShader, "textureScaleFactor"), sf);


	// bind the texture to use
	glActiveTexture(GL_TEXTURE0); // Texture unit 0
	// bind the texture to use
	glBindTexture(GL_TEXTURE_2D, textures[textureIndex]);



}
// Method to change to textured shader that uses light and terrain and bind uniforms
void useTexturedRenderingWithLightTerrain(glm::mat4 Model, glm::mat4 View, glm::mat4 Projection, float sf) {

	glUseProgram(textureWithLightShaderTerrain);

	glm::mat4 MVP = Projection * View * Model;

	glUniform3f(glGetUniformLocation(textureWithLightShaderTerrain, "lightColor"), 1.0f, 1.0f, 1.0f);

	glUniform3f(glGetUniformLocation(textureWithLightShaderTerrain, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

	glUniform3f(glGetUniformLocation(textureWithLightShaderTerrain, "viewPos"), 45.0f, 1.0f, 0.1f);

	// Bind our model variable to be a uniform called mvpmatrix in our shaderprogram 
	glUniformMatrix4fv(glGetUniformLocation(textureWithLightShaderTerrain, "model"), 1, GL_FALSE, glm::value_ptr(Model));
	// Bind our view variable to be a uniform called mvpmatrix in our shaderprogram 
	glUniformMatrix4fv(glGetUniformLocation(textureWithLightShaderTerrain, "view"), 1, GL_FALSE, glm::value_ptr(View));
	// Bind our projection variable to be a uniform called mvpmatrix in our shaderprogram 
	glUniformMatrix4fv(glGetUniformLocation(textureWithLightShaderTerrain, "projection"), 1, GL_FALSE, glm::value_ptr(Projection));
	// Bind our modelmatrix variable to be a uniform called mvpmatrix in our shaderprogram 
	glUniformMatrix4fv(glGetUniformLocation(textureWithLightShaderTerrain, "mvpmatrix"), 1, GL_FALSE, glm::value_ptr(MVP));

	glUniform1i(glGetUniformLocation(textureWithLightShaderTerrain, "texture_flat"), 0);
	glUniform1i(glGetUniformLocation(textureWithLightShaderTerrain, "texture_uneven"), 1);
	glUniform1i(glGetUniformLocation(textureWithLightShaderTerrain, "texture_steep"), 2);

	// bind the texture to use
	glActiveTexture(GL_TEXTURE0); // Texture unit 0 - flat
	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_SHIP]);
	glActiveTexture(GL_TEXTURE1); // Texture unit 1 - uneven
	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_MOON]);
	glActiveTexture(GL_TEXTURE2); // Texture unit 2 - steep
	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_GREYROCK]);



	// the scale factor indicates how many times to tile the texture. by default all UV mappings go from 0-1
	// so that the texture will fill the entire object
	glUniform1f(glGetUniformLocation(textureWithLightShaderTerrain, "textureScaleFactor"), sf);

}
//////////////////////// RENDERING HELPER METHODS

void drawATower(glm::mat4 Model, glm::mat4 View, glm::mat4 Projection) {
	// draw cylinder tower
	useTexturedRenderingWithLight(Model, View, Projection, TEXTURE_SHIP, 5.0f);
	glBindVertexArray(cylinderArrayObject);
	glDrawElements(GL_TRIANGLES, cylinder.numIndices, GL_UNSIGNED_INT, nullptr);
	Model = glm::translate(Model, glm::vec3(0.0f, 6.0f, 0.0f));
	Model = glm::scale(Model, glm::vec3(2.0f));
	useTexturedRendering(Model, View, Projection, TEXTURE_WINDOW, 5.0f);
	glBindVertexArray(sphereArrayObject);
	glDrawElements(GL_TRIANGLES, sphere.numIndices, GL_UNSIGNED_INT, nullptr);

}

void drawTowers(glm::mat4 Model, glm::mat4 View, glm::mat4 Projection) {
	// draw cylinder tower
	Model = glm::mat4(1.0);
	Model = glm::translate(Model, glm::vec3(-53.3f, 6.0f, -38.2f));
	drawATower(Model, View, Projection);

	Model = glm::mat4(1.0);
	Model = glm::translate(Model, glm::vec3(-16.0f, 6.0f, -72.5f));
	drawATower(Model, View, Projection);

	Model = glm::mat4(1.0);
	Model = glm::translate(Model, glm::vec3(-22.0f, 6.0f, -67.5f));
	drawATower(Model, View, Projection);

	Model = glm::mat4(1.0);
	Model = glm::translate(Model, glm::vec3(77.0f, 18.0f, -50.0f));
	drawATower(Model, View, Projection);

}

void drawDomes(glm::mat4 Model, glm::mat4 View, glm::mat4 Projection) {
	glBindVertexArray(sphereArrayObject);

	// draw domes
	Model = glm::mat4(1.0);
	Model = glm::translate(Model, glm::vec3(48.0f, 12.0f, 70.5f));
	Model = glm::scale(Model, glm::vec3(5.0f));
	useTexturedRendering(Model, View, Projection, TEXTURE_WINDOW, 5.0f);
	glDrawElements(GL_TRIANGLES, sphere.numIndices, GL_UNSIGNED_INT, nullptr);

	Model = glm::mat4(1.0);
	Model = glm::translate(Model, glm::vec3(59.0f, 12.5f, 62.5f));
	Model = glm::scale(Model, glm::vec3(4.0f));
	useTexturedRendering(Model, View, Projection, TEXTURE_WINDOW, 5.0f);

	glDrawElements(GL_TRIANGLES, sphere.numIndices, GL_UNSIGNED_INT, nullptr);
	Model = glm::mat4(1.0);
	Model = glm::translate(Model, glm::vec3(64.3f, 19.0f, -60.4f));
	Model = glm::scale(Model, glm::vec3(6.0f));
	useTexturedRendering(Model, View, Projection, TEXTURE_WINDOW, 5.0f);

	glDrawElements(GL_TRIANGLES, sphere.numIndices, GL_UNSIGNED_INT, nullptr);
	Model = glm::mat4(1.0);
	Model = glm::translate(Model, glm::vec3(63.5f, 19.0f, -50.0f));
	Model = glm::scale(Model, glm::vec3(4.0f));
	useTexturedRendering(Model, View, Projection, TEXTURE_WINDOW, 5.0f);

	glDrawElements(GL_TRIANGLES, sphere.numIndices, GL_UNSIGNED_INT, nullptr);
	Model = glm::mat4(1.0);
	Model = glm::translate(Model, glm::vec3(76.0f, 11.0f, -23.0f));
	Model = glm::scale(Model, glm::vec3(4.0f));
	useTexturedRendering(Model, View, Projection, TEXTURE_WINDOW, 5.0f);

	glDrawElements(GL_TRIANGLES, sphere.numIndices, GL_UNSIGNED_INT, nullptr);


}

void drawArcShips(glm::mat4 Model, glm::mat4 View, glm::mat4 Projection, float time) {

	glBindVertexArray(arcArrayObject);

	// stuff for rotation
	float t = time / 1000;
	float p = 10000;
	t = fmod(t, p);
	GLfloat angle = t * 360. / p;

	Model = glm::mat4(1.0);
	Model = glm::translate(Model, glm::vec3(-53.3f, 6.0f, -38.2f));
	Model = glm::rotate(Model, angle * -10.0f, glm::vec3(0.f, 1.f, 0.f)); // ROTATION ABOUT MODEL Y AXIS
	Model = glm::translate(Model, glm::vec3(10.f, 0.0f, 30.0f));
	Model = glm::rotate(Model, -PIo2, glm::vec3(0.f, 1.f, 0.f)); // ROTATION ABOUT MODEL Y AXIS
	Model = glm::rotate(Model, PIo4/2.0f, glm::vec3(0.f, 0.f, 1.0f)); // ROTATION ABOUT MODEL  AXIS
	Model = glm::scale(Model, glm::vec3(0.01f));
	useTexturedRendering(Model, View, Projection, TEXTURE_ARC170, 1.0);
	glDrawElements(GL_TRIANGLES, arc.numIndices, GL_UNSIGNED_INT, nullptr);

	Model = glm::mat4(1.0);
	Model = glm::translate(Model, glm::vec3(-20.f, 17.0f, 24.0f));
	Model = glm::rotate(Model, angle * 10.0f, glm::vec3(0.f, 1.f, 0.f)); // ROTATION ABOUT MODEL Y AXIS
	Model = glm::translate(Model, glm::vec3(10.f, 0.0f, 30.0f));
	Model = glm::rotate(Model, PIo2, glm::vec3(0.f, 1.f, 0.f)); // ROTATION ABOUT MODEL Y AXIS
	Model = glm::rotate(Model, -PIo4, glm::vec3(0.f, 0.f, 1.0f)); // ROTATION ABOUT MODEL  AXIS
	Model = glm::scale(Model, glm::vec3(0.01f));
	useTexturedRendering(Model, View, Projection, TEXTURE_ARC170, 1.0);
	glDrawElements(GL_TRIANGLES, arc.numIndices, GL_UNSIGNED_INT, nullptr);

	Model = glm::mat4(1.0);
	Model = glm::translate(Model, glm::vec3(59.3f, 60.0f, -38.2f));
	Model = glm::rotate(Model, angle * -5.0f, glm::vec3(0.f, 1.f, 0.f)); // ROTATION ABOUT MODEL Y AXIS
	Model = glm::translate(Model, glm::vec3(20.f, 0.0f, 30.0f));
	Model = glm::rotate(Model, -PIo4, glm::vec3(0.f, 1.f, 0.f)); // ROTATION ABOUT MODEL Y AXIS
	Model = glm::rotate(Model, PIo4 / 2.0f, glm::vec3(0.f, 0.f, 1.0f)); // ROTATION ABOUT MODEL  AXIS
	Model = glm::scale(Model, glm::vec3(0.01f));
	useTexturedRendering(Model, View, Projection, TEXTURE_ARC170, 1.0);
	glDrawElements(GL_TRIANGLES, arc.numIndices, GL_UNSIGNED_INT, nullptr);

	Model = glm::mat4(1.0);
	Model = glm::translate(Model, glm::vec3(41.f, 40.0f, 2.0f));
	Model = glm::rotate(Model, angle * 4.0f, glm::vec3(0.f, 1.f, 0.f)); // ROTATION ABOUT MODEL Y AXIS
	Model = glm::translate(Model, glm::vec3(40.f, 0.0f, 30.0f));
	Model = glm::rotate(Model, 3 * PIo4, glm::vec3(0.f, 1.f, 0.f)); // ROTATION ABOUT MODEL Y AXIS
	Model = glm::rotate(Model, -PIo4, glm::vec3(0.f, 0.f, 1.0f)); // ROTATION ABOUT MODEL  AXIS
	Model = glm::scale(Model, glm::vec3(0.01f));
	useTexturedRendering(Model, View, Projection, TEXTURE_ARC170, 1.0);
	glDrawElements(GL_TRIANGLES, arc.numIndices, GL_UNSIGNED_INT, nullptr);


	// static ones

	Model = glm::mat4(1.0);
	Model = glm::translate(Model, glm::vec3(-26.9f, 23.6f, 29.5f));
	Model = glm::scale(Model, glm::vec3(0.01f));
	useTexturedRendering(Model, View, Projection, TEXTURE_ARC170, 1.0);
	glDrawElements(GL_TRIANGLES, arc.numIndices, GL_UNSIGNED_INT, nullptr);

	Model = glm::mat4(1.0);
	Model = glm::translate(Model, glm::vec3(-73.9f, 30.0f, 35.0f));
	Model = glm::scale(Model, glm::vec3(0.01f));
	Model = glm::rotate(Model, 90.f, glm::vec3(0.0f, 1.0f, 0.0f));
	useTexturedRendering(Model, View, Projection, TEXTURE_ARC170, 1.0);
	glDrawElements(GL_TRIANGLES, arc.numIndices, GL_UNSIGNED_INT, nullptr);

	Model = glm::mat4(1.0);
	Model = glm::translate(Model, glm::vec3(-68.5f, 30.0f, 62.30f));
	Model = glm::scale(Model, glm::vec3(0.01f));
	Model = glm::rotate(Model, PI, glm::vec3(0.0f, 1.0f, 0.0f));
	useTexturedRendering(Model, View, Projection, TEXTURE_ARC170, 1.0);
	glDrawElements(GL_TRIANGLES, arc.numIndices, GL_UNSIGNED_INT, nullptr);

}

void drawRotatingPlanets(float time) {

	glm::mat4 Model;
	glm::mat4 Projection = glm::perspective(45.0f, (float)WIDTH / HEIGHT, NEAR_CLIPPING_PLANE, FAR_CLIPPING_PLANE);
	glm::mat4 View = camera.getWorldToViewMatrix();

	// stuff for rotation
	float t = time/1000;
	float p = 10000;
	t = fmod(t, p);
	GLfloat angle = t * 360. / p;

	glm::vec3 planetSize = glm::vec3(4.0f);

	Model = glm::mat4(1.0);
	//Model = glm::translate(Model, glm::vec3(-40.0f, 0.0f, 0.0f)); // TRANSLATION AROUND WORLD (OVERALL ROTATION CENTREPOINT)
	Model = glm::rotate(Model, angle * -1.0f, glm::vec3(0.f, 1.f, 0.f)); // ROTATION ABOUT MODEL Y AXIS
	Model = glm::translate(Model, glm::vec3(-55.0f, 80.0f, 55.0f)); // TRANSLATION ABOUT MODEL AXIS
	Model = glm::rotate(Model, angle * -10.0f, glm::vec3(0.f, 1.f, 0.f)); // ROTATION ABOUT PLANET Y AXIS
	Model = glm::scale(Model, planetSize); // PLANET SIZE
	useTexturedRenderingWithLight(Model, View, Projection, TEXTURE_NEPTUNE, 1.0f);
	glBindVertexArray(sphereArrayObject);
	glDrawElements(GL_TRIANGLES, sphere.numIndices, GL_UNSIGNED_INT, nullptr);

	Model = glm::mat4(1.0);
	//Model = glm::translate(Model, glm::vec3(-40.0f, 0.0f, 0.0f)); // TRANSLATION AROUND WORLD (OVERALL ROTATION CENTREPOINT)
	Model = glm::rotate(Model, angle * -2.0f, glm::vec3(0.f, 1.f, 0.f)); // ROTATION ABOUT MODEL Y AXIS
	Model = glm::translate(Model, glm::vec3(-65.0f, 70.0f, 65.0f)); // TRANSLATION ABOUT MODEL AXIS
	Model = glm::rotate(Model, angle * -15.0f, glm::vec3(0.f, 1.f, 0.f)); // ROTATION ABOUT PLANET Y AXIS
	Model = glm::scale(Model, planetSize); // PLANET SIZE
	useTexturedRenderingWithLight(Model, View, Projection, TEXTURE_SAND, 1.0f);
	glBindVertexArray(sphereArrayObject);
	glDrawElements(GL_TRIANGLES, sphere.numIndices, GL_UNSIGNED_INT, nullptr);

	Model = glm::mat4(1.0);
	//Model = glm::translate(Model, glm::vec3(50.0f, 0.0f, -20.0f)); // TRANSLATION AROUND WORLD (OVERALL ROTATION CENTREPOINT)
	Model = glm::rotate(Model, angle * -3.0f, glm::vec3(0.f, 1.f, 0.f)); // ROTATION ABOUT MODEL Y AXIS
	Model = glm::translate(Model, glm::vec3(75.0f, 60.0f, -65.0f)); // TRANSLATION ABOUT MODEL AXIS
	Model = glm::rotate(Model, angle * -15.0f, glm::vec3(0.f, 1.f, 0.f)); // ROTATION ABOUT PLANET Y AXIS
	Model = glm::scale(Model, planetSize); // PLANET SIZE
	useTexturedRenderingWithLight(Model, View, Projection, TEXTURE_SATURN, 1.0f);
	glBindVertexArray(sphereArrayObject);
	glDrawElements(GL_TRIANGLES, sphere.numIndices, GL_UNSIGNED_INT, nullptr);

	Model = glm::mat4(1.0);
	//Model = glm::translate(Model, glm::vec3(-40.0f, 0.0f, 0.0f)); // TRANSLATION AROUND WORLD (OVERALL ROTATION CENTREPOINT)
	Model = glm::rotate(Model, angle * -4.0f, glm::vec3(0.f, 1.f, 0.f)); // ROTATION ABOUT MODEL Y AXIS
	Model = glm::translate(Model, glm::vec3(-70.0f, 50.0f, -70.0f)); // TRANSLATION ABOUT MODEL AXIS
	Model = glm::rotate(Model, angle * -10.0f, glm::vec3(0.f, 1.f, 0.f)); // ROTATION ABOUT PLANET Y AXIS
	Model = glm::scale(Model, planetSize); // PLANET SIZE
	useTexturedRenderingWithLight(Model, View, Projection, TEXTURE_CLOUD, 1.0f);
	glBindVertexArray(sphereArrayObject);
	glDrawElements(GL_TRIANGLES, sphere.numIndices, GL_UNSIGNED_INT, nullptr);

	Model = glm::mat4(1.0);
	//Model = glm::translate(Model, glm::vec3(-40.0f, 0.0f, 0.0f)); // TRANSLATION AROUND WORLD (OVERALL ROTATION CENTREPOINT)
	Model = glm::rotate(Model, angle * -3.0f, glm::vec3(0.f, 1.f, 0.f)); // ROTATION ABOUT MODEL Y AXIS
	Model = glm::translate(Model, glm::vec3(85.0f, 45.0f, 80.0f)); // TRANSLATION ABOUT MODEL AXIS
	Model = glm::rotate(Model, angle * -10.0f, glm::vec3(0.f, 1.f, 0.f)); // ROTATION ABOUT PLANET Y AXIS
	Model = glm::scale(Model, planetSize); // PLANET SIZE
	useTexturedRenderingWithLight(Model, View, Projection, TEXTURE_SATURN, 1.0f);
	glBindVertexArray(sphereArrayObject);
	glDrawElements(GL_TRIANGLES, sphere.numIndices, GL_UNSIGNED_INT, nullptr);

}

// Rendering method
void render(float time) {

	glClearColor(0.0, 0.0, 0.0, 1.0);// Make our background black 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glm::mat4 Model;
	glm::mat4 View = camera.getWorldToViewMatrix();
	glm::mat4 Projection = glm::perspective(45.0f, (float)WIDTH / HEIGHT, NEAR_CLIPPING_PLANE, FAR_CLIPPING_PLANE);

	// RENDER STATIC STUFF

	//draw falcon
	Model = glm::mat4(1.0);
	Model = glm::translate(Model, glm::vec3(135.0f, 16.5f, -35.0f));
	Model = glm::scale(Model, glm::vec3(0.04f));
	Model = glm::rotate(Model, 40.0f, glm::vec3(0, 1, 0));
	useTexturedRendering(Model, View, Projection, TEXTURE_FALCON, 1.0);
	glBindVertexArray(falconArrayObject);
	glDrawElements(GL_TRIANGLES, falcon.numIndices, GL_UNSIGNED_INT, nullptr);

	//draw ground
	Model = glm::mat4(1.0);
	int sf = 50.0f;
	useTexturedRenderingWithLightTerrain(Model, View, Projection, sf);
	glBindVertexArray(surfaceArrayObject);
	glDrawElements(GL_TRIANGLES, terrain.numIndices, GL_UNSIGNED_INT, nullptr);

	//draw lava
	Model = glm::mat4(1.0);
	useTexturedWaterRendering(Model, View, Projection, TEXTURE_SUN, sf, time / 1000.0f);
	glBindVertexArray(liquidArrayObject);
	glDrawElements(GL_TRIANGLES, liquid.numIndices, GL_UNSIGNED_INT, nullptr);

	//draw space
	Model = glm::mat4(1.0);
	Model = glm::translate(Model, glm::vec3(0.0, -3.0f, 0.0f));
	Model = glm::scale(Model, glm::vec3(150.0f));
	Model = glm::rotate(Model, PIo2, glm::vec3(0, 0, 1.0f));
	useTexturedRendering(Model, View, Projection, TEXTURE_SPACE, 3.0f);
	glBindVertexArray(sphereArrayObject);
	glDrawElements(GL_TRIANGLES, sphere.numIndices, GL_UNSIGNED_INT, nullptr);

	// draw Sun
	Model = glm::mat4(1.0);
	Model = glm::translate(Model, lightPos);
	Model = glm::scale(Model, glm::vec3(6.0f));
	Model = glm::rotate(Model, 3.0f * PIo2, glm::vec3(0, 0, 1.0f));
	useTexturedRendering(Model, View, Projection, TEXTURE_SUN, 1.0f);
	glBindVertexArray(sphereArrayObject);
	glDrawElements(GL_TRIANGLES, sphere.numIndices, GL_UNSIGNED_INT, nullptr);

	//draw orbiting planets
	drawRotatingPlanets(time);
	// draw arc ships
	drawArcShips(Model, View, Projection, time);
	// draw towers on the island
	drawTowers(Model, View, Projection);
	// draw domes
	drawDomes(Model, View, Projection);

	// RENDER PHYSICS STUFF
	glBindVertexArray(sphereArrayObject);
	for (int i = 0; i < physicsEngine.getSphereMovingBits().size(); i++) {    // loop over shapes

		btTransform bulletTrans = physicsEngine.bullet_step_sphere(i, time);
		glm::mat4 transform;
		bulletTrans.getOpenGLMatrix(glm::value_ptr(transform));
		Model = transform;
		useTexturedRendering(Model, View, Projection, TEXTURE_SUN, 1.0f);
		glDrawElements(GL_TRIANGLES, sphere.numIndices, GL_UNSIGNED_INT, nullptr);
	}

	if (touring > 0) {
		// if the tour is not paused
		if (touring != 2)
			tourCounter += 0.001;

		int ret = camera.goToNextTourPos(tourCounter);
		
		// if 3 returned, got to end of spline so reset counter
		if ( ret == 3) {
			touring = 1; // and keep touring
			tourCounter = 0;
		}
		else if (ret == 4) {
			// tour is actually finished 
			touring = 0;
		}	
	}


}

// Main loop
int main(void)
{

	GLFWwindow* window;
	glfwSwapInterval(1);

	// Initialize the library 
	if (!glfwInit())
		return -1;

	// Create a windowed mode window and its OpenGL context 
	window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL World - Harry Talbot", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	// Make the window's context current 
	glfwMakeContextCurrent(window);
	// key callbacks for changing scene
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_callback);
	glfwSetCursorEnterCallback(window, cursor_enter_callback);

	// Initialise Glew 
	glewInit();
	glewExperimental = GL_TRUE;
	// Setup the shaders
	setupShaders();
	print("///////////////////////////////////////////");
	// load images
	LoadTextures();
	print("///////////////////////////////////////////");
	// Create some shapes
	CreateSphere(sphere, 10, 10);
	CreatePlane(terrain, liquid, 300);
	CreateCylinder(cylinder, 1.0f, 5.0f);
	print("///////////////////////////////////////////");
	// load .obj files
	LoadObject(falcon, OBJECT_FALCON, falconVertexBuffer, falconVertexIndexBuffer, falconArrayObject);
	LoadObject(arc, OBJECT_ARC170, arcVertexBuffer, arcVertexIndexBuffer, arcArrayObject);
	print("///////////////////////////////////////////");

	oldTime = 0;
	float currTime;
	float elapsedTime;

	// initialise the bullet physics
	physicsEngine.initialise();
	print("///////////////////////////////////////////");
	// Loop until the user closes the window 
	float renderDelta = 0;
	while (!glfwWindowShouldClose(window))
	{
		// Render here 
		glClear(GL_COLOR_BUFFER_BIT);

		// update movement
		moveCamera();

		currTime = clock();
		elapsedTime = currTime - oldTime;

		// do rendering!
		render(elapsedTime);

		// Swap front and back buffers 
		glfwSwapBuffers(window);

		// Poll for and process events 
		glfwPollEvents();
	}

	// shut things down
	physicsEngine.close();
	glfwTerminate();
	exit(EXIT_SUCCESS);// Exit program
}


