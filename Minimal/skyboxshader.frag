#version 330 core

in vec3 TexCoords;

out vec4 color;

uniform samplerCube skybox;
uniform sampler2D renderedTexture;
uniform float time;

void main()
{
	color = texture( renderedTexture, TexCoords + 0.005*vec2( sin(time+1024.0*TexCoords.x),cos(time+768.0*TexCoords.y)) ).xyz ;
}