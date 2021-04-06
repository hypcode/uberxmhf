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
 * uxmhf guest hypercall handler hub
 * author: amit vasudevan (amitvasudevan@acm.org)
*/

#include <uberspark/uobjcoll/platform/rpi3/uxmhf/uobjs/main/include/types.h>
#include <uberspark/uobjcoll/platform/rpi3/uxmhf/uobjs/main/include/arm8-32.h>
#include <uberspark/uobjcoll/platform/rpi3/uxmhf/uobjs/main/include/bcm2837.h>
#include <uberspark/uobjcoll/platform/rpi3/uxmhf/uobjs/main/include/uart.h>
#include <uberspark/uobjcoll/platform/rpi3/uxmhf/uobjs/main/include/atags.h>
#include <uberspark/uobjcoll/platform/rpi3/uxmhf/uobjs/main/include/fdt.h>
#include <uberspark/uobjcoll/platform/rpi3/uxmhf/uobjs/main/include/debug.h>
#include <uberspark/uobjcoll/platform/rpi3/uxmhf/uobjs/main/include/dmaprot.h>
//#include <uberspark/include/uberspark.h>

//////
// externs
//////
extern bool appnpf_activated;
extern u32 appnpf_page_pa;


//////
// guest hypercall handler hub
//////
void guest_hypercall_handler(arm8_32_regs_t *r, u32 hsr){
	u32 hvc_iss;
	u32 hvc_imm16;

	hvc_iss = ((hsr & HSR_ISS_MASK) >> HSR_ISS_SHIFT);
	hvc_imm16 = hvc_iss & 0x0000FFFFUL;

//#if defined (__UBERSPARK_UOBJCOLL_CONFIGDEF_ENABLE_UART_PL011__) || defined (__UBERSPARK_UOBJCOLL_CONFIGDEF_ENABLE_UART_MINI__)
//        //initialize uart
//        uart_init();
//#endif

    //_XDPRINTFSMP_("guest_hypercall_handler: hvc_imm16 = %x\n", hvc_imm16);


	if (hvc_imm16 == 0){
		//do nothing; null hypercall

	}else if (hvc_imm16 == 1){
		//hypercall hub interaction
		/*
		 * r0 = hypercall function number
		 * r1 = physical address of the guest buffer
		 * r2 = size of the guest buffer
		 * note: r1+r2 cannot cross page-boundary
		 */
	     //_XDPRINTFSMP_("guest_hypercall_handler: hyercall hub interaction: r0=%x\n", r->r0);


		#if defined (__UBERSPARK_UOBJCOLL_CONFIGDEF_ENABLE_UAPP_CTXTRACE__)
			// no hypercall handling required
		#endif

		#if defined (__UBERSPARK_UOBJCOLL_CONFIGDEF_ENABLE_UAPP_PA5ENCFS__)
		if( uapp_pa5encfs_handlehcall(r->r0, r->r1, r->r2) )
			return;
		#endif

		#if defined (__UBERSPARK_UOBJCOLL_CONFIGDEF_ENABLE_UAPP_PVDRIVER_UART__)
		if( uapp_pvdriver_uart_handlehcall(r->r0, r->r1, r->r2) )
			return;
		#endif

		#if defined (__UBERSPARK_UOBJCOLL_CONFIGDEF_ENABLE_UAPP_UAGENT__)		
		if( uapp_uagent_handlehcall(r->r0, r->r1, r->r2) )
			return;
		#endif		

		#if defined (__UBERSPARK_UOBJCOLL_CONFIGDEF_ENABLE_UAPP_UHCALLTEST__)		
		if( uapp_uhcalltest_handlehcall(r->r0, r->r1, r->r2) )
			return;
		#endif		

		#if defined (__UBERSPARK_UOBJCOLL_CONFIGDEF_ENABLE_UAPP_KHCALLTEST__)		
		if( uapp_khcalltest_handlehcall(r->r0, r->r1, r->r2) )
			return;
		#endif		

		#if defined (__UBERSPARK_UOBJCOLL_CONFIGDEF_ENABLE_UAPP_UHSIGN__)
		if( uapp_uhsign_handlehcall(r->r0, r->r1, r->r2) )
			return;
		#endif

		#if defined (__UBERSPARK_UOBJCOLL_CONFIGDEF_ENABLE_UAPP_UHSTATEDB__)
		if( uapp_uhstateDB_handlehcall(r->r0, r->r1, r->r2) )
			return;
		#endif

		#if defined (__UBERSPARK_UOBJCOLL_CONFIGDEF_ENABLE_UAPP_UTPMTEST__)
		if( uapp_utpmtest_handlehcall(r->r0, r->r1, r->r2) )
			return;
		#endif

		#if defined (__UBERSPARK_UOBJCOLL_CONFIGDEF_ENABLE_UAPP_WATCHDOG__)
			// no hypercall handling required
		#endif

		#if defined (__UBERSPARK_UOBJCOLL_CONFIGDEF_ENABLE_UAPP_PICAR_S__)
		if( uapp_picar_s_handlehcall(r->r0, r->r1, r->r2) )
			return;
		#endif

		#if defined (__UBERSPARK_UOBJCOLL_CONFIGDEF_ENABLE_UAPP_I2C_DRIVER__)
	    //_XDPRINTFSMP_("guest_hypercall_handler: proceeding to call i2c driver handehcall...\n");

		if( uapp_i2c_driver_handlehcall(r->r0, r->r1, r->r2) )
			return;
		#endif

                #if defined (__UBERSPARK_UOBJCOLL_CONFIGDEF_ENABLE_UAPP_I2C_IOACCESS__)
                if( uapp_i2c_ioaccess_handlehcall(r->r0, r->r1, r->r2) )
                        return;
                #endif

		_XDPRINTFSMP_("%s: hcall unhandled. Halting!\n", __func__);
		HALT();

	}else{
		_XDPRINTFSMP_("%s: unknown HVC instruction imm16=0x%08x. Halting!\n", __func__,
				hvc_imm16);
		HALT();
	}

}




