#ifndef _LINE_H_
#define _LINE_H_

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
#include <vector>

class Line
{
public:
	Line(glm::vec3 head, glm::vec3 tail);
	~Line();

	void update(glm::vec3 head, glm::vec3 tail);
	void draw(GLuint shaderProgram, glm::mat4 modelview, glm::mat4 projection, bool righteye, glm::mat4 toWorld);

	// These variables are needed for the shader program
	GLuint VBO, VAO, EBO;
	GLuint uProjection, uModelview;

	std::vector<glm::vec3> points;
};
#endif
