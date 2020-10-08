.. include:: /macros.rst


Other |uberapp| Micro-Hypervisor Extensions
===========================================

The following are other available |uberapp| micro-hypervisor extensions:


.. note:: You can enable these extensions by running ``./configure`` with the appropriate 
          parameters specified below while building the micro-hypervisor core framework. 


#. ``./configure --enable-uapp-uhsign`` to enable a |uberapp| micro-hypervisor extension which provides HMAC key signing functionality
#. ``./configure --enable-uapp-uagent`` to enable a |uberapp| micro-hypervisor extension which provides AES encryption functionality
#. ``./configure --enable-uapp-uhstateDB`` to enable a |uberapp| micro-hypervisor extension which provides a simple database that tracks the state of each entry   
#. ``./configure --enable-uapp-pvdriver-uart`` to enable a |uberapp| micro-hypervisor extension which provides a guest OS UART para-virtualized driver backend


For the UART para-virtualized driver backend |uberapp| micro-hypervisor extension, you 
will need to build the RPi kernel by ensuring that
``CONFIG_SERIAL_AMBA_PL011=y`` and ``CONFIG_SERIAL_AMBA_PL011_CONSOLE=y`` within the file ``.config`` is replaced
by ``# CONFIG_SERIAL_AMBA_PL011 is not set`` and ``# CONFIG_SERIAL_AMBA_PL011_CONSOLE is not set`` respectively between
the ``make ARCH=arm CROSS_COMPILE=~/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf- bcm2709_defconfig``
and ``make -j 4 ARCH=arm CROSS_COMPILE=~/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf- zImage modules dtbs``
steps in the **Install and build RPI kernel on development system** section within 
:doc:`Build Micro-Hypervisor Core Framework </rpi3-cortex_a53-armv8_32/build/build-core>`.

