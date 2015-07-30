#include <GL/gl.h>
#include <SDL/SDL.h>
#include "render.h"
#include "video.h"





int RenderScene( void )
{
	// Used to rotate the cube about its axis.
	//
	static GLfloat xrot = 0.0f, yrot = 0.0f, zrot = 0.0f;
	static GLfloat x  = 0.0f, y  = 0.0f, z  = -3.0f;
	static GLfloat vx = 0.002f, vy = 0.004f, vz =  0.007f;

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glLoadIdentity();

	// glTranslatef( 0.0f, 0.0f, -5.0f );
	glTranslatef( x, y, z );

	glRotatef( xrot, 1.0f, 0.0f, 0.0f );
	glRotatef( yrot, 0.0f, 1.0f, 0.0f );
	glRotatef( zrot, 0.0f, 0.0f, 1.0f );



	glBindTexture( GL_TEXTURE_2D, texture[ TEXTURE_JULIETTE0 ] );

	glBegin( GL_QUADS ); /* Front Face */

      /* Bottom Left Of The Texture and Quad */
      glTexCoord2f( 0.0f, 1.0f ); glVertex3f( -1.0f, -1.0f, 1.0f );
      /* Bottom Right Of The Texture and Quad */
      glTexCoord2f( 1.0f, 1.0f ); glVertex3f(  1.0f, -1.0f, 1.0f );
      /* Top Right Of The Texture and Quad */
      glTexCoord2f( 1.0f, 0.0f ); glVertex3f(  1.0f,  1.0f, 1.0f );
      /* Top Left Of The Texture and Quad */
      glTexCoord2f( 0.0f, 0.0f ); glVertex3f( -1.0f,  1.0f, 1.0f );

	glEnd();



	glBindTexture( GL_TEXTURE_2D, texture[ TEXTURE_JULIETTE1 ] );

	glBegin( GL_QUADS ); /* Back Face */

      /* Bottom Right Of The Texture and Quad */
      glTexCoord2f( 0.0f, 0.0f ); glVertex3f( -1.0f, -1.0f, -1.0f );
      /* Top Right Of The Texture and Quad */
      glTexCoord2f( 0.0f, 1.0f ); glVertex3f( -1.0f,  1.0f, -1.0f );
      /* Top Left Of The Texture and Quad */
      glTexCoord2f( 1.0f, 1.0f ); glVertex3f(  1.0f,  1.0f, -1.0f );
      /* Bottom Left Of The Texture and Quad */
      glTexCoord2f( 1.0f, 0.0f ); glVertex3f(  1.0f, -1.0f, -1.0f );

	glEnd();



	glBindTexture( GL_TEXTURE_2D, texture[ TEXTURE_RHONDA1 ] );

	glBegin( GL_QUADS ); /* Top Face */

      /* Top Left Of The Texture and Quad */
      glTexCoord2f( 1.0f, 1.0f ); glVertex3f( -1.0f,  1.0f, -1.0f );
      /* Bottom Left Of The Texture and Quad */
      glTexCoord2f( 1.0f, 0.0f ); glVertex3f( -1.0f,  1.0f,  1.0f );
      /* Bottom Right Of The Texture and Quad */
      glTexCoord2f( 0.0f, 0.0f ); glVertex3f(  1.0f,  1.0f,  1.0f );
      /* Top Right Of The Texture and Quad */
      glTexCoord2f( 0.0f, 1.0f ); glVertex3f(  1.0f,  1.0f, -1.0f );

	glEnd();



	glBindTexture( GL_TEXTURE_2D, texture[ TEXTURE_RHONDA2 ] );

	glBegin( GL_QUADS ); /* Bottom Face */

      /* Top Right Of The Texture and Quad */
      glTexCoord2f( 0.0f, 1.0f ); glVertex3f( -1.0f, -1.0f, -1.0f );
      /* Top Left Of The Texture and Quad */
      glTexCoord2f( 1.0f, 1.0f ); glVertex3f(  1.0f, -1.0f, -1.0f );
      /* Bottom Left Of The Texture and Quad */
      glTexCoord2f( 1.0f, 0.0f ); glVertex3f(  1.0f, -1.0f,  1.0f );
      /* Bottom Right Of The Texture and Quad */
      glTexCoord2f( 0.0f, 0.0f ); glVertex3f( -1.0f, -1.0f,  1.0f );

	glEnd();



	glBindTexture( GL_TEXTURE_2D, texture[ TEXTURE_GEORDI0 ] );

	glBegin( GL_QUADS ); /* Right face */

      /* Bottom Right Of The Texture and Quad */
      glTexCoord2f( 0.0f, 0.0f ); glVertex3f( 1.0f, -1.0f, -1.0f );
      /* Top Right Of The Texture and Quad */
      glTexCoord2f( 0.0f, 1.0f ); glVertex3f( 1.0f,  1.0f, -1.0f );
      /* Top Left Of The Texture and Quad */
      glTexCoord2f( 1.0f, 1.0f ); glVertex3f( 1.0f,  1.0f,  1.0f );
      /* Bottom Left Of The Texture and Quad */
      glTexCoord2f( 1.0f, 0.0f ); glVertex3f( 1.0f, -1.0f,  1.0f );

	glEnd();



	glBindTexture( GL_TEXTURE_2D, texture[ TEXTURE_GEORDI1 ] );

	glBegin( GL_QUADS ); /* Left Face */

      /* Bottom Left Of The Texture and Quad */
      glTexCoord2f( 1.0f, 0.0f ); glVertex3f( -1.0f, -1.0f, -1.0f );
      /* Bottom Right Of The Texture and Quad */
      glTexCoord2f( 0.0f, 0.0f ); glVertex3f( -1.0f, -1.0f,  1.0f );
      /* Top Right Of The Texture and Quad */
      glTexCoord2f( 0.0f, 1.0f ); glVertex3f( -1.0f,  1.0f,  1.0f );
      /* Top Left Of The Texture and Quad */
      glTexCoord2f( 1.0f, 1.0f ); glVertex3f( -1.0f,  1.0f, -1.0f );

	glEnd();


	SDL_GL_SwapBuffers( );

	xrot += 0.3f;  yrot += 0.2f;  zrot += 0.4f; 
	x += vx;       y += vy;       z += vz; 

	if (x <  -2.0f  ||  x >  2.0f)  vx *= -1.0f;
	if (y <  -2.0f  ||  y >  2.0f)  vy *= -1.0f;
	if (z < -15.0f  ||  z > -2.0f)  vz *= -1.0f;

	CalculateFrameRate();

	return 0;
}
