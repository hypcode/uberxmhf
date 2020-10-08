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

//#include <hypmtscheduler.h>
extern unsigned int sysreg_read_cntfrq();
extern unsigned long long int sysreg_read_cntvct();

void kmod_comms(unsigned int function){
	int ret, fd;

	//open uhcallkmod device
	fd = open("/dev/hypmtschedulerkmod", O_RDWR);
	if (fd < 0){
	    printf("%s: error: line %u\n", __FUNCTION__, __LINE__);
		exit(1); //Failed to open /dev/uhcallkmod
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


void test_createhyptask(void){
	kmod_comms(1);
}

void test_disablehyptask(void){
	kmod_comms(2);
}

void test_deletehyptask(void){
	kmod_comms(3);
}

void test_dumpdebuglog(void){
	kmod_comms(4);
}

void test_cyclecounter(void){
	unsigned long long te, ts;
	unsigned long freq;

	//asm volatile ("isb; mrrc %0, cntvct_el0" : "=r" (ts));
	ts = sysreg_read_cntvct();
	sleep (2);
	te = sysreg_read_cntvct();
	printf("Cycles Elapsed: %llu\n", (te-ts));
	printf("Frequency = %u\n",sysreg_read_cntfrq());
}

int main(int argc, char *argv[]){
    unsigned int testcase_num;
	printf("%s: start\n", __FUNCTION__);

    if(argc != 2){
  	   printf("usage: ugapp-hypmtschedulertest <testcasenum>\n");
  	   exit(1);
    }

    testcase_num = atoi(argv[1]);

	printf("%s: testcase_num=%u\n", __FUNCTION__, testcase_num);

	switch(testcase_num){
	case 1:
		test_createhyptask();
		break;

	case 2:
		test_disablehyptask();
		break;

	case 3:
		test_deletehyptask();
		break;

	case 4:
		//test_cyclecounter();
		test_dumpdebuglog();
		break;


	default:
		printf("%s: unknown testcase_num=%u, exiting!\n", __FUNCTION__, testcase_num);
		exit(1);
	}


    printf("%s: end-of-test\n", __FUNCTION__);
    return 0;
}


