.. include:: /macros.rst


Installing
==========

Refer to the :doc:`build process </rpi3-cortex_a53-armv8_32/build/intro>`
to build the required binaries for installation. Then proceed with the 
following instructions to deploy uberXMHF on the SD card:

Deploying binaries to SD Card
-----------------------------


#. 
   ``mkdir -p ~/mnt/pi-boot``

#. 
   ``mkdir -p ~/mnt/pi-root``

#. 
   ``sudo mount /dev/mmcblk0p1 ~/mnt/pi-boot``

#. 
   ``sudo mount /dev/mmcblk0p2 ~/mnt/pi-root``

#. 
   ``sudo cp ~/uxmhf-rpi3-staging/uhcalltest ~/mnt/pi-root/home/pi/.``

#. 
   ``sudo cp ~/uxmhf-rpi3-staging/uhcallkmod.ko ~/mnt/pi-root/home/pi/.``

#. 
   ``sudo cp ~/uxmhf-rpi3-staging/uxmhf-rpi3.img ~/mnt/pi-boot/.``

#. 
   ``sudo cp ~/uxmhf-rpi3-staging/config.txt ~/mnt/pi-boot/.``

#. 
   Append ``loglevel=8 memblock=debug dwc_otg.fiq_enable=0 dwc_otg.fiq_fsm_enable=0 dwc_otg.nak_holdoff=0`` to ``~/mnt/pi-boot/cmdline.txt``

#. 
   ``sudo cp ~/uxmhf-rpi3-staging/boot/* ~/mnt/pi-boot/.``

#. 
   ``sudo mkdir -p ~/mnt/pi-boot/overlays``

#. 
   ``sudo cp -R ~/uxmhf-rpi3-staging/overlays/* ~/mnt/pi-boot/overlays/.``

#. 
   ``sudo cp ~/uxmhf-rpi3-staging/kernel7.img ~/mnt/pi-boot/.``

#. 
   ``sudo mkdir -p ~/mnt/pi-root/lib/firmware``

#. 
   ``sudo cp -R ~/uxmhf-rpi3-staging/mod_install/lib/firmware/* ~/mnt/pi-root/lib/firmware/.``

#. 
   ``sudo mkdir -p ~/mnt/pi-root/lib/modules/4.4.50-v7+``

#. 
   ``sudo cp -R ~/uxmhf-rpi3-staging/mod_install/lib/modules/4.4.50-v7+/* ~/mnt/pi-root/lib/modules/4.4.50-v7+/.``

#. 
   Edit ``~/mnt/pi-root/etc/fstab`` and comment out line beginning with ``/dev/mmcblk0p1`` which is mounted to boot

Boot up and test
----------------


#. 
   ``umount ~/mnt/pi-boot``

#. 
   ``umount ~/mnt/pi-root``

#. 
   Insert sd-card into the Raspberry PI 3

#. 
   Power on the Raspberry PI 3

#. 
   ... and you should see uberXMHF booting up with debug output and the guest starting soon thereafter
