/*
	logging implementation

	author: amit vasudevan (amitvasudevan@acm.org)
*/

#include <types.h>
#include <arm8-32.h>
#include <bcm2837.h>

#include <uart.h>
#include <debug.h>

#include <hypmtscheduler.h>

__attribute__((section(".data"))) hypmtscheduler_logentry_t debug_log_buffer[DEBUG_LOG_SIZE];
__attribute__((section(".data"))) u32 debug_log_buffer_index = 0;

void debug_log_tsc(u32 hyptask_id, u64 timestamp, u32 event_type){

	if(debug_log_buffer_index >= DEBUG_LOG_SIZE){
	  //bcm2837_miniuart_puts("\n[debug_log_tsc]: WARNING: buffer full, not logging anymore!");
	}else{
		debug_log_buffer[debug_log_buffer_index].hyptask_id = hyptask_id;
		debug_log_buffer[debug_log_buffer_index].timestamp = timestamp;
		debug_log_buffer[debug_log_buffer_index].event_type = event_type;

#ifdef DEBUG_LOGGING_SERIAL
		uart_puts("\n[debug_log_tsc]: hid=0x");
		debug_hexdumpu32_nolf(hyptask_id);
		uart_puts(", timestamp=0x");
		debug_hexdumpu32_nolf(timestamp >> 32);
		debug_hexdumpu32_nolf((u32)timestamp);
		uart_puts(", event=0x");
		debug_hexdumpu32_nolf(event_type);
#endif

		debug_log_buffer_index++;
	}
}
