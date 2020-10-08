/*
	scheduler hypapp

	author: amit vasudevan (amitvasudevan@acm.org)
*/

#include <types.h>
#include <arm8-32.h>
#include <bcm2837.h>

#include <uart.h>
#include <debug.h>


#include <hypmtscheduler.h>




//////////////////////////////////////////////////////////////////////////////
// externs
//////////////////////////////////////////////////////////////////////////////


extern void uapp_sched_fiq_handler(void);
extern void uapp_hypmtsched_schedentry(void);

extern u32 uapp_sched_fiqhandler_stack[];
extern u32 uapp_sched_fiqhandler_stack_top[];

extern __attribute__(( section(".data") )) u32 priority_queue_lock=1;
extern __attribute__(( section(".data") )) u32 hypmtscheduler_execution_lock=1;
extern __attribute__((section(".data"))) u32 debug_log_buffer_index;
extern __attribute__((section(".data"))) hypmtscheduler_logentry_t debug_log_buffer[DEBUG_LOG_SIZE];


//////////////////////////////////////////////////////////////////////////////


/** ************************************
 * Dio: I moved the declaration of the hyptask table here to be able to access it 
 * from the timer functions
 */
__attribute__((section(".data"))) hypmtscheduler_hyptask_handle_t hyptask_handle_list[HYPMTSCHEDULER_MAX_HYPTASKS];


//////////////////////////////////////////////////////////////////////////////
// forward function prototypes
//////////////////////////////////////////////////////////////////////////////

u64 uapp_sched_read_cpucounter(void);
void uapp_sched_logic(void);


//////////////////////////////////////////////////////////////////////////////





//////////////////////////////////////////////////////////////////////////////
// global variables
//////////////////////////////////////////////////////////////////////////////

// set to 1 if any timer has expired upon handling a FIQ timer interrupt
__attribute__((section(".data"))) volatile u32 fiq_timer_handler_timerevent_triggered = 0;

// guest mode saved PC/SPSR to resume control at after we finish processing
// hyptask scheduling
__attribute__((section(".data"))) volatile u32 fiq_timer_handler_guestmode_pc = 0;
__attribute__((section(".data"))) volatile u32 fiq_timer_handler_guestmode_spsr = 0;

// set of timers
__attribute__((section(".data"))) struct sched_timer sched_timers[MAX_TIMERS];

// timer we expect to run down next
__attribute__((section(".data"))) struct sched_timer *timer_next = NULL;

// last time when physical timer was programmed
__attribute__((section(".data"))) TIME time_timer_set;


// this is a timer which should never expire, only there to serve as a guard
__attribute__((section(".data"))) struct sched_timer timer_last = {
  FALSE,	//inuse
  FALSE,	//event
  FALSE, 	//disable_tfunc
  0,		//priority
  0,		//first_time_period_expired
  VERY_LONG_TIME,	//sticky_time_to_wait
  VERY_LONG_TIME, 	//regular_time_period
  VERY_LONG_TIME, 	//first_time_period
  VERY_LONG_TIME, 	//time_to_wait
  0					//tfunc
};



#ifdef __ENABLE_UAPP_HYPMTSCHEDULER_SECURE_HYPTASK_BOOTSTRAP__

hypmtscheduler_secure_bootstrap_config_t hyptask_secure_bootstrap_config[] = {
		{ (0.5 * HYPMTSCHEDULER_TIME_1SEC),	//hyptask_first_period
		  (1 * HYPMTSCHEDULER_TIME_1SEC),	//hyptask_regular_period
		  4, 								//hyptask_priority
		  0 								//hyptask_id
		},

		{ (1.5 * HYPMTSCHEDULER_TIME_1SEC),	//hyptask_first_period
		  (2 * HYPMTSCHEDULER_TIME_1SEC),	//hyptask_regular_period
		  3, 								//hyptask_priority
		  1 								//hyptask_id
		},

		{ (2.5 * HYPMTSCHEDULER_TIME_1SEC),	//hyptask_first_period
		  (3 * HYPMTSCHEDULER_TIME_1SEC),	//hyptask_regular_period
		  2, 								//hyptask_priority
		  2 								//hyptask_id
		},

		{ (3.5 * HYPMTSCHEDULER_TIME_1SEC),	//hyptask_first_period
		  (4 * HYPMTSCHEDULER_TIME_1SEC),	//hyptask_regular_period
		  1, 								//hyptask_priority
		  3 								//hyptask_id
		}

};


#define HYPTASK_BSTRAP_CONFIG_TOTAL	1

#endif


//////////////////////////////////////////////////////////////////////////////









//////////////////////////////////////////////////////////////////////////////
// software timer implementation
//////////////////////////////////////////////////////////////////////////////


//////
// initialize timers struct
// run_context = interrupts disabled
//////
void uapp_sched_timers_init(void){
  u32 i;

  for(i=0; i < MAX_TIMERS; i++){
	  sched_timers[i].inuse = FALSE;
	  sched_timers[i].hyptask_handle = -1;
  }
}


