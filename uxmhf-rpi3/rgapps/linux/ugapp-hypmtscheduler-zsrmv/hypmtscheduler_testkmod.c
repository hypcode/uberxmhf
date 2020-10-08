/*
 * @UBERXMHF_LICENSE_HEADER_START@
 *
 * uber eXtensible Micro-Hypervisor Framework (Raspberry Pi)
 *
 * Copyright 2018 Carnegie Mellon University. All Rights Reserved.
 *
 * NO WARRANTY. THIS CARNEGIE MELLON UNIVERSITY AND SOFTWARE ENGINEERING
 * INSTITUTE MATERIAL IS FURNISHED ON AN "AS-IS" BASIS. CARNEGIE MELLON
 * UNIVERSITY MAKES NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR IMPLIED,
 * AS TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF FITNESS FOR
 * PURPOSE OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS OBTAINED FROM USE OF
 * THE MATERIAL. CARNEGIE MELLON UNIVERSITY DOES NOT MAKE ANY WARRANTY OF
 * ANY KIND WITH RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT
 * INFRINGEMENT.
 *
 * Released under a BSD (SEI)-style license, please see LICENSE or
 * contact permission@sei.cmu.edu for full terms.
 *
 * [DISTRIBUTION STATEMENT A] This material has been approved for public
 * release and unlimited distribution.  Please see Copyright notice for
 * non-US Government use and distribution.
 *
 * Carnegie Mellon is registered in the U.S. Patent and Trademark Office by
 * Carnegie Mellon University.
 *
 * @UBERXMHF_LICENSE_HEADER_END@
 */

/*
 * Author: Amit Vasudevan (amitvasudevan@acm.org)
 *
 */

#include <linux/init.h>           // macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // core header for loading LKMs into the kernel
#include <linux/device.h>         // header to support the kernel Driver Model
#include <linux/kernel.h>         // contains types, macros, functions for the kernel
#include <linux/fs.h>             // header for the Linux file system support
#include <linux/delay.h>
#include <asm/uaccess.h>          // required for the copy to user function

#include <hypmtscheduler.h>

#define  DEVICE_NAME "hypmtschedulerkmod"    			//device will appear at /dev/uhcallkmod
#define  CLASS_NAME  "hypmtschedulerkmodchar"     	   //we are a character device driver

MODULE_LICENSE("GPL");				//appease the kernel
MODULE_AUTHOR("Amit Vasudevan");
MODULE_DESCRIPTION("hypmtscheduler guest kernel-module char driver for uxmhf-rpi3");
MODULE_VERSION("0.1");

static int    major_number;
static int    number_opens = 0;
static struct class*  hypmtschedulercharClass  = NULL;
static struct device* hypmtschedulercharDevice = NULL;

//externals
extern  void __hvc(u32 uhcall_function, void *uhcall_buffer, u32 uhcall_buffer_len);
extern u64 hypmtscheduler_readtsc64(void);
extern u32 hypmtscheduler_readtscfreq(void);
extern bool hypmtscheduler_createhyptask(u32 first_period, u32 regular_period,
			u32 priority, u32 hyptask_id, u32 *hyptask_handle);
extern bool hypmtscheduler_disablehyptask(u32 hyptask_handle);
extern bool hypmtscheduler_deletehyptask(u32 hyptask_handle);
extern bool hypmtscheduler_getrawtick32(u32 *tickcount);
extern bool hypmtscheduler_getrawtick64(u64 *tickcount);
extern bool hypmtscheduler_dumpdebuglog(u8 *dst_log_buffer, u32 *num_entries);
extern u32 sysreg_read_cntfrq(void);
extern u64 sysreg_read_cntvct(void);


