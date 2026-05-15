#include <stdexcept>

#include <SDL3/SDL_mouse.h>

#include <glm/gtc/matrix_transform.hpp>

#include "camera/camera.hpp"


void Camera::processEvent(const SDL_Event &event, SDL_Window *window)
{
    if (!window)
    {
        throw std::runtime_error("Camera::processEvent(): SDL_Window is NULL!");
    }

    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        switch (event.key.key)
        {
            case SDLK_W:
                m_moveForward = true;
                break;
            
            case SDLK_A:
                m_moveLeft = true;
                break;
            
            case SDLK_S:
                m_moveBackward = true;
                break;
            
            case SDLK_D:
                m_moveRight = true;
                break;
            
            case SDLK_SPACE:
                m_moveUp = true;
                break;
            
            case SDLK_LCTRL:
                m_moveDown = true;
                break;
            
            case SDLK_ESCAPE:
                if (SDL_GetWindowRelativeMouseMode(window))
                {
                    SDL_SetWindowRelativeMouseMode(window, false);
                }
                else
                {
                    SDL_SetWindowRelativeMouseMode(window, true);
                }

                if (!SDL_CursorVisible())
                {
                    SDL_ShowCursor();
                }
                else
                {
                    SDL_HideCursor();
                }
                break;
            default:
                break;
        }
    }
    else if (event.type == SDL_EVENT_KEY_UP)
    {
        switch (event.key.key)
        {
            case SDLK_W:
                m_moveForward = false;
                break;
            
            case SDLK_A:
                m_moveLeft = false;
                break;
        
            case SDLK_S:
                m_moveBackward = false;
                break;
        
            case SDLK_D:
                m_moveRight = false;
                break;
        
            case SDLK_SPACE:
                m_moveUp = false;
                break;
        
            case SDLK_LCTRL:
                m_moveDown = false;
                break;
        
            default: break;
        }
    }
    else if (event.type == SDL_EVENT_MOUSE_MOTION)
    {
        m_yaw += static_cast<float>(event.motion.xrel) * m_mouseSensitivity;
        m_pitch -= static_cast<float>(event.motion.yrel) * m_mouseSensitivity;
        m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);
    }
}

void Camera::update(const float deltaTime)
{
    const float velocity = m_speed * deltaTime;

    const glm::vec3 front = glm::normalize(glm::vec3(
        glm::cos(glm::radians(m_pitch)) * glm::cos(glm::radians(m_yaw)),
        glm::sin(glm::radians(m_pitch)),
        glm::cos(glm::radians(m_pitch)) * glm::sin(glm::radians(m_yaw))
    ));

    const glm::vec3 right = glm::normalize(glm::cross(front, m_up));

    if (m_moveForward)
    {
        m_position += front * velocity;
    }
    
    if (m_moveBackward)
    {
        m_position -= front * velocity;
    }
    
    if (m_moveLeft)
    {
        m_position -= right * velocity;
    }
    
    if (m_moveRight)
    {
        m_position += right * velocity;
    }
    
    if (m_moveUp)
    {
        m_position += m_up * velocity;
    }
    
    if (m_moveDown)
    {
        m_position -= m_up * velocity; 
    }

    m_front = front;
}

glm::mat4 Camera::viewMatrix(void) const
{
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::projectionMatrix(const float aspectRatio) const
{
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1500.0f);
    
    /* GLM is originally for OpenGL, so for everything to not be upside down in Vulkan we must reverse the projection matrix */
    projection[1][1] *= -1;
    
    return projection;
}
