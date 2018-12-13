#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec4 position;
layout(location = 3) in vec2 vertexUV;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
out float visibility;

// Values that stay constant for the whole mesh.
uniform mat4 mvpmatrix;
uniform float textureScaleFactor;

void main(){

    // Output position of the vertex, in clip space : MVP * position
    gl_Position =  mvpmatrix * position;
	
	// stuff for scaling uv coords
	float u = vertexUV.x * textureScaleFactor;
	float v = vertexUV.y * textureScaleFactor;
    UV = vec2(u, v);

}