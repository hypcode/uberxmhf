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

#include <types.h>
#include <arm8-32.h>
#include <bcm2837.h>
#include <mailbox.h>
#include <uart.h>
#include <debug.h>

#include <atags.h>
#include <fdt.h>
#include <dmaprot.h>

//////
// externs
//////
extern u8 cpu_stacks[];
extern void chainload_os(u32 r0, u32 id, struct atag *at, u32 address);
extern void cpumodeswitch_hyp2svc(u32 r0, u32 id, struct atag *at, u32 address, u32 cpuid);




//////
// appnpf hypapp related variables
//////
bool appnpf_activated=false;
u32 appnpf_page_pa=0UL;



void hyp_rsvhandler(void){
	_XDPRINTF_("%s: unhandled exception\n", __func__);
	_XDPRINTF_("%s: Halting!\n", __func__);
	HALT();
}





void hyphvc_handler(void){
	_XDPRINTF_("%s: [IN]\n", __func__);
	_XDPRINTF_("%s: Hello world from hypercall\n", __func__);
	_XDPRINTF_("%s: [OUT]\n", __func__);
}


//////
// guest register and memory read/write helpers
//////

void guest_regwrite(arm8_32_regs_t *r, u32 regnum, u32 value){
	if(regnum == 0)
		r->r0 = value;
	else if(regnum == 1)
		r->r1 = value;
	else if(regnum == 2)
		r->r2 = value;
	else if(regnum == 3)
		r->r3 = value;
	else if(regnum == 4)
		r->r4 = value;
	else if(regnum == 5)
		r->r5 = value;
	else if(regnum == 6)
		r->r6 = value;
	else if(regnum == 7)
		r->r7 = value;
	else if(regnum == 8)
		r->r8 = value;
	else if(regnum == 9)
		r->r9 = value;
	else if(regnum == 10)
		r->r10 = value;
	else if(regnum == 11)
		r->r11 = value;
	else if(regnum == 12)
		r->r12 = value;
	else{
		_XDPRINTFSMP_("%s: Invalid regnum=%u. Halting!\n", __func__, regnum);
		HALT();
	}
}


u32 guest_regread(arm8_32_regs_t *r, u32 regnum){

	if(regnum == 0)
		return(r->r0);
	else if (regnum == 1)
		return(r->r1);
	else if (regnum == 2)
		return(r->r2);
	else if (regnum == 3)
		return(r->r3);
	else if (regnum == 4)
		return(r->r4);
	else if (regnum == 5)
		return(r->r5);
	else if (regnum == 6)
		return(r->r6);
	else if (regnum == 7)
		return(r->r7);
	else if (regnum == 8)
		return(r->r8);
	else if (regnum == 9)
		return(r->r9);
	else if (regnum == 10)
		return(r->r10);
	else if (regnum == 11)
		return(r->r11);
	else if (regnum == 12)
		return(r->r12);
	else{
			_XDPRINTFSMP_("%s: Invalid regnum=%u. Halting!\n", __func__, regnum);
			HALT();
	}

	return 0;	//never reached
}




