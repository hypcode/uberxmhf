/*
 * @UBERXMHF_LICENSE_HEADER_START@
 *
 * uber eXtensible Micro-Hypervisor Framework (Raspberry Pi)
 *
 * Copyright 2018 Carnegie Mellon University. All Rights Reserved.
 *
 * NO WARRANTY. THIS CARNEGIE MELLON UNIVERSITY AND SOFTWARE ENGINEERING
 * INSTITUTE MATERIAL IS FURNISHED ON AN "AS-IS" BASIS. CARNEGIE MELLON
 * UNIVERSITY MAKES NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR IMPLIED,
 * AS TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF FITNESS FOR
 * PURPOSE OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS OBTAINED FROM USE OF
 * THE MATERIAL. CARNEGIE MELLON UNIVERSITY DOES NOT MAKE ANY WARRANTY OF
 * ANY KIND WITH RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT
 * INFRINGEMENT.
 *
 * Released under a BSD (SEI)-style license, please see LICENSE or
 * contact permission@sei.cmu.edu for full terms.
 *
 * [DISTRIBUTION STATEMENT A] This material has been approved for public
 * release and unlimited distribution.  Please see Copyright notice for
 * non-US Government use and distribution.
 *
 * Carnegie Mellon is registered in the U.S. Patent and Trademark Office by
 * Carnegie Mellon University.
 *
 * @UBERXMHF_LICENSE_HEADER_END@
 */

/*
 * Author: Amit Vasudevan (amitvasudevan@acm.org)
 *
 */

/*
	ARM 8 32-bit low-level cpu state access functions

	author: amit vasudevan (amitvasudevan@acm.org)
*/

#include <types.h>
#include <arm8-32.h>

.section ".text"

.globl mmio_write32
mmio_write32:
    dsb st
    str r1,[r0]
    bx lr

.globl mmio_read32
mmio_read32:
    ldr r0,[r0]
    bx lr


.globl sysreg_read_sp
sysreg_read_sp:
	mov r0, sp
	bx lr


.globl sysreg_read_spsr_hyp
sysreg_read_spsr_hyp:
	mrs r0, SPSR_hyp
	bx lr

.globl sysreg_write_spsr_hyp
sysreg_write_spsr_hyp:
	msr SPSR_hyp, r0
	bx lr


.globl sysreg_read_scr
sysreg_read_scr:
	mrc p15, 0, r0, c1, c1, 0
	bx lr

.globl sysreg_read_cpsr
sysreg_read_cpsr:
	mrs r0, cpsr
	bx lr

.globl sysreg_write_cpsr
sysreg_write_cpsr:
	msr cpsr, r0
	bx lr


.global sysreg_read_hvbar
sysreg_read_hvbar:
	mrc p15, 4, r0, c12, c0, 0
	bx lr


.global sysreg_write_hvbar
sysreg_write_hvbar:
	mcr p15, 4, r0, c12, c0, 0
	bx lr


.global sysreg_read_hcr
sysreg_read_hcr:
	mrc p15, 4, r0, c1, c1, 0
	bx lr

.global sysreg_write_hcr
sysreg_write_hcr:
	mcr p15, 4, r0, c1, c1, 0
	bx lr

.global sysreg_read_hsctlr
sysreg_read_hsctlr:
	mrc p15, 4, r0, c1, c0, 0
	bx lr

.global sysreg_write_hsctlr
sysreg_write_hsctlr:
	isb
	mcr p15, 4, r0, c1, c0, 0
	isb
	bx lr

.global sysreg_read_actlr
sysreg_read_actlr:
	mrc p15, 0, r0, c1, c0, 1
	bx lr

.global sysreg_write_actlr
sysreg_write_actlr:
	mcr p15, 0, r0, c1, c0, 1
	bx lr

.global sysreg_read_idisar4
sysreg_read_idisar4:
	mrc p15, 0, r0, c0, c2, 4
	bx lr

.global cpu_isb
cpu_isb:
	isb
	bx lr

.global cpu_dsb
cpu_dsb:
	dsb
	bx lr

.global cpu_dmbish
cpu_dmbish:
	dmb ish
	bx lr

.global cpu_read_sp
cpu_read_sp:
	mov r0, sp
	bx lr


.global hypcall
hypcall:
	hvc #0
	bx lr

.global svccall
svccall:
	svc #0
	bx lr

.global sysreg_read_sctlr
sysreg_read_sctlr:
	mrc p15, 0, r0, c1, c0, 0
	bx lr

