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

/*
 * XMHF core base platform component
 * author: amit vasudevan (amitvasudevan@acm.org)
 */

#include <xmhf-core.h>

//get CPU vendor
u32 xmhf_baseplatform_getcpuvendor(void){
	return xmhf_baseplatform_arch_getcpuvendor();
}

//initialize basic platform elements
void xmhf_baseplatform_initialize(void){
	xmhf_baseplatform_arch_initialize();	
}

//initialize CPU state
//void xmhf_baseplatform_cpuinitialize(void){
//	xmhf_baseplatform_arch_cpuinitialize();
//}

//initialize SMP
void xmhf_baseplatform_smpinitialize(void){
	xmhf_baseplatform_arch_smpinitialize();
}

//reboot platform
//void xmhf_baseplatform_reboot(VCPU *vcpu){
//	xmhf_baseplatform_arch_reboot(vcpu);
//}
void xmhf_baseplatform_reboot(context_desc_t context_desc){
	xmhf_baseplatform_arch_reboot(context_desc);
}

//get info. on the number of CPUs in the platform and their IDs
//u32 xmhf_baseplatform_getcputable(void *buffer, u32 sizeofbuffer){
//	HALT_ON_ERRORCOND ( (sizeofbuffer >= (g_midtable_numentries * sizeof(MIDTAB))) );
//	memcpy(buffer, (void *)&g_midtable, (g_midtable_numentries * sizeof(MIDTAB)) );
//	return g_midtable_numentries; 
//}

//xmhfcoreapiretval_t xmhf_baseplatform_getcputable(void){
//	xmhfcoreapiretval_t retval;
//	retval.returnval = (u32) g_xc_cpu_count;
//	retval.returnptr1 = (void *)&g_xc_cputable;
//	return retval;
//}
