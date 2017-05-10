/*
	HYP mode verctor table and stubs

	author: amit vasudevan (amitvasudevan@acm.org)
*/


.section ".text"
/*
	G1.12.1 (Table G1-6 ARMv8)
	HYP vector table format:
	offsets:
		0x0: 	not used
		0x4: 	undefined instruction form HYP mode
		0x8: 	HVC in HYP mode
		0x0C:	prefetch abort from HYP mode
		0x10: 	data abort from HYP mode
		0x14:	HVC in non-HYP mode
		0x18:	IRQ
		0x1C:	FIQ
*/

/* need lower 5 bits (0-4) of the table address as 0, so use balign 32 */
	.balign	32
	.global g_hypvtable
g_hypvtable:
	//cpu-0
	b hypvtable_reserved_handler0
	b hypvtable_reserved_handler0
	b hypvtable_reserved_handler0
	b hypvtable_reserved_handler0
	b hypvtable_reserved_handler0
	b hypvtable_hypsvc_handler
	b hypvtable_reserved_handler0
	b hypvtable_reserved_handler0
	//cpu-1
	b hypvtable_reserved_handler1
	b hypvtable_reserved_handler1
	b hypvtable_reserved_handler1
	b hypvtable_reserved_handler1
	b hypvtable_reserved_handler1
	b hypvtable_hypsvc_handler
	b hypvtable_reserved_handler1
	b hypvtable_reserved_handler1
	//cpu-2
	b hypvtable_reserved_handler2
	b hypvtable_reserved_handler2
	b hypvtable_reserved_handler2
	b hypvtable_reserved_handler2
	b hypvtable_reserved_handler2
	b hypvtable_hypsvc_handler
	b hypvtable_reserved_handler2
	b hypvtable_reserved_handler2
	//cpu-3
	b hypvtable_reserved_handler3
	b hypvtable_reserved_handler3
	b hypvtable_reserved_handler3
	b hypvtable_reserved_handler3
	b hypvtable_reserved_handler3
	b hypvtable_hypsvc_handler
	b hypvtable_reserved_handler3
	b hypvtable_reserved_handler3



	.balign 32
	.global	hypvtable_reserved_handler0
hypvtable_reserved_handler0:
	ldr sp, =hypvtable_rsvhandler_stack_top0
	bl hyp_rsvhandler
1:	b 1b


	.global	hypvtable_reserved_handler1
hypvtable_reserved_handler1:
	ldr sp, =hypvtable_rsvhandler_stack_top1
	bl hyp_rsvhandler
1:	b 1b

	.global	hypvtable_reserved_handler2
hypvtable_reserved_handler2:
	ldr sp, =hypvtable_rsvhandler_stack_top2
	bl hyp_rsvhandler
1:	b 1b

	.global	hypvtable_reserved_handler3
hypvtable_reserved_handler3:
	ldr sp, =hypvtable_rsvhandler_stack_top3
	bl hyp_rsvhandler
1:	b 1b





/*
	G1.12.3 ARMv8
	exception return address is stored in ELR_hyp register and
	points to the instruction *after* the HVC instruction (Table G1-9)
*/
	.global	hypvtable_hypsvc_handler_common
hypvtable_hypsvc_handler_common:
	ldr sp, =hypvtable_hypsvc_stack_top

	// G1.9.2 (Figure G1-3)
	// HYP mode uses LR_usr, i.e, does not have LR banking, so save
	// since we are going to be using LR for C calling

	push {lr}


	// save guest gprs
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


	// invoke C handler
	mov r0, sp
	bl hypsvc_handler

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

	//
	//	G1.13.1 ARMv8
	//	exception returns from HYP mode is made via ERET instruction
	//	which basically returns to ELR_hyp and restores appropriate
	//	PE (processor execution) state
	//


/*
	//we can clobber r0
	//r1 = value, r2=register
	mrs r0, ELR_hyp
	add r0, #4
	msr ELR_hyp, r0
	dsb st
	str r1, [r2]
*/

	eret



.section ".stack"
	.balign 8
	.global hypvtable_stack
	stack:	.space	8192
	.global hypvtable_stack_top
	hypvtable_stack_top:

	.balign 8
	.global hypvtable_hypsvc_stack
	hypvtable_hypsvc_stack:	.space	16384
	.global hypvtable_hypsvc_stack_top
	hypvtable_hypsvc_stack_top:


	.balign 8
	.global hypvtable_rsvhandler_stack0
	hypvtable_rsvhandler_stack0:	.space	8192
	.global hypvtable_rsvhandler_stack_top0
	hypvtable_rsvhandler_stack_top0:

	.balign 8
	.global hypvtable_rsvhandler_stack1
	hypvtable_rsvhandler_stack1:	.space	8192
	.global hypvtable_rsvhandler_stack_top1
	hypvtable_rsvhandler_stack_top1:

	.balign 8
	.global hypvtable_rsvhandler_stack2
	hypvtable_rsvhandler_stack2:	.space	8192
	.global hypvtable_rsvhandler_stack_top2
	hypvtable_rsvhandler_stack_top2:

	.balign 8
	.global hypvtable_rsvhandler_stack3
	hypvtable_rsvhandler_stack3:	.space	8192
	.global hypvtable_rsvhandler_stack_top3
	hypvtable_rsvhandler_stack_top3:


/*
//
//	G1.12.3 ARMv8
//	exception return address is stored in ELR_hyp register and
//	points to the instruction *after* the HVC instruction (Table G1-9)
//
	.global	hypvtable_hyphvc_handler
hypvtable_hyphvc_handler:
	//ldr sp, =hypvtable_stack_top

	// G1.9.2 (Figure G1-3)
	//  HYP mode uses LR_usr, i.e, does not have LR banking, so save
	//  since we are going to be using LR for C calling
	//
	push {lr}


	// 5.1.1 AAPCS
	//  callee preserves r4-r8, r10, r11, r13 (SP)
	// save the rest
	//
	push {r0}
	push {r1}
	push {r2}
	push {r3}
	push {r9}
	push {r12}

	// invoke C handler
	bl hyphvc_handler


	// restore all saved registers
	pop	{r12}
	pop	{r9}
	pop	{r3}
	pop	{r2}
	pop	{r1}
	pop	{r0}

	pop	{lr}

	//
	//G1.13.1 ARMv8
	//	exception returns from HYP mode is made via ERET instruction
	//	which basically returns to ELR_hyp and restores appropriate
	//	PE (processor execution) state
	//
	eret
*/
