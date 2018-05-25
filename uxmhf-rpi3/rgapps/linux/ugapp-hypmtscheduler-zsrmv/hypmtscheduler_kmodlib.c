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

bool hypmtscheduler_createhyptask(u32 first_period, u32 regular_period,
			u32 priority, u32 hyptask_id, u32 *hyptask_handle){

	ugapp_hypmtscheduler_param_t *hmtsp;
	struct page *hmtsp_page;
	u32 hmtsp_paddr;

	hmtsp_page = alloc_page(GFP_KERNEL | __GFP_ZERO);

	if(!hmtsp_page){
		__free_page(hmtsp_page);
		return false;
	}

	hmtsp = (ugapp_hypmtscheduler_param_t *)page_address(hmtsp_page);

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

