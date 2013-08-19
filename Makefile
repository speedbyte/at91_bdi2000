
TOPDIR 		= $(CURDIR)
OBJECT_DIR 	= object-files
SOURCE_DIR 	= source
OBJTREE		:= $(if $(OBJECT_DIR),$(OBJECT_DIR),$(CURDIR))
SRCTREE		:= $(if $(SOURCE_DIR),$(SOURCE_DIR),$(CURDIR))

# load other configuration
include $(TOPDIR)/config.mk
include messages.mk

# Attempt to create a output directory. source/main.c       source/object-files/main.o
$(shell [ -d ${OBJECT_DIR} ] || mkdir -p ${OBJECT_DIR})

# List Assembler source files here which must be assembled in ARM-Mode..
ASRCARM 	= cstartup_gnu.S isr.S
SRCARM 		= main.c init.c mci_device.c usart_device.c st_device.c led_device.c

# Define all object files.

AOBJARM   	= $(addprefix $(OBJTREE)/,$(ASRCARM:.S=.o))
COBJARM   	= $(addprefix $(OBJTREE)/,$(SRCARM:.c=.o))


# Define all listing files.
LST 		= $(ASRCARM:.S=.lst) $(SRCARM:.c=.lst)

# Default target.
all: makevariables begin gccversion sizebefore build sizeafter finished end


begin:
	@echo
	@echo $(MSG_BEGIN)

# Display compiler version information.
gccversion : 
	@$(CC) --version
	
# Display size of file.
BINSIZE 	= $(SIZE) --target=$(FORMAT) $(TARGET).bin
ELFSIZE 	= $(SIZE) -A $(TARGET).elf

sizebefore:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_BEFORE); $(ELFSIZE); $(BINSIZE); echo; fi


build: elf bin obd sym

elf: $(TARGET).elf
bin: $(TARGET).bin 
obd: $(TARGET).obd
sym: $(TARGET).sym

# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(AOBJARM) $(COBJARM) 

%.elf:  $(AOBJARM) $(COBJARM)
	@echo
	@echo $(MSG_LINKING) $@
	$(CC) $(ALL_CFLAGS) $(AOBJARM) $(COBJARM) --output $@ $(LDFLAGS)
#	$(OBJCOPY) --strip-debug  $@ $@

# Compile: create object files from C source files. ARM-only
$(COBJARM) : $(OBJTREE)/%.o : $(SRCTREE)/%.c
	@echo
	@echo $(MSG_COMPILING_ARM) $<
	$(CC) -c $(ALL_CFLAGS) $(CONLYFLAGS) $< -o $@ 
	$(OBJDUMP) -h -S -C $@ > $(patsubst %.o,%.obd,$@)
	$(NM) -n $@ > $(patsubst %.o,%.sym,$@)
	
# Assemble: create object files from assembler source files. ARM-only
$(AOBJARM) : $(OBJTREE)/%.o : $(SRCTREE)/%.S
	@echo
	@echo $(MSG_ASSEMBLING_ARM) $<
	$(CC) -c $(ALL_ASFLAGS) $< -o $@
	$(OBJDUMP) -h -S -C $@ > $(patsubst %.o,%.obd,$@)
	$(NM) -n $@ > $(patsubst %.o,%.sym,$@)
	
# Create final output file (.bin) from ELF output file.
%.bin: %.elf
	@echo
	@echo $(MSG_FLASH) $@
	$(OBJCOPY) -O $(FORMAT) $< $(patsubst %.bin,%.bin,$@)         # just to show the use of patsubst
	
# Create extended listing file from ELF output file.
# testing: option -C
%.obd: %.elf
	@echo
	@echo $(MSG_EXTENDED_LISTING) $@
	$(OBJDUMP) -h -S -C $< > $@
#	$(OBJDUMP) -i > $(patsubst %.obd,%.lsq,$@) 
	
# Create a symbol table from ELF output file.
%.sym: %.elf
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
	$(REMOVE) $(TARGET).obd
	$(REMOVE) $(addprefix $(SRCTREE)/,$(LST))
	$(REMOVE) -r .dep
	$(REMOVE) -r $(OBJTREE)

# Listing of phony targets.
.PHONY : all makevariables begin gccversion sizebefore build sizeafter finished end elf bin obd sym clean clean_list


