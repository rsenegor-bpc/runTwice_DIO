/*
    kern_compat.h
    Kernel compatibility header file

    Copyright (C) 1997-8 David A. Schleef <ds@stm.lbl.gov>
    Copyright (C) 1999 Tomasz Motylewski <motyl@ip.pl>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

/*
   Portions taken from Don Becker's network device drivers, and
   modified.
*/

/*
   The purpose of this file is to make it easy to write modules
   that will compile correctly for multiple kernel versions.
   This file can only provide backward compatibility, i.e.,
   write your driver for 2.3.x, include this header file, and
   theoretically, it will compile and run on 2.0.37.  However,
   some interface changes require superset definitions to
   allow compilation for all supported kernel versions, so
   you have to use the interface provided in this file to
   compile for all kernels.

   If your driver is written for the 2.2.x interface, define
   COMPAT_V22 before including this file.
*/

/* added PRINT */

#ifdef DEBUG
#define PRINT(args...)  pd_printk(KERN_DEBUG ## args)
#else
#define PRINT(args...) do {} while(0)
#endif

#ifndef _KERN_COMPAT_H
#define _KERN_COMPAT_H

#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33))
   #include <generated/autoconf.h>
#else
   #include <linux/autoconf.h>
#endif

#include <linux/kdev_t.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 0)
   #include <linux/slab.h>
#else
   #include <linux/malloc.h>
#endif
#include <linux/errno.h>

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c)	(((a) << 16) + ((b) << 8) + (c))
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,0,0)
#error kernel versions prior to 2.0 not supported
#else
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,1,0)
#define LINUX_V20
#else
#define LINUX_V22
#endif
#endif


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,2,0)	/* XXX */
#define RDEV_OF_FILE(x)        ((x)->f_inode->i_rdev)
#else
#define RDEV_OF_FILE(x)        ((x)->f_dentry->d_inode->i_rdev)
#endif


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,2,0)	/* XXX */
#define signal_pending(x)	(((x)->signal) & (~(x)->blocked))
#endif


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,2,0)	/* XXX */
/* somewhere about 2.1.4 */

#include <asm/segment.h>

static inline int copy_to_user(void * to,const void *from,unsigned long n_bytes)
{
	int i;

	if((i=verify_area(VERIFY_WRITE,to,n_bytes)) != 0)
		return i;
	memcpy_tofs(to,from,n_bytes);
	return 0;
}

static inline int copy_from_user(void * to,const void *from,unsigned long n_bytes)
{
	int i;
	if((i=verify_area(VERIFY_READ,from,n_bytes))!=0)
		return i;
	memcpy_fromfs(to,from,n_bytes);
	return 0;
}

static inline int clear_user(void * mem,unsigned long len)
{
	char *cmem=mem;
	
	if(verify_area(VERIFY_WRITE,mem,len))
		return len;
	/* this is slow, but I'm lazy */
	while(len--){put_user(0,cmem);cmem++;}
	return 0;
}

#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,1,0)
static inline void __process_timeout(unsigned long __data)
{
	struct task_struct * p = (struct task_struct *) __data;

	p->timeout=0;
	wake_up_process(p);
}

static inline long interruptible_sleep_on_timeout(struct wait_queue ** p,
	long timeout)
{
	struct timer_list timer;
	unsigned long expires=jiffies+timeout;

	init_timer(&timer);
	timer.expires=expires;
	timer.data=(unsigned long)current;
	timer.function=__process_timeout;
	add_timer(&timer);

	interruptible_sleep_on(p);

	del_timer(&timer);

	return jiffies-expires;
}
#else
#define HAVE_INTERRUPTIBLE_SLEEP_ON_TIMEOUT
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,1,0)
#ifndef __alpha__
#define ioremap(a,b) \
        (((a)<0x100000) ? (void *)((u_long)(a)) : vremap(a,b))
#define iounmap(v) \
	        do { if ((u_long)(v) > 0x100000) vfree(v); } while (0)
#endif
#endif

#if LINUX_VERSION_CODE < 0x020115
#define MODULE_AUTHOR(a)
#define MODULE_DESCRIPTION(a)
#define MODULE_PARM(a,b)
#endif

#if LINUX_VERSION_CODE < 0x20138
#define test_and_set_bit(val, addr) set_bit(val, addr)
#define le32_to_cpu(val) (val)
#define cpu_to_le32(val) (val)
#endif

#if LINUX_VERSION_CODE <= 0x20139
#define net_device_stats enet_statistics
#define NETSTATS_VER2
#endif

#if LINUX_VERSION_CODE < 0x20155
#include <linux/bios32.h>
#define PCI_SUPPORT_VER1
#else
#define PCI_SUPPORT_VER2
#endif

#if LINUX_VERSION_CODE < 0x20159
#define DEV_FREE_SKB(skb) dev_kfree_skb (skb, FREE_WRITE);
#else  /* Grrr, unneeded incompatible change. */
#define DEV_FREE_SKB(skb) dev_kfree_skb(skb);
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,10)                /* ? */
#define file_atomic_inc(x)     ((*(x))++)
#else
#define file_atomic_inc(x)     atomic_inc(x)
#endif


#ifndef COMPAT_V22

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,5)
typedef struct wait_queue *wait_queue_head_t;
#define DECLARE_WAITQUEUE(x,y) struct wait_queue x={y,NULL}
#define init_waitqueue_head(x)
#endif

#endif

/* RTLinux changes */

#ifdef __RTL__
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,1,0)
#define RTL_V1
/* RTL 1.1 */
#include <asm/rt_irq.h>

#define rtl_request_global_irq(irq,handler) request_RTirq(irq,handler)
#define rtl_free_global_irq(irq) free_RTirq(irq)

#define rtl_request_local_irq(a,b,c) rtl_request_local_irq_not_available
#define rtl_free_local_irq(a) rtl_free_local_irq_not_available

static inline int rtl_printf(const char *format,...)
{
	/* at least it will compile... =) */

	return 0;
}

#define rt_printk(format,args...)	rtl_printf(format,## args)

#else
#define RTL_V2
/* RTL 2.2 */
#include <linux/rtl.h>
#include <linux/cons.h>
#define rt_printk(format, args...)	rtl_printf(format,## args)
#endif
#else /* !__RTL__ */
//#define rt_printk(format, args...)	pd_printk(format,## args)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,2,0)	/* XXX */

#ifdef __RTL__
/* XXX gack! don't look too closely */
#define claim_dma_lock()	0
#define release_dma_lock(x)	
#else
/* XXX gack! a cli/sti pair */
#define claim_dma_lock()	(cli(),0)
#define release_dma_lock()	sti()
#endif

#endif

#endif /* _KERN_COMPAT_H */