//////
// undeclare (and disable) a timer
// run_ccontext =
//////
void uapp_sched_timer_undeclare(struct sched_timer *t){

	if (!t->inuse) {
		return;
	}

	t->inuse = FALSE;
	t->hyptask_handle = -1;

	// check if we were waiting on this one
	if (t == timer_next) {
		uapp_sched_timers_update(uapp_sched_read_cpucounter() - time_timer_set);
		if (timer_next) {
			//debug_log_tsc((u32)timer_next->time_to_wait,
			//		uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_PHYSTIMERPROGRAM_UNDECLARE);
			uapp_sched_start_physical_timer(timer_next->time_to_wait);
			time_timer_set = uapp_sched_read_cpucounter();
		}
	}
}


//////
// instantiate timer for a given struct time entry
// run_context:
//////
struct sched_timer *uapp_sched_timer_instantiate(struct sched_timer *t, u32 first_time_period,
		u32 regular_time_period, int priority, HYPTHREADFUNC func){

	// initialize the timer struct
	t->event = FALSE;
	t->disable_tfunc = FALSE;
	t->first_time_period = first_time_period;
	t->regular_time_period = regular_time_period;
	t->priority = priority;
	t->tfunc = func;
	t->first_time_period_expired = 0;
	t->time_to_wait = first_time_period;
	t->sticky_time_to_wait = regular_time_period;

	if (!timer_next) {
		// no timers set at all, so this is shortest
		time_timer_set = uapp_sched_read_cpucounter();
		//debug_log_tsc((u32)(timer_next = t)->time_to_wait,
		//		uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_PHYSTIMERPROGRAM_INSTANTIATESHORTEST);
		uapp_sched_start_physical_timer((timer_next = t)->time_to_wait);

	} else if ((first_time_period + uapp_sched_read_cpucounter()) < (timer_next->time_to_wait + time_timer_set)) {
		// new timer is shorter than current one, so update all timers and set
		// this timer as the physical timer
		uapp_sched_timers_update(uapp_sched_read_cpucounter() - time_timer_set);
		time_timer_set = uapp_sched_read_cpucounter();
		//debug_log_tsc((u32)(timer_next = t)->time_to_wait,
		//		uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_PHYSTIMERPROGRAM_INSTANTIATESHORTER);
		uapp_sched_start_physical_timer((timer_next = t)->time_to_wait);

	} else {
		// new timer is longer, than current one, do nothing now
		// next call to uapp_sched_timers_update  will take care of bumping this
		// timer to the correct position

	}

	//this timer is in use now
	t->inuse = TRUE;

	return(t);
}


//////
// declare a timer
// returns NULL if we are out of timers
// run_context:
//////
struct sched_timer *uapp_sched_timer_declare(u32 first_time_period,
		u32 regular_time_period, int priority, HYPTHREADFUNC func){

	struct sched_timer *t;

	//grab a free timer entry
	for (t=sched_timers;t<&sched_timers[MAX_TIMERS];t++) {
		if (!t->inuse) break;
	}

	// out of timers?
	if (t == &sched_timers[MAX_TIMERS]) {
		return 0;
	}

	//instantiate this timer and return it
	return uapp_sched_timer_instantiate(t, first_time_period, regular_time_period,
			priority, func);
}


//////
// redeclare an expired timer
//////
struct sched_timer *uapp_sched_timer_redeclare(struct sched_timer *t, u32 first_time_period,
		u32 regular_time_period, int priority, HYPTHREADFUNC func){

	//(re) instantate the timer with specified values
	return uapp_sched_timer_instantiate(t, first_time_period, regular_time_period,
			priority, func);
}


//////
// update the timers subtracting time from all timers,
// enabling those that run out
//////
void uapp_sched_timers_update(TIME time){
	struct sched_timer *t;

	timer_next = &timer_last;

	for (t=sched_timers;t<&sched_timers[MAX_TIMERS];t++) {
		if (t->inuse) {
		  if (time < t->time_to_wait) { // unexpired
			t->time_to_wait -= time;
			if (t->time_to_wait < timer_next->time_to_wait){
			  timer_next = t;
			}

		  } else { // expired
			t->event = TRUE;
			t->inuse = FALSE; 	// remove timer
			fiq_timer_handler_timerevent_triggered=1; //set timerevent_triggered to true
		  }
		}
	  }

	// reset timer_next if no timers found
	if (!timer_next->inuse) {
	  timer_next = 0;
	}
}


//////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////
// interrupt control and physical timer functions
//////////////////////////////////////////////////////////////////////////////

#define PMCNTNSET_C_BIT		0x80000000
#define PMCR_C_BIT			0x00000004
#define PMCR_E_BIT			0x00000001


//////
// initialize CPU time-stamp counter
//////
void uapp_sched_init_cputsc(void){
	unsigned long tmp;

	tmp = PMCNTNSET_C_BIT;
	asm volatile ("mcr p15, 0, %0, c9, c12, 1" : : "r" (tmp));
	asm volatile ("mrc p15, 0, %0, c9, c12, 0" : "=r" (tmp));
	tmp |= PMCR_C_BIT | PMCR_E_BIT;
	asm volatile ("mcr p15, 0, %0, c9, c12, 0" : : "r" (tmp));
}


