#version 330 core

in vec3 fragPos;
in vec3 Normal;
in vec2 UV;

// Ouput data
out vec3 color;

uniform sampler2D texture_flat;
uniform sampler2D texture_uneven;
uniform sampler2D texture_steep;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;



void main(){

    // Output color = color of the texture at the specified UV
    // vec3 texColor = texture( myTextureSampler, UV).rgb;

    // sample all the textures
    vec3 textureFlatColor = texture( texture_flat, UV).rgb;
    vec3 textureUnevenColor = texture( texture_uneven, UV).rgb;
    vec3 textureSteepColor = texture( texture_steep, UV).rgb;

	float slope = 1.0f - Normal.y;
	float blendAmount;
	vec3 texColor;


	// Determine which texture to use based on height.
    if(slope < 0.2)
    {
		// basically flat area
        blendAmount = slope / 0.2f;
        texColor = mix(textureFlatColor, textureUnevenColor, blendAmount);
    }
	
    if((slope < 0.7) && (slope >= 0.2f))
    {

		//in between flat and steep
        blendAmount = (slope - 0.2f) * (1.0f / (0.7f - 0.2f));
        texColor = mix(textureUnevenColor, textureSteepColor, blendAmount);

    }

    if(slope >= 0.7) 
    {
		// really steep area
        texColor = textureSteepColor;
    }


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