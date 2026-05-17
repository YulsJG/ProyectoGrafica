#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cmath>

// ─────────────────────────────────────────────────────────────
//  VARIABLES GLOBALES DEL PÁJARO
//  (accesibles desde main.cpp con extern)
// ─────────────────────────────────────────────────────────────

// Posición y orientación actual
float pajaroPosX = 0.0f;
float pajaroPosY = 0.0f;
float pajaroPosZ = 0.0f;
float pajaroRotY = 45.0f;   // rotación horizontal

// Ángulos de partes del cuerpo
float pajaroAlaAngle = 0.0f;   // alas (sube/baja en vuelo)
float pajaroCola = 0.0f;   // inclinación cola
float pajaroCabeza = 0.0f;   // giro cabeza (sacudida)
float pajaroCuerpoTilt = 0.0f;   // inclinación cuerpo (sacudida)

// Visibilidad y control
bool  pajaroVisible = false;
bool  pajaroPlay = false;

// ─────────────────────────────────────────────────────────────
//  POSICIÓN DEL STAND (destino del pájaro)
//  *** Ajusta estas coordenadas a tu escena ***
// ─────────────────────────────────────────────────────────────
const glm::vec3 PAJARO_STAND_POS = glm::vec3(67.6411f, -18.0f, 82.4836f);

// ─────────────────────────────────────────────────────────────
//  CURVA DE BÉZIER CÚBICA
// ─────────────────────────────────────────────────────────────
static glm::vec3 BezierCubica(float t,
    const glm::vec3& P0, const glm::vec3& P1,
    const glm::vec3& P2, const glm::vec3& P3)
{
    float u = 1.0f - t;
    return u * u * u * P0 + 3 * u * u * t * P1 + 3 * u * t * t * P2 + t * t * t * P3;
}

// Curva de entrada: aparece desde lejos hasta el stand
static glm::vec3 TrayectoriaEntrada(float t) {
	glm::vec3 P0(-61.0015f, -20.0f, 33.6627f);   // punto de aparición lejano, por donde entra el pájaro
    glm::vec3 P1(-20.0f, 15.0f, -30.0f);   // control 1
    glm::vec3 P2(20.0f, 5.0f, 60.0f);   // control 2
    glm::vec3 P3 = PAJARO_STAND_POS;        // destino: stand
    return BezierCubica(t, P0, P1, P2, P3);
}

// Curva de salida: despega del stand y desaparece
static glm::vec3 TrayectoriaSalida(float t) {
    glm::vec3 P0 = PAJARO_STAND_POS;
    glm::vec3 P1(PAJARO_STAND_POS.x + 20.0f, PAJARO_STAND_POS.y + 15.0f, PAJARO_STAND_POS.z + 20.0f);
    glm::vec3 P2(120.0f, 30.0f, 80.0f);
    glm::vec3 P3(165.914f, 10.0f, 94.1968f);    // fuera de escena, donde desaparece
    return BezierCubica(t, P0, P1, P2, P3);
}

// ─────────────────────────────────────────────────────────────
//  MÁQUINA DE ESTADOS
// ─────────────────────────────────────────────────────────────
enum EstadoPajaro {
    PAJARO_OCULTO = 0,
    PAJARO_VUELO_ENTRADA,   // 0  → 3 seg  : Bézier entrada
    PAJARO_ATERRIZAJE,      // 3  → 4 seg  : alas se cierran
    PAJARO_REPOSO,          // 4  → 5.5 seg: quieto sobre stand
    PAJARO_SACUDIDA,        // 5.5→ 7.5 seg: sacudida plumas
    PAJARO_DESPEGUE,        // 7.5→10  seg : Bézier salida
    PAJARO_FIN
};

EstadoPajaro pajaroEstado = PAJARO_OCULTO;
float pajaroTimer = 0.0f;   // tiempo dentro del estado actual

// Tiempos de cada estado (segundos)
const float T_ENTRADA = 3.5f;
const float T_ATERRIZAJE = 1.0f;
const float T_REPOSO = 1.5f;
const float T_SACUDIDA = 2.0f;
const float T_DESPEGUE = 2.5f;

