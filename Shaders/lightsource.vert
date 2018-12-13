#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 normals;

out vec3 Normal;
out vec3 fragPos;

uniform mat4 model, view, proj, mvpmatrix;

void main() {

	gl_Position = mvpmatrix * position;

	fragPos = vec3(model * vec4(position));
	Normal = vec3(model * vec4(normals, 0));

}