//////
// read CPU's 64-bit time-stamp counter value
//////
u64 uapp_sched_rdtsc64(void){
	u32 tsc_lo, tsc_hi;
	u64 l_tickcount;

	asm volatile
		(	" isb\r\n"
			" mrrc p15, 0, r0, r1, c9 \r\n"
			" mov %0, r0 \r\n"
			" mov %1, r1 \r\n"
				: "=r" (tsc_lo), "=r" (tsc_hi) // outputs
				: // inputs
	           : "r0", "r1" //clobber
	    );

	l_tickcount = tsc_hi;
	l_tickcount = l_tickcount << 32;
	l_tickcount |= tsc_lo;

	return l_tickcount;
}


//////
// enable FIQs
//////
void enable_fiq(void){
	u32 cpsr;
	cpsr = sysreg_read_cpsr();
	cpsr &= ~(1UL << 6);	//clear CPSR.F to allow FIQs
	sysreg_write_cpsr(cpsr);
}


//////
// disable FIQs
//////
void disable_fiq(void){
	u32 cpsr;
	cpsr = sysreg_read_cpsr();
	cpsr |= (1UL << 6);	//set CPSR.F to prevent FIQs
	sysreg_write_cpsr(cpsr);
}


//////
// read current physical timer counter; we use this as current time
//////
u64 uapp_sched_read_cpucounter(void){
	return sysreg_read_cntpct();
}


//////
// start physical timer to fire off after specified clock ticks
//////
void uapp_sched_start_physical_timer(TIME time){
	sysreg_write_cnthp_tval(time);
	sysreg_write_cnthp_ctl(0x1);
}


//////
// stop physical timer
//////
void uapp_sched_stop_physical_timer(void){
	sysreg_write_cnthp_ctl(0x0);
}


//////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////
// hypmtscheduler non-preemptive priority scheduling logic
//////////////////////////////////////////////////////////////////////////////


//////
// process all the timers that have expired, entering them into the
// priority queue iff a disable signal is absent
//////
void uapp_sched_process_timers(u32 cpuid){
	u32 i;
	u32 time_to_wait;
	int priority;

	for(i=0; i < MAX_TIMERS; i++){
		if(sched_timers[i].event){
			sched_timers[i].event = FALSE;
			if(sched_timers[i].tfunc && !sched_timers[i].disable_tfunc){
				priority_queue_insert((void *)&sched_timers[i], sched_timers[i].priority);
			}

			if (sched_timers[i].hyptask_handle >=0){
			  hyptask_handle_list[sched_timers[i].hyptask_handle].valid_guest_mask &= ~GUEST_JOB_START_VALID_MASK;
			  hyptask_handle_list[sched_timers[i].hyptask_handle].last_enforcement_timestamp = uapp_sched_read_cpucounter();
			  hyptask_handle_list[sched_timers[i].hyptask_handle].num_periods ++;
			}

			time_to_wait = sched_timers[i].regular_time_period; //reload
			priority = sched_timers[i].priority;
			uapp_sched_timer_redeclare(&sched_timers[i], time_to_wait, time_to_wait, priority, sched_timers[i].tfunc);
		}
	}
}


//////
// process priority queue and run hyptasks associated with the timers
// hyptasks are run with interrupts enabled so we can continue to process
// timers that expire during hyptask execution
//////
void uapp_sched_run_hyptasks(void){
	int status;
	u32 queue_data;
	int priority;
	int cnt=0;
	struct sched_timer *task_timer;


	status=0;
	while(1){
	        cnt++;
		if (cnt > 5){
		  uart_puts("ERROR: Scheduling more than 5!!\n");
		  break;
		}
		
		status = priority_queue_remove(&queue_data, &priority);
		if(status == 0)
			break;
		task_timer = (struct sched_timer *)queue_data;

		//interrupts enable
		enable_fiq();

		if(task_timer->tfunc)
			task_timer->tfunc(task_timer);

		//interrupts disable
		disable_fiq();
	}
}



//////
// FIQ timer handler
//////
void uapp_sched_fiqhandler(void){
	uapp_sched_timerhandler();
}


//////
// FIQ timer handler
//////
void uapp_sched_timerhandler(void){

	//debug_log_tsc(0xFFFFFFFFFUL, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_TIMERHANDLER_BEFORE);

	//stop physical timer
	uapp_sched_stop_physical_timer();

	//set timerevent_triggered flag to false (0)
	fiq_timer_handler_timerevent_triggered=0;

	//update sw timers
	uapp_sched_timers_update(uapp_sched_read_cpucounter() - time_timer_set);

	// start physical timer for next shortest time if one exists
	if (timer_next) {
		time_timer_set = uapp_sched_read_cpucounter();
		//debug_log_tsc((u32)timer_next->time_to_wait,
		//		uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_PHYSTIMERPROGRAM_TIMERHANDLER);
		uapp_sched_start_physical_timer(timer_next->time_to_wait);
	}

	if (fiq_timer_handler_timerevent_triggered == 0){
		//no timers expired so just return from timer interrupt
		//debug_log_tsc(0xFFFFFFFFFUL, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_TIMERHANDLER_AFTER_NOTIMERSEXPIRED);
		return;

	}else{
		//timer has expired, so let us look at the PE state
		//which triggered this timer FIQ to decide on our course of
		//action
		if( (sysreg_read_spsr_hyp() & 0x0000000FUL) == 0xA){
			//PE state was hyp mode, so we simply resume
			//debug_log_tsc(0xFFFFFFFFFUL, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_TIMERHANDLER_AFTER_TIMEREXPIREDINHYP);
			return;
		}else{
			//PE state says we are in guest mode, so stow away guest mode
			//pc and spsr so we can resume guest after processing all the
			//timers that have expired
			fiq_timer_handler_guestmode_pc = sysreg_read_elrhyp();
			fiq_timer_handler_guestmode_spsr = sysreg_read_spsr_hyp();

			//resume at uapp_sched_logic in HYP mode
			sysreg_write_elrhyp(&uapp_hypmtsched_schedentry);
	    	sysreg_write_spsr_hyp(0x000001DA);
			//debug_log_tsc(0xFFFFFFFFFUL, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_TIMERHANDLER_AFTER_TIMEREXPIREDGOTOSCHEDLOGIC);
	    	return;
		}
	}
}



