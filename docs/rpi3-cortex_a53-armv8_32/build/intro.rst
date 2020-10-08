.. include:: /macros.rst


.. _uxmhfrpi3-build-intro:

Building
========

Building comprises of building the micro-hypervisor core configured with selected 
functionality corresponding to |uberapp| micro-hypervisor extensions. 

An |uberapp| comprises of a legacy guest OS application (found within ``uxmhf-rpi3/rgapps/linux``)
and a micro-hypervisor extension (found within ``uxmhf-rpi3/uapps/``)

.. note::  There are two libraries to support refactoring legacy guest OS code into
           |uberapps|. There is a userspace library (``uxmhf-rpi3/rgapps/linux/libs/libuhcall``) 
           and a kernel space library (``uxmhf-rpi3/rgapps/linux/libs/libkhcall``). 
           These provide a function call interface for making a micro-hypervisor
           extension call (e.g., ``uhcall()`` for making userspace calls to invoke an 
           |uberapp| micro-hypervisor extension).


.. toctree::
   :maxdepth: 4

   Build micro-hypervisor core framework <build-core.rst>
   uhcalltest überApp micro-hypervisor extension <build-uapp-uhcalltest.rst>
   hypmtscheduler überApp micro-hypervisor extension <build-uapp-hypmtscheduler.rst>
   mavlinkserhb überApp micro-hypervisor extension <build-uapp-mavlinkserhb.rst>
   Other überApp micro-hypervisor extensions <build-uapp-others.rst>