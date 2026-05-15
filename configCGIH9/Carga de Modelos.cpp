// Std. Includes
#include <string>
#include <iostream>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include "SOIL2/SOIL2.h"
#include "stb_image.h"

// Properties
const GLuint WIDTH = 1200, HEIGHT = 800;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Function prototypes
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();
void dibujarStandIndividual(Shader& lightingShader, Model& Mampara, Model& Mesa, Model& Silla, glm::vec3 posicionGlobal, float rotacionY);

// Camera (MEJOR POSICIÓN)
Camera camera(glm::vec3(0.0f, 10.0f, 50.0f));

bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

//Aparecer stands
bool mostrarStands = false;
float escalaAnimacion = 0.0f; //Stands agrupaciones
int estadoStands = 0;
float escalaFeria = 0.0f;  //Stands Feria de Empleo 

//Posiciones stands
glm::vec3 posicionesStands[] = {
    glm::vec3(150.195f, -30.0f, -25.2517f),  // Stand IZQUIERDA
    glm::vec3(52.8673f, -28.0f, 112.395f),  // Stand CENTRO 
    glm::vec3(-17.0f, -30.0f,  -62.1774f)   // Stand DERECHA 
};
// 105.389    68.543

//Arreglo de rotaciones para cada stand
float rotacionesStands[] = {
     -90.0f,  // Rotación para el stand de la izquierda
    180.0f,  // Rotación para el stand del centro
    90.0f   // Rotación para el stand de la derecha
};

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

int main()
{
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Explanada FI", nullptr, nullptr);

    if (nullptr == window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);

    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    // Shader
    //Shader shader("Shader/modelLoading.vs", "Shader/modelLoading.frag");
    Shader lightingShader("Shader/modelLoading.vs", "Shader/modelLoading.frag");

    // Modelo Escuela
    Model modeloFI((char*)"Models/Explanadafi/explanadafi.obj");

    //Decorativos
    Model moduloInfo((char*)"Models/Muebles/Moduloinfo.obj");
    Model Silla((char*)"Models/Muebles/SillaPlegable.obj");
    Model Estatua((char*)"Models/Muebles/Estatua.obj");

    //Stands
	Model stand1((char*)"Models/Muebles/StandFeria.obj");
    Model Mesa((char*)"Models/Muebles/MesaG.obj");
    Model Mampara((char*)"Models/Muebles/Mampara.obj");

    //Stands2
    Model StandIzq((char*)"Models/Stands/StandIzq.obj");
    Model StandDer((char*)"Models/Stands/StandDer.obj");
    Model StandCentro((char*)"Models/Stands/StandCentro.obj");
    Model StandCentro2((char*)"Models/Stands/StandCentro2.obj");
    Model StandCentro3((char*)"Models/Stands/StandCentro3.obj");
    Model StandCentro4((char*)"Models/Stands/StandCentro4.obj");


    //// PROYECCIÓN CORREGIDA
    //glm::mat4 projection = glm::perspective(
    //    glm::radians(camera.GetZoom()),
    //    (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
    //    0.1f,
    //    2000.0f
    //);
    // Projection matrix
    glm::mat4 projection = glm::perspective(
        camera.GetZoom(),
        (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT,
        0.1f,
        2000.0f
    );

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();

        // Fondo
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //shader.Use();
        lightingShader.Use();
        glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.difuse"), 0);
        glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.specular"), 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Camera position
        GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
        glUniform3f(
            viewPosLoc,
            camera.GetPosition().x,
            camera.GetPosition().y,
            camera.GetPosition().z
        );

        // ===============================
        // �NICA LUZ: LUZ DIRECCIONAL TIPO SOL
        // ===============================
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.3f, -1.0f, -0.4f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.45f, 0.45f, 0.45f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.75f, 0.75f, 0.75f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.25f, 0.25f, 0.25f);

        // Desactivar point lights si tu shader todav�a las tiene declaradas
        for (int i = 0; i < 4; i++)
        {
            std::string base = "pointLights[" + std::to_string(i) + "]";

            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".position").c_str()), 0.0f, 0.0f, 0.0f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".ambient").c_str()), 0.0f, 0.0f, 0.0f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".diffuse").c_str()), 0.0f, 0.0f, 0.0f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".specular").c_str()), 0.0f, 0.0f, 0.0f);

            glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".constant").c_str()), 1.0f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".linear").c_str()), 0.0f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".quadratic").c_str()), 0.0f);
        }

        // Desactivar spotlight si tu shader todav�a lo tiene declarado
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), 0.0f, 0.0f, -1.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.0f)));
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(18.0f)));

        // Material
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 5.0f);

        //View Matrix
        glm::mat4 view = camera.GetViewMatrix();

        // Uniform locations
        GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
        GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

        // Send view and projection
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // Carga modelo FI
        glm::mat4 model = glm::mat4(1.0f);
        // Escala    
        //model = glm::translate(model, glm::vec3(0.0f, -1.0f, -5.0f));
        model = glm::translate(model, glm::vec3(0.0f, -10.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        modeloFI.Draw(lightingShader);

        // MÓDULO DE INFORMACIÓN 
        model = glm::mat4(1.0f);
        // 1. Posicionamiento 
        model = glm::translate(model, glm::vec3(86.1851f, -30.0f, -73.5682f));
        // 3. Escala 
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
        // Enviamos la matriz al shader y dibujamos
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        moduloInfo.Draw(lightingShader);

        //Silla
        model = glm::mat4(1.0f);
        // 1. Posicionamiento 
        model = glm::translate(model, glm::vec3(88.4904f, -30.0f, -75.7144f));
        // 3. Escala 
        model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.0f));
        // Enviamos la matriz al shader y dibujamos
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Silla.Draw(lightingShader);

        //Estatua
        model = glm::mat4(1.0f);
        // 1. Posicionamiento 
        model = glm::translate(model, glm::vec3(98.3464f, -22.0f, -109.688f));
        //Rotación
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        // 3. Escala 
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
        // Enviamos la matriz al shader y dibujamos
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        Estatua.Draw(lightingShader);
        
		//Aparicion stands agrupaciones
        if (estadoStands == 1 && escalaAnimacion < 1.0f) {
            escalaAnimacion += 0.02f;
        }
        else if (estadoStands != 1 && escalaAnimacion > 0.0f) {
            escalaAnimacion -= 0.02f; // Si el estado cambia a Feria (2), estos bajan a 0
        }

        // Lógica de escala para la Feria de Empleo
        if (estadoStands == 2 && escalaFeria < 1.0f) {
            escalaFeria += 0.02f;
        }
        else if (estadoStands != 2 && escalaFeria > 0.0f) {
            escalaFeria -= 0.02f;
        }

		//Dibujar stands agrupaciones
        if (escalaAnimacion > 0.0f) {
            for (int i = 0; i < 3; i++) { // Ahora iteramos 3 veces
                dibujarStandIndividual(lightingShader, Mampara, Mesa, Silla, posicionesStands[i], rotacionesStands[i]);
            }
        }

		// Dibujar stands Feria de Empleo
        if (escalaFeria > 0.0f) {
            // --- STAND IZQUIERDO ---
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-9.90163f, -30.0f, -37.5067f));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.2f * escalaFeria)); // Multiplicamos por la animación
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            StandIzq.Draw(lightingShader);
            
            // --- STAND DERECHO ---
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(130.0f, -30.0f, -37.5067f));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.3f * escalaFeria));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            StandDer.Draw(lightingShader);

            // --- STAND CENTRO ---
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(52.8673f, -28.0f, 105.0f));
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.1f * escalaFeria));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            StandCentro.Draw(lightingShader);
                //Sillas
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(52.8673f, -26.0f, 100.0f));
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.2f * escalaFeria));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            StandCentro2.Draw(lightingShader);
                //Mueble enfrente
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(52.8673f, -28.0f, 100.0f));
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.2f * escalaFeria));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            StandCentro3.Draw(lightingShader);
                //Plantas
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(52.8673f, -28.0f, 90.0f));
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.2f * escalaFeria));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            StandCentro4.Draw(lightingShader);


        }

        //121.545
        // Imprimir posición de la cámara en la consola
        std::cout << "Posicion Camara: X: " << camera.GetPosition().x
            << " | Y: " << camera.GetPosition().y
            << " | Z: " << camera.GetPosition().z << std::endl;

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