void guest_data_abort_handler(arm8_32_regs_t *r, u32 hsr){
	u32 elr_hyp;
	u32 fault_va;
	u32 fault_pa_page;
	u32 fault_pa;
	u32 fault_va_page_offset;
	u32 fault_iss;
	u32 fault_iss_isv;
	u32 guest_regnum;
	u32 fault_il;
	info_intercept_data_abort_t ida;

	//get faulting iss
	fault_iss = (hsr & HSR_ISS_MASK) >> HSR_ISS_SHIFT;

	//compute validity bit of additional information
	fault_iss_isv = (fault_iss & 0x01000000UL) >> 24;

	//compute fault instruction length
	fault_il = ((hsr & HSR_IL_MASK) >> HSR_IL_SHIFT);

	//fix return address
	elr_hyp = sysreg_read_elrhyp();
	if(fault_il)
		elr_hyp += sizeof(u32);
	else
		elr_hyp += sizeof(u16);
	sysreg_write_elrhyp(elr_hyp);

	//get faulting va
	fault_va= sysreg_read_hdfar();

	//compute faulting va page_offset
	fault_va_page_offset = fault_va & 0x00000FFFUL;

	//get faulting pa page
	fault_pa_page = sysreg_read_hpfar();
	fault_pa_page = (fault_pa_page & 0xFFFFFFF0UL) << 8;

	//compute faulting pa
	fault_pa = 	fault_pa_page | fault_va_page_offset;

	//fill out 	info_intercept_data_abort_t ida
	ida.il = fault_il;
	ida.sas = (fault_iss & 0x00C00000UL) >> 22;
	ida.srt = (fault_iss & 0x000F0000UL) >> 16;
	ida.wnr = (fault_iss & 0x00000040UL) >> 6;
	ida.va = fault_va;
	ida.pa = fault_pa;
	ida.r = r;


	if(!fault_iss_isv){
		_XDPRINTFSMP_("%s: s2pgtbl DATA ABORT: invalid isv. Halting!\n", __func__);
		_XDPRINTFSMP_("%s: va=0x%08x, pa=0x%08x\n",	__func__, ida.va, ida.pa);
		HALT();
	}


	//handle data abort fault by passing it to appropriate module
	if ( fault_pa_page == ARMLOCALREGISTERS_BASE ){
		intprot_handle_intcontroller_access(&ida);

	}else if( fault_pa_page == BCM2837_EMMC_BASE){
		secboot_handle_sdio_access(&ida);

	}else if( fault_pa_page == BCM2837_SDHOST_BASE){
		secboot_handle_sdhost_access(&ida);

	}else if( (fault_pa_page == BCM2837_DMA0_REGS_BASE) ||
		(fault_pa_page == BCM2837_DMA15_REGS_BASE) ){
		dmaprot_handle_dmacontroller_access(&ida);


	}else if ( fault_pa_page == DWC_REGS_BASE ){
		dmaprot_handle_usbdmac_access(&ida);

	}else if ( fault_pa_page == appnpf_page_pa && appnpf_activated){
		//appnpf trigger, just omit the access

	}else{
		_XDPRINTFSMP_("%s: unknown s2pgtbl DATA ABORT. Halting! (va=0x%08x, pa=0x%08x)\n",
			__func__, ida.va, ida.pa);
		HALT();
	}


}



void guest_cp15_trap_handler(arm8_32_regs_t *r, u32 hsr){
	u32 trap_il;
	u32 elr_hyp;

	//compute trapped instruction length
	trap_il = ((hsr & HSR_IL_MASK) >> HSR_IL_SHIFT);

	//fix return address
	elr_hyp = sysreg_read_elrhyp();
	if(trap_il)
		elr_hyp += sizeof(u32);
	else
		elr_hyp += sizeof(u16);
	sysreg_write_elrhyp(elr_hyp);


	ctxtrace_cp15_trap_handler(r, hsr);
}




__attribute__(( section(".data") )) u32 hypsvc_handler_lock=1;

void hypsvc_handler(arm8_32_regs_t *r){
	u32 hsr;
	u32 hsr_ec;
	//_XDPRINTFSMP_("%s: ENTER\n", __func__);

	//acquire lock
	spin_lock(&hypsvc_handler_lock);

	//read hsr to determine the cause of the intercept
	hsr = sysreg_read_hsr();
	hsr_ec = ((hsr & HSR_EC_MASK) >> HSR_EC_SHIFT);
	////uart_puts(" HSR= ");
	////debug_hexdumpu32(hsr);

	//switch ( ((hsr & HSR_EC_MASK) >> HSR_EC_SHIFT) ){
	if(hsr_ec == HSR_EC_HVC){
		guest_hypercall_handler(r, hsr);

	}else if (hsr_ec == HSR_EC_DATA_ABORT_ELCHANGE){
		guest_data_abort_handler(r, hsr);

	}else if (hsr_ec == HSR_EC_CP15_TRAP){
		guest_cp15_trap_handler(r, hsr);

	}else{
		_XDPRINTFSMP_("uXMHF-rpi3: core: UNHANDLED INTERCEPT HALTING! hsr=0x%08x\n", hsr);
		HALT();
	}

	//_XDPRINTFSMP_("%s: EXIT\n", __func__);

	//release lock
	spin_unlock(&hypsvc_handler_lock);
}


