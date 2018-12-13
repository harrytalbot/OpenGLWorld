#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 normals;
layout(location = 3) in vec2 vertexUV;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
out vec3 Normal;
out vec3 fragPos;

// Values that stay constant for the whole mesh.
uniform mat4 model, view, proj, mvpmatrix;
uniform float textureScaleFactor;

void main(){

    // Output position of the vertex, in clip space : MVP * position
    gl_Position =  mvpmatrix * position;

	fragPos = vec3(model * vec4(position));
	Normal = vec3(model * vec4(normals, 0));

	float u = vertexUV.x * textureScaleFactor;
	float v = vertexUV.y * textureScaleFactor;
    UV = vec2(u, v);
}