#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdio>
#include <algorithm>

// ══════════════════════════════════════════════════════════════
//  ANIMACIÓN PERRO — KeyFrames + Curvas de Bézier Cúbicas
// ══════════════════════════════════════════════════════════════

#define MAX_FRAMES_PERRO 8

// ── Obstáculo (stand) ─────────────────────────────────────────
struct Obstaculo {
    float x, z;      // centro del stand en la escena
    float radio;     // distancia mínima que debe mantener el perro
};

// ── DEFINE TUS STANDS AQUÍ ────────────────────────────────────
// Copia las posiciones X y Z de tus stands del main.cpp
// El radio es cuánto espacio rodea al stand (ajusta si lo atraviesa)
std::vector<Obstaculo> obstaculos = {
    { 150.195f, -30.0f, -25.2517f },   // Stand IZQUIERDA
    {  86.1851f, -30.0f, -73.5682f },   // Stand CENTRO
    { -17.0f, -30.0f,  -62.1774f },   // Stand DERECHA
};

// ── Punto de ruta con tangentes Bézier ───────────────────────
struct PuntoRuta {
    glm::vec3 pos;
    glm::vec3 ctrl1;   // tangente de entrada
    glm::vec3 ctrl2;   // tangente de salida
};

// ── Variables del perro ───────────────────────────────────────
std::vector<PuntoRuta> rutaPerro;

float perroPosX = 0.0f, perroPosY = -25.0f, perroPosZ = 0.0f;
float perroRotY = 0.0f;
float perroFLegs = 0.0f, perroRLegs = 0.0f;
float perroHead = 0.0f, perroTail = 0.0f;

bool  perroPlay = false;
bool  perroVisible = false;
int   perroSegmento = 0;
int   perroCurrSteps = 0;
int   perroMaxSteps = 80;   // pasos por segmento (más = más lento)


// ── Evaluación de Bézier Cúbica ──────────────────────────────
// B(t) = (1-t)³·P0 + 3(1-t)²t·C1 + 3(1-t)t²·C2 + t³·P1
float BezierCubica(float p0, float c1, float c2, float p1, float t)
{
    float u = 1.0f - t;
    return (u * u * u * p0)
        + (3.0f * u * u * t * c1)
        + (3.0f * u * t * t * c2)
        + (t * t * t * p1);
}
// ── Empuja un punto fuera del obstáculo más cercano ───────────
// Si 'punto' está dentro del radio de un obstáculo, lo desplaza
// perpendicularmente al segmento origen→destino para rodearlo.
glm::vec3 EmpujarFueraDeObstaculo(glm::vec3 punto,
    glm::vec3 origen,
    glm::vec3 destino)
{
    for (auto& obs : obstaculos)
    {
        float dist = glm::length(glm::vec2(punto.x - obs.x,
            punto.z - obs.z));
        if (dist < obs.radio * 1.5f)
        {
            // Dirección del segmento (en XZ)
            glm::vec3 dir = glm::normalize(
                glm::vec3(destino.x - origen.x, 0.0f, destino.z - origen.z));

            // Perpendicular al segmento
            glm::vec3 perp(-dir.z, 0.0f, dir.x);

            // Detectar en qué lado está el obstáculo
            glm::vec3 hastaObs(obs.x - origen.x, 0.0f, obs.z - origen.z);
            float lado = glm::dot(hastaObs, perp);

            // Rodear por el lado CONTRARIO
            float signo = (lado >= 0.0f) ? -1.0f : 1.0f;
            float empuje = (obs.radio * 1.6f - dist) + obs.radio * 0.5f;
            punto.x += perp.x * signo * empuje;
            punto.z += perp.z * signo * empuje;
        }
    }
    return punto;
}

// ── Calcula tangentes suaves tipo Catmull-Rom → Bézier ───────
// Dado el vector de puntos, asigna ctrl1 y ctrl2 a cada uno
// usando la dirección entre sus vecinos. Resultado: curva suave
// que pasa exactamente por cada punto sin esquinas bruscas.
void CalcularControles(std::vector<PuntoRuta>& ruta) 
{
    int n = (int)ruta.size();
    for (int i = 0; i < n; i++)
    {
        glm::vec3 prev = (i > 0) ? ruta[i - 1].pos : ruta[i].pos;
        glm::vec3 curr = ruta[i].pos;
        glm::vec3 next = (i < n - 1) ? ruta[i + 1].pos : ruta[i].pos;

        glm::vec3 tangente = (next - prev) * 0.35f;
        ruta[i].ctrl2 = curr + tangente;
        ruta[i].ctrl1 = curr - tangente;
    }
}

// ── Genera puntos intermedios entre dos waypoints ─────────────
// Muestrea la línea recta en varios puntos y empuja cada uno
// fuera de los obstáculos si es necesario.
// Solo agrega el punto si fue desplazado (si no hay obstáculo
// en medio, la ruta sigue recta sin puntos extra innecesarios).
std::vector<glm::vec3> GenerarPuntosIntermedios(glm::vec3 origen,
    glm::vec3 destino)
{
    std::vector<glm::vec3> puntos;
    puntos.push_back(origen);

    int muestras = 6;
    for (int i = 1; i < muestras; i++)
    {
        float t = (float)i / muestras;
        glm::vec3 lineal = origen + (destino - origen) * t;
        glm::vec3 correcto = EmpujarFueraDeObstaculo(lineal, origen, destino);

        // Solo agregar si fue desplazado (hay obstáculo en esa zona)
        if (glm::length(correcto - lineal) > 2.0f)
            puntos.push_back(correcto);
    }

    puntos.push_back(destino);
    return puntos;
}

