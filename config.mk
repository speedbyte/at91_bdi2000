## Create ROM-Image (final)  
#RUN_MODE=ROM_RUN
## Create RAM-Image (debugging)  
RUN_MODE=RAM_RUN


#VECTOR_LOCATION=VECTORS_IN_ROM ## - Exception vectors in ROM:
VECTOR_LOCATION=VECTORS_IN_RAM  ## - Exception vectors in RAM:

#Format
FORMAT = binary

# target file name
TARGET = project


MCU      = arm920t
SUBMDL   = AT91RM9200
THUMB_IW = -mthumb-interwork

# Optimization level, can be [0, 1, 2, 3, s]. 
OPT 	= 0
#Debugging Information
DEBUG 	= gdb3


#--------------------------
# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
#--------------------------

CSTANDARD 	= -std=gnu99
CDEFS 		= -D$(RUN_MODE) # Place -D or -U options for C here
CINCS 		= -I./include/  # Place -I options here

ifdef VECTOR_LOCATION
CDEFS 		+=-D$(VECTOR_LOCATION)
ADEFS 		+=-D$(VECTOR_LOCATION)
endif

#---------------------------
# Compiler flags.
#  -Wa,...:      tell GCC to pass this to the assembler.
#  -adhlns...: create assembler listing
#---------------------------
CFLAGS 		=  -g$(DEBUG)
CFLAGS 		+= $(CDEFS) $(CINCS)
CFLAGS 		+= -O$(OPT)
CFLAGS 		+= -Wall -Wimplicit -Wcast-align
CFLAGS 		+= -Wpointer-arith -Wswitch
CFLAGS 		+= -Wredundant-decls -Wreturn-type -Wshadow -Wunused
CFLAGS 		+= -Wa,-adhlns=$(subst $(suffix $<),.lst,$<) 
CFLAGS 		+= -finline-functions
CFLAGS 		+= -Wno-implicit
#---------------------------

#-------------------------
# flags only for C
CONLYFLAGS += -Wnested-externs 
CONLYFLAGS += $(CSTANDARD)
#-------------------------

#--------------------------
# Assembler flags.
#  -Wa,...:   tell GCC to pass this to the assembler.
#  -adhlns:    create listing
#  -g$(DEBUG):have the assembler create line number information
ASFLAGS = $(ADEFS) -O1 -Wa,-adhlns=$(<:.S=.lst),-g$(DEBUG)
#--------------------------

MATH_LIB = -lm

#---------------------------
# Linker flags.
# -Wl,...	:     tell GCC to pass this to linker.
# -Map		:      create map file
# --cref	:    add cross reference to  map file
#--------------------------

LDFLAGS = -nostartfiles -Wl,-Map=$(TARGET).map,--cref
LDFLAGS += $(MATH_LIB)
LDFLAGS += -lc -lgcc 
# Set Linker-Script Depending On Selected Memory and Controller
ifeq ($(RUN_MODE),RAM_RUN)
LDFLAGS +=-T$(SUBMDL)-RAM.ld
else 
LDFLAGS +=-T$(SUBMDL)-ROM.ld
endif

#----------------------------

#----------------------------
# Compiler flags to generate dependency files.
# this options just outputs the stage to a file for later reading. nothing to do with compilation.
#Passing -M to the driver implies -E, and suppresses warnings with an implicit -w.
#-MP adds phony target helps in avoiding errors when header file is removed but makefile not upadated
#-MD #Passing -MD to the driver does not implies -E, and suppresses warnings with an implicit -w. 
# Combine all necessary flags and optional flags.
# Add target processor to flags.
#----------------------------
GENDEPFLAGS = -MD -MP -MF .dep/$(@F).d



# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS  = -mcpu=$(MCU) $(THUMB_IW) -I. $(CFLAGS) $(GENDEPFLAGS)
ALL_ASFLAGS = -mcpu=$(MCU) $(THUMB_IW) -I. -x assembler-with-cpp $(ASFLAGS)
