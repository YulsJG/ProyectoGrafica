#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

void Inputs(GLFWwindow* window);

const GLint WIDTH = 1200, HEIGHT = 800;

// Brazo muñeca y Cámara 
float movX = 0.0f, movY = 0.0f, movZ = -10.0f, rot = 0.0f;
float hombro = 0.0f, codo = 0.0f, muneca = 0.0f;

// Articulaciones Dedos (Falange1: Base, Falange2: Punta)
float d1_f1 = 0.0f, d1_f2 = 0.0f; // Dedo uno
float d2_f1 = 0.0f, d2_f2 = 0.0f; // Dedo dos
float d3_f1 = 0.0f, d3_f2 = 0.0f; // Dedo tres

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Modelo jerarquico", nullptr, nullptr);
	int screenWidth, screenHeight;
	glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
	if (nullptr == window) return EXIT_FAILURE;
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();

	glViewport(0, 0, screenWidth, screenHeight);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Shader ourShader("Shader/core.vs", "Shader/core.frag");

	float vertices[] = {
		-0.5f, -0.5f, 0.5f,  0.5f, -0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
		 0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f,
		 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, 0.5f,  0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f,
		 0.5f, 0.5f, -0.5f,  0.5f, 0.5f, 0.5f,   0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,
		-0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f,
		 0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,  0.5f, 0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
		 0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, -0.5f
	};

	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 100.0f);

	while (!glfwWindowShouldClose(window)) {
		Inputs(window);
		glfwPollEvents();
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ourShader.Use();
		glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(movX, movY, movZ));
		view = glm::rotate(view, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));

		GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
		GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");
		GLint uniCol = glGetUniformLocation(ourShader.Program, "color");

		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		glBindVertexArray(VAO);
		glm::mat4 model = glm::mat4(1.0f);

		// --- BICEP ---
		model = glm::rotate(model, glm::radians(hombro), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 bicepPivot = glm::translate(model, glm::vec3(1.5f, 0.0f, 0.0f));
		model = glm::scale(bicepPivot, glm::vec3(3.0f, 1.0f, 1.0f));
		glUniform3f(uniCol, 0.0f, 0.8f, 0.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// --- ANTEBRAZO ---
		model = glm::translate(bicepPivot, glm::vec3(1.5f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(codo), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 antebrazoPivot = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(antebrazoPivot, glm::vec3(2.0f, 1.0f, 1.0f));
		glUniform3f(uniCol, 0.8f, 0.0f, 0.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// --- MUÑECA ---
		model = glm::translate(antebrazoPivot, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(muneca), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 baseMano = glm::translate(model, glm::vec3(0.25f, 0.0f, 0.0f)); 
		model = glm::scale(baseMano, glm::vec3(0.5f, 1.0f, 1.0f)); 
		glUniform3f(uniCol, 0.0f, 0.0f, 0.8f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// DEDO 1
		// Falange 
		model = glm::translate(baseMano, glm::vec3(0.25f, 0.4f, 0.0f)); // Arriba
		model = glm::rotate(model, glm::radians(d1_f1), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 f1_d1 = glm::translate(model, glm::vec3(0.3f, 0.0f, 0.0f)); // Centro del cubo 0.6
		model = glm::scale(f1_d1, glm::vec3(0.6f, 0.15f, 0.15f));
		glUniform3f(uniCol, 1.0f, 1.0f, 0.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// Punta 
		model = glm::translate(f1_d1, glm::vec3(0.3f, 0.0f, 0.0f)); // Extremo F1
		model = glm::rotate(model, glm::radians(d1_f2), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(0.2f, 0.0f, 0.0f)); // Centro del cubo 0.4
		model = glm::scale(model, glm::vec3(0.4f, 0.12f, 0.12f));
		glUniform3f(uniCol, 1.0f, 0.5f, 0.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// DEDO 2 
		// Falange 
		model = glm::translate(baseMano, glm::vec3(0.25f, -0.3f, 0.3f)); // Abajo-Z
		model = glm::rotate(model, glm::radians(d2_f1), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 f1_d2 = glm::translate(model, glm::vec3(0.3f, 0.0f, 0.0f));
		model = glm::scale(f1_d2, glm::vec3(0.6f, 0.15f, 0.15f));
		glUniform3f(uniCol, 1.0f, 1.0f, 0.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// Punta
		model = glm::translate(f1_d2, glm::vec3(0.3f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(d2_f2), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(0.2f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.12f, 0.12f));
		glUniform3f(uniCol, 1.0f, 0.5f, 0.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// DEDO 3 
		// Falange
		model = glm::translate(baseMano, glm::vec3(0.25f, -0.3f, -0.3f)); 
		model = glm::rotate(model, glm::radians(d3_f1), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 f1_d3 = glm::translate(model, glm::vec3(0.3f, 0.0f, 0.0f));
		model = glm::scale(f1_d3, glm::vec3(0.6f, 0.15f, 0.15f));
		glUniform3f(uniCol, 1.0f, 1.0f, 0.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// Punta 3 
		model = glm::translate(f1_d3, glm::vec3(0.3f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(d3_f2), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(0.2f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.12f, 0.12f));
		glUniform3f(uniCol, 1.0f, 0.5f, 0.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glfwSwapBuffers(window);
	}
	glfwTerminate();
	return 0;
}

void Inputs(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

	// Cámara y Brazo 
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) movX += 0.05f;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) movX -= 0.05f;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) movY += 0.05f;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) movY -= 0.05f;
	if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) movZ -= 0.05f; 
	if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) movZ += 0.05f; 
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) rot += 0.5f;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) rot -= 0.5f;

	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) hombro += 0.5f;
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) hombro -= 0.5f;
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) codo += 0.5f;
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) codo -= 0.5f;
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) muneca += 0.5f;
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) muneca -= 0.5f;

	// Dedos
	float s = 0.5f;
	// Dedo 1: 1/2, Q/E
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) d1_f1 += s;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) d1_f1 -= s;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) d1_f2 += s;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) d1_f2 -= s;
	// Dedo 2: 3/4, Z/X
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) d2_f1 += s;
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) d2_f1 -= s;
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) d2_f2 += s;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) d2_f2 -= s;
	// Dedo 3: 5/6, C/V
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) d3_f1 += s;
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) d3_f1 -= s;
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) d3_f2 += s;
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) d3_f2 -= s;
}