//Animación de aparición de stands
void dibujarStandIndividual(Shader& lightingShader, Model& Mampara, Model& Mesa, Model& Silla, glm::vec3 posicionGlobal, float rotacionY) {
    float escalaBase = 17.0f;
    // 1. MATRIZ RAÍZ (MAMPARA)
    // Define la ubicación del stand completo en la explanada
    glm::mat4 mMampara = glm::mat4(1.0f);
    mMampara = glm::translate(mMampara, posicionGlobal);
    //Rotación para que cada stand mire hacia el centro de la explanada
    mMampara = glm::rotate(mMampara, glm::radians(rotacionY), glm::vec3(0.0f, 1.0f, 0.0f));
    mMampara = glm::scale(mMampara, glm::vec3(escalaAnimacion*escalaBase)); // Animación de aparición

    glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(mMampara));
    Mampara.Draw(lightingShader);

    // 2. MATRIZ HIJO (MESA)
    // Depende de la matriz de la Mampara
    glm::mat4 mMesa = mMampara;
    mMesa = glm::translate(mMesa, glm::vec3(-1.0f, -0.2f, 1.5f)); // Offset respecto al centro de la mampara
    mMesa = glm::scale(mMesa, glm::vec3(0.6f, 0.7f, 0.6f));
    glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(mMesa));
    Mesa.Draw(lightingShader);

    // 3. MATRIZ HIJO (SILLA)
    // También depende de la Mampara, pero tiene su propia rotación
    glm::mat4 mSilla = mMampara;
    mSilla = glm::translate(mSilla, glm::vec3(-1.0f, 0.0f, 0.5f)); // Se coloca entre la mesa y la mampara
    mSilla = glm::scale(mSilla, glm::vec3(1.2f, 1.2f, 1.2f));

    glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(mSilla));
    Silla.Draw(lightingShader);
}

// Movimiento de cámara
void DoMovement()
{
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
        camera.ProcessKeyboard(FORWARD, deltaTime);

    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
        camera.ProcessKeyboard(BACKWARD, deltaTime);

    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
        camera.ProcessKeyboard(LEFT, deltaTime);

    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Teclado
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
	//Control de aparición de stands
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        // Si presionas F, activas Feria (2) o apagas si ya estaba
        estadoStands = (estadoStands == 2) ? 0 : 2;
    }
    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        // Si presionas G, activas los otros (1) o apagas
        estadoStands = (estadoStands == 1) ? 0 : 1;
    }
}

// Mouse
void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse)
    {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos;

    lastX = xPos;
    lastY = yPos;

    camera.ProcessMouseMovement(xOffset, yOffset);
}