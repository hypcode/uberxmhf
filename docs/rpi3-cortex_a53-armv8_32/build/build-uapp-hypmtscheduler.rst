.. include:: /macros.rst


hypmtscheduler (Mixed-Trust Scheduler) überApp micro-hypervisor extension
=========================================================================

This |uberappuhvext| can be enabled by using the
configure option ``--enable-uapp-hypmtscheduler`` when  
building the micro-hypervisor core framework (See :doc:`Build Micro-Hypervisor Core Framework </rpi3-cortex_a53-armv8_32/build/build-core>`).

.. note:: You must also enable the FIQ interrupt reflection configure option 
          ``--enable-fiqreflection`` when using this extension.


.. note:: Secure hyptask boot-strapping for the mixed trust scheduler |uberappuhvext|
           can be optionally enabled via the configure option
           ``--enable-uapp-hypmtscheduler-secure-hyptask-bootstrap``

Running the hypmtscheduler |uberappuhvext| is supported with only a single-CPU configuration.
You will need to append ``maxcpus=1`` to the kernel command line when installing the micro-hypervisor
framework (See :doc:`Installing Micro-Hypervisor Framework </rpi3-cortex_a53-armv8_32/installing>`).



Instructions to build and run kernel-mode test application for mixed-trust scheduler
------------------------------------------------------------------------------------

1. Follow all instructions described in :doc:`Build Micro-Hypervisor Core Framework </rpi3-cortex_a53-armv8_32/build/build-core>`

2. At this point build ``hypmtschedulertestkmod.ko`` on development
   system

   1. ``cd rgapps/linux/ugapp-hypmtscheduler-zsrmv``
   2. ``./build.sh ~/linux ~/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/``
   3. ``cp ./hypmtschedulertestkmod.ko ~/uxmhf-rpi3-staging/.``

3. Build ``hypmtschedulertest_userapp`` on development system

   1. ``cd rgapps/linux/ugapp-hypmtscheduler-zsrmv``
   2. ``make builduserapp``
   3. ``cp ./hypmtschedulertest_userapp ~/uxmhf-rpi3-staging/.``

4. Continue with the installation of the framework as described within
   :doc:`Installing Micro-Hypervisor Framework </rpi3-cortex_a53-armv8_32/installing>` and boot
   the micro-hypervisor with the uberguest OS

5. Install ``hypmtschedulertestkmod.ko`` within uberguest

   1. ``sudo insmod hypmtschedulertestkmod.ko``

6. Run ``hypmtschedulertest_userapp`` (hypmtscheduler user-mode test
   application) within uberguest to create a hyptask

   1. ``sudo ./hypmtschedulertest_userapp 1``

7. Run ``hypmtschedulertest_userapp`` (hypmtscheduler user-mode test
   application) within uberguest to disable hyptask execution for a
   period

   1. ``sudo ./hypmtschedulertest_userapp 2``

8. Run ``hypmtschedulertest_userapp`` (hypmtscheduler user-mode test
   application) within uberguest to delete hyptask

   1. ``sudo ./hypmtschedulertest_userapp 3``

9. Note: ``disable`` and ``delete`` only work on the first hyptask
   created for the above example. However, it is straightforward to
   modify the kernel module and the user-mode application to support
   multiple hyptasks as required



Developer notes for kernel-mode test application for mixed-trust scheduler
--------------------------------------------------------------------------

1. The following kernel-mode module APIs are available to bridge the
   hypervisor scheduler APIs (defined within
   ``rgapps/linux/ugapp-hypmtscheduler-zsrmv\hypmtscheduler_kmodlib.c``)

   1. ``bool hypmtscheduler_createhyptask(u32 first_period, u32 regular_period,     u32 priority, u32 hyptask_id, u32 *hyptask_handle)``

      1. creates a hyptask with specified ``first_period``,
         ``regular_period`` and ``priority``
      2. ``first_period`` and ``regular_period`` are hyptask
         time-periods specified as clock-cycles. For convenience
         definitions ``HYPMTSCHEDULER_TIME_1SEC``,
         ``HYPMTSCHEDULER_TIME_1MSEC`` and ``HYPMTSCHEDULER_TIME_1USEC``
         are provided within ``include/hypmtscheduler.h`` for approx.
         clock-cycles corresponding to 1 second, milli-second and
         micro-second respectively.
      3. ``priority`` is a ``u32`` with higher number indicating higher
         priority
      4. ``hyptask_id`` is a number from 0 to
         ``HYPMTSCHEDULER_MAX_HYPTASKID`` (defined within
         ``include/hypmtscheduler.h``) which selects a hyptask function
         to execute
      5. hyptask functions are defined within
         ``uapps/uapp-hypmtscheduler/uapp-hypmtscheduler.c`` and mapped
         via the array ``HYPTHREADFUNC hyptask_idlist``
      6. ``hyptask_handle`` contains the hyptask reference upon
         successful hyptask creation (i.e., when the function
         ``hypmtscheduler_createhyptask`` returns ``true``)

   2. ``bool hypmtscheduler_disablehyptask(u32 hyptask_handle)``

      1. disables hyptask execution for a hyptask referenced by
         ``hyptask_handle``
      2. ``hyptask_handle`` is the reference of the hyptask returned by
         a previous invocation to ``hypmtscheduler_createhyptask``
      3. returns ``true`` on success

   3. ``bool hypmtscheduler_deletehyptask(u32 hyptask_handle)``

      1. deletes a hyptask for a hyptask referenced by
         ``hyptask_handle``
      2. ``hyptask_handle`` is the reference of the hyptask returned by
         a previous invocation to ``hypmtscheduler_createhyptask``
      3. returns ``true`` on success

   4. ``bool hypmtscheduler_getrawtick32(u32 *tickcount)``

      1. gets 32-bit raw time-stamp counter value for CPU
      2. returns ``true`` on success

   5. ``bool hypmtscheduler_getrawtick64(u64 *tickcount)``

      1. gets 64-bit raw time-stamp counter value for CPU
      2. returns ``true`` on success

2. You will need to include
   ``rgapps/linux/ugapp-hypmtscheduler-zsrmv\hypmtscheduler_kmodlib.c``
   and ``include/hypmtscheduler.h`` within your own custom kernel module
   build in the event you want to use the hyper scheduler APIs within
   your own kernel-module. See
   ``rgapps/linux/ugapp-hypmtscheduler-zsrmv\Makefile`` for further
   details.



Instructions to build and run user-mode test application for mixed-trust scheduler
----------------------------------------------------------------------------------

1. Follow all instructions described in README.md and stop after
   building ``uhcalltest`` on development system

2. At this point build ``ugapp-hypmtscheduler`` on development system

   1. ``cd rgapps/linux``
   2. ``make -w all``
   3. ``cd ugapp-hypmtscheduler``
   4. ``make -w all``
   5. ``cp ./ugapp-hypmtschedulert ~/uxmhf-rpi3-staging/.``

3. Continue with remaining instructions described in README.md and boot
   the micro-hypervisor with the uberguest

4. Install ``uhcallkmod`` within uberguest

   1. ``sudo insmod uhcallkmod``

5. Run ``ugapp-hypmtscheduler`` (hypmtscheduler user-mode test
   application) within uberguest

   1. ``sudo ./ugapp-hypmtschedulertest``

6. Remove ``uhcallkmod`` within uberguest once done

   1. ``sudo rmmod uhcallkmod``

7. Shutdown the uberguest

   1. ``sudo shutdown -hP now``
