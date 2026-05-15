#include "app/app.hpp"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

constexpr int WIDTH = 1280;
constexpr int HEIGHT = 720;

void App::run(void)
{
    createWindow();

    const World::GenerationSettings settings =
    {
        .seed = 12081973,
        .chunkColumnsX = 32,
        .chunkColumnsZ = 16,
        .enableLevelOfDetail = false,
    };

    m_world.requestChunkGeneration(settings);

    m_renderer.init(m_window, m_world);
    m_lastTime = SDL_GetTicks();
    mainLoop();
    cleanup();
}

void App::createWindow(void)
{
    /* Initialize SDL's video subsystem - we need this for working with windows */
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
    {
        throw std::runtime_error("SDL_InitSubSystem() failed: " + std::string(SDL_GetError()));
    }

    /* Hide cursor */
    if (!SDL_HideCursor())
    {
        throw std::runtime_error("SDL_HideCursor() failed: " + std::string(SDL_GetError()));
    }

    /* Create a Vulkan-compatible SDL window */
    m_window = SDL_CreateWindow("VKVoxel", WIDTH, HEIGHT, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    
    if (!m_window)
    {
        throw std::runtime_error("SDL_CreateWindow() failed: " + std::string(SDL_GetError()));
    }

    /* Make it so the mouse can't leave the window */
    if (!SDL_SetWindowRelativeMouseMode(m_window, true))
    {
        throw std::runtime_error("SDL_SetWindowRelativeMouseMode() failed: " + std::string(SDL_GetError()));
    }
}

void App::mainLoop(void)
{
    SDL_Event event;
    bool shouldRun = true;

    while (shouldRun)
    {
        const uint64_t currentTime = SDL_GetTicks();
        const float deltaTime = static_cast<float>(currentTime - m_lastTime) / 1000.0f;
        m_lastTime = currentTime;

        /* Handle events */
        while (SDL_PollEvent(&event))
        {
            /* Exit the application if the user requested it, e.g. when the 'X' on the title bar is clicked, or Alt+F4 is pressed */
            if (event.type == SDL_EVENT_QUIT)
            {
                shouldRun = false;
                break;
            }

            /* Recreate the swap chain if the window is resized or minimized */
            if (event.window.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED || event.window.type == SDL_EVENT_WINDOW_MINIMIZED)
            {
                m_renderer.setFramebufferResized(true);
            }

            m_camera.processEvent(event, m_window);
        }

        /* Update camera and uniform state */
        m_camera.update(deltaTime);

        World::Mesh generatedMesh;
        if (m_world.consumeGeneratedMesh(generatedMesh))
        {
            m_renderer.updateWorldMesh(std::move(generatedMesh));
        }

        m_renderer.updateUniformBuffer(m_camera);
        
        /* Render frame */
        m_renderer.drawFrame();
    }
}

void App::cleanup(void)
{
    m_renderer.cleanup();

    SDL_DestroyWindow(m_window);
    m_window = NULL;

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_Quit();
}
