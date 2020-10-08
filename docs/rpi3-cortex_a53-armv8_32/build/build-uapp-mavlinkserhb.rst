.. include:: /macros.rst


mavlinkserhb (MAVLINK serial heart-beat) überApp micro-hypervisor extension
===========================================================================

This |uberappuhvext| implements the MAVLINK serial heart-beat protocol and can be enabled by using the
configure option ``--enable-uapp-mavlinkserhb`` when  
building the micro-hypervisor core framework (See :doc:`Build Micro-Hypervisor Core Framework </rpi3-cortex_a53-armv8_32/build/build-core>`).

.. note:: You must also enable the mixed-trust scheduler |uberappuhvext| via configure option 
          ``--enable-uapp-hypmtscheduler`` when using this extension.


General notes and initial system configuration
----------------------------------------------

1. mavlinkserhb is controlled via the following components:
2. a user-space application (``mavlinkserhb_userapp``) located at
   ``rgapps/linux/ugapp-mavlinkserhb/mavlinkserhb_userapp.c``
3. a kernel-module (``mavlinkserhbkmod``) located at
   ``rgapps/linux/ugapp-mavlinkserhb/mavlinkserhb_kmod.c``
4. a hyptask uberapp (``uapp-mavlinkserhb``) located at
   ``uapps/uapp-mavlinkserhb/uapp-mavlinkserhb.c``

5. ``mavlinkserhb_userapp`` interacts with ``mavlinkserhbkmod`` via a
   system-call, which in turn interacts with ``uapp-mavlinkserhb`` via a
   hypercall.

6. boot the PI3 without uberXMHF and issue the following two commands to
   deactivate the serial port driver for the Raspbian (guest) OS
7. ``sudo systemctl stop serial-getty@serial0``
8. ``sudo systemctl disable serial-getty@serial0``

9. Note that we need to interact with the PI3 either via a dedicated
   screen and keyboard or via SSH (network). The serial interface will
   not be available for testing the mavlinkserhb uberapp. Further, the
   serial debug messages from uberXMHF will only be emitted until
   control is transferred to the raspbian linux kernel. Once the kernel
   gets control, you will not see any further serial debug messages from
   uberXMHF.


Developer notes for mavlinkserhb uapp
-------------------------------------

1. the periodic heat-beat is designed to be handled by the function
   ``uapp_mavlinkserhb_handleheartbeat`` within ``uapp-mavlinkserhb``.
   This periodic function is created within the function
   ``uapp_mavlinkserhb_handlehcall_initialize`` within
   ``uapp-mavlinkserhb`` with a period of 500ms (2Hz).

2. the following are the serial hw functions available to implement the
   serial protocol within ``uapp_mavlinkserhb_handleheartbeat`` :

   1. ``void uapp_mavlinkserhb_uart_init(u32 baudrate)`` : this function
      initializes the UART hardware with the specified ``baudrate`` with
      the typical ``8N1`` (8 data bits, no parity bit, 1 stop bit)
      configuration.
   2. ``void uapp_mavlinkserhb_uart_flush(void)`` : this function
      flushes the UART output (write) buffers
   3. ``void uapp_mavlinkserhb_uart_send(u8 *buffer, u32 buf_len)`` :
      this function sends (writes) a ``buffer`` of ``buf_len`` bytes to
      the UART. Note: You need to invoke
      ``uapp_mavlinkserhb_uart_flush`` explicitly after the send if you
      wish to flush the write buffer and ensure all bytes are written
      out prior to performing any further operations.
   4. ``int uapp_mavlinkserhb_uart_checkrecv(void)`` : this function
      returns 1 if there are bytes waiting to be read from the UART
      receive buffer. A return value of 0 signifies an empty receive
      buffer.
   5. ``int uapp_mavlinkserhb_uart_recv(u8 *buffer, u32 max_len, u32 *len_read)``
      : this function reads the UART receive buffer into the ``buffer``
      specified upto ``max_len`` bytes and returns the actual number of
      bytes read into ``len_read``. The function returns 1 if the UART
      receive buffer is empty as a result of this read. A return value
      of 0 indicates the UART receive buffer still has pending bytes to
      be read after this function returns (e.g., if ``max_len`` was less
      than number of bytes in the UART receive buffer).


Developer notes for mavlinkserhb kernel module and user-app
-----------------------------------------------------------

