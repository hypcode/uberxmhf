/*
	MAVLINK serial heart-beat uberapp

	author: amit vasudevan (amitvasudevan@acm.org)
*/

//////////////////////////////////////////////////////////////////////////////
// headers
//////////////////////////////////////////////////////////////////////////////

#include <types.h>
#include <arm8-32.h>
#include <bcm2837.h>

#include <uart.h>
#include <debug.h>

#include <hypmtscheduler.h>
#include <mavlinkserhb.h>


struct sched_timer *mavlinkserhb_hyptask=NULL;


//////////////////////////////////////////////////////////////////////////////
// mavlinkserhb serial port hw interface
//////////////////////////////////////////////////////////////////////////////


//////
// initialize UART hardware for given baudrate with the typical
// 8 stop bits, no parity, 1 stop bit configuration (8N1)
//////
void uapp_mavlinkserhb_uart_init(u32 baudrate){
    unsigned int gpio_fnsel, i;

    mmio_write32(AUX_ENABLES,1);
    mmio_write32(AUX_MU_IER_REG,0);		//clear interrupt enable
    mmio_write32(AUX_MU_CNTL_REG,0);
    mmio_write32(AUX_MU_LCR_REG,3);
    mmio_write32(AUX_MU_MCR_REG,0);
    mmio_write32(AUX_MU_IER_REG,0);
    mmio_write32(AUX_MU_IIR_REG,0xC6);
    //mmio_write32(AUX_MU_BAUD_REG, 270); //((250,000,000/115200)/8)-1 = 270
    mmio_write32(AUX_MU_BAUD_REG, (((250000000/baudrate)/8)-1) );

    gpio_fnsel=mmio_read32(GPFSEL1);
    gpio_fnsel &= ~(7<<12); 			//GPIO 14 (TX)
    gpio_fnsel |= 2<<12;    			//GPIO 14 -- Alternate Function 5
    gpio_fnsel &= ~(7<<15); 			//GPIO 15 (RX)
    gpio_fnsel |= 2<<15;    			//GPIO 15 -- Alternate Function 5
    mmio_write32(GPFSEL1,gpio_fnsel);

    mmio_write32(GPPUD,0);
    for(i=0; i<150; i++);				//delay
    mmio_write32(GPPUDCLK0,(1<<14)|(1<<15));
    for(i=0; i<150; i++);				//delay
    mmio_write32(GPPUDCLK0,0);
    mmio_write32(AUX_MU_CNTL_REG,3);
}



//////
// flush FIFO queue
//////
void uapp_mavlinkserhb_uart_flush(void){
    while( (mmio_read32(AUX_MU_LSR_REG) & 0x100) );
}


//////
// check is we have something in receive queue
// return 1 if true, else 0
//////
int uapp_mavlinkserhb_uart_checkrecv(void){

	if( mmio_read32(AUX_MU_LSR_REG) & 0x01 ) return 1;
    return 0;
}


//////
// write bytes to UART
//////
void uapp_mavlinkserhb_uart_send(u8 *buffer, u32 buf_len){
	u32 i;

	for(i=0; i < buf_len; i++){
	    while(! (mmio_read32(AUX_MU_LSR_REG) & 0x20) );
	    mmio_write32(AUX_MU_IO_REG,(u32)buffer[i]);
	}
}


//////
// read bytes from UART until max_len or read buffer exhausted
// return 0: read buffer still has characters
// return 1: read buffer exhausted
//////
int uapp_mavlinkserhb_uart_recv(u8 *buffer, u32 max_len, u32 *len_read){
	u32 i;

	i=0;
	while(uapp_mavlinkserhb_uart_checkrecv()){
		buffer[i] = mmio_read32(AUX_MU_IO_REG) & 0xFF;
		i++;
		if(i == max_len)
			break;
	}

	*len_read = i;

	if(uapp_mavlinkserhb_uart_checkrecv())
		return 0;
	else
		return 1;
}


//////////////////////////////////////////////////////////////////////////////





//////////////////////////////////////////////////////////////////////////////
// mavlinkserhb async timer functions
//////////////////////////////////////////////////////////////////////////////



//////
// the periodic function which handles the heart-beat protocol
//////
void uapp_mavlinkserhb_handleheartbeat(struct sched_timer *t){
	uapp_mavlinkserhb_uart_send("A", 1);
	uapp_mavlinkserhb_uart_flush();
}

//////////////////////////////////////////////////////////////////////////////





//////////////////////////////////////////////////////////////////////////////
// mavlinkserhb hypercall APIs
//////////////////////////////////////////////////////////////////////////////

//////
// mavlinkserhb initialize hypercall API
// initialize UART
//////
void uapp_mavlinkserhb_handlehcall_initialize(uapp_mavlinkserhb_param_t *mlhbsp){

	//initialize UART with specified parameter
	uapp_mavlinkserhb_uart_init(mlhbsp->iparam_1);

	mlhbsp->status=1;
}


//////
// mavlinkserhb send hypercall API
// send bytes via UART
//////
void uapp_mavlinkserhb_handlehcall_send(uapp_mavlinkserhb_param_t *mlhbsp){
	//iparam_1 = buffer physical address
	//iparam_2 = buffer length (in bytes)

	//sanity check buffer length, currently only a max of 4096 bytes
	//can be sent at a time
	if(mlhbsp->iparam_2 > 4096){
		mlhbsp->status=0;
		return;
	}

	//send the bytes out through the UART and flush output
	uapp_mavlinkserhb_uart_send(mlhbsp->iparam_1, mlhbsp->iparam_2);
	uapp_mavlinkserhb_uart_flush();

	//set status to indicate success
	mlhbsp->status=1;
}