//////
// scheduling logic that runs in HYP mode to process timers that
// have expired
//////
void uapp_sched_logic(void){
	struct sched_timer *task_timer;
	u32 queue_data;
	int priority;
	int status;
	volatile u32 sp, spnew;

	//debug_log_tsc(0xFFFFFFFFFUL, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_SCHEDLOGIC_BEFORE);

	//process expired timers
	uapp_sched_process_timers(0); //TBD: remove hard-coded cpuid (0)

	//run all queued hyptasks
	while(1){
		uapp_sched_run_hyptasks();
		uapp_sched_process_timers(0); //TBD: remove hard-coded cpuid (0)
		if(priority_queue_isempty())
			break;
	}

	//resume guest
	sysreg_write_elrhyp(fiq_timer_handler_guestmode_pc);
   	sysreg_write_spsr_hyp(fiq_timer_handler_guestmode_spsr);
   	fiq_timer_handler_guestmode_pc = 0;
   	fiq_timer_handler_guestmode_spsr = 0;

	//debug_log_tsc(0xFFFFFFFFFUL, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_SCHEDLOGIC_AFTER);

   	return;
}


//////////////////////////////////////////////////////////////////////////////


/// Dio: utility to plot 'busy' time
// pi3-1.2GHz
#define IN_LOOP_ONE_MS 70575
#define NUM_IN_LOOPS_ONE_MS 1
#define NUM_SILENT_ITERATIONS 10000

void busy(long millis)
{
  long i,j,k,s;  
  s=0;
  for (k=0;k<millis;k++)
    for (i=0;i<NUM_IN_LOOPS_ONE_MS;i++)
      for (j=0;j<IN_LOOP_ONE_MS;j++){
        s++;
	if (s >= NUM_SILENT_ITERATIONS){
	  s=0;
	}
      }
}

uint8_t heartbeat[] = { 0xfd, 0x09, 0x00, 0x00, 0x00, 0x00, 0xfb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x08, 0x00, 0x00, 0x03, 0x2f, 0x0f,0xfd, 0x35, 0x00, 0x00, 0x00, 0x00, 0xfb, 0x54, 0x00, 0x00, 0x1a, 0x9f, 0x62, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc7, 0x09, 0x00, 0x00, 0x01, 0x6c, 0xc1};


//////////////////////////////////////////////////////////////////////////////
// hyptask definitions
//////////////////////////////////////////////////////////////////////////////

void hyptask0(struct sched_timer *t){
  debug_log_tsc(0, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_HYPTASKEXEC_BEFORE);
  /* uapp_mavlinkserhb_uart_send(heartbeat, sizeof(heartbeat)); */
  /* uapp_mavlinkserhb_uart_flush(); */
  /*busy(10);*/
  //uart_puts("A");
  debug_log_tsc(0, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_HYPTASKEXEC_AFTER);
}

void hyptask1(struct sched_timer *t){
  debug_log_tsc(1, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_HYPTASKEXEC_BEFORE);
	/* //busy(10); */
  debug_log_tsc(1, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_HYPTASKEXEC_AFTER);
  //uart_puts("B");
}


void hyptask2(struct sched_timer *t){
  debug_log_tsc(2, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_HYPTASKEXEC_BEFORE);
	/* //busy(10); */
  debug_log_tsc(2, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_HYPTASKEXEC_AFTER);
  //uart_puts("C");
}

void hyptask3(struct sched_timer *t){
  debug_log_tsc(3, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_HYPTASKEXEC_BEFORE);
	/* //busy(10); */
  //uart_puts("hyptaskid=3 executed");
  debug_log_tsc(3, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_HYPTASKEXEC_AFTER);
}

__attribute__((section(".data"))) HYPTHREADFUNC hyptask_idlist[HYPMTSCHEDULER_MAX_HYPTASKID] =
		{
			&hyptask0,
			&hyptask1,
			&hyptask2,
			&hyptask3
		};


/******************************************************
 * Dio: I moved the declaration of the hyptask table
 * to the top to be able to access it from the
 * timer functions
 ******************************************************/
//__attribute__((section(".data"))) hypmtscheduler_hyptask_handle_t hyptask_handle_list[HYPMTSCHEDULER_MAX_HYPTASKS];


//////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////
// hypmtscheduler hypercall APIs
//////////////////////////////////////////////////////////////////////////////


