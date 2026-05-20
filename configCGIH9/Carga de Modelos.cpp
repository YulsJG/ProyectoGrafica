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
#include "Animator.h"
#include "AnimacionPerro.h"
#include "AnimacionPajaro.h"

#define _CRT_SECURE_NO_WARNINGS

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

// ── AGENTES ────────────────────────────────────────────────────
struct Agente {
    glm::vec3 pos;
    float     rotY = 0.0f;
    int       wpActual = 0;
    float     tiempoAnim = 0.0f;   // offset para que no estén sincronizados
    bool      activo = false;
};

// ── RUTAS INDEPENDIENTES PARA CADA AGENTE ─────────────────────

// Agente 0: Avanza por el centro, gira a la IZQUIERDA (X grande) y rodea los stands
std::vector<glm::vec3> rutaAgente0 = {
    glm::vec3(50.0f, -28.0f, -50.0f),  // Punto 1: Inicia al fondo del pasillo
    glm::vec3(50.0f, -28.0f,   0.0f),  // Punto 2: Llega al frente de la explanada y aquí frena
    glm::vec3(120.0f, -28.0f,  0.0f),  // Punto 3: Gira 90° y camina hacia la IZQUIERDA
    glm::vec3(120.0f, -28.0f, 60.0f),  // Punto 4: Sube por el pasillo lateral izquierdo
    glm::vec3(50.0f, -28.0f,  60.0f)   // Punto 5: Regresa al centro para reiniciar el ciclo
};

// Agente 1: Avanza por el centro, gira a la DERECHA (X chica) y hace el circuito opuesto
std::vector<glm::vec3> rutaAgente1 = {
    glm::vec3(70.0f, -28.0f, -50.0f),  // Punto 1: Inicia al fondo un poco desfasado en X
    glm::vec3(70.0f, -28.0f,   0.0f),  // Punto 2: Arriba al frente de la explanada
    glm::vec3(15.0f, -28.0f,   0.0f),  // Punto 3: Gira 90° y camina hacia la DERECHA
    glm::vec3(15.0f, -28.0f, 60.0f),  // Punto 4: Sube por el pasillo lateral derecho
    glm::vec3(70.0f, -28.0f,  60.0f)   // Punto 5: Regresa al centro
};

// Agente 2: Hace un recorrido libre en forma de "S" cruzando por el pasillo central
std::vector<glm::vec3> rutaAgente2 = {
    glm::vec3(60.0f, -28.0f, -50.0f),  // Inicia en el centro puro
    glm::vec3(60.0f, -28.0f,  25.0f),  // Avanza recto por el pasillo central
    glm::vec3(35.0f, -28.0f,  45.0f),  // Se desvía un poco en diagonal hacia la derecha
    glm::vec3(85.0f, -28.0f,  75.0f),  // Cruza en diagonal hacia la izquierda
    glm::vec3(60.0f, -28.0f,  10.0f)   // Corta camino de regreso al inicio
};



// ── SISTEMA DÍA / NOCHE ────────────────────────────────────────
bool esDeNoche = false;
float factorNoche = 0.0f;   // 0.0 = día pleno, 1.0 = noche plena
float velocidadTransicion = 1.5f;  // qué tan rápido cambia

// Posiciones de las lámparas (ajusta Y según la altura de tus stands)
// Agrega tantas como stands tengas
struct LuzPuntual {
    glm::vec3 posicion;
    glm::vec3 colorAmbiente;
    glm::vec3 colorDifuso;
    glm::vec3 colorEspecular;
};

LuzPuntual lucesPuntuales[] = {
    // Stand IZQUIERDA
    { glm::vec3(150.195f, -30.0f, -25.2517f),
      glm::vec3(0.08f, 0.05f, 0.01f),
      glm::vec3(3.5f,  2.2f,  0.4f),
      glm::vec3(2.0f,  1.4f,  0.3f) },  // especular — brillo cálido

      // Stand CENTRO
      { glm::vec3(52.8673f, -18.0f, 112.395f),
         glm::vec3(0.08f, 0.05f, 0.01f),
         glm::vec3(3.5f,  2.2f,  0.4f),
         glm::vec3(2.0f,  1.4f,  0.3f) },

         // Stand DERECHA
         {glm::vec3(-17.0f, -30.0f,  -62.1774f),
          glm::vec3(0.08f, 0.05f, 0.01f),
          glm::vec3(3.5f,  2.2f,  0.4f),
          glm::vec3(2.0f,  1.4f,  0.3f) },

          // Cuarta apagada(el shader necesita 4, esta no hace nada)
          {
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 0.0f)
           },
};
const int NUM_LUCES = 4;


