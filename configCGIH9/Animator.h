#pragma once
#include <vector>
#include <map>
#include <string>
#include <cassert>

#include <GL/glew.h>
#define GLM_ENABLE_EXPERIMENTAL   // ← ANTES de cualquier include de glm
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>        // ← value_ptr

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "BoneInfo.h"

using namespace std;

// Nodo del árbol de huesos
struct AssimpNodeData {
    glm::mat4 transformation;
    string name;
    vector<AssimpNodeData> children;
};

// ---- Un solo hueso con sus keyframes ----
class Bone {
public:
    string name;
    int id;
    glm::mat4 localTransform;

    struct KeyPosition { glm::vec3 pos;   float time; };
    struct KeyRotation { glm::quat rot;   float time; };
    struct KeyScale { glm::vec3 scale; float time; };

    vector<KeyPosition> positions;
    vector<KeyRotation> rotations;
    vector<KeyScale>    scales;

    Bone(const string& name, int id, const aiNodeAnim* channel) : name(name), id(id), localTransform(1.0f) {
        for (unsigned i = 0; i < channel->mNumPositionKeys; i++)
            positions.push_back({ toVec3(channel->mPositionKeys[i].mValue), (float)channel->mPositionKeys[i].mTime });
        for (unsigned i = 0; i < channel->mNumRotationKeys; i++)
            rotations.push_back({ toQuat(channel->mRotationKeys[i].mValue), (float)channel->mRotationKeys[i].mTime });
        for (unsigned i = 0; i < channel->mNumScalingKeys; i++)
            scales.push_back({ toVec3(channel->mScalingKeys[i].mValue), (float)channel->mScalingKeys[i].mTime });
    }

    void Update(float animTime) {
        glm::mat4 T = InterpolatePosition(animTime);
        glm::mat4 R = InterpolateRotation(animTime);
        glm::mat4 S = InterpolateScale(animTime);
        localTransform = T * R * S;
    }

private:
    static glm::vec3 toVec3(const aiVector3D& v) { return { v.x, v.y, v.z }; }
    static glm::quat toQuat(const aiQuaternion& q) { return { q.w, q.x, q.y, q.z }; }

    int GetIndex(const vector<KeyPosition>& keys, float t) {
        for (int i = 0; i < (int)keys.size() - 1; i++) if (t < keys[i + 1].time) return i;
        return 0;
    }
    int GetIndex(const vector<KeyRotation>& keys, float t) {
        for (int i = 0; i < (int)keys.size() - 1; i++) if (t < keys[i + 1].time) return i;
        return 0;
    }
    int GetIndex(const vector<KeyScale>& keys, float t) {
        for (int i = 0; i < (int)keys.size() - 1; i++) if (t < keys[i + 1].time) return i;
        return 0;
    }
    float Factor(float t0, float t1, float t) { return (t - t0) / (t1 - t0); }

    glm::mat4 InterpolatePosition(float t) {
        if (positions.size() == 1) return glm::translate(glm::mat4(1.0f), positions[0].pos);
        int i = GetIndex(positions, t);
        float f = Factor(positions[i].time, positions[i + 1].time, t);
        glm::vec3 p = glm::mix(positions[i].pos, positions[i + 1].pos, f);
        return glm::translate(glm::mat4(1.0f), p);
    }
    glm::mat4 InterpolateRotation(float t) {
        if (rotations.size() == 1) return glm::toMat4(glm::normalize(rotations[0].rot));
        int i = GetIndex(rotations, t);
        float f = Factor(rotations[i].time, rotations[i + 1].time, t);
        glm::quat r = glm::slerp(rotations[i].rot, rotations[i + 1].rot, f);
        return glm::toMat4(glm::normalize(r));
    }
    glm::mat4 InterpolateScale(float t) {
        if (scales.size() == 1) return glm::scale(glm::mat4(1.0f), scales[0].scale);
        int i = GetIndex(scales, t);
        float f = Factor(scales[i].time, scales[i + 1].time, t);
        glm::vec3 s = glm::mix(scales[i].scale, scales[i + 1].scale, f);
        return glm::scale(glm::mat4(1.0f), s);
    }
};