void main_svc(u32 r0, u32 id, struct atag *at){
	u32 cpsr;

	_XDPRINTF_("%s: now in SVC mode\n", __func__);
	_XDPRINTF_("%s: r0=0x%08x, id=0x%08x, ATAGS=0x%08x\n", __func__, r0, id, at);

	_XDPRINTF_("%s: CPSR[mode]=0x%08x\n", __func__, (sysreg_read_cpsr() & 0xF));

	_XDPRINTF_("%s: proceeding to test hypercall (HVC) in SVC mode...\n", __func__);
	hypcall();
	_XDPRINTF_("%s: successful return after hypercall test\n", __func__);


	//_XDPRINTF_("%s: chainloading OS kernel...\n", __func__);
	//_XDPRINTF_("%s: r0=0x%08x, id=0x%08x, ATAGS=0x%08x\n", __func__, r0, id, at);
	//chainload_os(r0, id, at);

	_XDPRINTF_("%s: should not be here. Halting!\n", __func__);
	HALT();
}


void core_fixresmemmap(u32 fdt_address){
	struct fdt_header *fdth = (struct fdt_header *)fdt_address;
	struct fdt_reserve_entry *fdtrsvmmapentryp;
	u32 newtotalsize, padding;

	//uart_puts("uxmhf-rpi3: core: core_fixresmemmap [IN]\n");

	//uart_puts(" fdt_address=0x");
	//debug_hexdumpu32(fdt_address);

	//uart_puts(" totalsize=0x");
	//debug_hexdumpu32(cpu_be2le_u32(fdth->totalsize));

	//uart_puts(" off_dt_struct=0x");
	//debug_hexdumpu32(cpu_be2le_u32(fdth->off_dt_struct));

	//uart_puts(" size_dt_struct=0x");
	//debug_hexdumpu32(cpu_be2le_u32(fdth->size_dt_struct));

	//uart_puts(" off_dt_strings=0x");
	//debug_hexdumpu32(cpu_be2le_u32(fdth->off_dt_strings));

	//uart_puts(" size_dt_strings=0x");
	//debug_hexdumpu32(cpu_be2le_u32(fdth->size_dt_strings));

	//uart_puts(" off_mem_rsvmap=0x");
	//debug_hexdumpu32(cpu_be2le_u32(fdth->off_mem_rsvmap));

	//uart_puts(" version=0x");
	//debug_hexdumpu32(cpu_be2le_u32(fdth->version));

	//uart_puts(" last_comp_version=0x");
	//debug_hexdumpu32(cpu_be2le_u32(fdth->last_comp_version));

	//pad totalsize to a page-boundary
	padding = PAGE_SIZE_4K - (cpu_be2le_u32(fdth->totalsize) % PAGE_SIZE_4K);
	//uart_puts("padding=0x");
	//debug_hexdumpu32(padding);

	//take totalsize and compute var = size + 8 * 2
	newtotalsize = cpu_be2le_u32(fdth->totalsize);
	newtotalsize += padding;
	newtotalsize += (2 * sizeof(struct fdt_reserve_entry));

	//put rsv_mem_off to size
	fdth->off_mem_rsvmap = cpu_le2be_u32(cpu_be2le_u32(fdth->totalsize) + padding);

	//set totalsize to var
	fdth->totalsize = cpu_le2be_u32(newtotalsize);

	//populate fdtrsvmmapentryp to rsv_mem_off
	fdtrsvmmapentryp = (struct fdt_reserve_entry *)(fdt_address + cpu_be2le_u32(fdth->off_mem_rsvmap));

	//uart_puts("fdtrsvmmapentryp=0x");
	//debug_hexdumpu32((u32)fdtrsvmmapentryp);
	//uart_puts("sizeof(fdtrsvmmapentryp)=0x");
	//debug_hexdumpu32(sizeof(struct fdt_reserve_entry));

	//write the guestos extent as first entry
	fdtrsvmmapentryp->address = cpu_le2be_u64((u64)UXMHF_CORE_START_ADDR);
	fdtrsvmmapentryp->size = cpu_le2be_u64((u64)UXMHF_CORE_SIZE);
	//fdtrsvmmapentryp->address = 0ULL;
	//fdtrsvmmapentryp->size = 0ULL;

	//terminate the list with 0sadd 16 bytes
	fdtrsvmmapentryp++;
	//uart_puts("fdtrsvmmapentryp=0x");
	//debug_hexdumpu32((u32)fdtrsvmmapentryp);

	fdtrsvmmapentryp->address = 0ULL;
	fdtrsvmmapentryp->size = 0ULL;

	//debug
	//uart_puts("uxmhf-rpi3: core: dumping reserved memmap...\n");
	fdtrsvmmapentryp = (struct fdt_reserve_entry *)(fdt_address + cpu_be2le_u32(fdth->off_mem_rsvmap));
	//uart_puts("fdtrsvmmapentryp=0x");
	//debug_hexdumpu32((u32)fdtrsvmmapentryp);


	while(1){
		u64 addr = cpu_be2le_u64(fdtrsvmmapentryp->address);
		u64 size = cpu_be2le_u64(fdtrsvmmapentryp->size);
		if( addr == 0ULL &&  size == 0ULL){
			break;
		}
		//uart_puts(" address:0x");
		//debug_hexdumpu32(addr >> 32);
		//debug_hexdumpu32((u32)addr);
		//uart_puts(" size:0x");
		//debug_hexdumpu32(size >> 32);
		//debug_hexdumpu32((u32)size);
		fdtrsvmmapentryp++;
		//uart_puts("fdtrsvmmapentryp=0x");
		//debug_hexdumpu32((u32)fdtrsvmmapentryp);
	}

	//uart_puts("uxmhf-rpi3: core: dumped reserved memmap...\n");

	//uart_puts("uxmhf-rpi3: core: core_fixresmemmap [OUT]\n");
}





