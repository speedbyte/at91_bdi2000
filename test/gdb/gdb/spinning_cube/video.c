#include "main.h"
#include "video.h"
#include "yerror.h"

SDL_Surface *surface;
int          VideoFlags;
GLenum       errCode;
GLuint       texture[2];





void CalculateFrameRate( void )
{
	static float fps = 0.0f;
	static float tock = 0.0f;
	static char  string[50] = {0};
	float tick = SDL_GetTicks() * 0.001f;

	++fps;

	if( tick - tock > 1.0f )
	{
		tock = tick;
			  
		sprintf(string, "Current FPS: %d", (int)fps);

		SDL_WM_SetCaption(string, string);

		fps = 0;
	}
}





int SizeOpenGLScreen( int width, int height )
{
	GLfloat AspectRatio;

	if ( height == 0 )
		height = 1;

	AspectRatio = ( GLfloat )width / ( GLfloat )height;

	glViewport( 0, 0, ( GLint )width, ( GLint )height );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, AspectRatio, 0.1f, 100.0f );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	return true;
}





void ToggleFullScreen( void )
{
	if(SDL_WM_ToggleFullScreen(surface) == 0)
		die("Failed to Toggle Fullscreen mode: %s\n", SDL_GetError());
}





// vim: tw=80
