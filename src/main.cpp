#include "app.h"
#include "sdl.h"

int main()
{
    SDLState<App> sdlState;

    if (!sdlState.init())
        return EXIT_FAILURE;

    sdlState.run();
    return EXIT_SUCCESS;
}
