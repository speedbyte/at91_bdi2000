#ifndef YERROR_H
#define YERROR_H

#include <stdbool.h>

// For the die() and Debug() function.  Used to shorten the call.
//
#define die if(\
	(die_filename = __FILE__) && \
	(die_function = __FUNCTION__) && \
	(die_line = __LINE__) \
) die_for_reals

#define Debug if(\
	(die_filename = __FILE__) && \
	(die_function = __FUNCTION__) && \
	(die_line = __LINE__) \
) debug_for_reals

// Globals
extern const char *die_filename;
extern const char *die_function;
extern int        die_line;
extern bool       debug;


// Prototypes
void debug_for_reals(const char *fmt, ...);
void die_for_reals(const char *fmt, ...);
void Warn(const char *fmt, ...);

#endif
