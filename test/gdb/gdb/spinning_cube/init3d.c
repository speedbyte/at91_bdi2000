#include <GL/gl.h>
#include <GL/glu.h>
#include <stdbool.h>
#include <stdlib.h>
#include "init3d.h"
#include "main.h"
#include "video.h"
#include "yerror.h"

const char *ImageName[ NUM_TEXTURES ] = {
	"data/juliette-0.bmp",
	"data/juliette-1.bmp",
	"data/rhonda-0.bmp",
	"data/rhonda-1.bmp",
	"data/geordi-0.bmp",
	"data/geordi-1.bmp",
};



void init_sdl( void )
{
	if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
		die( "SDL init failed: %s\n", SDL_GetError( ) );

	/*
	 *  Setup Pixel Format
	 */
	VideoFlags  = SDL_OPENGL;          /* Enable OpenGL in SDL */
	VideoFlags |= SDL_HWPALETTE;       /* Store the palette in hardware */
	VideoFlags |= SDL_RESIZABLE;       /* Enable window resizing */

	// Query SDL for information about our video hardware
	const SDL_VideoInfo *VideoInfo = SDL_GetVideoInfo();

	if ( ! VideoInfo ) die( "Video query failed: %s\n", SDL_GetError() );

	// Check to see if surfaces can be stored in video memory
	if ( VideoInfo->hw_available ) VideoFlags |= SDL_HWSURFACE;
	else                           VideoFlags |= SDL_SWSURFACE;

	// Checks if hardware blits can be done
	if ( VideoInfo->blit_hw ) VideoFlags |= SDL_HWACCEL;

	// Sets up OpenGL double buffering
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	// size of depth buffer
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, SCREEN_DEPTH);

	// we aren't going to use the stencil buffer
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 0);

	// this and the next three lines set the bits allocated per pixel -
	SDL_GL_SetAttribute( SDL_GL_ACCUM_RED_SIZE, 0);
	// - for the accumulation buffer to 0
	SDL_GL_SetAttribute( SDL_GL_ACCUM_GREEN_SIZE, 0);
	SDL_GL_SetAttribute( SDL_GL_ACCUM_BLUE_SIZE, 0);
	SDL_GL_SetAttribute( SDL_GL_ACCUM_ALPHA_SIZE, 0);

	/*
	 *  Create Surface
	 */
	surface = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH, VideoFlags);
	if( surface == NULL ) die( "Video mode set failed: %s\n", SDL_GetError( ) );
	SDL_WM_SetCaption("caption A", "caption B");
	atexit(Quit);
}




void init_gl( void )
{
	SizeOpenGLScreen( SCREEN_WIDTH, SCREEN_HEIGHT);
	LoadGLTextures();
	glEnable( GL_TEXTURE_2D );
	glShadeModel( GL_SMOOTH );
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	glClearDepth( 1.0f );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
}




void init_game( void )
{
	printf("Hit F1 Toggle Fullscreen.\nHit ESC to quit.\n");
}




void LoadGLTextures( void )
{

	SDL_Surface *TextureImage[ NUM_TEXTURES ];

	glGenTextures( NUM_TEXTURES, texture );



	for ( int i = 0; i < NUM_TEXTURES; ++i )
	{
		TextureImage[i] = SDL_LoadBMP( ImageName[i] );
		if ( TextureImage[i] == NULL )
			die("SDL_Surface %d is null.\n", i);

		glBindTexture( GL_TEXTURE_2D, texture[i] );

		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[i]->w,
			TextureImage[i]->h, 0, GL_BGR, GL_UNSIGNED_BYTE,
			TextureImage[i]->pixels );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		SDL_FreeSurface( TextureImage[i] );
	}

}






void Quit( void )
{
	SDL_Quit( );
	exit( 0 );
}
