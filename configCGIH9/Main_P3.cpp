#include<iostream>
//#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
//bibliotecas para trabajar con proyecciones
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
const GLint WIDTH = 800, HEIGHT = 600;

int main() {
	glfwInit();
	//Verificaci�n de compatibilidad 
	// Set all the required options for GLFW
	/*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);*/
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	//tamaño pantalla titulo
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Proyecciones y transformaciones basicas", nullptr, nullptr);
	int screenWidth, screenHeight;
	glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
	//Verificaci�n de errores de creacion  ventana
	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}


	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	//Verificaci�n de errores de inicializaci�n de glew
	if (GLEW_OK != glewInit()) {
		std::cout << "Failed to initialise GLEW" << std::endl;
		return EXIT_FAILURE;
	}
	// Define las dimensiones del viewport
	glViewport(0, 0, screenWidth, screenHeight);
	// Setup OpenGL options
	glEnable(GL_DEPTH_TEST);
	// enable alpha support
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Build and compile our shader program
	Shader ourShader("Shader/core.vs", "Shader/core.frag");

	//proyeccion ortogonal
	//los vertices se usan para el color y la posicion, se multiplican por 500 para que se vea mas grande el cubo, si es ortogonal no se ve profundidad, si es perspectiva se ve profundidad
	GLfloat vertices[] = {
       -0.5f*500, -0.5f, 0.5f, 1.0f, 0.0f,0.0f,//Front
		0.5f * 500, -0.5f * 500, 0.5f * 500,  1.0f, 0.0f,0.0f,
		0.5f * 500,  0.5f * 500, 0.5f * 500,  1.0f, 0.0f,0.0f,
		0.5f * 500,  0.5f * 500, 0.5f * 500,  1.0f, 0.0f,0.0f,
		-0.5f * 500,  0.5f * 500, 0.5f * 500, 1.0f, 0.0f,0.0f,
		-0.5f * 500, -0.5f * 500, 0.5f * 500, 1.0f, 0.0f,0.0f,
		
	    -0.5f * 500, -0.5f * 500,-0.5f * 500, 0.0f, 1.0f,0.0f,//Back
		 0.5f * 500, -0.5f * 500,-0.5f * 500, 0.0f, 1.0f,0.0f,
		 0.5f * 500,  0.5f * 500,-0.5f * 500, 0.0f, 1.0f,0.0f,
		 0.5f * 500,  0.5f * 500,-0.5f * 500, 0.0f, 1.0f,0.0f,
	    -0.5f * 500,  0.5f * 500,-0.5f * 500, 0.0f, 1.0f,0.0f,
	    -0.5f * 500, -0.5f * 500,-0.5f * 500, 0.0f, 1.0f,0.0f,
		
		 0.5f * 500, -0.5f * 500,  0.5f * 500,  0.0f, 0.0f,1.0f,
		 0.5f * 500, -0.5f * 500, -0.5f * 500,  0.0f, 0.0f,1.0f,
		 0.5f * 500,  0.5f * 500, -0.5f * 500,  0.0f, 0.0f,1.0f,
		 0.5f * 500,  0.5f * 500, -0.5f * 500,  0.0f, 0.0f,1.0f,
		 0.5f * 500,  0.5f * 500,  0.5f * 500,  0.0f, 0.0f,1.0f,
		 0.5f * 500,  -0.5f * 500, 0.5f * 500, 0.0f, 0.0f,1.0f,
      
		-0.5f * 500,  0.5f * 500,  0.5f * 500,  1.0f, 1.0f,0.0f,
		-0.5f * 500,  0.5f * 500, -0.5f * 500,  1.0f, 1.0f,0.0f,
		-0.5f * 500, -0.5f * 500, -0.5f * 500,  1.0f, 1.0f,0.0f,
		-0.5f * 500, -0.5f * 500, -0.5f * 500,  1.0f, 1.0f,0.0f,
		-0.5f * 500, -0.5f * 500,  0.5f * 500,  1.0f, 1.0f,0.0f,
		-0.5f * 500,  0.5f * 500,  0.5f * 500,  1.0f, 1.0f,0.0f,
		
		-0.5f * 500, -0.5f * 500, -0.5f * 500, 0.0f, 1.0f,1.0f,
		0.5f * 500, -0.5f * 500, -0.5f * 500,  0.0f, 1.0f,1.0f,
		0.5f * 500, -0.5f * 500,  0.5f * 500,  0.0f, 1.0f,1.0f,
		0.5f * 500, -0.5f * 500,  0.5f * 500,  0.0f, 1.0f,1.0f,
		-0.5f * 500, -0.5f * 500,  0.5f * 500, 0.0f, 1.0f,1.0f,
		-0.5f * 500, -0.5f * 500, -0.5f * 500, 0.0f, 1.0f,1.0f,
		
		-0.5f * 500,  0.5f * 500, -0.5f * 500, 1.0f, 0.2f,0.5f,
		0.5f * 500,  0.5f * 500, -0.5f * 500,  1.0f, 0.2f,0.5f,
		0.5f * 500,  0.5f * 500,  0.5f * 500,  1.0f, 0.2f,0.5f,
		0.5f * 500,  0.5f * 500,  0.5f * 500,  1.0f, 0.2f,0.5f,
		-0.5f * 500,  0.5f * 500,  0.5f * 500, 1.0f, 0.2f,0.5f,
		-0.5f * 500,  0.5f * 500, -0.5f * 500, 1.0f, 0.2f,0.5f,
	};


	//// use with Perspective Projection
	//float vertices[] = {
	//	-0.5f, -0.5f, 0.5f, 1.0f, 0.0f,0.0f,//Front
	//	0.5f, -0.5f, 0.5f,  1.0f, 0.0f,0.0f,
	//	0.5f,  0.5f, 0.5f,  1.0f, 0.0f,0.0f,
	//	0.5f,  0.5f, 0.5f,  1.0f, 0.0f,0.0f,
	//	-0.5f,  0.5f, 0.5f, 1.0f, 0.0f,0.0f,
	//	-0.5f, -0.5f, 0.5f, 1.0f, 0.0f,0.0f,

	//	-0.5f, -0.5f,-0.5f, 0.0f, 1.0f,0.0f,//Back
	//	 0.5f, -0.5f,-0.5f, 0.0f, 1.0f,0.0f,
	//	 0.5f,  0.5f,-0.5f, 0.0f, 1.0f,0.0f,
	//	 0.5f,  0.5f,-0.5f, 0.0f, 1.0f,0.0f,
	//	-0.5f,  0.5f,-0.5f, 0.0f, 1.0f,0.0f,
	//	-0.5f, -0.5f,-0.5f, 0.0f, 1.0f,0.0f,

	//	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,1.0f,
	//	 0.5f, -0.5f, -0.5f,  0.0f, 0.0f,1.0f,
	//	 0.5f,  0.5f, -0.5f,  0.0f, 0.0f,1.0f,
	//	 0.5f,  0.5f, -0.5f,  0.0f, 0.0f,1.0f,
	//	 0.5f,  0.5f,  0.5f,  0.0f, 0.0f,1.0f,
	//	 0.5f,  -0.5f, 0.5f, 0.0f, 0.0f,1.0f,

	//	-0.5f,  0.5f,  0.5f,  1.0f, 1.0f,0.0f,
	//	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,0.0f,
	//	-0.5f, -0.5f, -0.5f,  1.0f, 1.0f,0.0f,
	//	-0.5f, -0.5f, -0.5f,  1.0f, 1.0f,0.0f,
	//	-0.5f, -0.5f,  0.5f,  1.0f, 1.0f,0.0f,
	//	-0.5f,  0.5f,  0.5f,  1.0f, 1.0f,0.0f,

	//	-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,1.0f,
	//	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,1.0f,
	//	0.5f, -0.5f,  0.5f,  0.0f, 1.0f,1.0f,
	//	0.5f, -0.5f,  0.5f,  0.0f, 1.0f,1.0f,
	//	-0.5f, -0.5f,  0.5f, 0.0f, 1.0f,1.0f,
	//	-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,1.0f,

	//	-0.5f,  0.5f, -0.5f, 1.0f, 0.2f,0.5f,
	//	0.5f,  0.5f, -0.5f,  1.0f, 0.2f,0.5f,
	//	0.5f,  0.5f,  0.5f,  1.0f, 0.2f,0.5f,
	//	0.5f,  0.5f,  0.5f,  1.0f, 0.2f,0.5f,
	//	-0.5f,  0.5f,  0.5f, 1.0f, 0.2f,0.5f,
	//	-0.5f,  0.5f, -0.5f, 1.0f, 0.2f,0.5f,
	//};

	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);
	// Enlazar  Vertex Array Object
	glBindVertexArray(VAO);
	//2.- Copiamos nuestros arreglo de vertices en un buffer de vertices para que OpenGL lo use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// 3.Copiamos nuestro arreglo de indices en  un elemento del buffer para que OpenGL lo use
	/*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);*/
	// 4. Despues colocamos las caracteristicas de los vertices
	//Posicion
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	//Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs)
	
	//CREACION DE LA MATRIZ DE PROYECCION
	//crear una matriz para uesra proyeccion de 4*4 con una unidad en cada uno de sus elementos 
	glm::mat4 projection = glm::mat4(1);
	//enviando proyeccion en perspectiva, el primer parametro es el angulo de vision, el segundo es el radio de aspecto que se obtiene dividiendo el ancho entre el alto, el tercero es la cercania y el cuarto es la lejania, entre mas lejos se vea algo mas pequeño se vera
	//projection = glm::perspective(45.0f, (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 100.0f);//FOV, Radio de aspecto,znear,zfar
	//al graficarlo no se va a ver profundidad porque es ortogonal
	projection = glm::ortho(0.0f, (GLfloat)screenWidth, 0.0f, (GLfloat)screenHeight, 0.1f, 1000.0f);//Izq,Der,Fondo,Alto,Cercania,Lejania
	
	//BUCLE DE RENDERIZADO 
	while (!glfwWindowShouldClose(window))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		// Render
		// Clear the colorbuffer
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Draw our first triangle
		ourShader.Use();

		//variable uniform 
		GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
		GLint projecLoc = glGetUniformLocation(ourShader.Program, "projection");

		glm::mat4 model = glm::mat4(1); //matriz de modelo
		glm::mat4 view = glm::mat4(1); //matriz de vista


		//view = glm::translate(view, glm::vec3(0.0f, 0.0f, -20.0f)); //trasladando la vista es decir la camara
		// rotacion para el modelo rotando modelo .5 sobre x es decir gira sobre ese, por eso se ve hacia enfrente si es un costado es sobre el eje y
		//model = glm::rotate(model, 0.5f, glm::vec3(1.0f, 1.0f, 0.0f)); // use to compare orthographic and perspective projection
		//Tamaño diferente para el modelo 
		//model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));
		view = glm::translate( view, glm::vec3( screenWidth / 2, screenHeight / 2,-700.0f ) ); // traslacion a la vista, como es ortogonal no se ve nada use with orthographic projection
		glBindVertexArray(VAO);//se vinculan los vertices del cubo
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		//*****SEGUNDO CUBO ********
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(10.0f, 0.0f, 0.0f));
		//model = glm::rotate(model, 0.5f, glm::vec3(3.0f, 3.0f, 5.0f));
		//model = glm::scale(model, glm::vec3(3.0f, 3.0f, 6.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));// Envía matriz al shader
		//glDrawArrays(GL_TRIANGLES, 0, 36);// Dibuja los 36 vértices
		////*****TERCER CUBO ********
		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(-10.0f, 0.0f, 0.0f));
		//model = glm::rotate(model, 0.5f, glm::vec3(-7.0f, 7.0f, -3.0f));
		//model = glm::scale(model, glm::vec3(4.0f, 8.0f, 5.0f));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//glDrawArrays(GL_TRIANGLES, 0, 36);

		glUniformMatrix4fv(projecLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glBindVertexArray(0);
		// Swap the screen buffers
		glfwSwapBuffers(window);

	}
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glfwTerminate();
	return EXIT_SUCCESS;
}
//EJERCICIOS 
//Dibujar dos cubos extra el primero 1,1,1
//cubo a la izqueirda viendo hacia la izqueirda rotacion en z 
//cubo a la derecha viendo a la derecha rotacion en z


