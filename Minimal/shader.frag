#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D renderedTexture;

void main()
{
	//color = vec4(UV.x, UV.y, 0.0f, 1.0f);
	color = texture(renderedTexture, UV);
}