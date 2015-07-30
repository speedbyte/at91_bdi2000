# You'll probably want to change CC from colorgcc to gcc.  However, I'll tell
# you right here and now, if you don't use colorgcc it's because you don't
# know about colorgcc.  I *highly* recommend colorgcc.
CC     = colorgcc
WARN   = -W -Wall -Wmissing-prototypes -Waggregate-return -Wpointer-arith \
	-Wcast-qual -Wcast-align -Wmissing-declarations -pedantic -Wnested-externs \
	-Wredundant-decls -Wwrite-strings -Winline
CFLAGS = -g $(WARN) -std=c99
LDLIBS = -lm
TARGET1 = deriv
TARGET2 = deriv-broken


all: $(TARGET1) $(TARGET2)
	ctags *.c *.h

$(TARGET1): $(TARGET1).o derivdriver.o
$(TARGET2): $(TARGET2).o derivdriver-broken.o


.PHONY: clean nuke

clean:
nuke:
	$(RM) $(TARGET1) $(TARGET2) *o tags core