//////
// boot cpu enters here
//////
void main(u32 r0, u32 id, struct atag *at, u32 cpuid){
	u32 hvbar, hcr, spsr_hyp;
	u64 boardserial;

#if defined (__ENABLE_UART_PL011__) || defined (__ENABLE_UART_MINI__)
	//initialize uart
	uart_init();
#endif

	_XDPRINTF_("uberXMHF (Raspberry Pi 3) - Booting...\n", __func__, cpuid);
	_XDPRINTF_("%s[%u]: ENTER: sp=0x%08x (cpu_stacks=0x%08x)\n", __func__, cpuid,
			cpu_read_sp(), &cpu_stacks);
	_XDPRINTF_("%s[%u]: r0=0x%08x, id=0x%08x, ATAGS=0x%08x\n", __func__, cpuid, r0, id, at);


	//sanity check ATAGS pointer
	if(!(at->size == FDT_MAGIC)){
		_XDPRINTF_("%s[%u]: Error: require ATAGS to be FDT blob. Halting!\n", __func__, cpuid);
		HALT();
	}
	_XDPRINTF_("%s[%u]: ATAGS pointer is a FDT blob so no worries\n", __func__, cpuid);

	//fix reserved memory map
	core_fixresmemmap((u32)at);


#if 1
	boardserial = bcm2837_mailbox_get_board_serial();
	_XDPRINTF_("%s[%u]: board serial=0x%016llx\n", __func__, cpuid, boardserial);
#endif


#if 0
	uart_testrecv();
	HALT();
#endif

#if 0
	_XDPRINTF_("%s[%u]: Halting!\n", __func__, cpuid);
	HALT();
#endif


	//initialize base hardware platform
	bcm2837_platform_initialize();
	_XDPRINTF_("%s[%u]: initialized base hardware platform\n", __func__, cpuid);

	hyppgtbl_populate_tables();
	_XDPRINTF_("%s[%u]: page-tables populated\n", __func__, cpuid);

	hyppgtbl_activate();
	_XDPRINTF_("%s[%u]: hyp page-tables activated\n", __func__, cpuid);


	//initialize CPU vector table
	hypvtable_initialize(cpuid);

#if defined (__FIQREFLECTION__)

	//enable FIQ mask override; this should land us in HYP mode FIQ handler when FIQs are triggered inside guest
	hcr = sysreg_read_hcr();
	hcr |= (1UL << 3);
	sysreg_write_hcr(hcr);

#else

	//disable FIQ mask override; let guest process FIQs
	hcr = sysreg_read_hcr();
	hcr &= ~(1UL << 3);
	sysreg_write_hcr(hcr);

#endif

	// populate stage-2 page tables
	s2pgtbl_populate_tables();
	_XDPRINTF_("%s[%u]: stage-2 pts populated.\n", __func__, cpuid);

#if defined (__DMAPROT__)
	//activate DMA protection mechanism via stage-2 pts
	dmaprot_activate();
	_XDPRINTF_("%s[%u]: DMA protection mechanism activated via stage-2 pts\n", __func__, cpuid);
#endif

#if defined (__INTPROT__)
	//activate interrupt protection mechanism via stage-2 pts
	intprot_activate();
	_XDPRINTF_("%s[%u]: INTERRUPT protection mechanism activated via stage-2 pts\n", __func__, cpuid);
#endif

#if defined (__SECBOOT__)
	//activate secure boot protection mechanism
	secboot_activate();
#endif

	//dump hyp registers and load hvbar
	_XDPRINTF_("%s[%u]: HCR=0x%08x\n", __func__, cpuid, sysreg_read_hcr());
	_XDPRINTF_("%s[%u]: HSTR=0x%08x\n", __func__, cpuid, sysreg_read_hstr());
	_XDPRINTF_("%s[%u]: HCPTR=0x%08x\n", __func__, cpuid, sysreg_read_hcptr());
	_XDPRINTF_("%s[%u]: HDCR=0x%08x\n", __func__, cpuid, sysreg_read_hdcr());

	// initialize cpu support for second stage page table translations
	s2pgtbl_initialize();
	_XDPRINTF_("%s[%u]: cpu ready for stage-2 pts...\n", __func__, cpuid);

	// load page table base
	s2pgtbl_loadpgtblbase();
	_XDPRINTF_("%s[%u]: loaded stage-2 page-table base register\n", __func__, cpuid);

	// activate translation
	s2pgtbl_activatetranslation();
	_XDPRINTF_("%s[%u]: activated stage-2 translation\n", __func__, cpuid);


	//////
	// initialize uapps

	#if defined (__ENABLE_UAPP_PVDRIVER_UART__)
		uapp_pvdriver_uart_initialize_uapp(cpuid);
	#endif

	#if defined (__ENABLE_UAPP_HYPMTSCHEDULER__)
		uapp_sched_initialize(cpuid);
	#endif

	#if defined (__ENABLE_UAPP_MAVLINKSERHB__)
		uapp_mavlinkserhb_initialize(cpuid);
	#endif

	//////
	// boot secondary cores
	_XDPRINTF_("%s[%u]: proceeding to initialize SMP...\n", __func__, cpuid);
	bcm2837_platform_smpinitialize();
	_XDPRINTF_("%s[%u]: secondary cores booted, moving on...\n", __func__, cpuid);

	_XDPRINTF_("%s[%u]: booting guest in SVC mode\n", __func__, cpuid);
	_XDPRINTF_("%s[%u]: r0=0x%08x, id=0x%08x, at=0x%08x\n", __func__, cpuid, r0, id, at);

	chainload_os(r0,id,at,0x8000);

	_XDPRINTF_("%s[%u]: Should not come here.Halting\n", __func__, cpuid);
	HALT();
}


