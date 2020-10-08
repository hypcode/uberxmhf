#
# uxmhf-rpi3 core Makefile
# 
# author: amit vasudevan (amitvasudevan@acm.org)
#
#

OPT_FILES := 

.PHONY: all
all: core.bin


entry.o : entry.s
	$(TOOLPREFIX)gcc $(CFLAGS) entry.s -o entry.o


miniuart.o : ../common/miniuart.c
	$(TOOLPREFIX)gcc $(CFLAGS) ../common/miniuart.c -o miniuart.o

pl011uart.o : ../common/pl011uart.c
	$(TOOLPREFIX)gcc $(CFLAGS) ../common/pl011uart.c -o pl011uart.o

mailbox.o : ../common/mailbox.c
	$(TOOLPREFIX)gcc $(CFLAGS) ../common/mailbox.c -o mailbox.o


strlen.o : ../common/strlen.c
	$(TOOLPREFIX)gcc $(CFLAGS) ../common/strlen.c -o strlen.o

subr_prf.o : ../common/subr_prf.c
	$(TOOLPREFIX)gcc $(CFLAGS) ../common/subr_prf.c -o subr_prf.o


debug.o : ../common/debug.c
	$(TOOLPREFIX)gcc $(CFLAGS) ../common/debug.c -o debug.o

atags.o : ../common/atags.c
	$(TOOLPREFIX)gcc $(CFLAGS) ../common/atags.c -o atags.o

arm8-32.o : ../common/arm8-32.s
	$(TOOLPREFIX)gcc $(CFLAGS) -D__ASSEMBLY__ ../common/arm8-32.s -o arm8-32.o

ifeq ($(filter $(ENABLE_UAPP_UAGENT) $(ENABLE_UAPP_UHSIGN) $(ENABLE_UAPP_STATEDB),y),y)
whitelist.o : ../common/whitelist.c
	$(TOOLPREFIX)gcc $(CFLAGS) ../common/whitelist.c -o whitelist.o
OPT_FILES += whitelist.o
endif

hypvtablestubs.o : hypvtablestubs.s
	$(TOOLPREFIX)gcc $(CFLAGS) hypvtablestubs.s -o hypvtablestubs.o

hypvtable.o : hypvtable.c
	$(TOOLPREFIX)gcc $(CFLAGS) hypvtable.c -o hypvtable.o

mmu.o : mmu.c
	$(TOOLPREFIX)gcc $(CFLAGS) mmu.c -o mmu.o

hyppgtbl.o : hyppgtbl.c
	$(TOOLPREFIX)gcc $(CFLAGS) hyppgtbl.c -o hyppgtbl.o


s2pgtbl.o : s2pgtbl.c
	$(TOOLPREFIX)gcc $(CFLAGS) s2pgtbl.c -o s2pgtbl.o

uapi_s2pgtbl.o : uapi_s2pgtbl.c
	$(TOOLPREFIX)gcc $(CFLAGS) uapi_s2pgtbl.c -o uapi_s2pgtbl.o

dmaprot.o : dmaprot.c
	$(TOOLPREFIX)gcc $(CFLAGS) dmaprot.c -o dmaprot.o

intprot.o : intprot.c
	$(TOOLPREFIX)gcc $(CFLAGS) intprot.c -o intprot.o

secboot.o : secboot.c
	$(TOOLPREFIX)gcc $(CFLAGS) secboot.c -o secboot.o


bcm2837.o : bcm2837.c
	$(TOOLPREFIX)gcc $(CFLAGS) bcm2837.c -o bcm2837.o


ctxtrace.o : ../uapps/uapp-ctxtrace/ctxtrace.c
	$(TOOLPREFIX)gcc $(CFLAGS) ../uapps/uapp-ctxtrace/ctxtrace.c -o ctxtrace.o

hyptimer.o : hyptimer.c
	$(TOOLPREFIX)gcc $(CFLAGS) hyptimer.c -o hyptimer.o

uapp-uhcalltest.o : ../uapps/uapp-uhcalltest/uapp-uhcalltest.c
	$(TOOLPREFIX)gcc $(CFLAGS) ../uapps/uapp-uhcalltest/uapp-uhcalltest.c -o uapp-uhcalltest.o

ifeq ($(ENABLE_UAPP_UHSIGN), y)
uapp-uhsign.o : ../uapps/uapp-uhsign/uapp-uhsign.c
	$(TOOLPREFIX)gcc $(CFLAGS) ../uapps/uapp-uhsign/uapp-uhsign.c -o uapp-uhsign.o 
OPT_FILES += uapp-uhsign.o
endif

ifeq ($(ENABLE_UAPP_UAGENT), y)
uapp-uagent.o : ../uapps/uapp-uagent/uapp-uagent.c
	$(TOOLPREFIX)gcc $(CFLAGS) ../uapps/uapp-uagent/uapp-uagent.c -o uapp-uagent.o 
OPT_FILES += uapp-uagent.o
endif

ifeq ($(ENABLE_UAPP_STATEDB), y)
uapp-uhstateDB.o : ../uapps/uapp-uhstateDB/uapp-uhstateDB.c
	$(TOOLPREFIX)gcc $(CFLAGS) ../uapps/uapp-uhstateDB/uapp-uhstateDB.c -o uapp-uhstateDB.o 
