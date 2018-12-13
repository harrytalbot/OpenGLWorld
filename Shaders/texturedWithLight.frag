#version 330 core

in vec3 fragPos;
in vec3 Normal;
in vec2 UV;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

void main(){

    // Output color = color of the texture at the specified UV
    vec3 texColor = texture( myTextureSampler, UV ).rgb;

	//AMBIENT 
	float ambientStrength = .2f;
	vec3 ambient = ambientStrength * lightColor;

	//DIFFUSE
	float diffuseStrength = 1.0f;

	// get direction of light
	vec3 lightDir = normalize(lightPos - fragPos);
	vec3 norm = normalize(Normal);
	float diff = dot(lightDir, norm);
	vec3 diffuse = diff * lightColor * diffuseStrength;

	//SPECULAR - shiny bit
	float specularStrength = 0.5f;
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(clamp(dot(viewDir, reflectDir), 0.0, 1.0), 2);
	vec3 specular = specularStrength * spec * lightColor;

	vec3 result = (ambient + diffuse + specular) * texColor;

	color = result;
}