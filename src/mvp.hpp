#pragma once

#include <glm/glm.hpp>

/* TODO: replace with push constant? */
struct MVP
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};