////// deprecated stuff below
#if 0
void guest_hypercall_handler(arm8_32_regs_t *r, u32 hsr){
	u32 hvc_iss;
	u32 hvc_imm16;

	hvc_iss = ((hsr & HSR_ISS_MASK) >> HSR_ISS_SHIFT);
	hvc_imm16 = hvc_iss & 0x0000FFFFUL;


	if (hvc_imm16 == 1){
		//do nothing; null hypercall

	}else if (hvc_imm16 == 2){
		u64 attrs_noaccess = (LDESC_S2_MC_OUTER_WRITE_BACK_CACHEABLE_INNER_WRITE_BACK_CACHEABLE << LDESC_S2_MEMATTR_MC_SHIFT) |
			(LDESC_S2_S2AP_NO_ACCESS << LDESC_S2_MEMATTR_S2AP_SHIFT) |
			(MEM_INNER_SHAREABLE << LDESC_S2_MEMATTR_SH_SHIFT) |
			LDESC_S2_MEMATTR_AF_MASK;

		_XDPRINTFSMP_("%s: setprot_noaccess r0=0x%08x\n", __func__,
				r->r0);

		uapi_s2pgtbl_setprot(r->r0, attrs_noaccess);
		sysreg_tlbiallis();

		appnpf_page_pa = r->r0;
		appnpf_activated=true;

	}else if (hvc_imm16 == 3){
		u64 attrs = (LDESC_S2_MC_OUTER_WRITE_BACK_CACHEABLE_INNER_WRITE_BACK_CACHEABLE << LDESC_S2_MEMATTR_MC_SHIFT) |
			(LDESC_S2_S2AP_READ_WRITE << LDESC_S2_MEMATTR_S2AP_SHIFT) |
			(MEM_INNER_SHAREABLE << LDESC_S2_MEMATTR_SH_SHIFT) |
			LDESC_S2_MEMATTR_AF_MASK;

		_XDPRINTFSMP_("%s: setprot_restore-access r0=0x%08x\n", __func__,
				r->r0);

		uapi_s2pgtbl_setprot(r->r0, attrs);
		sysreg_tlbiallis();

		appnpf_page_pa = 0UL;
		appnpf_activated=false;


	}else{
		_XDPRINTFSMP_("%s: unknown HVC instruction imm16=0x%08x. Halting!\n", __func__,
				hvc_imm16);
		HALT();
	}

}
#endif
