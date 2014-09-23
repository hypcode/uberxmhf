#!/usr/bin/perl
# script to generate XMHF slab table data structure definitions 
# based on the slab names provided
# author: amit vasudevan (amitvasudevan@acm.org)

my $i = 0;

print "\n/* autogenerated XMHF slab table data structure */";
print "\n/* author: amit vasudevan (amitvasudevan@acm.org) */";

print "\n#include <xmhf.h>";
print "\n#include <xmhf-core.h>";

while( $i <= $#ARGV) {
	print "\n";
	print "\nextern u8 _slab_$ARGV[$i]_code_start[];";													
	print "\nextern u8 _slab_$ARGV[$i]_code_end[];";														
	print "\nextern u8 _slab_$ARGV[$i]_rodata_start[];";													
	print "\nextern u8 _slab_$ARGV[$i]_rodata_end[];";													
	print "\nextern u8 _slab_$ARGV[$i]_rwdata_start[];";													
	print "\nextern u8 _slab_$ARGV[$i]_rwdata_end[];";													
	print "\nextern u8 _slab_$ARGV[$i]_stack_start[];";													
	print "\nextern u8 _slab_$ARGV[$i]_stack_end[];";													
	print "\nextern u8 _slab_$ARGV[$i]_entrypoint[];";
	print "\nextern u8 _slab_$ARGV[$i]_entrypointnew[];";

	$i++;
}

print "\n";
print "\n__attribute__ ((section(\".sharedro_slab_table\"))) slab_header_t _slab_table[XMHF_SLAB_NUMBEROFSLABS] = {";

$i = 0;
while( $i <= $#ARGV) {
	print "\n";
	print "\n{";
	print "\n	.slab_index = 0,";															
	print "\n	.slab_macmid = 0,";															
	print "\n	.slab_privilegemask = 0,";													
	print "\n	.slab_tos = _slab_$ARGV[$i]_stack_end,"; 					
	print "\n	.slab_code.start = _slab_$ARGV[$i]_code_start,";									
	print "\n	.slab_code.end = _slab_$ARGV[$i]_code_end,";										
	print "\n	.slab_rodata.start = _slab_$ARGV[$i]_rodata_start,";									
	print "\n	.slab_rodata.end = _slab_$ARGV[$i]_rodata_end,";									
	print "\n	.slab_rwdata.start = _slab_$ARGV[$i]_rwdata_start,";									
	print "\n	.slab_rwdata.end = _slab_$ARGV[$i]_rwdata_end,";									
	print "\n	.slab_stack.start = _slab_$ARGV[$i]_stack_start,";									
	print "\n	.slab_stack.end = _slab_$ARGV[$i]_stack_end,";
	print "\n	.entry_cr3 = _slab_$ARGV[$i]_entrypoint,";												
	print "\n	.entry_cr3_new = _slab_$ARGV[$i]_entrypointnew,";												
	print "\n},";

	$i++;
}

print "\n};";																				

exit 0;
