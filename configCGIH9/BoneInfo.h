#pragma once

#include <glm/glm.hpp>
#define MAX_BONE_INFLUENCE 4

struct BoneInfo
{
    int  id;
    glm::mat4 offsetMatrix;
};