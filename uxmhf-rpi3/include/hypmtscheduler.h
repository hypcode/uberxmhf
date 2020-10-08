/*
	hypervisor mixed-trust scheduler

	author: amit vasudevan (amitvasudevan@acm.org)
*/

#ifndef __HYPMTSCHEDULER_H__
#define __HYPMTSCHEDULER_H__

#define UAPP_HYPMTSCHEDULER_UHCALL	0xC0

#define UAPP_HYPMTSCHEDULER_UHCALL_CREATEHYPTASK	1
#define UAPP_HYPMTSCHEDULER_UHCALL_DISABLEHYPTASK	2
#define UAPP_HYPMTSCHEDULER_UHCALL_DELETEHYPTASK	3
#define UAPP_HYPMTSCHEDULER_UHCALL_GETRAWTICK		4
#define UAPP_HYPMTSCHEDULER_UHCALL_INITTSC		5
#define UAPP_HYPMTSCHEDULER_UHCALL_LOGTSC		6
#define UAPP_HYPMTSCHEDULER_UHCALL_DUMPDEBUGLOG		7
#define UAPP_HYPMTSCHEDULER_UHCALL_GUESTJOBSTART        8


#define HYPMTSCHEDULER_MAX_HYPTASKID	4
#define HYPMTSCHEDULER_MAX_HYPTASKS	4

//debug logging event types
#define DEBUG_LOG_EVTTYPE_CREATEHYPTASK_BEFORE			50
#define DEBUG_LOG_EVTTYPE_CREATEHYPTASK_AFTER			51
#define DEBUG_LOG_EVTTYPE_DISABLEHYPTASK_BEFORE			52
#define DEBUG_LOG_EVTTYPE_DISABLEHYPTASK_AFTER			53
#define DEBUG_LOG_EVTTYPE_DELETEHYPTASK_BEFORE			54
#define DEBUG_LOG_EVTTYPE_DELETEHYPTASK_AFTER			55
#define DEBUG_LOG_EVTTYPE_INITTSC_BEFORE			56
#define DEBUG_LOG_EVTTYPE_INITTSC_AFTER				57
#define DEBUG_LOG_EVTTYPE_HYPTASKEXEC_BEFORE			58
#define DEBUG_LOG_EVTTYPE_HYPTASKEXEC_AFTER			59
#define DEBUG_LOG_EVTTYPE_HANDLEHCALL_BEFORE			60
#define DEBUG_LOG_EVTTYPE_HANDLEHCALL_AFTER			61
#define DEBUG_LOG_EVTTYPE_SCHEDLOGIC_BEFORE			62
#define DEBUG_LOG_EVTTYPE_SCHEDLOGIC_AFTER			63
#define DEBUG_LOG_EVTTYPE_TIMERHANDLER_BEFORE			64
#define DEBUG_LOG_EVTTYPE_TIMERHANDLER_AFTER_NOTIMERSEXPIRED	65
#define DEBUG_LOG_EVTTYPE_TIMERHANDLER_AFTER_TIMEREXPIREDINHYP	66
#define DEBUG_LOG_EVTTYPE_TIMERHANDLER_AFTER_TIMEREXPIREDGOTOSCHEDLOGIC 67

#define DEBUG_LOG_EVTTYPE_PHYSTIMERPROGRAM_TIMERHANDLER 	68
#define DEBUG_LOG_EVTTYPE_PHYSTIMERPROGRAM_UNDECLARE 		69
#define DEBUG_LOG_EVTTYPE_PHYSTIMERPROGRAM_INSTANTIATESHORTEST 	70
#define DEBUG_LOG_EVTTYPE_PHYSTIMERPROGRAM_INSTANTIATESHORTER 	71

#define DEBUG_LOG_EVTTYPE_STARTGUESTJOB_BEFORE			72
#define DEBUG_LOG_EVTTYPE_STARTGUESTJOB_AFTER			73
#define DEBUG_LOG_EVTTYPE_DISABLEHYPTASK_INVALID_START          74

#define DEBUG_LOG_EVTTYPE_FIRST_PERIOD_PARAM                    80
#define DEBUG_LOG_EVTTYPE_REGULAR_PERIOD_PARAM                  90

#define DEBUG_LOG_EVTTYPE_INVALID_START_NUM_PERIOD_OFFSET       100



//#define DEBUG_LOGGING_SERIAL	1


#ifndef __ASSEMBLY__

#define TRUE  	1
#define FALSE 	0

#define MAX_TIMERS	4	//number of timers
typedef uint64_t TIME;   	//our time type; 64-bits since we are using clock cycles
#define VERY_LONG_TIME  0xffffffffffffffffULL	//longest time possible

#define HYPMTSCHEDULER_TIME_1SEC			19200000UL
//#define HYPMTSCHEDULER_TIME_1SEC			15518102UL
#define HYPMTSCHEDULER_TIME_1MSEC			(HYPMTSCHEDULER_TIME_1SEC / 1000)
#define HYPMTSCHEDULER_TIME_1USEC			(HYPMTSCHEDULER_TIME_1MSEC / 1000)

struct sched_timer;

typedef void (*HYPTHREADFUNC)(struct sched_timer *);

struct sched_timer {
	uint32_t inuse;			// TRUE if in use
	uint32_t event;    		// set to TRUE at timeout
	uint32_t disable_tfunc;	// set to TRUE if tfunc should not be invoked
	int priority;		// priority associated with the timer
	int first_time_period_expired;	//1 if first_time_period has expired, 0 otherwise
	TIME sticky_time_to_wait;  // relative time to wait sticky
	TIME regular_time_period;	//the regular time period of this timer
	TIME first_time_period; //the first time period of this timer
	TIME time_to_wait;  // relative time to wait
	HYPTHREADFUNC tfunc;	//the hypthread function associated with the timer
        uint32_t hyptask_handle;
};

typedef struct {
	uint8_t uhcall_fn;
	uint32_t iparam_1;
	uint32_t iparam_2;
	uint32_t iparam_3;
	uint32_t iparam_4;
	uint32_t oparam_1;
	uint32_t oparam_2;
	uint32_t status;
}ugapp_hypmtscheduler_param_t;


#define GUEST_JOB_START_VALID_MASK 0x01
#define GUEST_JOB_END_VALID_MASK 0x02

typedef struct {
	uint32_t inuse;
	uint32_t hyptask_id;
	struct sched_timer *t;
        u64 guest_task_creation_timestamp;
        u64 guest_job_start_timestamp;
        u64 guest_job_end_timestamp;
        u64 last_enforcement_timestamp;
        uint8_t valid_guest_mask;
        u64 num_periods; // we increment in E timer so needs to be compensated
}hypmtscheduler_hyptask_handle_t;

typedef struct {
	u32 hyptask_id;
	u64 timestamp;
	u32 event_type;
} hypmtscheduler_logentry_t;

#define DEBUG_LOG_SIZE (4096/sizeof(hypmtscheduler_logentry_t))


struct sched_timer *uapp_sched_timer_declare(u32 first_time_period,
		u32 regular_time_period, int priority, HYPTHREADFUNC func);


#ifdef __ENABLE_UAPP_HYPMTSCHEDULER_SECURE_HYPTASK_BOOTSTRAP__
typedef struct {
	uint32_t hyptask_first_period;
	uint32_t hyptask_regular_period;
	uint32_t hyptask_priority;
	uint32_t hyptask_id;
} hypmtscheduler_secure_bootstrap_config_t;

#endif


#endif // __ASSEMBLY__



#endif //__HYPMTSCHEDULER_H__
