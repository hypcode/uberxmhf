AC_INIT([uxmhf-rpi3], [5.0])


###### autoconf general options
AC_SUBST([prefix])

###### simple configuration of tools. (just looks for matching binary on PATH)
AC_CHECK_PROG([MAKE], [make], [make])
AC_CHECK_PROG([CCERT], [arm-linux-gnueabihf-gcc], [arm-linux-gnueabihf-gcc])
AC_CHECK_PROG([CC], [arm-linux-gnueabihf-gcc], [arm-linux-gnueabihf-gcc])
AC_CHECK_PROG([AS], [arm-linux-gnueabihf-as], [arm-linux-gnueabihf-as])
AC_CHECK_PROG([AR], [arm-linux-gnueabihf-ar], [arm-linux-gnueabihf-ar])
AC_CHECK_PROG([RM], [rm -f], [rm -f])
AC_CHECK_PROG([CP], [cp], [cp])
AC_CHECK_PROG([MKDIR], [mkdir], [mkdir])
AC_CHECK_PROG([LD], [arm-linux-gnueabihf-ld], [arm-linux-gnueabihf-ld])
AC_CHECK_PROG([OBJCOPY], [arm-linux-gnueabihf-objcopy], [arm-linux-gnueabihf-objcopy])
AC_CHECK_PROG([CAT], [cat], [cat])



###### uxmhf-rpi3 specific options

# core start address
AC_SUBST([CORE_STARTADDR])
AC_ARG_WITH([core_startaddr],
        AS_HELP_STRING([--with-core-startaddr=@<:@STARTADDR@:>@],
                [select core_startaddr (0x1000000)]),
                , [with_core_startaddr=0x1000000])
CORE_STARTADDR=$[]with_core_startaddr

# core size
AC_SUBST([CORE_SIZE])
AC_ARG_WITH([core_size],
        AS_HELP_STRING([--with-core-size=@<:@SIZE@:>@],
                [select core_size (0xC00000)]),
                , [with_core_size=0xC00000])
CORE_SIZE=$[]with_core_size


# boot partition start sector
AC_SUBST([BOOT_PARTITION_START])
AC_ARG_WITH([boot_partition_start],
        AS_HELP_STRING([--with-boot-partition-start=@<:@SECTORNUM@:>@],
                [select boot_partition_start (8192)]),
                , [with_boot_partition_start=8192])
BOOT_PARTITION_START=$[]with_boot_partition_start

# boot partition end sector
AC_SUBST([BOOT_PARTITION_END])
AC_ARG_WITH([boot_partition_end],
        AS_HELP_STRING([--with-boot-partition-end=@<:@SECTORNUM@:>@],
                [select boot_partition_end (137215)]),
                , [with_boot_partition_end=137215])
BOOT_PARTITION_END=$[]with_boot_partition_end

# selectively enable/disable secure boot
AC_SUBST([SECBOOT])
AC_ARG_ENABLE([secboot],
        AS_HELP_STRING([--enable-secboot@<:@=yes|no@:>@],
                [enable Secure Boot]),
                , [enable_secboot=no])
AS_IF([test "x${enable_secboot}" != "xno"],
      [SECBOOT=y],
      [SECBOOT=n])

# selectively enable/disable DMA protection
AC_SUBST([DMAPROT])
AC_ARG_ENABLE([dmaprot],
        AS_HELP_STRING([--enable-dmaprot@<:@=yes|no@:>@],
                [enable DMA Protection]),
                , [enable_dmaprot=no])
AS_IF([test "x${enable_dmaprot}" != "xno"],
      [DMAPROT=y],
      [DMAPROT=n])

# selectively enable/disable interrupt protection
AC_SUBST([INTPROT])
AC_ARG_ENABLE([intprot],
        AS_HELP_STRING([--enable-intprot@<:@=yes|no@:>@],
                [enable Interrupt Protection]),
                , [enable_intprot=no])
AS_IF([test "x${enable_intprot}" != "xno"],
      [INTPROT=y],
      [INTPROT=n])

