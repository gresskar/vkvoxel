#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
#include <glm/glm.hpp>

class Camera
{
public:
    Camera() = default;

    void processEvent(const SDL_Event &event, SDL_Window *window);
    void update(const float deltaTime);

    glm::mat4 viewMatrix(void) const;
    glm::mat4 projectionMatrix(const float aspectRatio) const;

private:
    glm::vec3 m_position { 0.0f, 44.0f, -96.0f };
    glm::vec3 m_front { 0.0f, -0.24f, 0.97f };
    glm::vec3 m_up { 0.0f, 1.0f, 0.0f };

    float m_yaw = 90.0f;
    float m_pitch = -14.0f;
    
    float m_speed = 32.0f;
    float m_mouseSensitivity = 0.1f;

    bool m_moveForward = false;
    bool m_moveBackward = false;
    bool m_moveLeft = false;
    bool m_moveRight = false;
    bool m_moveUp = false;
    bool m_moveDown = false;
};