.global sysreg_write_sctlr
sysreg_write_sctlr:
	mcr p15, 0, r0, c1, c0, 0
	bx lr

.global sysreg_read_vbar
sysreg_read_vbar:
	mrc p15, 0, r0, c12, c0, 0
	bx lr

.global sysreg_write_vbar
sysreg_write_vbar:
	mcr p15, 0, r0, c12, c0, 0
	bx lr

.global sysreg_read_vtcr
sysreg_read_vtcr:
	mrc p15, 4, r0, c2, c1, 2
	bx lr

.global sysreg_write_vtcr
sysreg_write_vtcr:
	mcr p15, 4, r0, c2, c1, 2
	bx lr

.global sysreg_read_hdcr
sysreg_read_hdcr:
	mrc p15, 4, r0, c1, c1, 1
	bx lr

.global sysreg_write_hdcr
sysreg_write_hdcr:
	mcr p15, 4, r0, c1, c1, 1
	bx lr

.global sysreg_read_hcptr
sysreg_read_hcptr:
	mrc p15, 4, r0, c1, c1, 2
	bx lr

.global sysreg_write_hcptr
sysreg_write_hcptr:
	mcr p15, 4, r0, c1, c1, 2
	bx lr

.global sysreg_read_hstr
sysreg_read_hstr:
	mrc p15, 4, r0, c1, c1, 3
	bx lr

.global sysreg_write_hstr
sysreg_write_hstr:
	mcr p15, 4, r0, c1, c1, 3
	bx lr

//returns a 64-bit value:
//r0=lower 32-bits; r1=upper 32-bits (c.f. AAPCS)
.global sysreg_read_vttbr
sysreg_read_vttbr:
	mrrc p15, 6, r0, r1, c2
	bx lr

//inputs: 64-bit value
//r0=lower 32-bits; r1=upper 32-bits (c.f. AAPCS)
.global sysreg_write_vttbr
sysreg_write_vttbr:
	mcrr p15, 6, r0, r1, c2
	bx lr


.global sysreg_read_hsr
sysreg_read_hsr:
	mrc p15, 4, r0, c5, c2, 0
	bx lr


.global sysreg_read_elrhyp
sysreg_read_elrhyp:
	mrs r0, ELR_hyp
	bx lr


.global sysreg_write_elrhyp
sysreg_write_elrhyp:
	msr ELR_hyp, r0
	bx lr


.global sysreg_tlbiallh
sysreg_tlbiallh:
	mcr p15,4,r0,c8,c7,0
	dsb ish
	bx lr

.global sysreg_iciallu
sysreg_iciallu:
	mcr p15,0,r0,c7,c5,0
	bx lr


//r0=ipa to flush s2pgtbl tlb entries for
//vmid is obtained from current vttbr
.global sysreg_tlbiipas2is
sysreg_tlbiipas2is:
	mcr p15,4,r0,c8,c0,1
	isb
	dsb ish
	bx lr


.global sysreg_tlbiallis
sysreg_tlbiallis:
	mcr p15,0,r0,c8,c3,0
	isb
	dsb ish
	bx lr

.global sysreg_read_mair0
sysreg_read_mair0:
	mrc p15,0,r0,c10,c2,0
	bx lr

.global sysreg_write_mair0
sysreg_write_mair0:
	mcr p15,0,r0,c10,c2,0
	bx lr


.global sysreg_read_mair1
sysreg_read_mair1:
	mrc p15,0,r0,c10,c2,1
	bx lr

.global sysreg_write_mair1
sysreg_write_mair1:
	mcr p15,0,r0,c10,c2,1
	bx lr


.global sysreg_read_hmair0
sysreg_read_hmair0:
	mrc p15,4,r0,c10,c2,0
	bx lr

.global sysreg_write_hmair0
sysreg_write_hmair0:
	mcr p15,4,r0,c10,c2,0
	bx lr

.global sysreg_read_hmair1
sysreg_read_hmair1:
	mrc p15,4,r0,c10,c2,1
	bx lr

.global sysreg_write_hmair1
sysreg_write_hmair1:
	mcr p15,4,r0,c10,c2,1
	bx lr


//returns a 64-bit value:
//r0=lower 32-bits; r1=upper 32-bits (c.f. AAPCS)
.global sysreg_read_httbr
sysreg_read_httbr:
	mrrc p15,4,r0,r1,c2
	bx lr


//inputs: 64-bit value
//r0=lower 32-bits; r1=upper 32-bits (c.f. AAPCS)
.global sysreg_write_httbr
sysreg_write_httbr:
	mcrr p15, 4, r0, r1, c2
	bx lr