# selectively enable/disable FIQ reflection
AC_SUBST([FIQREFLECTION])
AC_ARG_ENABLE([fiqreflection],
        AS_HELP_STRING([--enable-fiqreflection@<:@=yes|no@:>@],
                [enable FIQ interrupt Reflection]),
                , [enable_fiqreflection=no])
AS_IF([test "x${enable_fiqreflection}" != "xno"],
      [FIQREFLECTION=y],
      [FIQREFLECTION=n])

# selectively enable/disable mixed-trusted scheduler functionality in hypervisor
AC_SUBST([ENABLE_UAPP_HYPMTSCHEDULER])
AC_ARG_ENABLE([uapp_hypmtscheduler],
	AS_HELP_STRING([--enable-uapp-hypmtscheduler@<:@=yes|no@:>@],
		[enable mixed-trust scheduler uapp functionality in hypervisor; you must also enable FIQ interrupt reflection configure option]),
		, [enable_uapp_hypmtscheduler=no])
AS_IF([test "x${enable_uapp_hypmtscheduler}" != "xno"],
      [ENABLE_UAPP_HYPMTSCHEDULER=y],
      [ENABLE_UAPP_HYPMTSCHEDULER=n])


# selectively enable/disable secure hyptask bootstrapping for mixed-trusted scheduler functionality in hypervisor
AC_SUBST([ENABLE_UAPP_HYPMTSCHEDULER_SECURE_HYPTASK_BOOTSTRAP])
AC_ARG_ENABLE([uapp_hypmtscheduler_secure_hyptask_bootstrap],
	AS_HELP_STRING([--enable-uapp-hypmtscheduler-secure-hyptask-bootstrap@<:@=yes|no@:>@],
		[enable secure hyptask bootstrapping for mixed-trust scheduler uapp functionality in hypervisor; you must also enable mixed-trust scheduler functionality]),
		, [enable_uapp_hypmtscheduler_secure_hyptask_bootstrap=no])
AS_IF([test "x${enable_uapp_hypmtscheduler_secure_hyptask_bootstrap}" != "xno"],
      [ENABLE_UAPP_HYPMTSCHEDULER_SECURE_HYPTASK_BOOTSTRAP=y],
      [ENABLE_UAPP_HYPMTSCHEDULER_SECURE_HYPTASK_BOOTSTRAP=n])


# selectively enable/disable MAVLINK serial heart-beat protocol functionality in hypervisor
AC_SUBST([ENABLE_UAPP_MAVLINKSERHB])
AC_ARG_ENABLE([uapp_mavlinkserhb],
	AS_HELP_STRING([--enable-uapp-mavlinkserhb@<:@=yes|no@:>@],
		[enable MAVLINK serial heart-beat protocol uapp functionality in hypervisor; you must also enable hypervisor scheduler uapp configure option]),
		, [enable_uapp_mavlinkserhb=no])
AS_IF([test "x${enable_uapp_mavlinkserhb}" != "xno"],
      [ENABLE_UAPP_MAVLINKSERHB=y],
      [ENABLE_UAPP_MAVLINKSERHB=n])



# selectively enable/disable HMAC functionality in hypervisor
AC_SUBST([ENABLE_UAPP_UHSIGN])
AC_ARG_ENABLE([uapp_uhsign],
	AS_HELP_STRING([--enable-uapp-uhsign@<:@=yes|no@:>@],
		[enable signing uapp]),
		, [enable_uapp_uhsign=no])
AS_IF([test "x${enable_uapp_uhsign}" != "xno"],
      [ENABLE_UAPP_UHSIGN=y],
      [ENABLE_UAPP_UHSIGN=n])

# selectively enable/disable AES encryption functionality in hypervisor
AC_SUBST([ENABLE_UAPP_UAGENT])
AC_ARG_ENABLE([uapp_uagent],
	AS_HELP_STRING([--enable-uapp-uagent@<:@=yes|no@:>@],
		[enable signing uapp]),
		, [enable_uapp_uagent=no])