// Parámetros de aleteo en vuelo
static float aleteoFase = 0.0f;   // fase acumulada para sin()
const  float ALETEO_VEL = 14.0f;   // velocidad angular del aleteo
const  float ALETEO_AMP = 50.0f;  // amplitud máxima en grados

// ─────────────────────────────────────────────────────────────
//  UTILIDADES
// ─────────────────────────────────────────────────────────────
static float Lerp(float a, float b, float t) { return a + (b - a) * t; }
static float Smoothstep(float t) { return t * t * (3.0f - 2.0f * t); }
static float Clamp01(float t) { return t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t); }

// ─────────────────────────────────────────────────────────────
//  INICIALIZACIÓN
// ─────────────────────────────────────────────────────────────
void InicializarPajaro() {
    pajaroEstado = PAJARO_OCULTO;
    pajaroTimer = 0.0f;
    pajaroVisible = false;
    pajaroPlay = false;
    aleteoFase = 0.0f;

    // Posición inicial fuera de escena
    glm::vec3 inicio = TrayectoriaEntrada(0.0f);
    pajaroPosX = inicio.x;
    pajaroPosY = inicio.y;
    pajaroPosZ = inicio.z;
}

// ─────────────────────────────────────────────────────────────
//  LÓGICA DE ANIMACIÓN — llamar cada frame desde main.cpp
// ─────────────────────────────────────────────────────────────
void AnimacionPajaro(float deltaTime) {

    if (!pajaroPlay || pajaroEstado == PAJARO_OCULTO || pajaroEstado == PAJARO_FIN)
        return;

    pajaroTimer += deltaTime;

    switch (pajaroEstado)
    {
        // ── VUELO DE ENTRADA ────────────────────────────────────
    case PAJARO_VUELO_ENTRADA:
    {
        float t = Clamp01(pajaroTimer / T_ENTRADA);
        glm::vec3 pos = TrayectoriaEntrada(t);
        pajaroPosX = pos.x;
        pajaroPosY = pos.y;
        pajaroPosZ = pos.z;

        // Orientar el pájaro hacia donde va (tangente de la curva)
        float dt2 = 0.01f;
        glm::vec3 posNext = TrayectoriaEntrada(Clamp01(t + dt2));
        glm::vec3 dir = posNext - pos;
        if (glm::length(dir) > 0.001f)
            pajaroRotY = glm::degrees(atan2(dir.x, dir.z));

        // Aleteo activo
        aleteoFase += ALETEO_VEL * deltaTime;
        pajaroAlaAngle = ALETEO_AMP * sinf(aleteoFase);
        pajaroCola = -8.0f;   // cola levantada en vuelo
        pajaroCabeza = 0.0f;

        if (pajaroTimer >= T_ENTRADA) {
            pajaroEstado = PAJARO_ATERRIZAJE;
            pajaroTimer = 0.0f;
        }
        break;
    }

    // ── ATERRIZAJE ──────────────────────────────────────────
    case PAJARO_ATERRIZAJE:
    {
        float t = Smoothstep(Clamp01(pajaroTimer / T_ATERRIZAJE));

        // Posición: ya está en el stand, solo leve rebote vertical
        pajaroPosX = PAJARO_STAND_POS.x;
        pajaroPosY = Lerp(PAJARO_STAND_POS.y + 2.0f, PAJARO_STAND_POS.y, t);
        pajaroPosZ = PAJARO_STAND_POS.z;

        // Alas se cierran suavemente
        pajaroAlaAngle = Lerp(ALETEO_AMP, 5.0f, t);
        pajaroCola = Lerp(-8.0f, 0.0f, t);

        if (pajaroTimer >= T_ATERRIZAJE) {
            pajaroEstado = PAJARO_REPOSO;
            pajaroTimer = 0.0f;
        }
        break;
    }

    // ── REPOSO ──────────────────────────────────────────────
    case PAJARO_REPOSO:
    {
        pajaroPosX = PAJARO_STAND_POS.x;
        pajaroPosY = PAJARO_STAND_POS.y;
        pajaroPosZ = PAJARO_STAND_POS.z;

        //pajaroAlaAngle = 5.0f;   // alas cerradas
        //pajaroCola = 0.0f;
        //pajaroCabeza = 0.0f;
        //pajaroCuerpoTilt = 0.0f;
        // Pequeño balanceo continuo mientras está parado
        float balanceo = sinf(pajaroTimer * 3.0f) * 3.0f;
        pajaroCuerpoTilt = balanceo;
        pajaroCabeza = sinf(pajaroTimer * 2.5f) * 5.0f;  // gira cabeza suave
        pajaroAlaAngle = 5.0f + fabsf(sinf(pajaroTimer * 2.0f)) * 8.0f; // alas respiran
        pajaroCola = sinf(pajaroTimer * 4.0f) * 4.0f;  // colita se mueve

        if (pajaroTimer >= T_REPOSO) {
            pajaroEstado = PAJARO_SACUDIDA;
            pajaroTimer = 0.0f;
        }
        break;
    }

    // ── SACUDIDA DE PLUMAS ───────────────────────────────────
    case PAJARO_SACUDIDA:
    {
        // Sacudida rápida usando seno de alta frecuencia
        float freq = 12.0f;
        float amp = 10.0f;
        float shake = amp * sinf(pajaroTimer * freq);

        pajaroAlaAngle = 15.0f + fabsf(shake);          // alas se abren/cierran
        pajaroCuerpoTilt = shake * 0.8f;                   // cuerpo se balancea
        pajaroCabeza = shake * 0.8f;                   // cabeza sacude
        pajaroCola = shake * 0.4f;

        // Hacia el final de la sacudida, se calma
        float calma = Clamp01(pajaroTimer / T_SACUDIDA);
        float factor = 1.0f - Smoothstep(calma);
        pajaroAlaAngle = 5.0f + fabsf(shake) * factor;
        pajaroCuerpoTilt *= factor;
        pajaroCabeza *= factor;
        pajaroCola *= factor;

        if (pajaroTimer >= T_SACUDIDA) {
            pajaroEstado = PAJARO_DESPEGUE;
            pajaroTimer = 0.0f;
            aleteoFase = 0.0f;
        }
        break;
    }

    // ── DESPEGUE ─────────────────────────────────────────────
    case PAJARO_DESPEGUE:
    {
        float t = Clamp01(pajaroTimer / T_DESPEGUE);
        glm::vec3 pos = TrayectoriaSalida(t);
        pajaroPosX = pos.x;
        pajaroPosY = pos.y;
        pajaroPosZ = pos.z;

        // Orientar según tangente de salida
        float dt2 = 0.01f;
        glm::vec3 posNext = TrayectoriaSalida(Clamp01(t + dt2));
        glm::vec3 dir = posNext - pos;
        if (glm::length(dir) > 0.001f)
            pajaroRotY = glm::degrees(atan2(dir.x, dir.z));

        // Aleteo activo
        aleteoFase += ALETEO_VEL * deltaTime;
        pajaroAlaAngle = ALETEO_AMP * sinf(aleteoFase);
        pajaroCola = -8.0f;
        pajaroCabeza = 0.0f;

        if (pajaroTimer >= T_DESPEGUE) {
            pajaroEstado = PAJARO_FIN;
            pajaroPlay = false;
            pajaroVisible = false;
        }
        break;
    }

    default: break;
    }
}

// ─────────────────────────────────────────────────────────────
//  ACTIVAR ANIMACIÓN (llamar desde KeyCallback)
// ─────────────────────────────────────────────────────────────
void ActivarPajaro() {
    InicializarPajaro();
    pajaroVisible = true;
    pajaroPlay = true;
    pajaroEstado = PAJARO_VUELO_ENTRADA;
    pajaroTimer = 0.0f;
}