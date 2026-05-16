#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in ivec4 boneIds;
layout (location = 4) in vec4 weights;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main() {
    vec4 totalPosition = vec4(0.0);
    vec3 totalNormal   = vec3(0.0);

    for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        if (boneIds[i] < 0) continue;
        if (boneIds[i] >= MAX_BONES) {
            totalPosition = vec4(position, 1.0);
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(position, 1.0);
        totalPosition     += localPosition * weights[i];
        vec3 localNormal   = mat3(finalBonesMatrices[boneIds[i]]) * normal;
        totalNormal       += localNormal * weights[i];
    }

    gl_Position = projection * view * model * totalPosition;
    FragPos     = vec3(model * totalPosition);
    Normal      = normalize(mat3(transpose(inverse(model))) * totalNormal);
    TexCoords   = texCoords;
}