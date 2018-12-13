#pragma once
#include <glm\glm.hpp>

struct Texture
{
	unsigned int width, height;
	unsigned int imageSize;   // = width*height*3							  
	unsigned char * data; // Actual RGB data
};