// ── Ruta predefinida del perro ────────────────────────────────
// Ajusta las coordenadas usando el cout de tu cámara
void InicializarRutaPerro()
{
    rutaPerro.clear();
    float Y = -25.0f;
    // Formato: { posX, posY, posZ,  ctrl1X,ctrl1Y,ctrl1Z,  ctrl2X,ctrl2Y,ctrl2Z,  rotY }
    // ctrl1 = cómo llega a este punto (tangente de entrada)
    // ctrl2 = cómo sale hacia el siguiente (tangente de salida)
    // rotY  = hacia dónde mira el perro (grados)

    // ── WAYPOINTS PRINCIPALES ────────────────────────────────
    // Solo necesitas poner los puntos generales de la ruta.
    // El algoritmo se encarga de rodear los stands.
    std::vector<glm::vec3> waypoints = {
        glm::vec3(-25.2716f, Y, 9.22667f),   // Entrada
        glm::vec3(34.0721f, Y,  -33.617f),   // Zona stand DERECHO
		glm::vec3(89.0516f, Y,    -53.9264f),   // Modulo Información
        glm::vec3(123.608f, Y, -26.2666f),   // Zona stand CENTRAL
        glm::vec3(104.944f, Y,  44.0344f),   // Hacia salida
        glm::vec3(163.268f, Y,    18.6108f),   // Salida
    };
    // ────────────────────────────────────────────────────────

    // Genera puntos intermedios con evitación de obstáculos
    for (int i = 0; i < (int)waypoints.size() - 1; i++)
    {
        auto subPuntos = GenerarPuntosIntermedios(waypoints[i], waypoints[i + 1]);
        for (int j = 0; j < (int)subPuntos.size() - 1; j++)
        {
            PuntoRuta pr;
            pr.pos = pr.ctrl1 = pr.ctrl2 = subPuntos[j];
            rutaPerro.push_back(pr);
        }
    }
    PuntoRuta ultimo;
    ultimo.pos = ultimo.ctrl1 = ultimo.ctrl2 = waypoints.back();
    rutaPerro.push_back(ultimo);

    // Calcula tangentes suaves para toda la ruta generada
    CalcularControles(rutaPerro);

    printf("Ruta perro: %d puntos generados.\n", (int)rutaPerro.size());

}

// ── Reset al inicio de la ruta ────────────────────────────────
// ── Reset al inicio ───────────────────────────────────────────
void ResetPerro()
{
    if (rutaPerro.empty()) return;
    perroPosX = rutaPerro[0].pos.x;
    perroPosY = rutaPerro[0].pos.y;
    perroPosZ = rutaPerro[0].pos.z;
    perroRotY = 0.0f;
    perroFLegs = perroRLegs = perroHead = perroTail = 0.0f;
    perroSegmento = perroCurrSteps = 0;
}

// ── Función principal: llámala en el game loop ────────────────
void AnimacionPerro()
{
    if (!perroPlay || rutaPerro.empty()) return;

    int total = (int)rutaPerro.size() - 1;

    if (perroCurrSteps >= perroMaxSteps)
    {
        perroSegmento++;
        if (perroSegmento >= total)
        {
            printf("Perro: salio de escena.\n");
            perroPlay = perroVisible = false;
            perroSegmento = 0;
            return;
        }
        perroCurrSteps = 0;
    }

    float t = (float)perroCurrSteps / (float)perroMaxSteps;

    PuntoRuta& kf0 = rutaPerro[perroSegmento];
    PuntoRuta& kf1 = rutaPerro[perroSegmento + 1];

    float nuevoX = BezierCubica(kf0.pos.x, kf0.ctrl2.x, kf1.ctrl1.x, kf1.pos.x, t);
    float nuevoZ = BezierCubica(kf0.pos.z, kf0.ctrl2.z, kf1.ctrl1.z, kf1.pos.z, t);

    // Rotación automática: mira hacia donde avanza
    float dx = nuevoX - perroPosX;
    float dz = nuevoZ - perroPosZ;
    if (fabsf(dx) > 0.01f || fabsf(dz) > 0.01f)
        perroRotY = glm::degrees(atan2f(dx, dz));

    perroPosX = nuevoX;
    perroPosZ = nuevoZ;
    perroPosY = kf0.pos.y;

    // Patas: ciclo de carrera con seno
    float tiempo = (float)glfwGetTime();
    perroFLegs = 45.0f * sinf(tiempo * 10.0f);
    perroRLegs = 45.0f * sinf(tiempo * 10.0f + glm::radians(90.0f));
    perroHead = 8.0f * sinf(tiempo * 5.0f);
    perroTail = 35.0f * sinf(tiempo * 6.0f);

    perroCurrSteps++;
}