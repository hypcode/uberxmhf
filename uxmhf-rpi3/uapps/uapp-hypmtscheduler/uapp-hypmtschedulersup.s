/*
	uapp scheduler low-level support routines

	author: amit vasudevan (amitvasudevan@acm.org)
*/


.section ".text"


/*
	we enter below in FIQ mode
	we need to save r0-r7
	rest is banked
	r13 = sp = stack
	r14 = link register
	r15 = pc
*/

	.balign 32
	.global	uapp_sched_fiq_handler
uapp_sched_fiq_handler:
	//msr SPSR_und, r0		//save r0

	//mrs r0, SPSR_hyp		//load SPSR_hyp into r0
	//and r0, r0, #0xF		//get the mode bits
	//cmp r0, #0xA			//did we get called from HYP mode?
	//beq uapp_sched_fiq_handler_skip_setup_sp		//if so, skip loading sp

	//ldr sp, =uapp_sched_fiqhandler_stack_top	//load base into stack-pointer

uapp_sched_fiq_handler_skip_setup_sp:

	push {r14}
	//push {r13}
	push {r12}
	push {r11}
	push {r10}
	push {r9}
	push {r8}
	push {r7}
	push {r6}
	push {r5}
	push {r4}
	push {r3}
	push {r2}
	push {r1}
	push {r0}

	bl uapp_sched_fiqhandler


	// restore all saved registers
	pop {r0}
	//mrs r0, SPSR_und		//restore r0
	pop {r1}
	pop {r2}
	pop {r3}
	pop {r4}
	pop {r5}
	pop {r6}
	pop {r7}
	pop {r8}
	pop {r9}
	pop {r10}
	pop {r11}
	pop {r12}
	//pop {r13}
	pop	{r14}


	eret



	.balign 32
	.global	uapp_hypmtsched_schedentry
uapp_hypmtsched_schedentry:
	ldr sp, =uapp_sched_main_stack_top	//load base into stack-pointer

	push {r14}
	push {r13}
	push {r12}
	push {r11}
	push {r10}
	push {r9}
	push {r8}
	push {r7}
	push {r6}
	push {r5}
	push {r4}
	push {r3}
	push {r2}
	push {r1}
	push {r0}

	bl uapp_sched_logic

	// restore all saved registers
	pop {r0}
	pop {r1}
	pop {r2}
	pop {r3}
	pop {r4}
	pop {r5}
	pop {r6}
	pop {r7}
	pop {r8}
	pop {r9}
	pop {r10}
	pop {r11}
	pop {r12}
	pop {r13}
	pop	{r14}

	eret





.section ".stack"

	.balign 8
	.global uapp_sched_fiqhandler_stack
	uapp_sched_fiqhandler_stack:	.space	16384
	.global uapp_sched_fiqhandler_stack_top
	uapp_sched_fiqhandler_stack_top:


	.balign 8
	.global uapp_sched_main_stack
	uapp_sched_main_stack:	.space	16384
	.global uapp_sched_main_stack_top
	uapp_sched_main_stack_top:


.section ".data"

	.balign 8
	.global uapp_sched_inprocessing
	uapp_sched_inprocessing:	.long	0


/*
	ldr r13, =uapp_sched_fiqhandler_stack_top

	push {lr}

	push {r12}
	push {r11}
	push {r10}
	push {r9}
	push {r8}
	push {r7}
	push {r6}
	push {r5}
	push {r4}
	push {r3}
	push {r2}
	push {r1}
	push {r0}

	bl uapp_sched_fiqhandler

	// restore all saved registers
	pop {r0}
	pop {r1}
	pop {r2}
	pop {r3}
	pop {r4}
	pop {r5}
	pop {r6}
	pop {r7}
	pop {r8}
	pop {r9}
	pop {r10}
	pop {r11}
	pop {r12}

	pop	{lr}

	eret
*/
