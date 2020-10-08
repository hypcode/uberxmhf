/*
 * hypmtscheduler test program low-level functions
 * author: amit vasudevan (amitvasudevan@acm.org)
 *
 */

 .section ".text"

 .global sysreg_read_cntfrq
sysreg_read_cntfrq:
	isb
	mrc p15, 0, r0, c14, c0, 0
	bx lr

.global sysreg_read_cntvct
sysreg_read_cntvct:
	isb
	mrrc p15, 1, r0, r1, c14
	bx lr