// ---- Carga UNA animación del FBX ----
class Animation {
public:
    float duration;
    float ticksPerSecond;
    vector<Bone> bones;
    AssimpNodeData rootNode;
    map<string, BoneInfo>& boneInfoMap;

    Animation(const string& path, map<string, BoneInfo>& boneMap, int animIndex = 0)
        : boneInfoMap(boneMap)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);
        //assert(scene && scene->mNumAnimations > animIndex);
        if (!scene || scene->mNumAnimations == 0) {
            cout << "ERROR: El FBX no contiene animaciones!" << endl;
            duration = 0; ticksPerSecond = 30;
            return;
        }
        aiAnimation* anim = scene->mAnimations[animIndex];
        duration = (float)anim->mDuration;
        ticksPerSecond = anim->mTicksPerSecond != 0 ? (float)anim->mTicksPerSecond : 25.0f;
        ReadHierarchy(rootNode, scene->mRootNode);
        ReadBones(anim);
    }

    Bone* FindBone(const string& name) {
        for (auto& b : bones) if (b.name == name) return &b;
        return nullptr;
    }

private:
    void ReadBones(const aiAnimation* anim) {
        for (unsigned i = 0; i < anim->mNumChannels; i++) {
            auto* ch = anim->mChannels[i];
            string name = ch->mNodeName.data;
            if (boneInfoMap.find(name) == boneInfoMap.end()) {
                BoneInfo bi; bi.id = (int)boneInfoMap.size(); bi.offsetMatrix = glm::mat4(1.0f);
                boneInfoMap[name] = bi;
            }
            bones.emplace_back(name, boneInfoMap[name].id, ch);
        }
    }
    void ReadHierarchy(AssimpNodeData& dest, const aiNode* src) {
        dest.name = src->mName.data;
        dest.transformation = toMat4(src->mTransformation);
        for (unsigned i = 0; i < src->mNumChildren; i++) {
            AssimpNodeData child;
            ReadHierarchy(child, src->mChildren[i]);
            dest.children.push_back(child);
        }
    }
    static glm::mat4 toMat4(const aiMatrix4x4& m) {
        return glm::mat4(m.a1, m.b1, m.c1, m.d1, m.a2, m.b2, m.c2, m.d2,
            m.a3, m.b3, m.c3, m.d3, m.a4, m.b4, m.c4, m.d4);
    }
};

// ---- Animator: actualiza las matrices finales cada frame ----
class Animator {
public:
    vector<glm::mat4> finalBoneMatrices;
    Animation* currentAnimation = nullptr;
    float currentTime = 0.0f;
    bool  playing = false;
    bool  loop = true;

    Animator() { finalBoneMatrices.resize(100, glm::mat4(1.0f)); }

    void PlayAnimation(Animation* anim) {
        currentAnimation = anim;
        currentTime = 0.0f;
        playing = true;
    }

    void Update(float deltaTime) {
        if (!playing || !currentAnimation) return;
        currentTime += currentAnimation->ticksPerSecond * deltaTime;
        if (currentTime >= currentAnimation->duration) {
            if (loop) currentTime = fmod(currentTime, currentAnimation->duration);
            else { currentTime = currentAnimation->duration; playing = false; }
        }
        CalculateBoneTransform(&currentAnimation->rootNode, glm::mat4(1.0f));
    }

private:
    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform) {
        glm::mat4 nodeTransform = node->transformation;
        Bone* bone = currentAnimation->FindBone(node->name);
        if (bone) { bone->Update(currentTime); nodeTransform = bone->localTransform; }
        glm::mat4 globalTransform = parentTransform * nodeTransform;
        auto& boneMap = currentAnimation->boneInfoMap;
        if (boneMap.find(node->name) != boneMap.end()) {
            int idx = boneMap[node->name].id;
            finalBoneMatrices[idx] = globalTransform * boneMap[node->name].offsetMatrix;
        }
        for (auto& child : node->children)
            CalculateBoneTransform(&child, globalTransform);
    }
};
