.. uberXMHF documentation master index
   author: Amit Vasudevan (amitvasudevan@acm.org)

.. include:: /macros.rst

Welcome to |uxmhflong|'s documentation!
=======================================

The |uxmhflong| (|uxmhf|) is a compositionally verifiable, extensible, micro-hypervisor framework for 
commodity platforms advocating the design and development of a new class of security-oriented
micro-hypervisor based applications (“|uberapps|”).

|uxmhf| is designed to achieve three goals: modular extensibility, automated (compositional) verification, and 
high performance.

|uxmhf| includes a core that provides functionality common to many hypervisor-based security architectures and 
supports extensions that augment the core with additional security or functional properties while preserving
the fundamental hypervisor security property of memory integrity (i.e., ensuring that the hypervisor’s memory is 
not modified by software running at a lower privilege level).

|uxmhf| advocates a “rich” commodity single-guest execution model (uberguest) where the hypervisor framework 
supports only a single, commodity guest OS and allows the guest direct access to all performance-critical system 
devices and device interrupts. In principle, the uberguest could also be a traditional hypervisor/VMM.

|uxmhf| currently runs on both x86 (Intel and AMD) and ARM (Raspberry PI) multi-core hardware virtualized
platforms with support for nested (2-dimensional) paging. The framework is capable of running unmodified 
legacy multiprocessor capable OSes such as Linux and Windows.

This documentation describes the details on the supported hardware platforms, 
supported OSes, building, verifying, installing and debugging of the 
framework and associated components.

.. toctree::
   :maxdepth: 2
   
   general-swreq.rst

.. toctree::
   :maxdepth: 2
   :caption: PC Intel x86 32-bit:

   pc-intel-x86_32/hw-requirements
   pc-intel-x86_32/supported-os
   pc-intel-x86_32/verify-build
   pc-intel-x86_32/installing
   pc-intel-x86_32/debugging


.. toctree::
   :maxdepth: 2
   :caption: Raspberry Pi3 ARMv8 32-bit:

   rpi3-cortex_a53-armv8_32/hw-requirements
   rpi3-cortex_a53-armv8_32/supported-os
   rpi3-cortex_a53-armv8_32/build/intro.rst
   rpi3-cortex_a53-armv8_32/installing
   rpi3-cortex_a53-armv8_32/debugging


.. toctree::
   :maxdepth: 2
   :caption: Legacy PC AMD/Intel x86 32-bit:

   pc-legacy-x86_32/hw-requirements
   pc-legacy-x86_32/supported-os
   pc-legacy-x86_32/verify-build
   pc-legacy-x86_32/installing   
   pc-legacy-x86_32/debugging
   pc-legacy-x86_32/uberapp-trustvisor
   pc-legacy-x86_32/uberapp-lockdown

