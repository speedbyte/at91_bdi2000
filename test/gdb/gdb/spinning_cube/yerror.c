// NOTES
// +-=-+
// 1. Quit() is really program dependent; it's where we free memory for various
// 	dynamic things that we create.  Make sure we use "atexit" to call it when
// 	the program quits because we do NOT call it here for die()!


#include <SDL/SDL.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "yerror.h"


// Global Debugging/Dying Variables
const char *die_filename;
const char *die_function;
int        die_line;
bool       debug = true;


// Extern declarations.  Put them HERE because this module is very general; I
// I use it in all my programs, and don't want to have to modify it every time
// it's used in another program.  Most effective code reuse is "drop in" code
// without fiddling with include statements.
//
extern void Quit(int retval);





void die_for_reals(const char *fmt, ...)
{
	int die_errno = errno;
	va_list args;

	va_start(args, fmt);
	fprintf(stderr, "\n\nFatal Error:\n");
	vfprintf(stderr, fmt, args);
	fprintf(stderr,  "\n   File: %s, Function: %s(), Line: %d\n",
		die_filename, die_function, die_line);
	fprintf(stderr,  "   strerror reports: %s\n", strerror(die_errno));
	fprintf(stderr,  "   SDL reports: %s\n   ", SDL_GetError());
	fprintf(stderr, "\n");
	va_end(args);
	exit(0);
}





void debug_for_reals(const char *fmt, ...)
{
	if ( ! debug ) return;

	va_list args;

	va_start(args, fmt);
	fprintf(stderr, "\n");
	vfprintf(stderr, fmt, args);
	fprintf(stderr,  "\n   File: %s, Function: %s(), Line: %d\n\n",
		die_filename, die_function, die_line);
	va_end(args);
	fflush(stderr);
}





void Warn(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	fprintf(stderr, "\nWarning: ");
	vfprintf(stderr, fmt, args);
	va_end(args);
	putchar('\n');
}
