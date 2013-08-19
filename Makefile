PLATFORM_RELFLAGS += -fno-strict-aliasing  -fno-common -ffixed-r8 \
	-msoft-float

PLATFORM_CPPFLAGS += -march=armv4
# =========================================================================
#
# Supply options according to compiler version
#
# =========================================================================
PLATFORM_CPPFLAGS +=$(call cc-option,-mapcs-32,-mabi=apcs-gnu)
PLATFORM_RELFLAGS +=$(call cc-option,-mshort-load-bytes,$(call cc-option,-malignment-traps,)) 


#program for LED Blinking

MCU      = arm920t
SUBMDL   = AT91RM9200
THUMB_IW = -mthumb-interwork

## Create ROM-Image (final)
#RUN_MODE=ROM_RUN
## Create RAM-Image (debugging)
RUN_MODE=RAM_RUN

## Exception-Vector placement only supported for "ROM_RUN"
## (placement settings ignored when using "RAM_RUN")
## - Exception vectors in ROM:
#VECTOR_LOCATION=VECTORS_IN_ROM
## - Exception vectors in RAM:
VECTOR_LOCATION=VECTORS_IN_RAM
#FLASH_TOOL = ULINK
## Output format. (can be ihex or binary)
FORMAT = binary
#Target File name
TARGET = main

SRCARM = $(TARGET).c init.c mci_device.c usart_device.c st_device.c
# List Assembler source files here which must be assembled in ARM-Mode..
ASRCARM = cstartup_gnu.S isr.S

# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = turn off optimization. s = optimize for size.
# (Note: 3 is not always the best optimization level. See avr-libc FAQ.)



OPT = 0

#Debugging Information
DEBUG = gdb3
# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99
# Place -D or -U options for C here
CDEFS =  -D$(RUN_MODE)
# Place -I options here
CINCS =
ifdef VECTOR_LOCATION
CDEFS += -D$(VECTOR_LOCATION)
ADEFS += -D$(VECTOR_LOCATION)
endif
# Compiler flags.
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
#
# Flags for C and C++ (arm-elf-gcc/arm-elf-g++)
CFLAGS = -g$(DEBUG)
CFLAGS += $(CDEFS) $(CINCS)
CFLAGS += -O$(OPT)
CFLAGS += -Wall  -Wimplicit -Wcast-align
CFLAGS += -Wpointer-arith -Wswitch
CFLAGS += -Wredundant-decls -Wreturn-type -Wshadow -Wunused
CFLAGS += -Wa,-adhlns=$(subst $(suffix $<),.lst,$<) 
CFLAGS += -finline-functions
CFLAGS += -Wno-implicit
#AT91-lib warnings with:
##CFLAGS += -Wcast-qual
# flags only for C
CONLYFLAGS += -Wnested-externs 
CONLYFLAGS += $(CSTANDARD)
#AT91-lib warnings with:
##CONLYFLAGS += -Wmissing-prototypes 
##CONLYFLAGS  = -Wstrict-prototypes
##CONLYFLAGS += -Wmissing-declarations

# Assembler flags.
#  -Wa,...:   tell GCC to pass this to the assembler.
#  -ahlms:    create listing
#  -g$(DEBUG):have the assembler create line number information
ASFLAGS = $(ADEFS) -O1 -Wa,-adhlns=$(<:.S=.lst),-g$(DEBUG)


#Support for newlibc-lpc (file: libnewlibc-lpc.a)
#NEWLIBLPC = -lnewlib-lpc
MATH_LIB = -lm


# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -nostartfiles -Wl,-Map=$(TARGET).map,--cref
#LDFLAGS += -lc
LDFLAGS += $(NEWLIBLPC) $(MATH_LIB)
LDFLAGS += -lc -lgcc 
# Set Linker-Script Depending On Selected Memory and Controller
ifeq ($(RUN_MODE),RAM_RUN)
LDFLAGS +=-T$(SUBMDL)-RAM.ld
else 
LDFLAGS +=-T$(SUBMDL)-ROM.ld
endif

# Define programs and commands.
SHELL = sh
CC = arm-elf-gcc
OBJCOPY = arm-elf-objcopy
OBJDUMP = arm-elf-objdump
SIZE = arm-elf-size
NM = arm-elf-nm
REMOVE = rm -f
COPY = cp

# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_BEGIN = "-------- begin (mode: $(RUN_MODE)) --------"
MSG_END = --------  end  --------
MSG_SIZE_BEFORE = Size before: 
MSG_SIZE_AFTER = Size after:
MSG_FLASH = Creating load file for Flash:
MSG_EXTENDED_LISTING = Creating Extended Listing:
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_LINKING = Linking:
MSG_COMPILING = Compiling C:
MSG_COMPILING_ARM = "Compiling C (ARM-only):"
MSG_COMPILINGCPP = Compiling C++:
MSG_COMPILINGCPP_ARM = "Compiling C++ (ARM-only):"
MSG_ASSEMBLING = Assembling:
MSG_ASSEMBLING_ARM = "Assembling (ARM-only):"
MSG_CLEANING = Cleaning project:
MSG_FORMATERROR = Can not handle output-format
MSG_LPC21_RESETREMINDER = You may have to bring the target in bootloader-mode now.