//////
// create hyptask API
//////
void uapp_hypmtscheduler_handlehcall_createhyptask(ugapp_hypmtscheduler_param_t *hmtsp){
	uint32_t hyptask_first_period = hmtsp->iparam_1;
	uint32_t hyptask_regular_period = hmtsp->iparam_2;
	uint32_t hyptask_priority = hmtsp->iparam_3;
	uint32_t hyptask_id = hmtsp->iparam_4;
	uint32_t i;
	uint32_t hyptask_handle_found;
	TIME now;


	debug_log_tsc(hyptask_id, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_CREATEHYPTASK_BEFORE);
	now = hyptask_first_period;
	debug_log_tsc(hyptask_id, now , DEBUG_LOG_EVTTYPE_FIRST_PERIOD_PARAM);
	now = hyptask_regular_period;
	debug_log_tsc(hyptask_id, now, DEBUG_LOG_EVTTYPE_REGULAR_PERIOD_PARAM);

	//allocate hyptask_handle
	hyptask_handle_found=0;
	for(i=0; i< HYPMTSCHEDULER_MAX_HYPTASKS; i++){
		if(!hyptask_handle_list[i].inuse){
			hyptask_handle_list[i].inuse = true;
			hyptask_handle_found=1;
			break;
		}
	}

	//check if we were able to allocate a hytask handle
	if(!hyptask_handle_found){
		hmtsp->status=0; //fail
		return;
	}

	//now check if the given hyptask_id is valid
	if(hyptask_id > (HYPMTSCHEDULER_MAX_HYPTASKID-1) ){
		hmtsp->status=0; //fail
		return;
	}

	//ok now populate the hyptask_id within the hyptask handle
	hyptask_handle_list[i].hyptask_id = hyptask_id;

	// record creation timestampt before programming the timer
	// we are pulling back the clock by 1000 ticks : debugging
	hyptask_handle_list[i].guest_task_creation_timestamp = uapp_sched_read_cpucounter();// - 1000;
	hyptask_handle_list[i].guest_job_start_timestamp = 0;
	hyptask_handle_list[i].num_periods = 0;

	
	hyptask_handle_list[i].t = uapp_sched_timer_declare(hyptask_first_period, hyptask_regular_period,
			hyptask_priority,
			hyptask_idlist[hyptask_id]);

	//check if we were able to allocate a timer for the hyptask
	if(!hyptask_handle_list[i].t){
		hmtsp->status=0; //fail
		return;
	}

	hyptask_handle_list[i].t->hyptask_handle = i;

	/*
	hyptask_handle_list[i].guest_task_creation_timestamp = uapp_sched_read_cpucounter();
	hyptask_handle_list[i].guest_job_start_timestamp = 0;
	hyptask_handle_list[i].num_periods = 0;
	*/

	debug_log_tsc(hyptask_id, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_CREATEHYPTASK_AFTER);

	hmtsp->oparam_1 = i;	//return hyptask handle
	hmtsp->status=1;	//success
}


//////
// disable hyptask API
//////
void uapp_hypmtscheduler_handlehcall_disablehyptask(ugapp_hypmtscheduler_param_t *hmtsp){
	uint32_t hyptask_handle = hmtsp->iparam_1;
	struct sched_timer *hyptask_timer;
	uint32_t i;
	uint32_t hyptask_handle_found;

	//check if provided hyptask handle is within limits
	if(hyptask_handle >= HYPMTSCHEDULER_MAX_HYPTASKS){
		hmtsp->status=0; //fail
		return;
	}

	//check if provided hyptask handle is in use
	if(!hyptask_handle_list[hyptask_handle].inuse){
		hmtsp->status=0; //fail
		return;
	}

	debug_log_tsc(hyptask_handle_list[hyptask_handle].hyptask_id, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_DISABLEHYPTASK_BEFORE);

	hyptask_handle_list[hyptask_handle].guest_job_end_timestamp = uapp_sched_read_cpucounter();

	//ok grab the timer for the hyptask
	hyptask_timer = hyptask_handle_list[hyptask_handle].t;

	//only disable hyptask if guest job started and ended in the same period
	//there are other conditions as below
	//there should only be one job per period
	//only one job and job started in the previous period --> invalid 
	if ( (hyptask_handle_list[hyptask_handle].valid_guest_mask & GUEST_JOB_START_VALID_MASK)
	     &&
	     (
	      (hyptask_handle_list[hyptask_handle].guest_job_end_timestamp <= hyptask_handle_list[hyptask_handle].guest_task_creation_timestamp +
	       hyptask_handle_list[hyptask_handle].t->first_time_period)
	      ||
	      (
	       hyptask_handle_list[hyptask_handle].guest_job_end_timestamp >=  hyptask_handle_list[hyptask_handle].guest_task_creation_timestamp +
	       (hyptask_handle_list[hyptask_handle].t->regular_time_period * hyptask_handle_list[hyptask_handle].num_periods)
	       &&
		hyptask_handle_list[hyptask_handle].guest_job_end_timestamp <=  hyptask_handle_list[hyptask_handle].guest_task_creation_timestamp +
		(hyptask_handle_list[hyptask_handle].t->regular_time_period * hyptask_handle_list[hyptask_handle].num_periods) +
		hyptask_handle_list[hyptask_handle].t->first_time_period
	      )
	      )
	     )
	  {
	    hyptask_handle_list[hyptask_handle].valid_guest_mask |= GUEST_JOB_END_VALID_MASK;
	  }
	else
	  {
	    hyptask_handle_list[hyptask_handle].valid_guest_mask &= ~GUEST_JOB_END_VALID_MASK;
	  }

	if ((hyptask_handle_list[hyptask_handle].valid_guest_mask & GUEST_JOB_END_VALID_MASK))
	  {
	    //set the disabled flag so that hyptask function is not executed
	    hyptask_timer->disable_tfunc = TRUE;
	  }		    


	// Dio: debug conditions that are not working
	// hyptask_timer->disable_tfunc = TRUE;

	if (!(hyptask_handle_list[hyptask_handle].valid_guest_mask & GUEST_JOB_START_VALID_MASK)){
	  debug_log_tsc(hyptask_handle_list[hyptask_handle].hyptask_id, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_DISABLEHYPTASK_INVALID_START);
	  debug_log_tsc(hyptask_handle_list[hyptask_handle].hyptask_id, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_INVALID_START_NUM_PERIOD_OFFSET+hyptask_handle_list[hyptask_handle].num_periods);
	}
	
	hmtsp->status=1; //success

	debug_log_tsc(hyptask_handle_list[hyptask_handle].hyptask_id, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_DISABLEHYPTASK_AFTER);
}



