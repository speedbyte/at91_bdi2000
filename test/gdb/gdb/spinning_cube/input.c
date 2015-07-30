// NOTES
// +-=-+
//
// 1. These functions must be defined elsewhere
extern void ToggleFullScreen(void);
extern void Quit(int);

#include <SDL/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include "input.h"
#include "yerror.h"

bool
upPressed    = false,
downPressed  = false,
leftPressed  = false,
rightPressed = false,
aPressed     = false,
zPressed     = false,
qPressed     = false,
wPressed     = false;





void HandleKeyPress(SDL_keysym *keysym)
{
	switch(keysym->sym)
	{
		case SDLK_a:      aPressed     = true; break;
		case SDLK_z:      zPressed     = true; break;
		case SDLK_q:      qPressed     = true; break;
		case SDLK_w:      wPressed     = true; break;
		case SDLK_UP:     upPressed    = true; break;
		case SDLK_DOWN:   downPressed  = true; break;
		case SDLK_LEFT:   leftPressed  = true; break;
		case SDLK_RIGHT:  rightPressed = true; break;
		case SDLK_F1:     ToggleFullScreen();  break;
		case SDLK_ESCAPE: exit(0);             break;
		default:                               break;
    }
	Debug("hello!");
}





void HandleKeyRelease(SDL_keysym *keysym)
{
	switch(keysym->sym)
	{
		case SDLK_a:     aPressed     = false; break;
		case SDLK_z:     zPressed     = false; break;
		case SDLK_q:     qPressed     = false; break;
		case SDLK_w:     wPressed     = false; break;
		case SDLK_UP:    upPressed    = false; break;
		case SDLK_DOWN:  downPressed  = false; break;
		case SDLK_RIGHT: rightPressed = false; break;
		case SDLK_LEFT:  leftPressed  = false; break;
		default:                               break;
	}
}
