#include "Screen.h"
#include "texture.h"
#include <Windows.h>
#include <iostream>
#include "Window.h"

Screen::Screen(int face)
{	
	this->face = face;

	corners.push_back(glm::vec3(-1.2f, -1.2f, -1.2f)); //pa
	corners.push_back(glm::vec3(1.2f, -1.2f, -1.2f));  //pb
	corners.push_back(glm::vec3(1.2f, 1.2f, -1.2f));
	corners.push_back(glm::vec3(-1.2f, 1.2f, -1.2f));  //pc
	
	//toWorld = glm::mat4(1.0f);

	if (face == 0) { //front
		toWorld = glm::mat4(1.0f) * glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	else if (face == 1) { //left
		toWorld = glm::mat4(1.0f) * glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//toWorld = toWorld * glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else {  //bottom
		toWorld = glm::mat4(1.0f) * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		toWorld = toWorld * glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	}
	

	lineShader = LoadShaders("./lineshader.vert", "./lineshader.frag");

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(front_indices), front_indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

Screen::~Screen()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void Screen::draw(GLuint shaderProgram, glm::mat4 modelview, glm::mat4 projection, GLuint texture, bool rightEye, bool debugOn, glm::vec3 eyePosition)
{
	glUseProgram(shaderProgram);
	//glm::vec3 eyePosition = glm::vec3(modelview[3]);

	lines.clear();
	glm::vec3 pa = glm::vec3(-1.2f, 1.2f, -1.2f);
	glm::vec3 pb = glm::vec3(1.2f, 1.2f, -1.2f);
	glm::vec3 pc = glm::vec3(-1.2f, -1.2f, -1.2f);
	glm::vec3 pd = glm::vec3(1.2f, -1.2f, -1.2f);

	lines.push_back(new Line(eyePosition, pa));
	lines.push_back(new Line(eyePosition, pb));
	lines.push_back(new Line(eyePosition, pc));
	lines.push_back(new Line(eyePosition, pd));
	lines.push_back(new Line(pa, pb));
	lines.push_back(new Line(pa, pc));
	lines.push_back(new Line(pb, pd));
	lines.push_back(new Line(pc, pd));

	char buff[100];
	sprintf_s(buff, "position: %f %f %f \n", eyePosition.x, eyePosition.y, eyePosition.z);
	//OutputDebugStringA(buff);

	glm::mat4 newmv = modelview * toWorld;

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	uProjection = glGetUniformLocation(shaderProgram, "projection");
	uModelview = glGetUniformLocation(shaderProgram, "modelview");

	glUniformMatrix4fv(uProjection, 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(uModelview, 1, GL_FALSE, &newmv[0][0]);

	/*glBindVertexArray(VAO);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(shaderProgram, "skybox"), 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);*/

	glBindVertexArray(VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	// Set our "renderedTexture" sampler to user Texture Unit 0
	glUniform1i(glGetUniformLocation(shaderProgram, "renderedTexture"), 0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	if (debugOn) {
		for (Line * l : lines) {
			l->draw(lineShader, modelview, projection, rightEye, toWorld);
		}
	}
}