AS_IF([test "x${enable_uapp_uagent}" != "xno"],
      [ENABLE_UAPP_UAGENT=y],
      [ENABLE_UAPP_UAGENT=n])


# selectively enable/disable simple state DB functionality in hypervisor
AC_SUBST([ENABLE_UAPP_STATEDB])
AC_ARG_ENABLE([uapp_stateDB],
	AS_HELP_STRING([--enable-uapp-uhstateDB@<:@=yes|no@:>@],
		[enable stateDB uapp]),
		, [enable_uapp_stateDB=no])
AS_IF([test "x${enable_uapp_stateDB}" != "xno"],
      [ENABLE_UAPP_STATEDB=y],
      [ENABLE_UAPP_STATEDB=n])

# selectively enable/disable UART para-virtualized driver backend in hypervisor
AC_SUBST([ENABLE_UAPP_PVDRIVER_UART])
AC_ARG_ENABLE([uapp_pvdriver_uart],
	AS_HELP_STRING([--enable-uapp-pvdriver-uart@<:@=yes|no@:>@],
		[enable UART para-virtualized driver backend uapp]),
		, [enable_uapp_pvdriver_uart=no])
AS_IF([test "x${enable_uapp_pvdriver_uart}" != "xno"],
      [ENABLE_UAPP_PVDRIVER_UART=y],
      [ENABLE_UAPP_PVDRIVER_UART=n])


# selectively enable/disable debugging via UART
AC_SUBST([ENABLE_DEBUG_UART])
AC_ARG_ENABLE([debug_uart],
	AS_HELP_STRING([--enable-debug-uart@<:@=yes|no@:>@],
		[enable debug via UART, default is via mini UART]),
		, [enable_debug_uart=no])
AS_IF([test "x${enable_debug_uart}" != "xno"],
      [ENABLE_DEBUG_UART=y],
      [ENABLE_DEBUG_UART=n])


# selectively enable/disable Mini UART support
AC_SUBST([ENABLE_UART_MINI])

AC_ARG_ENABLE([uart_mini],
	AS_HELP_STRING([--enable-uart-mini@<:@=yes|no@:>@],
		[enable Mini UART]),
		, [enable_uart_mini=no])
AS_IF([test "x${enable_uart_mini}" != "xno"],
      [ENABLE_UART_MINI=y],
      [ENABLE_UART_MINI=n])



# selectively enable/disable PL011 UART support
AC_SUBST([ENABLE_UART_PL011])
AC_SUBST([RPI3CONFIG_ENABLE_UART_PL011])

AC_ARG_ENABLE([uart_pl011],
	AS_HELP_STRING([--enable-uart-pl011@<:@=yes|no@:>@],
		[enable PL011 UART]),
		, [enable_uart_pl011=no])
AS_IF([test "x${enable_uart_pl011}" != "xno"],
      [ENABLE_UART_PL011=y],
      [ENABLE_UART_PL011=n])
AS_IF([test "x${enable_uart_pl011}" != "xno"],
      [RPI3CONFIG_ENABLE_UART_PL011=dtoverlay=pi3-miniuart-bt],
      [RPI3CONFIG_ENABLE_UART_PL011=])


# selectively enable/disable PL011 UART hardware flow control
AC_SUBST([ENABLE_UART_PL011_CTSRTS])

AC_ARG_ENABLE([uart_pl011_ctsrts],
	AS_HELP_STRING([--enable-uart-pl011-ctsrts@<:@=yes|no@:>@],
		[enable PL011 UART hardware flow control (CTS/RTS)]),
		, [enable_uart_pl011_ctsrts=no])
AS_IF([test "x${enable_uart_pl011_ctsrts}" != "xno"],
      [ENABLE_UART_PL011_CTSRTS=y],
      [ENABLE_UART_PL011_CTSRTS=n])


###### autoconf output files
AC_CONFIG_FILES(
			[./include/config.h Makefile rpi3-config.txt]
	)

AC_OUTPUT
