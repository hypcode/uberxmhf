/*
	MAVLINK serial heart-beat uberapp

	author: amit vasudevan (amitvasudevan@acm.org)
*/

#ifndef __MAVLINKSERHB_H__
#define __MAVLINKSERHB_H__

#define UAPP_MAVLINKSERHB_UHCALL	0xD0

#define UAPP_MAVLINKSERHB_UHCALL_INITIALIZE				1
#define UAPP_MAVLINKSERHB_UHCALL_SEND					2
#define UAPP_MAVLINKSERHB_UHCALL_CHECKRECV				3
#define UAPP_MAVLINKSERHB_UHCALL_RECV					4
#define UAPP_MAVLINKSERHB_UHCALL_ACTIVATEHBHYPTASK		5
#define UAPP_MAVLINKSERHB_UHCALL_DEACTIVATEHBHYPTASK	6


#ifndef __ASSEMBLY__

typedef struct {
	uint8_t uhcall_fn;
	uint32_t iparam_1;
	uint32_t iparam_2;
	uint32_t iparam_3;
	uint32_t iparam_4;
	uint32_t oparam_1;
	uint32_t oparam_2;
	uint32_t status;
}uapp_mavlinkserhb_param_t;



#endif // __ASSEMBLY__



#endif //__MAVLINKSERHB_H__
