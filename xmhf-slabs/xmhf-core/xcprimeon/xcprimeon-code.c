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

#include <xmhf.h>
#include <xmhf-core.h>
#include <xmhf-debug.h>

#include <xcprimeon.h>

#define __XMHF_SLAB_CALLER_INDEX__	XMHF_SLAB_XCPRIMEON_INDEX
#include <xcsmp.h>


#include <testslab1.h>
#undef __XMHF_SLAB_CALLER_INDEX__

void xcprimeon_entry(void){

	//initialize debugging early on
	xmhfhw_platform_serial_init((char *)&xcbootinfo->debugcontrol_buffer);

	_XDPRINTF_("%s: alive and starting...\n", __FUNCTION__);
	
	//debug: dump bootinfo
	_XDPRINTF_("SL: xcbootinfo at = 0x%08x\n", (u32)xcbootinfo);
	_XDPRINTF_("	numE820Entries=%u\n", xcbootinfo->memmapinfo_numentries);
	_XDPRINTF_("	system memory map buffer at 0x%08x\n", (u32)&xcbootinfo->memmapinfo_buffer);
	_XDPRINTF_("	numCPUEntries=%u\n", xcbootinfo->cpuinfo_numentries);
	_XDPRINTF_("	cpuinfo buffer at 0x%08x\n", (u32)&xcbootinfo->cpuinfo_buffer);
	_XDPRINTF_("	SL + core size= %u bytes\n", xcbootinfo->size);
	_XDPRINTF_("	OS bootmodule at 0x%08x, size=%u bytes\n", 
		xcbootinfo->richguest_bootmodule_base, xcbootinfo->richguest_bootmodule_size);
    _XDPRINTF_("\tcmdline = \"%s\"\n", xcbootinfo->cmdline_buffer);

	_XDPRINTF_("SL: runtime at 0x%08x; size=0x%08x bytes\n", __TARGET_BASE_SL, xcbootinfo->size);

	//setup bootinfo with required parameters
	_XDPRINTF_("SL: XMHF_BOOTINFO at 0x%08x, magic=0x%08x\n", (u32)xcbootinfo, xcbootinfo->magic);
	HALT_ON_ERRORCOND(xcbootinfo->magic == RUNTIME_PARAMETER_BLOCK_MAGIC);
			
	//store runtime physical and virtual base addresses along with size
	xcbootinfo->physmem_base = __TARGET_BASE_SL; 
	xcbootinfo->virtmem_base = __TARGET_BASE_SL;
	xcbootinfo->size = xcbootinfo->size;

	//[debug] dump memory map
 	_XDPRINTF_("\nNumber of E820 entries = %u", xcbootinfo->memmapinfo_numentries);
	{
		int i;
		for(i=0; i < (int)xcbootinfo->memmapinfo_numentries; i++){
			_XDPRINTF_("\n0x%08x%08x, size=0x%08x%08x (%u)", 
			  xcbootinfo->memmapinfo_buffer[i].baseaddr_high, xcbootinfo->memmapinfo_buffer[i].baseaddr_low,
			  xcbootinfo->memmapinfo_buffer[i].length_high, xcbootinfo->memmapinfo_buffer[i].length_low,
			  xcbootinfo->memmapinfo_buffer[i].type);
		}
  	}

	//initialize basic platform elements
	xcprimeon_arch_initialize();
	
	//sanitize post launch (e.g., cache/MTRR/SMRAM)
#if defined (__DRT__)
    xcprimeon_arch_postdrt();
#endif	//__DRT__

#if defined (__DMAP__)    
	//setup DMA protection on runtime (xcprimeon is already DMA protected)
	xcprimeon_arch_earlydmaprot(__TARGET_BASE_SL, xcbootinfo->size);
#endif

	//print out slab table
	{
			u32 i;
			
			for(i=0; i < XMHF_SLAB_NUMBEROFSLABS; i++){
				_XDPRINTF_("\nslab %u: dumping slab header", i);
				_XDPRINTF_("\n	slab_index=%u", _slab_table[i].slab_index);
				_XDPRINTF_("\n	slab_macmid=%08x", _slab_table[i].slab_macmid);
				_XDPRINTF_("\n	slab_privilegemask=%08x", _slab_table[i].slab_privilegemask);
				_XDPRINTF_("\n	slab_tos=%08x", _slab_table[i].slab_tos);
				_XDPRINTF_("\n  slab_rodata(%08x-%08x)", _slab_table[i].slab_rodata.start, _slab_table[i].slab_rodata.end);
				_XDPRINTF_("\n  slab_rwdata(%08x-%08x)", _slab_table[i].slab_rwdata.start, _slab_table[i].slab_rwdata.end);
				_XDPRINTF_("\n  slab_code(%08x-%08x)", _slab_table[i].slab_code.start, _slab_table[i].slab_code.end);
				_XDPRINTF_("\n  slab_stack(%08x-%08x)", _slab_table[i].slab_stack.start, _slab_table[i].slab_stack.end);
				//_XDPRINTF_("\n  slab_trampoline(%08x-%08x)", _slab_table[i].slab_trampoline.start, _slab_table[i].slab_trampoline.end);
				_XDPRINTF_("\n  slab_entrycr3=%08x", _slab_table[i].entry_cr3);
			}
	}

	//initialize slab page tables
	xcprimeon_initialize_slab_tables();

	//[test] testslab1
	//{
	//		//invoke slab interfaces
	//		_XDPRINTF_("%s: preparing to invoke entry_0, esp=%x\n", __FUNCTION__, read_esp());
	//		XMHF_SLAB_CALL_P2P(testslab1, XMHF_SLAB_XCPRIMEON_INDEX, XMHF_SLAB_TESTSLAB1_INDEX, 0, 0);
	//		_XDPRINTF_("%s: came back from entry_0, esp=%x\n", __FUNCTION__, read_esp());
	//}
	
		
	//proceed with SMP initialization
	XMHF_SLAB_CALL_P2P(xcsmp, XMHF_SLAB_XCPRIMEON_INDEX, XMHF_SLAB_XCSMP_INDEX, 0, 0);

	//we should never get here
	_XDPRINTF_("\nSL: Fatal, should never be here!");
	HALT();
}

///////
XMHF_SLAB("xcprimeon")


