#include "Line.h"
#include "Window.h"
#include <Windows.h>

Line::Line(glm::vec3 head, glm::vec3 tail)
{
	points.push_back(head);
	points.push_back(tail);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

Line::~Line()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void Line::update(glm::vec3 head, glm::vec3 tail) {
	points.clear();

	points.push_back(head);
	points.push_back(tail);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
}

void Line::draw(GLuint shaderProgram, glm::mat4 modelview, glm::mat4 projection, bool righteye, glm::mat4 toWorld)
{
	glUseProgram(shaderProgram);
	glm::mat4 newmv = modelview * glm::mat4(1.0f);

	uProjection = glGetUniformLocation(shaderProgram, "projection");
	uModelview = glGetUniformLocation(shaderProgram, "modelview");

	GLuint eye = glGetUniformLocation(shaderProgram, "eye");

	if (righteye == false) {
		glUniform1f(eye, 0.0f);
	}
	else {
		glUniform1f(eye, 1.0f);
	}

	glUniformMatrix4fv(uProjection, 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(uModelview, 1, GL_FALSE, &newmv[0][0]);

	glLineWidth(10.0f);

	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, points.size());
	glBindVertexArray(0);
}