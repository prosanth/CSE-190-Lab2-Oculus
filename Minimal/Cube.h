#ifndef _CUBE_H_
#define _CUBE_H_

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
// Use of degrees is deprecated. Use radians instead.
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Cube
{
public:
	Cube(bool skybox, bool left, bool loadTest);
	~Cube();

	glm::mat4 toWorld;
	int width;
	int height;
	bool skybox;

	void draw(GLuint shaderProgram, glm::mat4 modelview, glm::mat4 projection);
	void setUpSkybox(bool skybox);
	void loadBearRightEye();
	void loadBearLeftEye();
	void loadTest();
	void changeSize(bool increase, bool decrease, bool default);

	// These variables are needed for the shader program
	GLuint VBO, VAO, EBO;
	GLuint uProjection, uModelview;
	GLuint textureID;
};

// Define the coordinates and indices needed to draw the cube. Note that it is not necessary
// to use a 2-dimensional array, since the layout in memory is the same as a 1-dimensional array.
// This just looks nicer since it's easy to tell what coordinates/indices belong where.
const GLfloat vertices[8][3] = {
	// "Front" vertices
	{-2.0, -2.0,  2.0}, {2.0, -2.0,  2.0}, {2.0,  2.0,  2.0}, {-2.0,  2.0,  2.0},
	// "Back" vertices
	{-2.0, -2.0, -2.0}, {2.0, -2.0, -2.0}, {2.0,  2.0, -2.0}, {-2.0,  2.0, -2.0}
};

// Note that GL_QUADS is deprecated in modern OpenGL (and removed from OSX systems).
// This is why we need to draw each face as 2 triangles instead of 1 quadrilateral
const GLuint indices[6][6] = {
	// Front face
	{ 0, 3, 2, 2, 1, 0 },
	// Top face
	{ 1, 2, 6, 6, 5, 1 },
	// Back face
	{ 7, 4, 5, 5, 6, 7 },
	// Bottom face
	{ 4, 7, 3, 3, 0, 4 },
	// Left face
	{ 4, 0, 1, 1, 5, 4 },
	// Right face
	{ 3, 7, 6, 6, 2, 3 }
};

#endif

