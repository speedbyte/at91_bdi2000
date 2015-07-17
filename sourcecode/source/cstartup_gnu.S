@ 5 exceptions ( abt, und, giq, irq, svc ) ,  6 priviliege modes ( abt, und, fiq, irq, svc, sys ) 
@ the exception vectors are located at the following places 
@ 0  , 4,  8, c, 10, 18 , 1c
@ A label is a symbol immediately followed by a colon (:). Whitespace before a label or after a colon is permitted, but you may not have whitespace between a labels symbol and its colon.Labels
@A label is written as a symbol immediately followed by a colon :. The symbol then represents the current value of the active location counter, and is, for example, a suitable instruction operand. You are warned if you use the same symbol to represent two different locations: the first definition overrides any other definitions
@ .equ symbol, expression .........This directive sets the value of symbol to expression.

.data

.equ ARM_MODE_USER  ,   0x10     
.equ ARM_MODE_IRQ   ,   0x12
.equ ARM_MODE_FIQ   ,   0x11
.equ ARM_MODE_SVC   ,   0x13
.equ ARM_MODE_ABORT ,   0x17
.equ ARM_MODE_UNDEF ,   0x1B
.equ ARM_MODE_SYS   ,   0x1F

.equ I_BIT , 0x80
.equ F_BIT , 0x40
.equ T_BIT , 0x20


@------------------------------------------------------------------------------
@- Stack Area Definition
@-----------------------
@-
@------------------------------------------------------------------------------

.equ IRQ_STACK_SIZE   , 0x200
.equ FIQ_STACK_SIZE   , 0x10
.equ ABT_STACK_SIZE   , 0x10
.equ UND_STACK_SIZE   , 0x10
.equ SVC_STACK_SIZE   , 0x800
.equ USER_STACK_SIZE  , 0x200

@- IRQ stack definition
AT91_IRQ_Stack_End: 
.word 0x55AA55AA
.space (IRQ_STACK_SIZE - 4)			
.equ AT91_IRQ_Stack_Begin , (AT91_IRQ_Stack_End + (IRQ_STACK_SIZE - 4))

@- FIQ stack definition
AT91_FIQ_Stack_End: 
.word 0x55AA55AA
.space (FIQ_STACK_SIZE - 4)
.equ AT91_FIQ_Stack_Begin , (AT91_FIQ_Stack_End + (FIQ_STACK_SIZE - 4))

@- ABORT stack definition
AT91_ABT_Stack_End: 
.word 0x55AA55AA
.space (ABT_STACK_SIZE - 4)
.equ AT91_ABT_Stack_Begin , (AT91_ABT_Stack_End + (ABT_STACK_SIZE - 4))

@- UNDEF stack definition
AT91_UND_Stack_End: 
.word 0x55AA55AA
.space (UND_STACK_SIZE - 4)
.equ AT91_UND_Stack_Begin , (AT91_UND_Stack_End + (UND_STACK_SIZE - 4))

@- SVC stack definition
AT91_SVC_Stack_End: 
.word 0x55AA55AA
.space (SVC_STACK_SIZE-4)
.equ AT91_SVC_Stack_Begin , (AT91_SVC_Stack_End + (SVC_STACK_SIZE- 4))

@- USER and SYSTEM stack definition
AT91_USER_Stack_End: 
.word 0x55AA55AA
.space (USER_STACK_SIZE-4)
.equ AT91_USER_Stack_Begin , (AT91_USER_Stack_End + (USER_STACK_SIZE-4))

@.global AT91_IRQ_Stack_End
@.global AT91_IRQ_Stack_Begin
@.global AT91_FIQ_Stack_End
@.global AT91_FIQ_Stack_Begin
@.global AT91_ABT_Stack_End
@.global AT91_ABT_Stack_Begin
@.global AT91_UND_Stack_End
@.global AT91_UND_Stack_Begin
@.global AT91_SVC_Stack_End
@.global AT91_SVC_Stack_Begin
@.global AT91_USER_Stack_End
@.global AT91_USER_Stack_Begin