.global sysreg_read_htcr
sysreg_read_htcr:
	mrc p15,4,r0,c2,c0,2
	bx lr

.global sysreg_write_htcr
sysreg_write_htcr:
	mcr p15,4,r0,c2,c0,2
	bx lr

.global sysreg_read_dacr
sysreg_read_dacr:
	mrc p15,0,r0,c3,c0,0
	bx lr

.global sysreg_write_dacr
sysreg_write_dacr:
	mcr p15,0,r0,c3,c0,0
	bx lr


.global sysreg_read_hdfar
sysreg_read_hdfar:
	mrc p15,4,r0,c6,c0,0
	bx lr

.global sysreg_read_hpfar
sysreg_read_hpfar:
	mrc p15,4,r0,c6,c0,4
	bx lr






//r0 specifies the 32-bit lock variable address
.global spin_lock
spin_lock:
	mov r1, #0				//load r1 with 32-bit constant 0, signifies lock is occupied
1:
	ldrex r2, [r0]			//load current 32-bit value of lock and mark its memory region exclusive
	teq r2, #1				//check if it is 1 i.e, free
	wfene					//if not free, then put this core in the wait-for-event state
	strexeq r3, r1, [r0]	//if free, try occupying the lock (storing 0); status is in r3
	teqeq r3, #0			//if free, test the status of the store, 0=success, 1=fail
	bne 1b					//start all over again if failure to store

	dmb ish					//data memory barrier inner shareability; make memory updates visible to all cores

	bx lr


//r0 specifies the 32-bit lock variable address
.global spin_unlock
spin_unlock:
	dmb ish					//data memory barrier
	mov r1, #1				//load r1 with 32-bit constant 1, signifies lock is free
	str r1, [r0]			//store 1 into lock indicating it is now free
	dsb ishst				//allow other cores to see the value
	sev						//signal other cores to wake up (if they are in the spinloop)

	bx lr


//////
// pl0,1 system register access functions
// chiefly used for emulation/pass-thru
//////
.global sysreg_read_ttbcr
sysreg_read_ttbcr:
	mrc p15,0,r0,c2,c0,2
	bx lr

.global sysreg_write_ttbcr
sysreg_write_ttbcr:
	mcr p15,0,r0,c2,c0,2
	bx lr

.global sysreg_read_ttbr0
sysreg_read_ttbr0:
	mrc p15,0,r0,c2,c0,0
	bx lr

.global sysreg_write_ttbr0
sysreg_write_ttbr0:
	mcr p15,0,r0,c2,c0,0
	bx lr

.global sysreg_read_ttbr1
sysreg_read_ttbr1:
	mrc p15,0,r0,c2,c0,1
	bx lr

.global sysreg_write_ttbr1
sysreg_write_ttbr1:
	mcr p15,0,r0,c2,c0,1
	bx lr

//r0 is the input virtual address
.global sysreg_ats12nsour
sysreg_ats12nsour:
	mcr p15,0,r0,c7,c8,6
	bx lr


//r0 is the input virtual address
.global sysreg_ats1cpr
sysreg_ats1cpr:
	mcr p15,0,r0,c7,c8,0
	bx lr


.global sysreg_read_par
sysreg_read_par:
	mrc p15,0,r0,c7,c4,0
	bx lr

//////
// generic timer system registers
//////
.global sysreg_read_cntfrq
sysreg_read_cntfrq:
	mrc p15, 0, r0, c14, c0, 0
	bx lr

.global sysreg_read_cntpct
sysreg_read_cntpct:
	isb
	mrrc p15, 0, r0, r1, c14
	bx lr

.global sysreg_read_cntvct
sysreg_read_cntvct:
	isb
	mrrc p15, 1, r0, r1, c14
	bx lr


.global sysreg_read_cnthp_tval
sysreg_read_cnthp_tval:
	mrc p15,4,r0,c14,c2,0
	bx lr

.global sysreg_write_cnthp_tval
sysreg_write_cnthp_tval:
	mcr p15,4,r0,c14,c2,0
	bx lr

.global sysreg_read_cnthp_ctl
sysreg_read_cnthp_ctl:
	mrc p15,4,r0,c14,c2,1
	bx lr

.global sysreg_write_cnthp_ctl
sysreg_write_cnthp_ctl:
	mcr p15,4,r0,c14,c2,1
	bx lr

.global cpu_eret
cpu_eret:
	eret

