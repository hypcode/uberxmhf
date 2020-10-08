#top-level Makefile for usr mode application to test hypmtscheduler
# author: amit vasudevan (amitvasudevan@acm.org)

export CURRENT_DIR := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
export UXMHFRPI3_DIR := $(CURRENT_DIR)/../../..
export RGAPPS_LIB_DIR := $(realpath $(CURRENT_DIR)/../libs)

INCS = -I$(UXMHFRPI3_DIR)/include
INCS += -I$(RGAPPS_LIB_DIR)/libuhcall/include

LIBS = $(RGAPPS_LIB_DIR)/libuhcall/libuhcall.a


CFLAGS = -I. $(INCS) -c -g -Wall
LFLAGS = -g -Wall -Wextra     


.PHONY: all
all: 
	arm-linux-gnueabihf-gcc $(CFLAGS) ugapp-hypmtschedulertest.c -o ugapp-hypmtschedulertest.o
	arm-linux-gnueabihf-gcc $(LFLAGS) ugapp-hypmtschedulertest.o -o ugapp-hypmtschedulertest $(LIBS)

.PHONY: clean
clean:
	rm -f ugapp-hypmtschedulertest
	rm -f *.o
	