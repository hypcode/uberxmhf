CURRENT_DIR := $(realpath $(src))
UXMHFRPI3_DIR := $(CURRENT_DIR)/../../..

UINCS = -I$(CURRENT_DIR)/.
UINCS += -I$(UXMHFRPI3_DIR)/include
UINCS += -I$(UXMHFRPI3_DIR)/rgapps/linux/libs/libuhcall/include

EXTRA_CFLAGS := $(UINCS)

obj-m := mavlinkserhbkmod.o

mavlinkserhbkmod-objs := mavlinkserhb_kmod.o mavlinkserhb_kmodsup.o mavlinkserhb_kmodlib.o

INCS = -I$(UXMHFRPI3_DIR)/include

USER_CFLAGS = -I. $(INCS) -c -g -Wall -mcpu=cortex-a53
USER_LFLAGS = -g -Wall -Wextra     

.PHONY: builduserapp
builduserapp: 
	arm-linux-gnueabihf-gcc $(USER_CFLAGS) mavlinkserhb_userapp.c -o mavlinkserhb_userapp.o
	arm-linux-gnueabihf-gcc $(USER_CFLAGS) mavlinkserhb_userappsup.S -o mavlinkserhb_userappsup.o
	arm-linux-gnueabihf-gcc $(USER_LFLAGS) mavlinkserhb_userapp.o mavlinkserhb_userappsup.o -o mavlinkserhb_userapp 

.PHONY: cleanuserapp
cleanuserapp:
	rm -f mavlinkserhb_userapp
	rm -f mavlinkserhb_userapp.o
	rm -f mavlinkserhb_userappsup.o
	