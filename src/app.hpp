#pragma once

#include <SDL3/SDL_video.h>

#include "camera.hpp"
#include "renderer.hpp"
#include "world.hpp"

class App
{
public:
    void run(void);

private:
    void createWindow(void);
    void mainLoop(void);
    void cleanup(void);

    SDL_Window *m_window{ NULL };
    Renderer m_renderer{};
    Camera m_camera{};
    World m_world{};
    uint64_t m_lastTime{ 0 };
};
