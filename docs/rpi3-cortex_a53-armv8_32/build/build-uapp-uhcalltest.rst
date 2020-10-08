.. include:: /macros.rst


uhcalltest |uberapp| micro-hypervisor extension
===============================================

The following instructions show how the example |uberapp| 
``uxmhf-rpi3/rgapps/linux/rgapp-uhcalltest`` (to test 
hypercalls) is built. 


#. Building ``uhcallkmod`` on development system

   #. ``cd rgapps/linux/rgapp-uhcallkmod``
   #. ``./build.sh ~/linux ~/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/``
   #. ``cp ./uhcallkmod.ko ~/uxmhf-rpi3-staging/.``

#. Building ``uhcalltest`` on development system

   #. ``cd rgapps/linux``
   #. ``make -w all``
   #. ``cd rgapp-uhcalltest``
   #. ``make -w all``
   #. ``cp ./uhcalltest ~/uxmhf-rpi3-staging/.``
