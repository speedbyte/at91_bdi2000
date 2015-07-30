TARGET = 07-texturemap
CFILES = $(wildcard *.c)
OFILES = $(patsubst %.c, %.o, $(CFILES))
DFILES = $(patsubst %.c, .deps/%.d, $(CFILES))
CC     = colorgcc
WARN   = -W -Wall -Wstrict-prototypes -Wmissing-prototypes -Waggregate-return \
	-Wpointer-arith -Wcast-qual -Wcast-align -Wmissing-declarations \
	-Wnested-externs -Wredundant-decls -Wwrite-strings -Winline #-Werror
CFLAGS = -std=c99 $(WARN) -g3 `sdl-config --cflags`
LDLIBS   = `sdl-config --libs` -lGL -lGLU -lm

all: $(TARGET)
	ctags *.c *.h
	./$(TARGET)

$(TARGET): $(OFILES)
	$(CC) -o $(TARGET) $(OFILES) $(LDLIBS)

$(TARGET): $(OFILES)

.deps/%.d: %.c
	@$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm $@.$$$$

ifneq ($(MAKECMDGOALS),clean)
-include $(DFILES)
endif



.PHONY: clean

clean:
	$(RM) $(TARGET) *.o core .deps/* tags