//////
// secondary cores enter here after they are booted up
//////
void secondary_main(u32 cpuid){
	u32 start_address;
	armlocalregisters_mailboxwrite_t *armlocalregisters_mailboxwrite;


	_XDPRINTF_("%s[%u]: ENTER: sp=0x%08x (cpu_stacks=0x%08x)\n", __func__, cpuid,
			cpu_read_sp(), &cpu_stacks);

	armlocalregisters_mailboxwrite = (armlocalregisters_mailboxwrite_t *)(ARMLOCALREGISTERS_MAILBOXWRITE_BASE + (0 * sizeof(armlocalregisters_mailboxwrite_t)));

	hyppgtbl_activate();
	_XDPRINTF_("%s[%u]: hyp page-tables activated\n", __func__, cpuid);

	//dump hyp registers and load hvbar
	_XDPRINTF_("%s[%u]: HCR=0x%08x\n", __func__, cpuid, sysreg_read_hcr());
	_XDPRINTF_("%s[%u]: HSTR=0x%08x\n", __func__, cpuid, sysreg_read_hstr());
	_XDPRINTF_("%s[%u]: HCPTR=0x%08x\n", __func__, cpuid, sysreg_read_hcptr());
	_XDPRINTF_("%s[%u]: HDCR=0x%08x\n", __func__, cpuid, sysreg_read_hdcr());

	//setup vector table for CPU
	hypvtable_initialize(cpuid);

	// initialize cpu support for second stage page table translations
	s2pgtbl_initialize();
	_XDPRINTF_("%s[%u]: cpu ready for stage-2 pts...\n", __func__, cpuid);

	// load page table base
	s2pgtbl_loadpgtblbase();
	_XDPRINTF_("%s[%u]: loaded stage-2 page-table base register\n", __func__, cpuid);

	// activate translation
	s2pgtbl_activatetranslation();
	_XDPRINTF_("%s[%u]: activated stage-2 translation\n", __func__, cpuid);


	_XDPRINTF_("%s[%u]: Signalling SMP readiness and entering SMP boot wait loop...\n", __func__, cpuid);
	armlocalregisters_mailboxwrite->mailbox3write = 1;
	cpu_dsb();

	start_address=bcm2837_platform_waitforstartup(cpuid);

	_XDPRINTFSMP_("%s[%u]: Got startup signal, address=0x%08x\n", __func__, cpuid, start_address);

	chainload_os(0, 0, 0, start_address);

	_XDPRINTFSMP_("%s[%u]: Should never be here. Halting!\n", __func__, cpuid);
	HALT();
}


