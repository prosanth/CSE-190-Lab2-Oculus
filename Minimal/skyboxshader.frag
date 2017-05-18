#version 330 core

in vec2 TexCoords;

out vec4 color;

uniform sampler2D renderedTexture;
uniform float time;

void main()
{
	//color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	color = texture(renderedTexture, TexCoords);
	//color = texture( renderedTexture, TexCoords + 0.005*vec2( sin(time+1000.0*TexCoords.x),cos(time+1000.0*TexCoords.y)) );
}