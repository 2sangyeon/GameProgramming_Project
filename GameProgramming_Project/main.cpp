#include "Game.h"

#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2_image.lib")

int main(int argc, char** argv)
{
    Game game;

    if (game.Initialize())
    {
        game.RunLoop();
    }

    game.Shutdown();
    return 0;
}