.text
.arm
.global _start

_start:

B InitReset @ reset
undefvec:
B undefvec  @ Undefined Instruction
swivec:
B swivec    @ Software Interrupt
pabtvec:
B pabtvec   @ Prefetch Abort
dabtvec:	
B dabtvec   @ Data Abort
rsvdvec:
B rsvdvec   @ reserved
irqvec:
ldr pc, [pc,#-0xF20] @ IRQ : read the AIC, and it loads the interrupt vector address from there.
fiqvec:
B fiqvec @ FIQ


InitReset:


@------------------------------------------------------------------------------
@- Setup the stack for each mode
@-------------------------
@- The processor will remain in the last initialized mode.
@------------------------------------------------------------------------------


@- Load the stack base addresses
add r0, pc,#-(8+.-StackData) @ @ where to read values (relative)
ldmia r0, {r1-r6}

@- Set up Supervisor Mode and set SVC Mode Stack
msr cpsr_c, #ARM_MODE_SVC | I_BIT | F_BIT
bic r1, r1, #3 @ Insure word alignement
mov sp, r1 @ Init stack SYS

@- Set up Interrupt Mode and set IRQ Mode Stack
msr CPSR_c, #ARM_MODE_IRQ | I_BIT | F_BIT
bic r2, r2, #3 @ Insure word alignement
mov sp, r2 @ Init stack IRQ

@- Set up Fast Interrupt Mode and set FIQ Mode Stack
msr CPSR_c, #ARM_MODE_FIQ | I_BIT | F_BIT
bic r3, r3, #3 @ Insure word alignement
mov sp, r3 @ Init stack FIQ

@- Set up Abort Mode and set Abort Mode Stack
msr CPSR_c, #ARM_MODE_ABORT | I_BIT | F_BIT
bic r4, r4, #3 @ Insure word alignement
mov sp, r4 @ Init stack Abort

@- Set up Undefined Instruction Mode and set Undef Mode Stack
msr CPSR_c, #ARM_MODE_UNDEF | I_BIT | F_BIT
bic r5, r5, #3 @ Insure word alignement
mov sp, r5 @ Init stack Undef

@- Set up user Mode and set Undef Mode Stack
msr CPSR_c, #ARM_MODE_USER | F_BIT
bic r6, r6, #3 @ Insure word alignement
mov sp, r6 @ Init stack Undef

b EndInitStack

StackData:
.word AT91_SVC_Stack_Begin
.word AT91_IRQ_Stack_Begin
.word AT91_FIQ_Stack_Begin
.word AT91_ABT_Stack_Begin
.word AT91_UND_Stack_Begin
.word AT91_USER_Stack_Begin

EndInitStack:


#ifdef ROM_RUN
// Relocate .data section (Copy from ROM to RAM)
                LDR     R1, =_etext
                LDR     R2, =_data
                LDR     R3, =_edata
LoopRel:        CMP     R2, R3
                LDRLO   R0, [R1], #4
                STRLO   R0, [R2], #4
                BLO     LoopRel
#endif

// Clear .bss section (Zero init)
                MOV     R0, #0
                LDR     R1, =__bss_start__
                LDR     R2, =__bss_end__
LoopZI:         CMP     R1, R2
                STRLO   R0, [R1], #4
                BLO     LoopZI


.extern AT91F_LowLevelInit

ldr r0, = AT91F_LowLevelInit
mov lr, pc
bx r0

@------------------------------------------------------------------------------
@- Branch on C code Main function (with interworking)
@----------------------------------------------------
@- Branch must be performed by an interworking call as either an ARM or Thumb
@- main C function must be supported. This makes the code not position-
@- independant. A Branch with link would generate errors
@------------------------------------------------------------------------------

.extern main
ldr r0, =main
mov lr, pc
bx r0

@------------------------------------------------------------------------------
@- Loop for ever
@---------------
@- End of application. Normally, never occur.
@- Could jump on Software Reset ( B 0x0 ).
@------------------------------------------------------------------------------
End:
	b 	End