//prototypes for character driver interaction
static int     dev_open(struct inode *, struct file *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static int     dev_release(struct inode *, struct file *);

//file operations structure to interface with the above
static struct file_operations fops =
{
   .open = dev_open,
   .write = dev_write,
   .release = dev_release,
};

u32 hyptask_handle;

hypmtscheduler_logentry_t debug_log[DEBUG_LOG_SIZE];
u32 debug_log_buffer_index = 0;


static int dev_open(struct inode *inodep, struct file *filep){
   number_opens++;
   printk(KERN_INFO "hypmtschedulerkmod: device has been opened %d time(s)\n", number_opens);
   return 0;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
	switch(len){
	case 1:
		printk(KERN_INFO "hypmtschedulerkmod: create_hyptask\n");
		if(!hypmtscheduler_createhyptask(1 * HYPMTSCHEDULER_TIME_1SEC, 2 * HYPMTSCHEDULER_TIME_1SEC,
					3, 3, &hyptask_handle)){
			printk(KERN_INFO "hypmtschedulerkmod: create_hyptask failed\n");
			return -EINVAL;
		}
		break;

	case 2:
		printk(KERN_INFO "hypmtschedulerkmod: disable_hyptask\n");
		if(!hypmtscheduler_disablehyptask(hyptask_handle)){
			printk(KERN_INFO "hypmtschedulerkmod: disable_hyptask failed\n");
			return -EINVAL;
		}
		break;

	case 3:
		printk(KERN_INFO "hypmtschedulerkmod: delete_hyptask\n");
		if(!hypmtscheduler_deletehyptask(hyptask_handle)){
			printk(KERN_INFO "hypmtschedulerkmod: delete_hyptask failed\n");
			return -EINVAL;
		}
		break;

	case 4:
		{
			u32 i;
			printk(KERN_INFO "hypmtschedulerkmod: dump debug log...\n");

			if(!hypmtscheduler_dumpdebuglog(&debug_log, &debug_log_buffer_index)){
				printk(KERN_INFO "hypmtschedulerkmod: dumpdebuglog hypercall API failed\n");
				return -EINVAL;
			}

			printk(KERN_INFO "hypmtschedulerkmod: total debug log entries: %u\n",
					debug_log_buffer_index);
			printk(KERN_INFO "hypmtschedulerkmod: dump of log follows...\n");

			for(i=0; i < debug_log_buffer_index; i++){
				printk(KERN_INFO "hypmtschedulerkmod: %03u --> 0x%08x 0x%016llx 0x%08x\n",
						i,
						debug_log[i].hyptask_id,
						debug_log[i].timestamp,
						debug_log[i].event_type);
			}
		}
		break;


	default:
		printk(KERN_INFO "hypmtschedulerkmod: unknown function, ignoring\n");
		break;
	}

	return 0;
}

static int dev_release(struct inode *inodep, struct file *filep){
   number_opens--;
   printk(KERN_INFO "hypmtschedulerkmod: device successfully closed\n");
   return 0;
}




//module initialization function
int hypmtschedulerkmod_init(void)
{
#if 0
	u32 rawtick_32;
	u64 rawtick_64;
#endif

#if 0
	u64 ts, te;
	u32 cntfrq;
#endif

#if 0
	u64 ts, tm, te;
#endif


	printk(KERN_INFO "hypmtschedulerkmod: LOAD\n");
	printk(KERN_INFO "author: amit vasudevan (amitvasudevan@acm.org)\n");

#if 0
	if(!hypmtscheduler_getrawtick32(&rawtick_32)){
		printk(KERN_INFO "hypmtschedulerkmod: error getting rawtick_32, exiting!\n");
		return -EINVAL;
	}

	if(!hypmtscheduler_getrawtick64(&rawtick_64)){
		printk(KERN_INFO "hypmtschedulerkmod: error getting rawtick_64, exiting!\n");
		return -EINVAL;
	}

	printk(KERN_INFO "hypmtschedulerkmod: rawtick_32 = 0x%08x\n", rawtick_32);
	printk(KERN_INFO "hypmtschedulerkmod: rawtick_64 = 0x%016llx\n", rawtick_64);
#endif


#if 0
	ts=sysreg_read_cntvct();
	msleep(2000);
	te=sysreg_read_cntvct();
	printk(KERN_INFO "hypmtschedulerkmod: elapsed cycles = %llu\n",  (te-ts));
	printk(KERN_INFO "hypmtschedulerkmod: cntfrq = %u\n", sysreg_read_cntfrq());
#endif

#if 0
	ts=hypmtscheduler_readtsc64();
	if(!hypmtscheduler_getrawtick64(&tm)){
		printk(KERN_INFO "hypmtschedulerkmod: error getting rawtick_64, exiting!\n");
		return -EINVAL;
	}
	te=hypmtscheduler_readtsc64();
	printk(KERN_INFO "hypmtschedulerkmod: ts=%llu\n",  ts);
	printk(KERN_INFO "hypmtschedulerkmod: tm=%llu\n",  tm);
	printk(KERN_INFO "hypmtschedulerkmod: te=%llu\n",  te);
	printk(KERN_INFO "hypmtschedulerkmod: cntfrq = %u\n", hypmtscheduler_readtscfreq());
#endif



	//try to allocate a major number dynamically
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number<0){
	  printk(KERN_ALERT "hypmtschedulerkmod: failed to register a major number\n");
	  return major_number;
	}
	printk(KERN_INFO "hypmtschedulerkmod: registered correctly with major number %d\n", major_number);

	// Register the device class
	hypmtschedulercharClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(hypmtschedulercharClass)){
	  unregister_chrdev(major_number, DEVICE_NAME);
	  printk(KERN_ALERT "hypmtschedulerkmod: Failed to register device class\n");
	  return PTR_ERR(hypmtschedulercharClass);
	}
	printk(KERN_INFO "hypmtschedulerkmod: device class registered correctly\n");

	// register the device driver
	hypmtschedulercharDevice = device_create(hypmtschedulercharClass, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(hypmtschedulercharDevice)){
	  class_destroy(hypmtschedulercharClass);
	  unregister_chrdev(major_number, DEVICE_NAME);
	  printk(KERN_ALERT "hypmtschedulerkmod:Failed to create the device\n");
	  return PTR_ERR(hypmtschedulercharDevice);
	}
	printk(KERN_INFO "hypmtschedulerkmod: device class created correctly\n");




	return 0;
}

//module unload function
void hypmtschedulerkmod_exit(void)
{
	device_destroy(hypmtschedulercharClass, MKDEV(major_number, 0));     // remove the device
	class_unregister(hypmtschedulercharClass);                          // unregister the device class
	class_destroy(hypmtschedulercharClass);                             // remove the device class
	unregister_chrdev(major_number, DEVICE_NAME);             // unregister the major number
	printk(KERN_INFO "hypmtschedulerkmod: UNLOAD\n");
}

module_init(hypmtschedulerkmod_init);
module_exit(hypmtschedulerkmod_exit);
