#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec4 color;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 Color;

// Uniform variables can be updated by fetching their location and passing values to that location
uniform mat4 projection;
uniform mat4 modelview;

void main()
{
    gl_Position = projection * modelview * vec4(position.x, position.y, position.z, 1.0);
	FragPos = vec3(modelview * vec4(position, 1.0f));
    Normal = normal;
    TexCoords = texCoords;
	Color = color;
}

