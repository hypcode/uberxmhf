/*
 * hypmtscheduler test program
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

#include <uhcall.h>
#include <hypmtscheduler.h>

__attribute__((aligned(4096))) __attribute__((section(".data"))) ugapp_hypmtscheduler_param_t hmtsp;


void test_createhyptask(void){
    memset(&hmtsp, 0, sizeof(hmtsp));

    hmtsp.uhcall_fn = UAPP_HYPMTSCHEDULER_UHCALL_CREATEHYPTASK;
    hmtsp.iparam_1 = 4 * 20 * 1024 * 1024;	//first period
    hmtsp.iparam_2 = 8 * 20 * 1024 * 1024;	//regular period thereafter
    hmtsp.iparam_3 = 3;						//priority
    hmtsp.iparam_4 = 3;						//hyptask id

    if(!uhcall(UAPP_HYPMTSCHEDULER_UHCALL, &hmtsp, sizeof(ugapp_hypmtscheduler_param_t))){
 	   printf("hypercall FAILED. Exiting\n");
 	   exit(1);
    }
    else
 	   printf("hypercall SUCCESS\n");

    if(!hmtsp.status){
  	   printf("createhyptask failed. Exiting\n");
  	   exit(1);
    }

    printf("%s: createhyptask success oparam_1=%u\n", __FUNCTION__, hmtsp.oparam_1);
}


void test_disablehyptask(void){
    memset(&hmtsp, 0, sizeof(hmtsp));

    hmtsp.uhcall_fn = UAPP_HYPMTSCHEDULER_UHCALL_DISABLEHYPTASK;
    hmtsp.iparam_1 = 0;				//hyptask id

    if(!uhcall(UAPP_HYPMTSCHEDULER_UHCALL, &hmtsp, sizeof(ugapp_hypmtscheduler_param_t))){
 	   printf("hypercall FAILED. Exiting\n");
 	   exit(1);
    }
    else
 	   printf("hypercall SUCCESS\n");

    if(!hmtsp.status){
  	   printf("disablehyptask failed. Exiting\n");
  	   exit(1);
    }

    printf("%s: disablehyptask success\n", __FUNCTION__);
}


void test_deletehyptask(void){
    memset(&hmtsp, 0, sizeof(hmtsp));

    hmtsp.uhcall_fn = UAPP_HYPMTSCHEDULER_UHCALL_DELETEHYPTASK;
    hmtsp.iparam_1 = 0;				//hyptask id

    if(!uhcall(UAPP_HYPMTSCHEDULER_UHCALL, &hmtsp, sizeof(ugapp_hypmtscheduler_param_t))){
 	   printf("hypercall FAILED. Exiting\n");
 	   exit(1);
    }
    else
 	   printf("hypercall SUCCESS\n");

    if(!hmtsp.status){
  	   printf("deletehyptask failed. Exiting\n");
  	   exit(1);
    }

    printf("%s: deletehyptask success\n", __FUNCTION__);
}


int main(int argc, char *argv[]){
    unsigned int testcase_num;
	printf("%s: start\n", __FUNCTION__);

    memset(&hmtsp, 0, sizeof(hmtsp));

    if(argc != 2){
  	   printf("usage: ugapp-hypmtschedulertest <testcasenum>\n");
  	   exit(1);
    }

    testcase_num = atoi(argv[1]);

	printf("%s: testcase_num=%u\n", __FUNCTION__, testcase_num);

	switch(testcase_num){
	case 0:
		test_createhyptask();
		break;

	case 1:
		test_disablehyptask();
		break;

	case 2:
		test_deletehyptask();
		break;

	default:
		printf("%s: unknown testcase_num=%u, exiting!\n", __FUNCTION__, testcase_num);
		exit(1);
	}


    printf("%s: end-of-test\n", __FUNCTION__);
    return 0;
}