//////
// mark start of guest job for hyptask API
//////
void uapp_hypmtscheduler_handlehcall_guestjobstart(ugapp_hypmtscheduler_param_t *hmtsp){
	uint32_t hyptask_handle = hmtsp->iparam_1;
	struct sched_timer *hyptask_timer;
	uint32_t i;
	uint32_t hyptask_handle_found;
	TIME now;

	//check if provided hyptask handle is within limits
	if(hyptask_handle >= HYPMTSCHEDULER_MAX_HYPTASKS){
		hmtsp->status=0; //fail
		return;
	}

	//check if provided hyptask handle is in use
	if(!hyptask_handle_list[hyptask_handle].inuse){
		hmtsp->status=0; //fail
		return;
	}

	debug_log_tsc(hyptask_handle_list[hyptask_handle].hyptask_id, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_STARTGUESTJOB_BEFORE);

	now = uapp_sched_read_cpucounter();
	
	if (
	       (hyptask_handle_list[hyptask_handle].guest_job_start_timestamp == 0 &&
	          now <= hyptask_handle_list[hyptask_handle].guest_task_creation_timestamp + hyptask_handle_list[hyptask_handle].t->first_time_period
	       ) ||
	       (hyptask_handle_list[hyptask_handle].guest_job_start_timestamp <= hyptask_handle_list[hyptask_handle].last_enforcement_timestamp
		&&
		now >=  hyptask_handle_list[hyptask_handle].guest_task_creation_timestamp +
		(hyptask_handle_list[hyptask_handle].t->regular_time_period * hyptask_handle_list[hyptask_handle].num_periods)
		&&
		now <=  hyptask_handle_list[hyptask_handle].guest_task_creation_timestamp +
		(hyptask_handle_list[hyptask_handle].t->regular_time_period * hyptask_handle_list[hyptask_handle].num_periods) +
		hyptask_handle_list[hyptask_handle].t->first_time_period
		)
	    )
	  {
	    hyptask_handle_list[hyptask_handle].valid_guest_mask |= GUEST_JOB_START_VALID_MASK ;
	  } else
	  {
	    hyptask_handle_list[hyptask_handle].valid_guest_mask &= ~GUEST_JOB_START_VALID_MASK ;
	  }

	hyptask_handle_list[hyptask_handle].guest_job_start_timestamp = now;
	
	hmtsp->status=1; //success

	debug_log_tsc(hyptask_handle_list[hyptask_handle].hyptask_id, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_STARTGUESTJOB_AFTER);
}


//////
// delete hyptask API
//////
void uapp_hypmtscheduler_handlehcall_deletehyptask(ugapp_hypmtscheduler_param_t *hmtsp){
	uint32_t hyptask_handle = hmtsp->iparam_1;
	struct sched_timer *hyptask_timer;
	u32 hyptask_id;

	//check if provided hyptask handle is within limits
	if(hyptask_handle >= HYPMTSCHEDULER_MAX_HYPTASKS){
		hmtsp->status=0; //fail
		return;
	}

	//check if provided hyptask handle is in use
	if(!hyptask_handle_list[hyptask_handle].inuse){
		hmtsp->status=0; //fail
		return;
	}

	hyptask_id = hyptask_handle_list[hyptask_handle].hyptask_id;

	debug_log_tsc(hyptask_id, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_DELETEHYPTASK_BEFORE);

	//ok grab the timer for the hyptask
	hyptask_timer = hyptask_handle_list[hyptask_handle].t;

	//disable the timer assocated with this hyptask
	uapp_sched_timer_undeclare(hyptask_timer);

	//reset handle for future use
	hyptask_handle_list[hyptask_handle].inuse = FALSE;
	hyptask_handle_list[hyptask_handle].hyptask_id = 0;
	hyptask_handle_list[hyptask_handle].t = NULL;

	hmtsp->status=1; //success

	debug_log_tsc(hyptask_id, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_DELETEHYPTASK_AFTER);
}


