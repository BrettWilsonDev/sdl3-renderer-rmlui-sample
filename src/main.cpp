#include <iostream>
#include "SDL3/SDL.h"

#include <RmlUi/Core.h>
#include <RmlUi/Core/Context.h>

#include "rmlui_backends/RmlUi_Backend_SDL_SDLrenderer.hpp" // The backend you showed earlier

#include <fstream>
#include <iostream>

#include "rmlui_backends/RobotoRegularFont.h"

int main(int argc, char **argv)
{
    // --- SDL Init ---
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Backend::Initialize creates the SDL window + renderer internally
    if (!Backend::Initialize("RmlUi Example", 800, 600, true))
    {
        std::cerr << "RmlUi backend initialization failed." << std::endl;
        return 1;
    }

    // --- RmlUi Init ---
    Rml::SetSystemInterface(Backend::GetSystemInterface());
    Rml::SetRenderInterface(Backend::GetRenderInterface());

    if (!Rml::Initialise())
    {
        std::cerr << "RmlUi Initialise() failed." << std::endl;
        return 1;
    }

    if (!LoadEmbeddedFont("Roboto", Roboto_Regular_ttf, Roboto_Regular_ttf_len))
    {
        std::cerr << "Failed to load embedded font!" << std::endl;
    }

    // Create a single context (like a root GUI layer)
    Rml::Context *context = Rml::CreateContext("main", Rml::Vector2i(800, 600));
    if (!context)
    {
        std::cerr << "Failed to create RmlUi context." << std::endl;
        return 1;
    }

    context->GetRootElement()->SetProperty("font-family", "Roboto");

    // --- Simple inline RML document ---
    const char *rml =
        R"(
            <rml>
                <body>
                    <p style="font-size: 102px; color: #fff; margin: 20px;">
                        Hello from <b>RmlUi</b> + SDL3!
                    </p>
                </body>
            </rml>
        )";

    Rml::ElementDocument *doc = context->CreateDocument();
    doc->SetInnerRML(rml);
    doc->Show();

    // --- Main loop ---
    bool running = true;
    while (running)
    {
        // Handle SDL input + route to RmlUi
        running = Backend::ProcessEvents(context);

        // Begin frame
        Backend::BeginFrame();

        // Update + render RmlUi
        context->Update();
        context->Render();

        // Finish frame (swap buffers)
        Backend::PresentFrame();
    }

    // Cleanup
    doc->Close();
    Rml::Shutdown();
    Backend::Shutdown();

    SDL_Quit();
    return 0;
}
