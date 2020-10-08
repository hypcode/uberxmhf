/*
 * mavlinkserhb user-mode application
 * author: amit vasudevan (amitvasudevan@acm.org)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include "../../../include/mavlinkserhb.h"

//////
// function to communicate with kernel-module
//////
void kmod_comms(unsigned int function){
	int ret, fd;

	//open device
	fd = open("/dev/mavlinkserhbkmod", O_RDWR);
	if (fd < 0){
	    printf("%s: error: line %u\n", __FUNCTION__, __LINE__);
		exit(1); //Failed to open
	}

	ret = write(fd, NULL, function);
	if (ret < 0){
	    printf("%s: error: line %u\n", __FUNCTION__, __LINE__);
		exit(1);	//error in write
	}

	if ( close(fd) < 0 ){
	    printf("%s: error: line %u\n", __FUNCTION__, __LINE__);
		exit(1);	//error in closing uhcallkmod device
	}
}



//////
// main function
//////
int main(int argc, char *argv[]){
	printf("%s: start\n", __FUNCTION__);

    if(argc != 2){
  	   printf("usage: mavlinkserhb_userapp <api_id>\n");
  	   exit(1);
    }

	switch(atoi(argv[1])){
	case 1:
		kmod_comms(UAPP_MAVLINKSERHB_UHCALL_INITIALIZE);
		break;

	case 2:
		kmod_comms(UAPP_MAVLINKSERHB_UHCALL_SEND);
		break;

	case 3:
		kmod_comms(UAPP_MAVLINKSERHB_UHCALL_CHECKRECV);
		break;

	case 4:
		kmod_comms(UAPP_MAVLINKSERHB_UHCALL_RECV);
		break;

	case 5:
		kmod_comms(UAPP_MAVLINKSERHB_UHCALL_ACTIVATEHBHYPTASK);
		break;

	case 6:
		kmod_comms(UAPP_MAVLINKSERHB_UHCALL_DEACTIVATEHBHYPTASK);
		break;

	default:
		printf("%s: unknown testcase_num=%u, exiting!\n", __FUNCTION__, atoi(argv[1]));
		exit(1);
	}

    printf("%s: end-of-test\n", __FUNCTION__);
    return 0;
}