//////
// init TSC API
//////
void uapp_hypmtscheduler_handlehcall_inittsc(ugapp_hypmtscheduler_param_t *hmtsp){
	debug_log_tsc(0xFFFFFFFFUL, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_INITTSC_BEFORE);
	uapp_sched_init_cputsc();
	hmtsp->status=1; //success
	debug_log_tsc(0xFFFFFFFFUL, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_INITTSC_AFTER);
}


//////
//dump debug log API
//////
void uapp_hypmtscheduler_handlehcall_dumpdebuglog(ugapp_hypmtscheduler_param_t *hmtsp){
	u8 *debug_log_target_buffer = (u8 *)hmtsp->iparam_1;

	//we need a non-NULL log target buffer. if not bail out gracefully
	if(!debug_log_target_buffer){
		hmtsp->status=0;
		return;
	}

	//copy over the debug log for the number of entries logged
	memcpy(debug_log_target_buffer, &debug_log_buffer,
			debug_log_buffer_index * sizeof(hypmtscheduler_logentry_t));

	hmtsp->oparam_1 = debug_log_buffer_index;
	hmtsp->status=1; //success

	// Dio: resetting buffer to zero
	debug_log_buffer_index = 0;
}


//////
// top-level hypercall handler hub
// return true if handled the hypercall, false if not
//////
bool uapp_hypmtscheduler_handlehcall(u32 uhcall_function, void *uhcall_buffer,
		u32 uhcall_buffer_len){
	ugapp_hypmtscheduler_param_t *hmtsp;

	//debug_log_tsc(0xFFFFFFFFUL, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_HANDLEHCALL_BEFORE);

	if(uhcall_function != UAPP_HYPMTSCHEDULER_UHCALL){
		return false;
	}

	hmtsp = (ugapp_hypmtscheduler_param_t *)uhcall_buffer;

	if(hmtsp->uhcall_fn == UAPP_HYPMTSCHEDULER_UHCALL_CREATEHYPTASK){

#ifndef __ENABLE_UAPP_HYPMTSCHEDULER_SECURE_HYPTASK_BOOTSTRAP__
		uapp_hypmtscheduler_handlehcall_createhyptask(hmtsp);
#endif

	}else if(hmtsp->uhcall_fn == UAPP_HYPMTSCHEDULER_UHCALL_DISABLEHYPTASK){
		uapp_hypmtscheduler_handlehcall_disablehyptask(hmtsp);
	}else if(hmtsp->uhcall_fn == UAPP_HYPMTSCHEDULER_UHCALL_GUESTJOBSTART){
	        uapp_hypmtscheduler_handlehcall_guestjobstart(hmtsp);
	}else if(hmtsp->uhcall_fn == UAPP_HYPMTSCHEDULER_UHCALL_DELETEHYPTASK){
		uapp_hypmtscheduler_handlehcall_deletehyptask(hmtsp);

	}else if(hmtsp->uhcall_fn == UAPP_HYPMTSCHEDULER_UHCALL_INITTSC){
		uapp_hypmtscheduler_handlehcall_inittsc(hmtsp);

	}else if(hmtsp->uhcall_fn == UAPP_HYPMTSCHEDULER_UHCALL_DUMPDEBUGLOG){
		uapp_hypmtscheduler_handlehcall_dumpdebuglog(hmtsp);

	}else{
		uart_puts("\nHYPMTSCHED: UHCALL: ignoring unknown uhcall_fn=0x");
		debug_hexdumpu32(hmtsp->uhcall_fn);
		uart_puts("\n");
	}

	//debug_log_tsc(0xFFFFFFFFUL, uapp_sched_read_cpucounter(), DEBUG_LOG_EVTTYPE_HANDLEHCALL_AFTER);

	return true;
}


//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
// hypmtscheduler helper functions
//////////////////////////////////////////////////////////////////////////////

#ifdef __ENABLE_UAPP_HYPMTSCHEDULER_SECURE_HYPTASK_BOOTSTRAP__

void uapp_sched_bootstrap_hyptasks(u32 cpuid){
	uint32_t i;
	ugapp_hypmtscheduler_param_t hmtsp;

	for(i=0; i < HYPTASK_BSTRAP_CONFIG_TOTAL; i++){
		hmtsp.iparam_1 = hyptask_secure_bootstrap_config[i].hyptask_first_period;
		hmtsp.iparam_2 = hyptask_secure_bootstrap_config[i].hyptask_regular_period;
		hmtsp.iparam_3 = hyptask_secure_bootstrap_config[i].hyptask_priority;
		hmtsp.iparam_4 = hyptask_secure_bootstrap_config[i].hyptask_id;

		uapp_hypmtscheduler_handlehcall_createhyptask(&hmtsp);

		if(hmtsp.status != 1){
			_XDPRINTFSMP_("%s[%u]: Halting!. Error in bootstrapping hyptask index=%u\n",
					__func__, cpuid, i);
			while(1);
		}
	}

}

#endif


//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
// hypmtscheduler initialization
//////////////////////////////////////////////////////////////////////////////