#if defined (__ENABLE_UART_PL011__) || defined (__ENABLE_UART_MINI__)
//////
// test functions
//////
/* UART receive test function */
void uart_testrecv(void){
    unsigned int i;
    unsigned int pl011_uart_rxff;
    unsigned int num_count;
    unsigned char ch;

    unsigned int pl011_uart_rts;
    unsigned int pl011_uart_cts;

    num_count=0;
    pl011_uart_rts = !(mmio_read32(PL011_UART_CR_REG) & 0x800) >> 11;
    pl011_uart_cts = !(mmio_read32(PL011_UART_FR_REG) & 0x1);
    _XDPRINTF_("%s: RTS=%u, CTS=%u\n", __func__, pl011_uart_rts, pl011_uart_cts);

    //wait a little for reception to begin hitting the UART
    for(i=0; i < 12*1024*1024; i++);
    _XDPRINTF_("%s: going into read loop...\n", __func__);

    pl011_uart_rxff = (mmio_read32(PL011_UART_FR_REG) & 0x40) >> 6;

    if(pl011_uart_rxff){
        _XDPRINTF_("%s: RX FULL!!\n", __func__);
        pl011_uart_rts = !(mmio_read32(PL011_UART_CR_REG) & 0x800) >> 11;
        pl011_uart_cts = !(mmio_read32(PL011_UART_FR_REG) & 0x1);
        _XDPRINTF_("%s: RTS=%u, CTS=%u\n", __func__,	pl011_uart_rts, pl011_uart_cts);
    }

    while(uart_getc(&ch)){
        _XDPRINTF_("%c|0x%02x\n", ch, ch);
        num_count++;
    }

    _XDPRINTF_("%s: Total chars received=%u\n", __func__, num_count);
}

#endif
