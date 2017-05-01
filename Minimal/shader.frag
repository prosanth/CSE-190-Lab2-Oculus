#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 Color;

out vec4 color;

void main()
{    
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(vec3(10.0f, 10.0f, 0.0f) - FragPos);  
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * vec3(1.0f, 1.0f, 1.0f);
	vec3 result = diffuse * vec3(Color.x, Color.y, Color.z);
	color = vec4(result, 1.0f);
}
