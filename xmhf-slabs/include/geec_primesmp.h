/*
 * @XMHF_LICENSE_HEADER_START@
 *
 * eXtensible, Modular Hypervisor Framework (XMHF)
 * Copyright (c) 2009-2012 Carnegie Mellon University
 * Copyright (c) 2010-2012 VDG Inc.
 * All Rights Reserved.
 *
 * Developed by: XMHF Team
 *               Carnegie Mellon University / CyLab
 *               VDG Inc.
 *               http://xmhf.org
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the names of Carnegie Mellon or VDG Inc, nor the names of
 * its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * @XMHF_LICENSE_HEADER_END@
 */

// XMHF/GEEC prime SMP header file
//author: amit vasudevan (amitvasudevan@acm.org)

#ifndef __GEEC_PRIMESMP_H_
#define __GEEC_PRIMESMP_H_

//---platform
//VMX MSR indices
#define INDEX_IA32_VMX_BASIC_MSR            0x0
#define INDEX_IA32_VMX_PINBASED_CTLS_MSR    0x1
#define INDEX_IA32_VMX_PROCBASED_CTLS_MSR   0x2
#define INDEX_IA32_VMX_EXIT_CTLS_MSR        0x3
#define INDEX_IA32_VMX_ENTRY_CTLS_MSR       0x4
#define INDEX_IA32_VMX_MISC_MSR       	    0x5
#define INDEX_IA32_VMX_CR0_FIXED0_MSR       0x6
#define INDEX_IA32_VMX_CR0_FIXED1_MSR       0x7
#define INDEX_IA32_VMX_CR4_FIXED0_MSR       0x8
#define INDEX_IA32_VMX_CR4_FIXED1_MSR       0x9
#define INDEX_IA32_VMX_VMCS_ENUM_MSR        0xA
#define INDEX_IA32_VMX_PROCBASED_CTLS2_MSR  0xB

// segment selectors
#define 	__CS_CPL0 	    0x0008 	//CPL-0 code segment selector
#define 	__DS_CPL0 	    0x0010 	//CPL-0 data segment selector
#define		__CS_CPL3	    0x001b	//CPL-3 code segment selector
#define		__DS_CPL3	    0x0023  //CPL-3 data segment selector
#define		__CS_CPL3_SE	0x002b	//CPL-3 code segment selector
#define		__DS_CPL3_SE	0x0033  //CPL-3 data segment selector
#define 	__TRSEL 	    0x0038  //TSS (task) selector

#define	EMHF_XCPHANDLER_MAXEXCEPTIONS	32



#ifndef __ASSEMBLY__

typedef struct {
    u32 ap_cr3;             //0
    u32 ap_cr4;             //4
    u32 ap_entrypoint;      //8
    u32 _filler0;           //12
    u32 ap_gdtdesc_limit;   //16
    u32 ap_gdtdesc_base;    //20
    u32 ap_cs_selector;     //24
    u32 ap_eip;             //28
    u32 cpuidtable;         //32
    u32 _filler1;
    u32 _filler2;
    u32 _filler3;
    u64 ap_gdt[X86SMP_APBOOTSTRAP_MAXGDTENTRIES];
}__attribute__((packed)) x86smp_apbootstrapdata_t;

typedef struct {
  u8 vmx_vmxon_region[PAGE_SIZE_4K];
  u8 vmx_vmcs_region[PAGE_SIZE_4K];
  u8 vmx_msr_area_host_region[2*PAGE_SIZE_4K];
  u8 vmx_msr_area_guest_region[2*PAGE_SIZE_4K];
  u8 vmx_iobitmap_region[2][PAGE_SIZE_4K];		//I/O Bitmap area
  u8 vmx_msrbitmaps_region[PAGE_SIZE_4K];		//MSR bitmap area
  u64 vmx_msrs[IA32_VMX_MSRCOUNT];
  u64 vmx_msr_efer;
  u64 vmx_msr_efcr;
  x86regs_t vmx_gprs;
  u8 _filler0[3952]; //page-align the whole structure
} __attribute__((packed)) xc_cpuarchdata_x86vmx_t;


extern __attribute__((section(".data"))) __attribute__(( aligned(4096) )) xc_cpuarchdata_x86vmx_t __xmhfhic_x86vmx_archdata[MAX_PLATFORM_CPUS];
extern __attribute__((section(".data"))) __attribute__(( aligned(16) )) idtentry_t __xmhfhic_x86vmx_idt_start[EMHF_XCPHANDLER_MAXEXCEPTIONS]; //ro
extern __attribute__((section(".data"))) __attribute__(( aligned(16) )) u64 __xmhfhic_x86vmx_gdt_start[];     //ro
extern __attribute__((section(".data"))) __attribute__(( aligned(16) )) arch_x86_gdtdesc_t __xmhfhic_x86vmx_gdt;  //ro
extern __attribute__((section(".data"))) __attribute__(( aligned(4096) )) u8 __xmhfhic_x86vmx_tss[MAX_PLATFORM_CPUS][PAGE_SIZE_4K]; //ro
extern __attribute__((section(".data"))) __attribute__(( aligned(8) )) u32 __xmhfhic_x86vmx_cpuidtable[MAX_X86_APIC_ID]; //ro
extern __attribute__((section(".data"))) u32  __xmhfhic_exceptionstubs[]; //ro
extern __attribute__((section(".data"))) __attribute__(( aligned(16) )) arch_x86_idtdesc_t __xmhfhic_x86vmx_idt; //ro
extern __attribute__((section(".data"))) __attribute__(( aligned(4096) )) u8 __xmhfhic_x86vmx_tss_stack[MAX_PLATFORM_CPUS][PAGE_SIZE_4K];
extern __attribute__((section(".data"))) __attribute__(( aligned(4096) )) u8 __xmhfhic_rtm_trampoline_stack[MAX_PLATFORM_CPUS][MAX_PLATFORM_CPUSTACK_SIZE];



void xmhfhic_arch_switch_to_smp(void);
void xmhfhic_arch_setup_base_cpu_data_structures(void);
void xmhf_hic_arch_setup_cpu_state(u64 cpuid);
void xmhfhic_smp_entry(u32 cpuid);

CASM_FUNCDECL(void _ap_bootstrap_code(void *noparam));
CASM_FUNCDECL(bool __xmhfhic_ap_entry(void *noparam));
CASM_FUNCDECL(void __xmhfhic_x86vmx_reloadCS(u32 cs_sel));
CASM_FUNCDECL(void __xmhfhic_x86vmx_reloadsegregs(u32 ds_sel));


#endif // __ASSEMBLY__


#endif /* __GEEC_PRIMESMP_H_ */
