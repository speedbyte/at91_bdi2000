#include <SDL/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "init3d.h"
#include "input.h"
#include "main.h"
#include "render.h"
#include "video.h"
#include "yerror.h"



int main( void )
{
	init_sdl();
	init_gl();
	init_game();
	MainLoop();

	return 0;
}



void MainLoop( void )
{
	bool done     = false;
	bool isActive = true;
	SDL_Event event;

	while(! done)
	{
		while( SDL_PollEvent(& event) )
		{
			switch ( event.type )
			{
				case SDL_ACTIVEEVENT:
					if ( event.active.gain == 0 ) isActive = false;
					else                          isActive = true;
					break;

				case SDL_QUIT:
					done = true;
					break;

				case SDL_KEYDOWN:
					HandleKeyPress( &event.key.keysym );
					break;

				case SDL_KEYUP:
					HandleKeyRelease( &event.key.keysym );
					break;

				case SDL_VIDEORESIZE:
					surface = SDL_SetVideoMode( event.resize.w, event.resize.h,
							SCREEN_DEPTH, VideoFlags );

					if ( !surface )
						die( "No surface after resize: %s\n", SDL_GetError() );

					SizeOpenGLScreen( event.resize.w, event.resize.h );
    
					if( surface == NULL)
						die( "Failed SDL win resize: %s", SDL_GetError() );

					break;

				default:
					break;

			}
		}

		RenderScene();
	}
}


// vim: tw=80