OPT_FILES += uapp-uhstateDB.o
endif

ifeq ($(ENABLE_UAPP_PVDRIVER_UART), y)
uapp-pvdriver-uart.o : ../uapps/uapp-pvdriver-uart/uapp-pvdriver-uart.c
	$(TOOLPREFIX)gcc $(CFLAGS) ../uapps/uapp-pvdriver-uart/uapp-pvdriver-uart.c -o uapp-pvdriver-uart.o 
OPT_FILES += uapp-pvdriver-uart.o
endif

uapp-utpmtest.o : ../uapps/uapp-utpmtest/uapp-utpmtest.c
	$(TOOLPREFIX)gcc $(CFLAGS) ../uapps/uapp-utpmtest/uapp-utpmtest.c -o uapp-utpmtest.o

uapp-pa5encfs.o : ../uapps/uapp-pa5encfs/uapp-pa5encfs.c
	$(TOOLPREFIX)gcc $(CFLAGS) ../uapps/uapp-pa5encfs/uapp-pa5encfs.c -o uapp-pa5encfs.o


uapp-watchdogsup.o : ../uapps/uapp-watchdog/uapp-watchdogsup.s
	$(TOOLPREFIX)gcc $(CFLAGS) ../uapps/uapp-watchdog/uapp-watchdogsup.s -o uapp-watchdogsup.o

uapp-watchdog.o : ../uapps/uapp-watchdog/uapp-watchdog.c
	$(TOOLPREFIX)gcc $(CFLAGS) ../uapps/uapp-watchdog/uapp-watchdog.c -o uapp-watchdog.o


ifeq ($(ENABLE_UAPP_HYPMTSCHEDULER), y)
uapp-hypmtschedulersup.o : ../uapps/uapp-hypmtscheduler/uapp-hypmtschedulersup.s
	$(TOOLPREFIX)gcc $(CFLAGS) ../uapps/uapp-hypmtscheduler/uapp-hypmtschedulersup.s -o uapp-hypmtschedulersup.o

uapp-hypmtscheduler.o : ../uapps/uapp-hypmtscheduler/uapp-hypmtscheduler.c
	$(TOOLPREFIX)gcc $(CFLAGS) ../uapps/uapp-hypmtscheduler/uapp-hypmtscheduler.c -o uapp-hypmtscheduler.o

pqueue.o : ../uapps/uapp-hypmtscheduler/pqueue.c
	$(TOOLPREFIX)gcc $(CFLAGS) ../uapps/uapp-hypmtscheduler/pqueue.c -o pqueue.o

logging.o : ../uapps/uapp-hypmtscheduler/logging.c
	$(TOOLPREFIX)gcc $(CFLAGS) ../uapps/uapp-hypmtscheduler/logging.c -o logging.o

OPT_FILES += pqueue.o logging.o uapp-hypmtschedulersup.o uapp-hypmtscheduler.o
endif


ifeq ($(ENABLE_UAPP_HYPMTSCHEDULER), y)

uapp-mavlinkserhb.o : ../uapps/uapp-mavlinkserhb/uapp-mavlinkserhb.c
	$(TOOLPREFIX)gcc $(CFLAGS) ../uapps/uapp-mavlinkserhb/uapp-mavlinkserhb.c -o uapp-mavlinkserhb.o

OPT_FILES += uapp-mavlinkserhb.o

endif



ghcall.o : ghcall.c
	$(TOOLPREFIX)gcc $(CFLAGS) ghcall.c -o ghcall.o


main.o : main.c
	$(TOOLPREFIX)gcc $(CFLAGS) main.c -o main.o

core.bin: entry.o miniuart.o pl011uart.o mailbox.o strlen.o subr_prf.o debug.o atags.o arm8-32.o hypvtablestubs.o hypvtable.o mmu.o hyppgtbl.o s2pgtbl.o uapi_s2pgtbl.o dmaprot.o intprot.o secboot.o bcm2837.o hyptimer.o ctxtrace.o uapp-uhcalltest.o uapp-utpmtest.o uapp-pa5encfs.o uapp-watchdogsup.o uapp-watchdog.o $(OPT_FILES) ghcall.o main.o
	cp -f core.lscript core.lscript.c
	echo $(OPT_FILES)
	$(TOOLPREFIX)gcc -E -P $(CFLAGS_DEFS) core.lscript.c -o core.ls 
	$(TOOLPREFIX)ld entry.o miniuart.o pl011uart.o mailbox.o strlen.o subr_prf.o debug.o atags.o arm8-32.o hypvtablestubs.o hypvtable.o mmu.o hyppgtbl.o s2pgtbl.o uapi_s2pgtbl.o dmaprot.o intprot.o secboot.o bcm2837.o hyptimer.o ctxtrace.o uapp-uhcalltest.o uapp-utpmtest.o uapp-pa5encfs.o uapp-watchdogsup.o uapp-watchdog.o $(OPT_FILES) ghcall.o main.o -T core.ls -o core.elf $(LIBS) $(CCLIB)
	$(TOOLPREFIX)objcopy core.elf -O binary core.bin

.PHONY: clean
clean:
	rm -f *.o
	rm -f *.elf
	rm -f *.bin
	rm -f core.lscript.c