1. the following are functions available to the kernel-module,
   ``mavlinkserhbkmod``, to interact with ``uapp-mavlinkserhb`` uberapp
   via hypercalls:

   1. ``void mavlinkserhb_initialize(u32 baudrate)`` : this function
      initializes the UART hardware with the specified ``baudrate`` with
      the typical ``8N1`` (8 data bits, no parity bit, 1 stop bit)
      configuration.
   2. ``bool mavlinkserhb_send(u8 *buffer, u32 buf_len)`` : this
      function sends (writes) a ``buffer`` of ``buf_len`` bytes to the
      UART. returns ``true`` on sucess and ``false`` on failure.
   3. ``bool mavlinkserhb_checkrecv(void)`` : this function returns
      ``true`` if there are bytes waiting to be read from the UART
      receive buffer. A return value of ``false`` signifies an empty
      receive buffer.
   4. ``bool mavlinkserhb_recv(u8 *buffer, u32 max_len, u32 *len_read, bool *uartreadbufexhausted)``
      : this function reads the UART receive buffer into the ``buffer``
      specified upto ``max_len`` bytes and returns the actual number of
      bytes read into ``len_read``. The function sets
      ``uartreadbufexhausted`` to ``true`` if the UART receive buffer is
      empty as a result of this read. A return value of ``false`` in
      ``uartreadbufexhausted`` indicates the UART receive buffer still
      has pending bytes to be read after this function returns (e.g., if
      ``max_len`` was less than number of bytes in the UART receive
      buffer).
   5. ``bool mavlinkserhb_activatehbhyptask(u32 first_period, u32 recurring_period, u32 priority)``
      : this function activates the heart-beat hyptask (handled by
      ``uapp_mavlinkserhb_handleheartbeat`` within
      ``uapp-mavlinkserhb``) with the specified time-periods and
      priority. ``first_period`` and ``recurring_period`` are specified
      as clock-cycles. For convenience, definitions
      ``HYPMTSCHEDULER_TIME_1SEC``, ``HYPMTSCHEDULER_TIME_1MSEC`` and
      ``HYPMTSCHEDULER_TIME_1USEC`` are provided for approx.
      clock-cycles corresponding to 1 second, milli-second and
      micro-second respectively. ``priority`` is a ``u32`` with higher
      number indicating higher priority. For our purpose
      ``first_period`` and ``recurring_period`` should be equal and
      ``priority`` set to 99. The function returns ``true`` on success
      and ``false`` on failure.
   6. ``bool mavlinkserhb_deactivatehbhyptask(void)`` : this function
      de-activates the heart-beat hyptask. returns ``true`` on success
      and ``false`` on failure.

2. the test user-space application (``mavlinkserhb_userapp``) employs
   ioctl(write) to communicate with the kernel-module
   (``mavlinkserhbkmod``)

3. ``mavlinkserhb_userapp`` is invoked with parameters ``1`` through
   ``6`` to test the aforementioned kernel-module interfaces. The
   test-rig can be found within the function ``dev_write`` within the
   ``mavlinkserhbkmod`` sources.


Instructions to build and deploy mavlinkserhb components
--------------------------------------------------------

1. Follow all instructions described in :doc:`Build Micro-Hypervisor Core Framework </rpi3-cortex_a53-armv8_32/build/build-core>`

2. At this point build ``mavlinkserhbkmod.ko`` on development system

   1. ``cd rgapps/linux/ugapp-mavlinkserhb``
   2. ``make -C ~/linux ARCH=arm CROSS_COMPILE=~/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf- M=$PWD``
   3. ``cp ./mavlinkserhbkmod.ko ~/uxmhf-rpi3-staging/.``

3. Build ``mavlinkserhb_userapp`` on development system

   1. ``cd rgapps/linux``
   2. ``make``
   3. ``cd rgapps/linux/ugapp-mavlinkserhb``
   4. ``make builduserapp``
   5. ``cp ./mavlinkserhb_userapp ~/uxmhf-rpi3-staging/.``

4. Continue with the installation of the framework as described within
   :doc:`Installing Micro-Hypervisor Framework </rpi3-cortex_a53-armv8_32/installing>` and boot
   the micro-hypervisor with the uberguest OS


Instructions to run mavlinkserhb
--------------------------------

1. Install ``mavlinkserhbkmod.ko`` within uberguest once booted

   1. ``sudo insmod mavlinkserhbkmod.ko``

2. Run ``mavlinkserhb_userapp n`` (mavlinkserhb user-mode test
   application) within uberguest to test various APIs. ``n`` is a number
   from ``1`` through ``6``. see Developer notes for mavlinkserhb kernel
   module and user-app above. For example, the following will initialize
   the UART

   1. ``sudo ./mavlinkserhb_userapp 1``

3. Remove ``mavlinkserhbkmod.ko`` within uberguest once done

   1. ``sudo rmmod uhcallkmod``

4. Shutdown the uberguest

   1. ``sudo shutdown -hP now``
