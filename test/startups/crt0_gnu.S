/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support  -  ROUSSET  -
 * ----------------------------------------------------------------------------
 * Copyright (c) 2006, Atmel Corporation

 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 * 
 * - Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the disclaimer below in the documentation and/or
 * other materials provided with the distribution. 
 * 
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission. 
 * 
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 * File Name           : crt0_gnu.S
 * Object              : 
 * Creation            : ODi Apr 19th 2006
 *-----------------------------------------------------------------------------
 */

.section start
	.text

 	#include "include/part.h"

/*----------------------------------------------------------------------------
 Area Definition
----------------
 Must be defined as function to put first in the code as it must be mapped
 at offset 0 of the flash EBI_CSR0, ie. at address 0 before remap.
_---------------------------------------------------------------------------*/

/* Application startup entry point */

	.globl reset
	.align 4
reset:

/* Exception vectors (should be a branch to be detected as a valid code by the rom */
_exception_vectors:
	b 	reset_vector    /* reset */
	b 	undef_vector 	/* Undefined Instruction */
	b 	swi_vector   	/* Software Interrupt */
	b 	pabt_vector  	/* Prefetch Abort */
	b 	dabt_vector  	/* Data Abort */
	b 	rsvd_vector  	/* reserved */
	b 	irq_vector	/* IRQ : read the AIC */
	b 	fiq_vector      /* FIQ */

undef_vector:
	b 	undef_vector
swi_vector:
	b 	swi_vector
pabt_vector:
	b 	pabt_vector
dabt_vector:
	b 	dabt_vector
rsvd_vector:
	b 	rsvd_vector
irq_vector:
	b 	irq_vector
fiq_vector:
	b 	fiq_vector
reset_vector:

/* Init the stack */
_init_stack:
	ldr     sp,=TOP_OF_MEM

/* Test if main oscillator is enabled */
	ldr 	r0,=AT91C_PMC_SR
	ldr 	r1, [r0]
	ldr 	r2,=AT91C_PMC_MOSCS
	ands 	r1, r1, r2
	bne     _switch_to_mosc


/* Enable the main oscillator */
_enable_mosc:
	ldr 	r0,=AT91C_PMC_MOR
	mov 	r1, #(0x40 << 8)
	ldr 	r2,=AT91C_CKGR_MOSCEN
	orr 	r1, r1, r2
	str	r1, [r0]
	ldr 	r0,=AT91C_PMC_SR
1:
	ldr 	r1, [r0]
	ldr 	r2,=AT91C_PMC_MOSCS
	ands 	r1, r1, r2
	beq     1b
	
/* Test if MCK == SLOW CLOCK */
_switch_to_mosc:
	ldr 	r0,=AT91C_PMC_MCKR
	ldr 	r1,=AT91C_PMC_CSS
	ldr 	r2, [r0]
	and 	r2, r2, r1
	mov	r1, #0
	cmp    	r1, r2
/* No => Do nothing */
	bne	_init_bss
/* Yes => Switch to the main oscillator */
	ldr 	r1,=AT91C_PMC_CSS_MAIN_CLK
	ldr 	r2,=AT91C_PMC_PRES_CLK
	orr	r1, r1, r2
	str 	r1, [r0]
	ldr 	r0,=AT91C_PMC_SR
1:
	ldr     r1, [r0]
	ldr	r2,=AT91C_PMC_MCKRDY
	ands    r1, r1, r2
	beq     1b

/* Initialize the bss segment */
_init_bss:
	adr    r2, _lp_bss
	ldmia  r2, {r3, r4}
	mov    r2, #0
1:
	cmp    r3, r4
	strcc  r2, [r3], #4
	bcc    1b

/* Branch on C code Main function (with interworking) */
_branch_main:
	ldr     r4, = main
	mov     lr, pc
	bx      r4

/* Branch to the application at the end of the bootstrap init */
_go:
	mov     lr, pc
	bx      r0

	.align
_lp_bss:
	.word __bss_start__
	.word __bss_end__