//////
// mavlinkserhb recv check hypercall API
// check if UART receive buffer has any characters
// returns 0 if no characters, 1 if there are characters
//////
void uapp_mavlinkserhb_handlehcall_checkrecv(uapp_mavlinkserhb_param_t *mlhbsp){

	if(uapp_mavlinkserhb_uart_checkrecv())
		mlhbsp->status=1;
	else
		mlhbsp->status=0;

	return;
}



//////
// mavlinkserhb recv hypercall API
// recv bytes via UART
//////
void uapp_mavlinkserhb_handlehcall_recv(uapp_mavlinkserhb_param_t *mlhbsp){
	//iparam_1 = buffer physical address
	//iparam_2 = buffer max length (in bytes)

	//oparam_1 = length read
	//oparam_2 = UART read buffer status (1=exhausted, 0=not exhausted)

	//sanity check buffer length, currently only a max of 4096 bytes
	//can be received at a time
	if(mlhbsp->iparam_2 > 4096){
		mlhbsp->status=0;
		return;
	}

	//read bytes from UART
	mlhbsp->oparam_2 = uapp_mavlinkserhb_uart_recv(mlhbsp->iparam_1, mlhbsp->iparam_2, &mlhbsp->oparam_1);

	//set status to indicate success
	mlhbsp->status=1;
}



//////
// mavlinkserhb activate heart-beat hyptask
// creates the heart-beat thread to begin processing mavlink heart-beat
// messages
//////
uapp_mavlinkserhb_handlehcall_activatehbhyptask(uapp_mavlinkserhb_param_t *mlhbsp){

	//iparam_1 is the first time period of the hb hyptask
	//iparam_2 is the recurring time period thereafter
	//iparam_3 is the priority of the hb hyptask

	//declare a timer to deal with heart-beat
	//uapp_sched_timer_declare((0.5 * HYPMTSCHEDULER_TIME_1SEC),
	//		(0.5 * HYPMTSCHEDULER_TIME_1SEC), 99, &uapp_mavlinkserhb_handleheartbeat);
	mavlinkserhb_hyptask = uapp_sched_timer_declare(mlhbsp->iparam_1,
			mlhbsp->iparam_2, mlhbsp->iparam_3, &uapp_mavlinkserhb_handleheartbeat);

	if(mavlinkserhb_hyptask == NULL)
		mlhbsp->status=0;	//fail
	else
		mlhbsp->status=1; 	//success
}


//////
// mavlinkserhb de-activate heart-beat hyptask
// deactivates the heart-beat thread to stop processing mavlink heart-beat
// messages
//////
void uapp_mavlinkserhb_handlehcall_deactivatehbhyptask(uapp_mavlinkserhb_param_t *mlhbsp){

	if(mavlinkserhb_hyptask == NULL){
		mlhbsp->status=0;	//fail
		return;
	}

	uapp_sched_timer_undeclare(mavlinkserhb_hyptask);

	mlhbsp->status=1; 	//success
	return;
}



//////
// top-level hypercall handler hub
// return true if handled the hypercall, false if not
//////
bool uapp_mavlinkserhb_handlehcall(u32 uhcall_function, void *uhcall_buffer,
		u32 uhcall_buffer_len){
	uapp_mavlinkserhb_param_t *mlhbsp;

	if(uhcall_function != UAPP_MAVLINKSERHB_UHCALL){
		return false;
	}

	mlhbsp = (uapp_mavlinkserhb_param_t *)uhcall_buffer;

	if(mlhbsp->uhcall_fn == UAPP_MAVLINKSERHB_UHCALL_INITIALIZE){
		uapp_mavlinkserhb_handlehcall_initialize(mlhbsp);

	}else if(mlhbsp->uhcall_fn == UAPP_MAVLINKSERHB_UHCALL_SEND){
			uapp_mavlinkserhb_handlehcall_send(mlhbsp);

	}else if(mlhbsp->uhcall_fn == UAPP_MAVLINKSERHB_UHCALL_CHECKRECV){
			uapp_mavlinkserhb_handlehcall_checkrecv(mlhbsp);

	}else if(mlhbsp->uhcall_fn == UAPP_MAVLINKSERHB_UHCALL_RECV){
			uapp_mavlinkserhb_handlehcall_recv(mlhbsp);

	}else if(mlhbsp->uhcall_fn == UAPP_MAVLINKSERHB_UHCALL_ACTIVATEHBHYPTASK){
			uapp_mavlinkserhb_handlehcall_activatehbhyptask(mlhbsp);

	}else if(mlhbsp->uhcall_fn == UAPP_MAVLINKSERHB_UHCALL_DEACTIVATEHBHYPTASK){
			uapp_mavlinkserhb_handlehcall_deactivatehbhyptask(mlhbsp);

	}else{
		//ignore unknown uhcall_fn silently

	}

	return true;
}


//////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////
// mavlinkserhb initialization
//////////////////////////////////////////////////////////////////////////////

//////
// the main initialization function
//////
void uapp_mavlinkserhb_initialize(u32 cpuid){

	if(cpuid == 0){
		_XDPRINTFSMP_("%s[%u]: Initializing mavlinkserhb...\n", __func__, cpuid);

	}else{
		_XDPRINTFSMP_("%s[%u]: AP CPU: nothing to do, moving on...\n", __func__, cpuid);
	}

}


//////////////////////////////////////////////////////////////////////////////

#if 0
void uapp_mavlinkserhb_uartreadexample(void){
	u32 len;
	u8 ch;

	//initialize UART
	uapp_mavlinkserhb_uart_init(115200);

	while(1){
		uapp_mavlinkserhb_uart_recv(&ch, sizeof(ch), &len);
		if(len == 1){
			uapp_mavlinkserhb_uart_send(&ch, sizeof(ch));
			uapp_mavlinkserhb_uart_flush();
		}
	}
}
#endif
