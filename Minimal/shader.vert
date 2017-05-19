#version 330 core

layout (location = 0) in vec3 position;

out vec2 UV;

uniform mat4 projection;
uniform mat4 modelview;

void main()
{
    gl_Position = projection * modelview * vec4(position.x, position.y, position.z, 1.0);
    
	UV = (vec2(position.x, position.y) + vec2(1.2f, 1.2f)) / 2.4f;
}