# Define all object files.
COBJARM   = $(SRCARM:.c=.o)
AOBJARM   = $(ASRCARM:.S=.o)

# Define all listing files.
LST = $(ASRCARM:.S=.lst) $(SRCARM:.c=.lst)

# Compiler flags to generate dependency files.
### GENDEPFLAGS = -Wp,-M,-MP,-MT,$(*F).o,-MF,.dep/$(@F).d
GENDEPFLAGS = -MD -MP -MF .dep/$(@F).d

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = -mcpu=$(MCU) $(THUMB_IW) -I. $(CFLAGS) $(GENDEPFLAGS)
ALL_ASFLAGS = -mcpu=$(MCU) $(THUMB_IW) -I. -x assembler-with-cpp $(ASFLAGS)


# Default target.
all: makevariables begin gccversion sizebefore build sizeafter finished end

makevariables:
	@echo The Makefile inbuilt Variables
	@echo MAKEFILES = $(MAKEFILES)
	@echo MAKEFILE_LIST = $(MAKEFILE_LIST)
	@echo .DEFAULT_GOAL = $(.DEFAULT_GOAL)
	@echo MAKE_RESTARTS = $(MAKE_RESTARTS)
	@echo .FEATURES = $(.FEATURES)
	@echo .INCLUDE_DIRS = $(.INCLUDE_DIRS)
.PHONY : .VARIABLES
	@echo .VARIABLES = $(.VARIABLES)
	
begin:
	@echo
	@echo $(MSG_BEGIN)

# Display compiler version information.
gccversion : 
	@$(CC) --version
	
# Display size of file.
BINSIZE = $(SIZE) --target=$(FORMAT) $(TARGET).bin
ELFSIZE = $(SIZE) -A $(TARGET).elf

sizebefore:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_BEFORE); $(ELFSIZE); $(BINSIZE); echo; fi


build: elf bin lssall symall
elf: $(TARGET).elf
bin: $(TARGET).bin 
lssall: $(TARGET).lssall
symall: $(TARGET).symall

# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(AOBJARM) $(COBJARM) 

%.elf:  $(AOBJARM) $(COBJARM) 
	@echo
	@echo $(MSG_LINKING) $@
	$(CC) $(ALL_CFLAGS) $(AOBJARM) $(COBJARM) --output $@ $(LDFLAGS)
#	$(OBJCOPY) --strip-debug  $@ $@

# Compile: create object files from C source files. ARM-only
$(COBJARM) : %.o : %.c
	@echo
	@echo $(MSG_COMPILING_ARM) $<
	$(CC) -c $(ALL_CFLAGS) $(CONLYFLAGS) $< -o $@ 
	$(OBJDUMP) -h -S -C $@ > $(patsubst %.o,%.lss,$@)
	$(NM) -n $@ > $(patsubst %.o,%.sym,$@)
	
# Assemble: create object files from assembler source files. ARM-only
$(AOBJARM) : %.o : %.S
	@echo
	@echo $(MSG_ASSEMBLING_ARM) $<
	$(CC) -c $(ALL_ASFLAGS) $< -o $@
	$(OBJDUMP) -h -S -C $@ > $(patsubst %.o,%.lss,$@)
	$(NM) -n $@ > $(patsubst %.o,%.sym,$@)
	
# Create final output file (.bin) from ELF output file.
%.bin: %.elf
	@echo
	@echo $(MSG_FLASH) $@
	$(OBJCOPY) -O $(FORMAT) $< $(patsubst %.bin,%.bin,$@)         # just to show the use of patsubst
	
# Create extended listing file from ELF output file.
# testing: option -C
%.lssall: %.elf
	@echo
	@echo $(MSG_EXTENDED_LISTING) $@
	$(OBJDUMP) -h -S -C $< > $@
#	$(OBJDUMP) -i > $(patsubst %.lss,%.lsq,$@) 
	
# Create a symbol table from ELF output file.
%.symall: %.elf
	@echo
	@echo $(MSG_SYMBOL_TABLE) $@
	$(NM) -n $< > $@
	
sizeafter:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_AFTER); $(ELFSIZE); $(BINSIZE); echo; fi

finished:
	@echo $(MSG_ERRORS_NONE)

end:
	@echo $(MSG_END)
	@echo


# Include the dependency files.
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)


# Target: clean project.
clean: begin clean_list finished end


clean_list :
	@echo
	@echo $(MSG_CLEANING)
	$(REMOVE) $(TARGET).bin
	$(REMOVE) $(TARGET).elf
	$(REMOVE) $(TARGET).map
	$(REMOVE) $(TARGET).sym
	$(REMOVE) $(TARGET).lnk
	$(REMOVE) $(TARGET).lss
	$(REMOVE) $(COBJARM)
	$(REMOVE) $(AOBJARM)
	$(REMOVE) $(LST)
	$(REMOVE) *.lss
	$(REMOVE) *.lssall
	$(REMOVE) *.sym
	$(REMOVE) *.symall
	#$(REMOVE) $(SRC:.c=.s)
	$(REMOVE) -rf .dep


# Listing of phony targets.
.PHONY : all makevariables begin gccversion sizebefore build sizeafter finished end elf bin lss sym clean clean_list
