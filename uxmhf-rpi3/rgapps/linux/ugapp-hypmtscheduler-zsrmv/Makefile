CURRENT_DIR := $(realpath $(src))
UXMHFRPI3_DIR := $(CURRENT_DIR)/../../..

UINCS = -I$(CURRENT_DIR)/.
UINCS += -I$(UXMHFRPI3_DIR)/include
UINCS += -I$(UXMHFRPI3_DIR)/rgapps/linux/libs/libuhcall/include

EXTRA_CFLAGS := $(UINCS)

obj-m := hypmtschedulertestkmod.o

hypmtschedulertestkmod-objs := hypmtscheduler_testkmod.o hypmtscheduler_testkmodsup.o hypmtscheduler_kmodlib.o

INCS = -I$(UXMHFRPI3_DIR)/include

USER_CFLAGS = -I. $(INCS) -c -g -Wall -mcpu=cortex-a53
USER_LFLAGS = -g -Wall -Wextra     

.PHONY: builduserapp
builduserapp: 
	arm-linux-gnueabihf-gcc $(USER_CFLAGS) hypmtschedulertest_userapp.c -o hypmtschedulertest_userapp.o
	arm-linux-gnueabihf-gcc $(USER_CFLAGS) hypmtschedulertest_userappsup.s -o hypmtschedulertest_userappsup.o
	arm-linux-gnueabihf-gcc $(USER_LFLAGS) hypmtschedulertest_userapp.o hypmtschedulertest_userappsup.o -o hypmtschedulertest_userapp 

.PHONY: cleanuserapp
cleanuserapp:
	rm -f hypmtschedulertest_userapp
	rm -f hypmtschedulertest_userapp.o
	