const int NUM_AGENTES = 3;
Agente agentes[NUM_AGENTES];

float velocidadPersona = 15.0f;

// Animación — UNA sola instancia compartida
Animator    animatorPersona;
Animation* animWalking = nullptr;
bool        personasActivas = false;

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

    //Persona
    //Model personaje2((char*)"Models/Persona/scene.gltf");
    Model Torso((char*)"Models/Persona/Torso.obj");
    Model Cabeza((char*)"Models/Persona/Cabeza.obj");
    Model Pierna((char*)"Models/Persona/Piernas.obj");
    Model PiernaDer((char*)"Models/Persona/PiernaDer.obj");
    Model PiernaIzq((char*)"Models/Persona/PiernaIzq.obj");
    Model BrazoDer((char*)"Models/Persona/BrazoDer.obj");
    Model BrazoIzq((char*)"Models/Persona/BrazoIzq.obj");

    //Perro (Cambiar el modelo despues)
    Model DogBody((char*)"Models/Perro/DogBody.obj");
    Model HeadDog((char*)"Models/Perro/HeadDog.obj");
    Model DogTail((char*)"Models/Perro/TailDog.obj");
    Model F_RightLeg((char*)"Models/Perro/F_RightLegDog.obj");
    Model F_LeftLeg((char*)"Models/Perro/F_LeftLegDog.obj");
    Model B_RightLeg((char*)"Models/Perro/B_RightLegDog.obj");
    Model B_LeftLeg((char*)"Models/Perro/B_LeftLegDog.obj");

    InicializarRutaPerro();

    //Pajaro
    Model BirdBody((char*)"Models/Pajaro/CuerpoPajaro2.obj");
    Model BirdTail((char*)"Models/Pajaro/ColitaPajaro.obj");
    Model P_patader((char*)"Models/Pajaro/PataDer.obj");
    Model P_pataizq((char*)"Models/Pajaro/PataIzq.obj");
    Model P_alader((char*)"Models/Pajaro/AlaIzq2.obj");
    Model P_alaizq((char*)"Models/Pajaro/AlaDer2.obj");

    InicializarPajaro();

  
    // Projection matrix
    glm::mat4 projection = glm::perspective(
        camera.GetZoom(),
        (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT,
        0.1f,
        2000.0f
    );

    // Inicialización alineada con sus respectivas rutas individuales
    agentes[0] = { rutaAgente0[0], 0.0f, 0, 0.0f, true }; // Sigue Ruta 0
    agentes[1] = { rutaAgente1[0], 0.0f, 0, 1.5f, true }; // Sigue Ruta 1 (con desfase de tiempo)
    agentes[2] = { rutaAgente2[0], 0.0f, 0, 3.0f, true }; // Sigue Ruta 2 (con desfase de tiempo)

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();
        AnimacionPerro();
        AnimacionPajaro(deltaTime);

        // ── Actualizar animación ──
        if (personasActivas) {
            animatorPersona.Update(deltaTime);
        }

        // ── Transición día/noche ──────────────────────────────────────
        if (esDeNoche && factorNoche < 1.0f)
            factorNoche = glm::min(factorNoche + velocidadTransicion * deltaTime, 1.0f);
        else if (!esDeNoche && factorNoche > 0.0f)
            factorNoche = glm::max(factorNoche - velocidadTransicion * deltaTime, 0.0f);

        // El color de fondo también cambia (cielo)
        float r = glm::mix(0.1f, 0.01f, factorNoche);
        float g = glm::mix(0.1f, 0.01f, factorNoche);
        float b = glm::mix(0.1f, 0.04f, factorNoche);
        glClearColor(r, g, b, 1.0f);

        // ── Luz direccional (sol → casi apagado de noche) ─────────────
        float ambDia = 0.45f, ambNoche = 0.01f;
        float difDia = 0.75f, difNoche = 0.02f;
        float specDia = 0.25f, specNoche = 0.0f;

        float amb = glm::mix(ambDia, ambNoche, factorNoche);
        float dif = glm::mix(difDia, difNoche, factorNoche);
        float spec = glm::mix(specDia, specNoche, factorNoche);

        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.3f, -1.0f, -0.4f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), amb, amb, amb);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), dif, dif, dif);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), spec, spec, spec);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "factorNoche"), factorNoche);


        // ── Point lights (lámparas de stands) ────────────────────────
        for (int i = 0; i < NUM_LUCES && i < 4; i++)
        {
            std::string base = "pointLights[" + std::to_string(i) + "]";

            // Multiplica por factorNoche para que solo brillen de noche
            glm::vec3 amb3 = lucesPuntuales[i].colorAmbiente * factorNoche;
            glm::vec3 dif3 = lucesPuntuales[i].colorDifuso * factorNoche;
            glm::vec3 spec3 = lucesPuntuales[i].colorEspecular * factorNoche;
            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".position").c_str()),
                lucesPuntuales[i].posicion.x,
                lucesPuntuales[i].posicion.y,
                lucesPuntuales[i].posicion.z);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".ambient").c_str()),
                amb3.x, amb3.y, amb3.z);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".diffuse").c_str()),
                dif3.x, dif3.y, dif3.z);
            glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".specular").c_str()),
                spec3.x, spec3.y, spec3.z);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".constant").c_str()), 1.0f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".linear").c_str()), 0.001f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".quadratic").c_str()), 0.00005f);
        }

        // Las point lights sobrantes (índices 4 a 3 del shader) quedan apagadas como antes
        // (tu código original ya las apaga, déjalas igual)

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
        // LUZ DIRECCIONAL TIPO SOL
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
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material_shininess"), 5.0f);

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
        // ── VELOCIDAD DE APARICIÓN (Ajusta este número: más grande = más rápido) ──
        //float velocidadAparicion = 8.0f;
        if (estadoStands == 1 && escalaAnimacion < 1.0f) {
            escalaAnimacion += 0.02f;
            //escalaAnimacion += velocidadAparicion * deltaTime;
            if (escalaAnimacion > 1.0f) escalaAnimacion = 1.0f;
        }
        else if (estadoStands != 1 && escalaAnimacion > 0.0f) {
            escalaAnimacion -= 0.02f; // Si el estado cambia a Feria (2), estos bajan a 0
            //escalaAnimacion -= velocidadAparicion * deltaTime;
            if (escalaAnimacion < 0.0f) escalaAnimacion = 0.0f;
        }

        // Lógica de escala para la Feria de Empleo
        if (estadoStands == 2 && escalaFeria < 1.0f) {
            escalaFeria += 0.02f;
            //escalaFeria += velocidadAparicion * deltaTime;
            if (escalaFeria > 1.0f) escalaFeria = 1.0f;
        }
        else if (estadoStands != 2 && escalaFeria > 0.0f) {
            escalaFeria -= 0.02f;
            //escalaFeria -= velocidadAparicion * deltaTime;
            if (escalaFeria < 0.0f) escalaFeria = 0.0f;
        }

        //Dibujar stands agrupaciones
        if (escalaAnimacion > 0.0f) {
            for (int i = 0; i < 3; i++) { // Ahora iteramos 3 veces
                dibujarStandIndividual(lightingShader, Mampara, Mesa, Silla, posicionesStands[i], rotacionesStands[i]);
            }
        }

        // Dibujar stands Feria de Empleo
        //if (escalaFeria > 0.0f) {
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


        //}

        if (personasActivas)
        {
            // ── Dibujar Personas ──
            for (int i = 0; i < 3; i++) {
                // 1. Mover hacia el waypoint
                std::vector<glm::vec3>* rutaActual;
                if (i == 0)      rutaActual = &rutaAgente0;
                else if (i == 1) rutaActual = &rutaAgente1;
                else             rutaActual = &rutaAgente2;

                // Mover hacia el waypoint de SU propia ruta
                glm::vec3 dest = (*rutaActual)[agentes[i].wpActual];
                glm::vec3 dir = dest - agentes[i].pos;
                if (glm::length(dir) > 1.5f) {
                    agentes[i].pos += glm::normalize(dir) * velocidadPersona * deltaTime;
                    agentes[i].rotY = glm::degrees(atan2(dir.x, dir.z));
                }
                else {
                    // Avanza al siguiente waypoint dentro de SU ruta
                    agentes[i].wpActual = (agentes[i].wpActual + 1) % rutaActual->size();
                }

                // 2. Variables de Animación (Suaves para que no se clipeen)
                float t = (glfwGetTime() + agentes[i].tiempoAnim) * 5.0f;
                float anglePiernas = sin(t) * 5.0f;
                float angleBrazos = sin(t) * 0.0f;


                // 3. MATRIZ BASE
                glm::mat4 mBase = glm::mat4(1.0f);
                mBase = glm::translate(mBase, agentes[i].pos);
                mBase = glm::rotate(mBase, glm::radians(agentes[i].rotY), glm::vec3(0, 1, 0));
                mBase = glm::scale(mBase, glm::vec3(10.0f));

                // TORSO
                glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(mBase));
                Torso.Draw(lightingShader);

                // CABEZA
                glm::mat4 mCabeza = mBase;
                mCabeza = glm::translate(mCabeza, glm::vec3(0.0f, -0.02f, 0.0f));
                glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(mCabeza));
                Cabeza.Draw(lightingShader);

                // =========================================================
                // PIERNAS (Muslos unidos + Pantorrillas independientes)
                // =========================================================

                // 1. MUSLOS (PADRE) - Se quedan estáticos como la base de las piernas
                glm::mat4 mMuslos = mBase;
                glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(mMuslos));
                Pierna.Draw(lightingShader); // Dibuja "Piernas.obj" (Ambos muslos juntos)

                // 2. PANTORRILLA DERECHA (HIJO)
                glm::mat4 mPDer = mMuslos; // Hereda la matriz de los muslos para mantenerse conectada
                // El pivote ahora está en la RODILLA (aproximadamente a la mitad, Y = 0.045f)
                glm::vec3 pivotRodillaDer(-0.01f, 0.045f, 0.0f);
                mPDer = glm::translate(mPDer, pivotRodillaDer);
                mPDer = glm::rotate(mPDer, glm::radians(anglePiernas), glm::vec3(1, 0, 0)); // Gira la rodilla
                mPDer = glm::translate(mPDer, -pivotRodillaDer); // Regresa a su posición
                glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(mPDer));
                PiernaDer.Draw(lightingShader);

                // 3. PANTORRILLA IZQUIERDA (HIJO)
                glm::mat4 mPIzq = mMuslos;
                glm::vec3 pivotRodillaIzq(0.01f, 0.045f, 0.0f);
                mPIzq = glm::translate(mPIzq, pivotRodillaIzq);
                mPIzq = glm::rotate(mPIzq, glm::radians(-anglePiernas), glm::vec3(1, 0, 0)); // Fase opuesta
                mPIzq = glm::translate(mPIzq, -pivotRodillaIzq);
                glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(mPIzq));
                PiernaIzq.Draw(lightingShader);

                // BRAZO DERECHO 
                glm::mat4 mBDer = mBase;
                mBDer = glm::translate(mBDer, glm::vec3(-0.010f, -0.025f, 0.0f)); // Posición del hombro derecho
                mBDer = glm::rotate(mBDer, glm::radians(-angleBrazos), glm::vec3(1, 0, 0));
                glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(mBDer));
                BrazoDer.Draw(lightingShader);

                // BRAZO IZQUIERDO
                glm::mat4 mBIzq = mBase;
                mBIzq = glm::translate(mBIzq, glm::vec3(-0.010f, -0.025f, -0.005f)); // Posición del hombro izquierdo
                mBIzq = glm::rotate(mBIzq, glm::radians(angleBrazos), glm::vec3(1, 0, 0));
                glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(mBIzq));
                BrazoIzq.Draw(lightingShader);
            }
        }

        // ── Dibujar perro ──────────────────────────────────────────
        if (perroVisible)
        {
            glm::mat4 modelTemp2;

            // Cuerpo (raíz — todo lo demás depende de esta matriz)
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(perroPosX, perroPosY, perroPosZ));
            model = glm::rotate(model, glm::radians(perroRotY), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(15.0f));   // ajusta si es muy grande/pequeño
            modelTemp2 = model;
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"),
                1, GL_FALSE, glm::value_ptr(model));
            DogBody.Draw(lightingShader);

            // Cabeza
            model = modelTemp2;
            model = glm::translate(model, glm::vec3(0.0f, 0.093f, 0.208f));
            model = glm::rotate(model, glm::radians(perroHead), glm::vec3(1.0f, 0.0f, 0.0f));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            HeadDog.Draw(lightingShader);

            // Cola
            model = modelTemp2;
            model = glm::translate(model, glm::vec3(0.0f, 0.026f, -0.288f));
            model = glm::rotate(model, glm::radians(perroTail), glm::vec3(0.0f, 0.0f, -1.0f));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            DogTail.Draw(lightingShader);

            // Pata delantera izquierda
            model = modelTemp2;
            model = glm::translate(model, glm::vec3(0.112f, -0.044f, 0.074f));
            model = glm::rotate(model, glm::radians(perroFLegs), glm::vec3(-1.0f, 0.0f, 0.0f));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            F_LeftLeg.Draw(lightingShader);

            // Pata delantera derecha (fase opuesta)
            model = modelTemp2;
            model = glm::translate(model, glm::vec3(-0.111f, -0.055f, 0.074f));
            model = glm::rotate(model, glm::radians(-perroFLegs), glm::vec3(-1.0f, 0.0f, 0.0f));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            F_RightLeg.Draw(lightingShader);

            // Pata trasera izquierda
            model = modelTemp2;
            model = glm::translate(model, glm::vec3(0.082f, -0.046f, -0.218f));
            model = glm::rotate(model, glm::radians(perroRLegs), glm::vec3(-1.0f, 0.0f, 0.0f));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            B_LeftLeg.Draw(lightingShader);

            // Pata trasera derecha
            model = modelTemp2;
            model = glm::translate(model, glm::vec3(-0.083f, -0.057f, -0.231f));
            model = glm::rotate(model, glm::radians(-perroRLegs), glm::vec3(-1.0f, 0.0f, 0.0f));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            B_RightLeg.Draw(lightingShader);
        }

        //Dibujar pájaro
        if (pajaroVisible)
        {
            glm::mat4 modelBase;

            // ── Cuerpo (raíz) ──────────────────────────────────
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(pajaroPosX, pajaroPosY, pajaroPosZ));
            model = glm::rotate(model, glm::radians(pajaroRotY), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(pajaroCuerpoTilt), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(0.3f));   // ajusta escala a tu modelo
            modelBase = model;
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"),
                1, GL_FALSE, glm::value_ptr(model));
            BirdBody.Draw(lightingShader);

            // ── Cola ───────────────────────────────────────────
            model = modelBase;
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.15f));   // offset hacia atrás
            model = glm::rotate(model, glm::radians(pajaroCola), glm::vec3(1.0f, 0.0f, 0.0f));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"),
                1, GL_FALSE, glm::value_ptr(model));
            BirdTail.Draw(lightingShader);

            // ── Pata derecha ───────────────────────────────────
            model = modelBase;
            model = glm::translate(model, glm::vec3(-0.05f, -0.08f, 0.0f));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"),
                1, GL_FALSE, glm::value_ptr(model));
            P_patader.Draw(lightingShader);

            // ── Pata izquierda ─────────────────────────────────
            model = modelBase;
            model = glm::translate(model, glm::vec3(0.05f, -0.08f, 0.0f));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"),
                1, GL_FALSE, glm::value_ptr(model));
            P_pataizq.Draw(lightingShader);

            // ── Ala derecha (bate hacia abajo con +ángulo) ──────
            model = modelBase;
            model = glm::translate(model, glm::vec3(-0.12f, 0.02f, 0.0f));  // pivot ala der
            model = glm::rotate(model, glm::radians(-pajaroAlaAngle), glm::vec3(0.0f, 0.0f, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"),
                1, GL_FALSE, glm::value_ptr(model));
            P_alader.Draw(lightingShader);

            // ── Ala izquierda (bate simétricamente) ────────────
            model = modelBase;
            model = glm::translate(model, glm::vec3(0.12f, 0.02f, 0.0f));   // pivot ala izq
            model = glm::rotate(model, glm::radians(pajaroAlaAngle), glm::vec3(0.0f, 0.0f, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"),
                1, GL_FALSE, glm::value_ptr(model));
            P_alaizq.Draw(lightingShader);
        }

        // Imprimir posición de la cámara en la consola
        /*std::cout << "Posicion Camara: X: " << camera.GetPosition().x
            << " | Y: " << camera.GetPosition().y
            << " | Z: " << camera.GetPosition().z << std::endl*/

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
    mMampara = glm::scale(mMampara, glm::vec3(escalaAnimacion * escalaBase)); // Animación de aparición

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
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        personasActivas = !personasActivas;  // toggle ON/OFF
        if (personasActivas)
            cout << "Personas activadas" << endl;
    }
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
    {
        if (!perroPlay) {
            ResetPerro();
            perroVisible = true;
            perroPlay = true;
            //perroPlayIndex = 0;
            perroCurrSteps = 0;
            printf("Perro activado!\n");
        }
        else {
            perroPlay = false;
            perroVisible = false;
            printf("Perro detenido.\n");
        }
    }
    if (key == GLFW_KEY_B && action == GLFW_PRESS)   // B de Bird / Pájaro
    {
        if (!pajaroPlay) {
            ActivarPajaro();
            printf("Pajaro activado!\n");
        }
        else {
            pajaroPlay = false;
            pajaroVisible = false;
            printf("Pajaro detenido.\n");
        }

    }
    if (key == GLFW_KEY_N && action == GLFW_PRESS)
    {
        esDeNoche = !esDeNoche;
        printf(esDeNoche ? "Modo noche activado\n" : "☀Modo día activado\n");
    }
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        // Creamos una nueva cámara en la posición inicial y reemplazamos la vieja
        camera = Camera(glm::vec3(0.0f, 10.0f, 50.0f));
        std::cout << "Cámara reseteada a la ENTRADA PRINCIPAL" << std::endl;
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
