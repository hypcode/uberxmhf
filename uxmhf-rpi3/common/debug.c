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
	debug module definitions

	author: amit vasudevan (amitvasudevan@acm.org)
*/

#include <types.h>
#include <bcm2837.h>
#include <uart.h>
#include <debug.h>

extern __attribute__(( section(".data") )) u32 xdprintfsmp_lock=1;


// gcc requires this for division by 0 on 64-bit values; used for debugging output
void raise(void){
	//uart_puts("uXMHF-rpi3: core: compiler raise invoked -- division by 0!\n");
	//uart_puts("uXMHF-rpi3: core: Halting!\n");
	HALT();
}


void __ll_debug_hexdumpu32(u32 value){
    u32 num_bits;
    u32 ch;

    num_bits = 32;

    while(num_bits){
        num_bits -= 4;

        ch=(value >> num_bits) & 0xF;
        if(ch > 9)
        	ch += 0x37;
        else
        	ch += 0x30;

        uart_putc((u8)ch);
    }
}


void debug_hexdumpu32(u32 value){
	__ll_debug_hexdumpu32(value);
    bcm2837_miniuart_putc('\n');
}

void debug_hexdumpu32_nolf(u32 value){
	__ll_debug_hexdumpu32(value);
}


