#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec4 position;
layout(location = 3) in vec2 vertexUV;

// Output data ; will be interpolated for each fragment.
out vec2 UV;

// Values that stay constant for the whole mesh.
uniform mat4 mvpmatrix;
uniform float textureScaleFactor;
uniform float waveTime;

const float waveLength = 1.1f;
const float waveAmplitude = 1.1f;
const float PI = 3.141592653589793;
const float waveSpeed = 0.1;

vec3 createOffset(float x, float z){
	float radiansX = ((x / waveLength) + waveTime) * 2.0f * PI * waveSpeed;
	float radiansZ = ((z / waveLength) + waveTime) * 2.0f * PI * waveSpeed;
	float r = waveAmplitude * 0.5 * (sin(radiansZ) + cos(radiansX));
	return vec3(x, r, z);
}

vec3 distort(vec3 vertex) {
	/*
	float distortionX = createOffset(vertex.x, vertex.z);
	float distortionY = createOffset(vertex.x, vertex.z);
	float distortionZ = createOffset(vertex.x, vertex.z);
	*/

	return vertex + createOffset(vertex.x, vertex.z);

}

void main(){

    // Output position of the vertex, in clip space : MVP * position]
	vec3 newPosition = distort(vec3(position.x, position.y, position.z));
    gl_Position =  mvpmatrix * vec4(newPosition, 1.0f);

    // UV of the vertex. No special space for this one.
	//UV = vertexUV;

	float u = vertexUV.x * textureScaleFactor;
	float v = vertexUV.y * textureScaleFactor;

    UV = vec2(u, v);
}