//////
// initialize hypmtscheduler timing functionality
//////
void uapp_sched_timer_initialize(u32 cpuid){
	u32 cpsr;
	u64 cntpct_val;
	u32 cpu0_tintctl_value;
	u32 loop_counter=0;

	_XDPRINTFSMP_("%s[%u]: ENTER\n", __func__, cpuid);

	//disable FIQs
	disable_fiq();

	cpsr = sysreg_read_cpsr();
	_XDPRINTFSMP_("%s[%u]: CPSR[after disable_fiq]=0x%08x; CPSR.A=%u, CPSR.I=%u, CPSR.F=%u\n",
			__func__, cpuid, cpsr, ((cpsr & (1UL << 8)) >> 8),
			((cpsr & (1UL << 7)) >> 7),
			((cpsr & (1UL << 6)) >> 6) );


	//enable cpu0 timer interrupt control to generate FIQs
	cpu0_tintctl_value = mmio_read32(LOCAL_TIMER_INT_CONTROL0);
	_XDPRINTFSMP_("%s[%u]: cpu0_tintctl_value[before]=0x%08x, CNTHPFIQ=%u, CNTHPIRQ=%u\n",
			__func__, cpuid,
			cpu0_tintctl_value,
			((cpu0_tintctl_value & (1UL << 6)) >> 6),
			((cpu0_tintctl_value & (1UL << 2)) >> 2)
			);

	cpu0_tintctl_value &= ~(1UL << 2); //disable IRQs
	cpu0_tintctl_value |= (1UL << 6); //enable FIQs
	mmio_write32(LOCAL_TIMER_INT_CONTROL0, cpu0_tintctl_value);


	cpu0_tintctl_value = mmio_read32(LOCAL_TIMER_INT_CONTROL0);
	_XDPRINTFSMP_("%s[%u]: cpu0_tintctl_value[after]=0x%08x, CNTHPFIQ=%u, CNTHPIRQ=%u\n",
			__func__, cpuid,
			cpu0_tintctl_value,
			((cpu0_tintctl_value & (1UL << 6)) >> 6),
			((cpu0_tintctl_value & (1UL << 2)) >> 2)
			);

	cpsr = sysreg_read_cpsr();
	_XDPRINTFSMP_("%s[%u]: CPSR[after enable_fiq]=0x%08x; CPSR.A=%u, CPSR.I=%u, CPSR.F=%u\n",
			__func__, cpuid, cpsr, ((cpsr & (1UL << 8)) >> 8),
			((cpsr & (1UL << 7)) >> 7),
			((cpsr & (1UL << 6)) >> 6) );


	//read out generic timer frequency
	_XDPRINTFSMP_("%s[%u]: CNTFRQ=%u\n",
			__func__, cpuid, sysreg_read_cntfrq());


	_XDPRINTFSMP_("%s[%u]: EXIT\n", __func__, cpuid);
}



//////
// the main initialization function
//////
void uapp_sched_initialize(u32 cpuid){
	int value;
	int priority;
	struct sched_timer *task_timer;
	u32 queue_data;
	int status;
	volatile u32 sp, spnew;


	if(cpuid == 0){
		_XDPRINTFSMP_("%s[%u]: Initializing scheduler...\n", __func__, cpuid);

		//normal_sp =sysreg_read_sp();
		_XDPRINTFSMP_("%s[%u]: FIQ Stack pointer base=0x%08x\n", __func__, cpuid,
				&uapp_sched_fiqhandler_stack);
		_XDPRINTFSMP_("%s[%u]: normal_sp=0x%08x\n", __func__, cpuid, sysreg_read_sp());
		_XDPRINTFSMP_("%s[%u]: cpsr=0x%08x\n", __func__, cpuid, sysreg_read_cpsr());

		_XDPRINTFSMP_("%s[%u]: proceeding to initialize TSC...\n",
				__func__, cpuid);
		uapp_sched_init_cputsc();
		_XDPRINTFSMP_("%s[%u]: TSC initialized\n",
				__func__, cpuid);


		_XDPRINTFSMP_("%s[%u]: Current CPU counter=0x%016llx\n", __func__, cpuid,
				uapp_sched_read_cpucounter());

		_XDPRINTFSMP_("%s[%u]: Current CPU counter=0x%016llx\n", __func__, cpuid,
				uapp_sched_read_cpucounter());

		_XDPRINTFSMP_("%s[%u]: Current CPU counter=0x%016llx\n", __func__, cpuid,
				uapp_sched_read_cpucounter());

		//zero-initialize hyptask_handle_list
		memset(&hyptask_handle_list, 0, sizeof(hyptask_handle_list));

		//initialize timers
		uapp_sched_timer_initialize(cpuid);

		//declare a keep-alive timer to initialize timer subsystem
		//uapp_sched_timer_declare((20 * HYPMTSCHEDULER_TIME_1SEC), (20 * HYPMTSCHEDULER_TIME_1SEC), 1, &hyptask0);

#ifdef __ENABLE_UAPP_HYPMTSCHEDULER_SECURE_HYPTASK_BOOTSTRAP__
		
		uapp_sched_bootstrap_hyptasks(cpuid);
#endif

	}else{
		_XDPRINTFSMP_("%s[%u]: AP CPU: nothing to do, moving on...\n", __func__, cpuid);
	}

}


//////////////////////////////////////////////////////////////////////////////
