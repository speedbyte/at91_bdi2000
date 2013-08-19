@------------------------------------------------------------------------------
@-         ATMEL Microcontroller Software Support  -  ROUSSET  -
@------------------------------------------------------------------------------
@ The software is delivered "AS IS" without warranty or condition of any
@ kind, either express, implied or statutory. This includes without
@ limitation any warranty or condition with respect to merchantability or
@ fitness for any particular purpose, or against the infringements of
@ intellectual property rights of others.
@------------------------------------------------------------------------------
@- File source          : irq_ghs.mac
@- Object               : Entry and Exit Macro for IRQ and FIQ.
@-
@- 1.0 01/04/00 JCZ     : Creation
@- 1.1 02/11/00 JPP     : Green Hills
@- 1.2 10/10/02 JPP     : Update FIQ and IRQ management to reduce interrupt cycle
@-                        You can also win in the standard Cstartup
@-IRQ_STACK_SIZE      EQU     (3*8*4)     @ 3 words per interrupt priority level
@-FIQ_STACK_SIZE      EQU     (3*4)       @ 3 words
@- Can be changed by
@-IRQ_STACK_SIZE      EQU     (2*8*4)     @ 2 words per interrupt priority level
@-FIQ_STACK_SIZE      EQU     (0*4)       @ 0 words
@------------------------------------------------------------------------------

@------------------------------------------------------------------------------
@- Area Definition
@------------------------------------------------------------------------------
    .text
    .align  4

@include "AT91RM9200_inc.h"


#define AT91C_BASE_AIC 0xFFFFF000 
#define AIC_IVR 256
#define AIC_EOICR 304
#define AT91C_BASE_US1 0xFFFC4000

@--------------------------------
@- ARM Core Mode and Status Bits
@--------------------------------

.equ ARM_MODE_USER , 0x10     
.equ ARM_MODE_IRQ , 0x12
.equ ARM_MODE_FIQ , 0x11
.equ ARM_MODE_SVC , 0x13
.equ ARM_MODE_ABORT , 0x17
.equ ARM_MODE_UNDEF , 0x1B
.equ ARM_MODE_SYS , 0x1F

.equ I_BIT , 0x80
.equ F_BIT , 0x40
.equ T_BIT , 0x20


@- IRQ Entry
@- Register initialization
@-      Banked Registers        @ Irq_register
@-      SPSR need to be saved for nested interrupt
@-      R13_irq <- Irq Stack register
@-      R14_irq <- Irq link register
@-----------
	.macro	IRQ_ENTRY

@- Adjust and save LR_irq in IRQ stack
            sub         lr, lr, #4
            stmfd       sp!, {lr}

@- Write in the IVR to support Protect Mode
@- No effect in Normal Mode
@- De-assert the NIRQ and clear the source in Protect Mode
            ldr         r14, =AT91C_BASE_AIC
            str         r14, [r14, #AIC_IVR]

@- Save SPSR and r0 in IRQ stack
            mrs         r14, SPSR
            stmfd       sp!, {r14}

@- Enable Interrupt and Switch in Supervisor Mode
           msr         CPSR_c, #ARM_MODE_SYS 

@- Save scratch/used registers and LR in User Stack
            stmfd       sp!, { r0-r3, r12, r14}

            .endm

@- IRQ Exit
@-----------
            .macro      IRQ_EXIT
@- Restore scratch/used registers and LR from User Stack
            ldmia       sp!, { r0-r3, r12, r14}

@- Disable Interrupt and switch back in IRQ mode
            msr         CPSR_c, #I_BIT | ARM_MODE_IRQ

@- Mark the End of Interrupt on the AIC
            ldr         r14, =AT91C_BASE_AIC
            str         r14, [r14, #AIC_EOICR]

@- Restore SPSR_irq and r0 from IRQ stack
            ldmia       sp!, {r14}
            msr         SPSR_cxsf, r14

@- Restore adjusted  LR_irq from IRQ stack directly in the PC
            ldmia       sp!, {pc}^

            .endm

@-------------------
@- FIQ Entry Macro -
@-------------------
@- FIQ Entry with no nested (reentrant) interrupt allowed.
@- Register initialization
@-      Banked Registers        @ FIQ_register
@-      SPSR does not need to be save because we do not want nested FIQ
@-      R13_fiq <- Same as above. No need to save it.
@-      R14_irq <- link register
@------------------------------------------------------------------------

            .macro      FIQ_ENTRY

@- Switch in SYS/User Mode to allow User Stack access for C code and Disable FIQ and IRQ Interrupt for this mode
@ because the FIQ is not yet acknowledged

            msr         CPSR_c,#I_BIT | F_BIT | #ARM_MODE_SVC

@- Save scratch/used registers and LR in User Stack
            stmfd       sp!, { r0-r3, r12, lr}
            .endm

@------------
@- FIQ Exit -
@------------
            .macro      FIQ_EXIT

@- Restore scratch/used registers and LR from User Stack
            ldmia       sp!, { r0-r3, r12, lr}

@- Leave Interrupts disabled and switch back in FIQ mode
            msr         CPSR_c, #I_BIT | F_BIT | ARM_MODE_FIQ

@- Restore the Program Counter using the LR_fiq directly in the PC
            subs        pc,lr,#4
            .endm
	    

@------------------------------------------------------------------------------
@- Function             : usart_asm_irq_handler
@- Treatments           : USART Controller Interrupt Handler.
@- Called Functions     : Usart_c_irq_handler
@- Called Macros        : IRQ_ENTRY, IRQ_EXIT
@------------------------------------------------------------------------------
    .global usart_asm_irq_handler
usart_asm_irq_handler:
            .extern      Usart_c_irq_handler

@- Manage Exception Entry
        IRQ_ENTRY
@- Call the PIO Interrupt C handler
            ldr         r0, =AT91C_BASE_US1
            ldr         r1, =Usart_c_irq_handler
            mov         r14, pc
            bx          r1
@- Irq exit
exit_irq:
@- Manage Exception Exit
        IRQ_EXIT

@    .type   usart_asm_irq_handler,$function
@    .size   usart_asm_irq_handler,.-usart_asm_irq_handler

    .end

@------------------------------------------------------------------------------
