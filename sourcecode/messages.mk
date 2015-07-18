# Define programs and commands.
INSTALLPATH = c:/winarm-20060606/WinARM/bin/
INSTALLPATH = 
TOOLCHAIN = elf
# Define programs and commands.
SHELL 	= sh
CC 		= $(INSTALLPATH)arm-$(TOOLCHAIN)-gcc
OBJCOPY = $(INSTALLPATH)arm-$(TOOLCHAIN)-objcopy

#objdump has a problem with the full windows path somehow and hence throws an error with
#invalid argument.
OBJDUMP = arm-$(TOOLCHAIN)-objdump


SIZE 	= $(INSTALLPATH)arm-$(TOOLCHAIN)-size
NM 		= $(INSTALLPATH)arm-$(TOOLCHAIN)-nm
REMOVE 	= rm -f
COPY 	= cp

OBJDUMP_EXE = c:/winarm-20060606/WinARM/arm-elf/bin/objdump.exe



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
	
# Define Messages

MSG_ERRORS_NONE 		= Errors: none
MSG_BEGIN 				= "------- begin (mode: $(RUN_MODE)) --------"
MSG_END 				= --------  end  --------
MSG_SIZE_BEFORE 		= Size before: 
MSG_SIZE_AFTER 			= Size after:
MSG_FLASH 				= Preparing load file for Flash:
MSG_EXTENDED_LISTING 	= Creating Extended Listing:
MSG_SYMBOL_TABLE 		= Creating Symbol Table:
MSG_LINKING 			= Linking:
MSG_COMPILING_ARM 		= "Compiling C (ARM-only):"
MSG_ASSEMBLING_ARM 		= "Assembling (ARM-only):"
MSG_CLEANING 			= Cleaning project:

