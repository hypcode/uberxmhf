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

#include <linux/init.h>           // macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // core header for loading LKMs into the kernel
#include <linux/device.h>         // header to support the kernel Driver Model
#include <linux/kernel.h>         // contains types, macros, functions for the kernel
#include <linux/fs.h>             // header for the Linux file system support
#include <linux/mm.h>             // header for the Linux memory management support
#include <linux/highmem.h>             // header for the Linux memory management support
#include <asm/uaccess.h>          // required for the copy to user function
#include <asm/io.h>          // required for the copy to user function

#include <hypmtscheduler.h>

#ifndef __HVC__
#define __HVC__
void __hvc(u32 uhcall_function, void *uhcall_buffer,
		u32 uhcall_buffer_len){

	asm volatile
		(	" mov r0, %[in_0]\r\n"
			" mov r1, %[in_1]\r\n"
			" mov r2, %[in_2]\r\n"
			".long 0xE1400071 \r\n"
				: // outputs
				: [in_0] "r" (uhcall_function), [in_1] "r" (uhcall_buffer), [in_2] "r" (uhcall_buffer_len)  // inouts
	           : "r0", "r1", "r2" //clobber
	    );
}
#endif

u64 hypmtscheduler_readtsc64(void){
	u32 tsc_lo, tsc_hi;
	u64 l_tickcount;

	asm volatile
		(	" isb\r\n"
			" mrrc p15, 1, r0, r1, c14 \r\n"
			" mov %0, r0 \r\n"
			" mov %1, r1 \r\n"
				: "=r" (tsc_lo), "=r" (tsc_hi) // outputs
				: // inputs
	           : "r0", "r1" //clobber
	    );

	l_tickcount = tsc_hi;
	l_tickcount = l_tickcount << 32;
	l_tickcount |= tsc_lo;

}


u32 hypmtscheduler_readtscfreq(void){
	u32 tsc_freq;

	asm volatile
		(	" isb\r\n"
			"mrc p15, 0, r0, c14, c0, 0 \r\n"
			" mov %0, r0 \r\n"
				: "=r" (tsc_freq) // outputs
				: // inputs
	           : "r0" //clobber
	    );

	return tsc_freq;
}

bool hypmtscheduler_createhyptask(u32 first_period, u32 regular_period,
			u32 priority, u32 hyptask_id, u32 *hyptask_handle){

	ugapp_hypmtscheduler_param_t *hmtsp;
	struct page *hmtsp_page;
	u32 hmtsp_paddr;

	hmtsp_page = alloc_page(GFP_KERNEL | __GFP_ZERO);

	if(!hmtsp_page){
		return false;
	}

	hmtsp = (ugapp_hypmtscheduler_param_t *)page_address(hmtsp_page);

	hmtsp->uhcall_fn = UAPP_HYPMTSCHEDULER_UHCALL_CREATEHYPTASK;
    hmtsp->iparam_1 = first_period;	//first period
    hmtsp->iparam_2 = regular_period;	//regular period thereafter
    hmtsp->iparam_3 = priority;						//priority
    hmtsp->iparam_4 = hyptask_id;						//hyptask id

	hmtsp_paddr = page_to_phys(hmtsp_page);
	__hvc(UAPP_HYPMTSCHEDULER_UHCALL, hmtsp_paddr, sizeof(ugapp_hypmtscheduler_param_t));

	if(!hmtsp->status){
		__free_page(hmtsp_page);
		return false;
	}

	*hyptask_handle = hmtsp->oparam_1;

	__free_page(hmtsp_page);
	return true;
}


bool hypmtscheduler_disablehyptask(u32 hyptask_handle){

	ugapp_hypmtscheduler_param_t *hmtsp;
	struct page *hmtsp_page;
	u32 hmtsp_paddr;

	hmtsp_page = alloc_page(GFP_KERNEL | __GFP_ZERO);

	if(!hmtsp_page){
		return false;
	}

	hmtsp = (ugapp_hypmtscheduler_param_t *)page_address(hmtsp_page);

	hmtsp->uhcall_fn = UAPP_HYPMTSCHEDULER_UHCALL_DISABLEHYPTASK;
    hmtsp->iparam_1 = hyptask_handle;	//handle of hyptask

	hmtsp_paddr = page_to_phys(hmtsp_page);
	__hvc(UAPP_HYPMTSCHEDULER_UHCALL, hmtsp_paddr, sizeof(ugapp_hypmtscheduler_param_t));

	if(!hmtsp->status){
		__free_page(hmtsp_page);
		return false;
	}

	__free_page(hmtsp_page);
	return true;
}


bool hypmtscheduler_guestjobstart(u32 hyptask_handle){

	ugapp_hypmtscheduler_param_t *hmtsp;
	struct page *hmtsp_page;
	u32 hmtsp_paddr;

	hmtsp_page = alloc_page(GFP_KERNEL | __GFP_ZERO);

	if(!hmtsp_page){
		return false;
	}

	hmtsp = (ugapp_hypmtscheduler_param_t *)page_address(hmtsp_page);

	hmtsp->uhcall_fn = UAPP_HYPMTSCHEDULER_UHCALL_GUESTJOBSTART;
    hmtsp->iparam_1 = hyptask_handle;	//handle of hyptask

	hmtsp_paddr = page_to_phys(hmtsp_page);
	__hvc(UAPP_HYPMTSCHEDULER_UHCALL, hmtsp_paddr, sizeof(ugapp_hypmtscheduler_param_t));

	if(!hmtsp->status){
		__free_page(hmtsp_page);
		return false;
	}

	__free_page(hmtsp_page);
	return true;
}


bool hypmtscheduler_deletehyptask(u32 hyptask_handle){

	ugapp_hypmtscheduler_param_t *hmtsp;
	struct page *hmtsp_page;
	u32 hmtsp_paddr;

	hmtsp_page = alloc_page(GFP_KERNEL | __GFP_ZERO);

	if(!hmtsp_page){
		return false;
	}

	hmtsp = (ugapp_hypmtscheduler_param_t *)page_address(hmtsp_page);

	hmtsp->uhcall_fn = UAPP_HYPMTSCHEDULER_UHCALL_DELETEHYPTASK;
    hmtsp->iparam_1 = hyptask_handle;	//handle of hyptask

	hmtsp_paddr = page_to_phys(hmtsp_page);
	__hvc(UAPP_HYPMTSCHEDULER_UHCALL, hmtsp_paddr, sizeof(ugapp_hypmtscheduler_param_t));

	if(!hmtsp->status){
		__free_page(hmtsp_page);
		return false;
	}

	__free_page(hmtsp_page);
	return true;
}


bool hypmtscheduler_getrawtick64(u64 *tickcount){

	ugapp_hypmtscheduler_param_t *hmtsp;
	struct page *hmtsp_page;
	u32 hmtsp_paddr;
	u64 l_tickcount;

	hmtsp_page = alloc_page(GFP_KERNEL | __GFP_ZERO);

	if(!hmtsp_page || !tickcount){
		return false;
	}

	hmtsp = (ugapp_hypmtscheduler_param_t *)page_address(hmtsp_page);

	hmtsp->uhcall_fn = UAPP_HYPMTSCHEDULER_UHCALL_GETRAWTICK;

	hmtsp_paddr = page_to_phys(hmtsp_page);

	__hvc(UAPP_HYPMTSCHEDULER_UHCALL, hmtsp_paddr, sizeof(ugapp_hypmtscheduler_param_t));

	if(!hmtsp->status){
		__free_page(hmtsp_page);
		return false;
	}


	l_tickcount = hmtsp->oparam_1;
	l_tickcount = l_tickcount << 32;
	l_tickcount |= hmtsp->oparam_2;

	printk(KERN_INFO "hypmtscheduler_getrawtick64: oparam_1 = 0x%08x\n", hmtsp->oparam_1);
	printk(KERN_INFO "hypmtscheduler_getrawtick64: oparam_2 = 0x%08x\n", hmtsp->oparam_2);
	printk(KERN_INFO "hypmtscheduler_getrawtick64: l_tickcount = 0x%016llx\n", l_tickcount);

	//*tickcount = (u64)((hmtsp->oparam_1 << 32) | hmtsp->oparam_2);
	*tickcount = l_tickcount;
	__free_page(hmtsp_page);

	return true;
}

bool hypmtscheduler_getrawtick32(u32 *tickcount){

	ugapp_hypmtscheduler_param_t *hmtsp;
	struct page *hmtsp_page;
	u32 hmtsp_paddr;

	hmtsp_page = alloc_page(GFP_KERNEL | __GFP_ZERO);

	if(!hmtsp_page || !tickcount){
		return false;
	}

	hmtsp = (ugapp_hypmtscheduler_param_t *)page_address(hmtsp_page);

	hmtsp->uhcall_fn = UAPP_HYPMTSCHEDULER_UHCALL_GETRAWTICK;

	hmtsp_paddr = page_to_phys(hmtsp_page);
	__hvc(UAPP_HYPMTSCHEDULER_UHCALL, hmtsp_paddr, sizeof(ugapp_hypmtscheduler_param_t));

	if(!hmtsp->status){
		__free_page(hmtsp_page);
		return false;
	}

	*tickcount = hmtsp->oparam_2;

	__free_page(hmtsp_page);
	return true;
}


bool hypmtscheduler_inittsc(void){

	ugapp_hypmtscheduler_param_t *hmtsp;
	struct page *hmtsp_page;
	u32 hmtsp_paddr;

	hmtsp_page = alloc_page(GFP_KERNEL | __GFP_ZERO);

	if(!hmtsp_page){
		return false;
	}

	hmtsp = (ugapp_hypmtscheduler_param_t *)page_address(hmtsp_page);

	hmtsp->uhcall_fn = UAPP_HYPMTSCHEDULER_UHCALL_INITTSC;

	hmtsp_paddr = page_to_phys(hmtsp_page);
	__hvc(UAPP_HYPMTSCHEDULER_UHCALL, hmtsp_paddr, sizeof(ugapp_hypmtscheduler_param_t));

	if(!hmtsp->status){
		__free_page(hmtsp_page);
		return false;
	}

	__free_page(hmtsp_page);
	return true;
}


bool hypmtscheduler_logtsc(u32 event){

	ugapp_hypmtscheduler_param_t *hmtsp;
	struct page *hmtsp_page;
	u32 hmtsp_paddr;

	hmtsp_page = alloc_page(GFP_KERNEL | __GFP_ZERO);

	if(!hmtsp_page){
		return false;
	}

	hmtsp = (ugapp_hypmtscheduler_param_t *)page_address(hmtsp_page);

	hmtsp->uhcall_fn = UAPP_HYPMTSCHEDULER_UHCALL_LOGTSC;
	hmtsp->iparam_1 = event;

	hmtsp_paddr = page_to_phys(hmtsp_page);
	__hvc(UAPP_HYPMTSCHEDULER_UHCALL, hmtsp_paddr, sizeof(ugapp_hypmtscheduler_param_t));

	if(!hmtsp->status){
		__free_page(hmtsp_page);
		return false;
	}

	__free_page(hmtsp_page);
	return true;
}



bool hypmtscheduler_dumpdebuglog(u8 *dst_log_buffer, u32 *num_entries){

	ugapp_hypmtscheduler_param_t *hmtsp;
	struct page *hmtsp_page;
	u32 hmtsp_paddr;
	struct page *debug_log_buffer_page;
	u32 debug_log_buffer_page_paddr;

	if(!dst_log_buffer || !num_entries){
		return false;
	}

	*num_entries=0;

	//allocate physical page for parameter passing
	hmtsp_page = alloc_page(GFP_KERNEL | __GFP_ZERO);

	if(!hmtsp_page){
		return false;
	}

	hmtsp = (ugapp_hypmtscheduler_param_t *)page_address(hmtsp_page);


	//allocate debug log buffer page and compute its physical address
	debug_log_buffer_page = alloc_page(GFP_KERNEL | __GFP_ZERO);

	if(!debug_log_buffer_page){
		return false;
	}

	debug_log_buffer_page_paddr = page_to_phys(debug_log_buffer_page);


	hmtsp->uhcall_fn = UAPP_HYPMTSCHEDULER_UHCALL_DUMPDEBUGLOG;
	hmtsp->iparam_1 = debug_log_buffer_page_paddr;

	hmtsp_paddr = page_to_phys(hmtsp_page);
	__hvc(UAPP_HYPMTSCHEDULER_UHCALL, hmtsp_paddr, sizeof(ugapp_hypmtscheduler_param_t));

	if(!hmtsp->status){
		__free_page(debug_log_buffer_page);
		__free_page(hmtsp_page);
		return false;
	}

	memcpy(dst_log_buffer, page_address(debug_log_buffer_page),
			hmtsp->oparam_1 * sizeof(hypmtscheduler_logentry_t));
	*num_entries = hmtsp->oparam_1;

	__free_page(debug_log_buffer_page);
	__free_page(hmtsp_page);
	return true;
}


