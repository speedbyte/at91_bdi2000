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
@- Function             : Interrupt_Handler_USART_Lowlevel
@- Treatments           : USART Controller Interrupt Handler.
@- Called Functions     : Interrupt_Handler_USART_Highlevel
@- Called Macros        : IRQ_ENTRY, IRQ_EXIT
@------------------------------------------------------------------------------
    .global Interrupt_Handler_USART_Lowlevel
Interrupt_Handler_USART_Lowlevel:
            .extern      Interrupt_Handler_USART_Highlevel

@- Manage Exception Entry
        IRQ_ENTRY
@- Call the PIO Interrupt C handler
            ldr         r0, =AT91C_BASE_US1
            ldr         r1, =Interrupt_Handler_USART_Highlevel
            mov         r14, pc
            bx          r1
@- Irq exit
exit_irq:
@- Manage Exception Exit
        IRQ_EXIT

@    .type   Interrupt_Handler_USART_Lowlevel,$function
@    .size   Interrupt_Handler_USART_Lowlevel,.-Interrupt_Handler_USART_Lowlevel

@------------------------------------------------------------------------------
@------------------------------------------------------------------------------
@ Interrupt_Handler_MCI_Lowlevel
@ ---------------------
@       Handler called by the AIC
@       
@	Save context
@       Call C handler
@ 	Restore context
@------------------------------------------------------------------------------
	.global Interrupt_Handler_MCI_Lowlevel
	.extern Interrupt_Handler_MCI_Highlevel
	
Interrupt_Handler_MCI_Lowlevel:
	IRQ_ENTRY

	ldr     r1, =Interrupt_Handler_MCI_Highlevel
	mov     r14, pc
	bx      r1

	IRQ_EXIT
@------------------------------------------------------------------------------

@------------------------------------------------------------------------------
@ Interrupt_Handler_SysTimer_Lowlevel
@ ---------------------
@       Handler called by the AIC
@       
@	Save context
@       Call C handler
@ 	Restore context
@------------------------------------------------------------------------------
	.global Interrupt_Handler_SysTimer_Lowlevel
	.extern Interrupt_Handler_SysTimer_Highlevel

Interrupt_Handler_SysTimer_Lowlevel:
	IRQ_ENTRY

	ldr     r1, =Interrupt_Handler_SysTimer_Highlevel
	mov     r14, pc
	bx      r1

	IRQ_EXIT


@------------------------------------------------------------

@------------------------------------------------------------------------------
@ Measured_Interrupt_Lowlevel
@ ---------------------
@       Handler called by the AIC
@       
@	Save context
@       Call C handler
@ 	Restore context
@------------------------------------------------------------------------------
	.global Measured_Interrupt_Lowlevel
	.extern Measured_Interrupt_Highlevel
    .extern Start_Latency_Measurement
	.extern Stop_Latency_Measurement
	
Measured_Interrupt_Lowlevel:
	IRQ_ENTRY

	@Branch with Link to start the the interrupt latency measuremnt
	BL Start_Latency_Measurement  

	ldr     r1, =Measured_Interrupt_Highlevel
	mov     r14, pc
	bx      r1
	
	@Branch with Link to Stop the interrupt latency measurment
	BL Stop_Latency_Measurement

	IRQ_EXIT


@------------------------------------------------------------

@------------------------------------------------------------------------------
@ Interrupt_Handler_TC0_Lowlevel
@ ---------------------
@       Handler called by the AIC
@       
@	Save context
@       Call C handler
@ 	Restore context
@------------------------------------------------------------------------------
	.global Interrupt_Handler_TC0_Lowlevel
	.extern Interrupt_Handler_TC0_Highlevel


Interrupt_Handler_TC0_Lowlevel:
	IRQ_ENTRY
	
	ldr     r1, =Interrupt_Handler_TC0_Highlevel
	mov     r14, pc
	bx      r1

	IRQ_EXIT


@------------------------------------------------------------

    .end


