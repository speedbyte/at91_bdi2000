@------------------------------------------------------------------------------
@-         ATMEL Microcontroller Software Support  -  ROUSSET  -
@------------------------------------------------------------------------------
@ The software is delivered "AS IS" without warranty or condition of any
@ kind, either express, implied or statutory. This includes without
@ limitation any warranty or condition with respect to merchantability or
@ fitness for any particular purpose, or against the infringements of
@ intellectual property rights of others.
@-----------------------------------------------------------------------------
@- File source          : arm_isr.s
@- Object               : Example of IT handler calling a C function
@- Compilation flag     : None
@-
@- 1.0 26/11/01 	FB  : Creation ARM ADS
@------------------------------------------------------------------------------
.text
@------------------------------------------------------------------------------
@- LISR vector handler for system peripherals
@--------------------------------------------
@- This macro save the context, call the LISR dispatch routine, and restore
@- the context
@------------------------------------------------------------------------------
 
 
@--------------------------------
@- ARM Core Mode and Status Bits
@--------------------------------

.equ ARM_MODE_USER           ,     0x10
.equ ARM_MODE_FIQ            ,     0x11
.equ ARM_MODE_IRQ            ,     0x12
.equ ARM_MODE_SVC            ,     0x13
.equ ARM_MODE_ABORT          ,     0x17
.equ ARM_MODE_UNDEF          ,     0x1B
.equ ARM_MODE_SYS            ,     0x1F

.equ I_BIT                   ,     0x80
.equ F_BIT                   ,     0x40
.equ T_BIT                   ,     0x20

#define AT91C_BASE_AIC 0xFFFFF000 
#define AIC_IVR 256
#define AIC_EOICR 304
@------------------------------------------------------------------------------
@- IRQ Entry
@-----------
@------------------------------------------------------------------------------
	.macro	\
	IRQ_ENTRY reg

@- Adjust and save LR_irq in IRQ stack
 	sub         r14, r14, #4      
	stmfd       sp!, {r14}      
	
@- Write in the IVR to support Protect Mode
@- No effect in Normal Mode
@- De-assert the NIRQ and clear the source in Protect Mode
	ldr         r14, =AT91C_BASE_AIC              
	str         r14, [r14, #AIC_IVR]  @store AT91C_BASE_AIC + 256 at tohe address speci

@- Save SPSR and r0 in IRQ stack
	mrs         r14, SPSR                  @ mov dest, source : move PSR to General Purpose reg, MSR -> move GPR to PSR.
	stmfd       sp!, {r0, r14} 			@ store these registers r0 and r14 in the memory with the base address sp

@- Enable Interrupt and Switch in SYS Mode
	mrs         r0, CPSR                   @read CPSR
	bic         r0, r0, #I_BIT
	orr         r0, r0, #ARM_MODE_SYS
	msr         CPSR_c, r0            @ change mode signal  

@- Save scratch/used registers and LR in User Stack
	.if  reg == 0
	stmfd       sp!, { r1-r3, r12, r14}
	.else
	stmfd       sp!, { r1-r3, r12, r14}
	.endif

	.endm

@------------------------------------------------------------------------------
@- IRQ Exit
@ ---------
@------------------------------------------------------------------------------
 	.macro	\
	IRQ_EXIT    reg
   
@ Restore scratch/used registers and LR from User Stack
 	.if  reg == 0
	ldmia       sp!, { r1-r3, r12, r14}
	.else
	ldmia       sp!, { r1-r3,  r12, r14}
	.endif

@- Disable Interrupt and switch back in IRQ mode
	mrs         r0, CPSR
	bic         r0, r0, #ARM_MODE_SYS
	orr         r0, r0, #I_BIT | ARM_MODE_IRQ 
	msr         CPSR_c, r0

@- Mark the End of Interrupt on the AIC
	ldr         r0, =AT91C_BASE_AIC
	str         r0, [r0, #AIC_EOICR]

@- Restore SPSR_irq and r0 from IRQ stack
	ldmia       sp!, {r0, r14}
	msr         SPSR_cxsf, r14

@- Restore adjusted  LR_irq from IRQ stack directly in the PC
	ldmia       sp!, {pc}^

	.endm


@------------------------------------------------------------------------------
@ AT91F_ASM_MCI_Handler
@ ---------------------
@       Handler called by the AIC
@       
@	Save context
@       Call C handler
@ 	Restore context
@------------------------------------------------------------------------------
	.global AT91F_ASM_MCI_Handler
	.extern AT91F_MCI_Handler
	
AT91F_ASM_MCI_Handler:
	IRQ_ENTRY

	ldr     r1, =AT91F_MCI_Handler
	mov     r14, pc
	bx      r1

	IRQ_EXIT
@------------------------------------------------------------------------------


.end
