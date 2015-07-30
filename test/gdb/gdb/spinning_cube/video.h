#ifndef VIDEO_H
#define VIDEO_H

#include <GL/glu.h>

enum _TextureNames {
	TEXTURE_JULIETTE0,
	TEXTURE_JULIETTE1,
	TEXTURE_RHONDA1,
	TEXTURE_RHONDA2,
	TEXTURE_GEORDI0,
	TEXTURE_GEORDI1,
	NUM_TEXTURES
};


extern SDL_Surface *surface;
extern int          VideoFlags;
extern GLuint       texture[2];
extern GLenum       errCode;

void CalculateFrameRate( void );
void ToggleFullScreen( void );
int  SizeOpenGLScreen( int width, int height );

#endif
