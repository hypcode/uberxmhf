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

#include "../../../include/hypmtscheduler.h"
#include "../../../include/mavlinkserhb.h"

#define  DEVICE_NAME "mavlinkserhbkmod"    			//device will appear at /dev/uhcallkmod
#define  CLASS_NAME  "mavlinkserhbkmodchar"     	   //we are a character device driver

MODULE_LICENSE("GPL");				//appease the kernel
MODULE_AUTHOR("Amit Vasudevan");
MODULE_DESCRIPTION("mavlinkserhb guest kernel-module char driver for uxmhf-rpi3");
MODULE_VERSION("0.1");

static int    major_number;
static int    number_opens = 0;
static struct class*  mavlinkserhbcharClass  = NULL;
static struct device* mavlinkserhbcharDevice = NULL;

//////
//externals
//////
extern  void __hvc(u32 uhcall_function, void *uhcall_buffer, u32 uhcall_buffer_len);
extern void mavlinkserhb_initialize(u32 baudrate);
extern bool mavlinkserhb_send(u8 *buffer, u32 buf_len);
extern bool mavlinkserhb_checkrecv(void);
extern bool mavlinkserhb_recv(u8 *buffer, u32 max_len, u32 *len_read, bool *uartreadbufexhausted);
extern bool mavlinkserhb_activatehbhyptask(u32 first_period, u32 recurring_period,
		u32 priority);
extern bool mavlinkserhb_deactivatehbhyptask(void);


//////
//prototypes for character driver interaction
//////
static int     dev_open(struct inode *, struct file *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static int     dev_release(struct inode *, struct file *);

//////
//file operations structure to interface with the above
//////
static struct file_operations fops =
{
   .open = dev_open,
   .write = dev_write,
   .release = dev_release,
};

static int dev_open(struct inode *inodep, struct file *filep){
   number_opens++;
   printk(KERN_INFO "mavlinkserhbkmod: device has been opened %d time(s)\n", number_opens);
   return 0;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
	switch(len){
		case UAPP_MAVLINKSERHB_UHCALL_INITIALIZE:
			printk(KERN_INFO "mavlinkserhbkmod: initialize\n");
			mavlinkserhb_initialize(115200);
			printk(KERN_INFO "mavlinkserhbkmod: initialization successful!\n");
			break;

		case UAPP_MAVLINKSERHB_UHCALL_SEND:
			printk(KERN_INFO "mavlinkserhbkmod: send\n");
			if(mavlinkserhb_send("Test", 4))
				printk(KERN_INFO "mavlinkserhbkmod: send successful!\n");
			else
				printk(KERN_INFO "mavlinkserhbkmod: error in send\n");
			break;

		case UAPP_MAVLINKSERHB_UHCALL_CHECKRECV:
			printk(KERN_INFO "mavlinkserhbkmod: checkrecv\n");
			if(mavlinkserhb_checkrecv())
				printk(KERN_INFO "mavlinkserhbkmod: recv buffer has data!\n");
			else
				printk(KERN_INFO "mavlinkserhbkmod: no data in recv buffer\n");
			break;

		case UAPP_MAVLINKSERHB_UHCALL_RECV:
		{
			bool readbufferexhausted;
			u32 len_read;
			u8 buffer[3];
			printk(KERN_INFO "mavlinkserhbkmod: recv\n");
			memset(buffer, sizeof(buffer), 0);
			if(mavlinkserhb_recv(&buffer, sizeof(buffer), &len_read, &readbufferexhausted)){
				printk(KERN_INFO "mavlinkserhbkmod: read: <0x%02x|0x%02x|0x%02x>, len_read=%u, \
						readbufferexhausted=%u\n", buffer[0], buffer[1], buffer[2],
						len_read, readbufferexhausted);

			}else{
				printk(KERN_INFO "mavlinkserhbkmod: no data in recv buffer\n");
			}
		}
			break;

		case UAPP_MAVLINKSERHB_UHCALL_ACTIVATEHBHYPTASK:
			if(mavlinkserhb_activatehbhyptask((0.5 * HYPMTSCHEDULER_TIME_1SEC),
					(0.5 * HYPMTSCHEDULER_TIME_1SEC), 99)){
				printk(KERN_INFO "mavlinkserhbkmod: started hb hyptask\n");
			}else{
				printk(KERN_INFO "mavlinkserhbkmod: could not activate hb hyptask\n");
			}
			break;

		case UAPP_MAVLINKSERHB_UHCALL_DEACTIVATEHBHYPTASK:
			if(mavlinkserhb_deactivatehbhyptask()){
				printk(KERN_INFO "mavlinkserhbkmod: deactivated hb hyptask\n");
			}else{
				printk(KERN_INFO "mavlinkserhbkmod: could not de-activate hb hyptask\n");
			}
			break;

		default:
			printk(KERN_INFO "mavlinkserhbkmod: unknown function, ignoring\n");
			break;
	}

	return 0;
}

static int dev_release(struct inode *inodep, struct file *filep){
   number_opens--;
   printk(KERN_INFO "mavlinkserhbkmod: device successfully closed\n");
   return 0;
}




//////
//module initialization function
//////
int mavlinkserhbkmod_init(void)
{

	printk(KERN_INFO "mavlinkserhbkmod: LOAD\n");
	printk(KERN_INFO "author: amit vasudevan (amitvasudevan@acm.org)\n");

	//try to allocate a major number dynamically
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number<0){
	  printk(KERN_ALERT "mavlinkserhbkmod: failed to register a major number\n");
	  return major_number;
	}
	printk(KERN_INFO "mavlinkserhbkmod: registered correctly with major number %d\n", major_number);

	// Register the device class
	mavlinkserhbcharClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(mavlinkserhbcharClass)){
	  unregister_chrdev(major_number, DEVICE_NAME);
	  printk(KERN_ALERT "mavlinkserhbkmod: Failed to register device class\n");
	  return PTR_ERR(mavlinkserhbcharClass);
	}
	printk(KERN_INFO "mavlinkserhbkmod: device class registered correctly\n");

	// register the device driver
	mavlinkserhbcharDevice = device_create(mavlinkserhbcharClass, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(mavlinkserhbcharDevice)){
	  class_destroy(mavlinkserhbcharClass);
	  unregister_chrdev(major_number, DEVICE_NAME);
	  printk(KERN_ALERT "mavlinkserhbkmod:Failed to create the device\n");
	  return PTR_ERR(mavlinkserhbcharDevice);
	}
	printk(KERN_INFO "mavlinkserhbkmod: device class created correctly\n");


	return 0;
}



//////
//module unload function
//////
void mavlinkserhbkmod_exit(void)
{
	device_destroy(mavlinkserhbcharClass, MKDEV(major_number, 0));     // remove the device
	class_unregister(mavlinkserhbcharClass);                          // unregister the device class
	class_destroy(mavlinkserhbcharClass);                             // remove the device class
	unregister_chrdev(major_number, DEVICE_NAME);             // unregister the major number
	printk(KERN_INFO "mavlinkserhbkmod: UNLOAD\n");
}

module_init(mavlinkserhbkmod_init);
module_exit(mavlinkserhbkmod_exit);
