#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/unistd.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/timer.h>

#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");

/**
	BCM(broadcom)
******************************
		25	26	27
	21	[1] [2] [3]
	22	[4] [5] [6]
	23	[7] [8] [9]
	24	[*] [0] [#]
******************************
	output BCM 21, 22, 23, 24
	input BCM 25, 26, 27	
*/

#define GPIO_BASE_PHY 0x3F200000		// base address
void	*GPIO_BASE_VIR,					// registers to control gpio
		*GPFSEL2_VIR, 
		*GPSET0_VIR, 
		*GPCLR0_VIR, 
		*GPLEV0_VIR;

#define GPFSEL_2	( *(volatile unsigned long*)GPFSEL2_VIR )
#define GPSET_0		( *(volatile unsigned long*)GPSET0_VIR )
#define GPCLR_0		( *(volatile unsigned long*)GPCLR0_VIR )
#define GPLEV_0		( *(volatile unsigned long*)GPLEV0_VIR )

#define BCM25_NUM 25    // BCM25(Key *)
#define BCM27_NUM 27    // BCM27(Key #)

// device's name & major number
#define DEVICE_NAME "doorlock"
int MAJOR_NUMBER = 0;

#define SET_GPIO(num) (GPSET_0 = 0x00000001 << num)
#define CLR_GPIO(num) (GPCLR_0 = 0x00000001 << num)

short int	bcm25_irqnum = 0;
short int	bcm27_irqnum = 0;
int exeIrqNum = 0;
static int ev_press	= 0;
static DECLARE_WAIT_QUEUE_HEAD(key_waitqueue);
/**
	Interrupt handler 
*/
static irqreturn_t 
keypad_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	printk(KERN_WARNING "Doorlock : irq_handler() open, irq:%d\n", irq);
	
	exeIrqNum	= irq;
	ev_press	= 1;
	CLR_GPIO(24);
	wake_up_interruptible(&key_waitqueue);
	return IRQ_HANDLED;	// Success
}


/**
	Initialize Interrupt(BCM 25, 27)
*/
static void 
interrupt_init(void)
{
	printk(KERN_EMERG "interrupt_init() opened\n");
	if( (bcm25_irqnum = gpio_to_irq(BCM25_NUM) ) < 0) {
		printk("%s : GPIO to IRQ mapping failure:%s\n", __FUNCTION__, "BCM25");
		return; 
	}   

	if( (bcm27_irqnum = gpio_to_irq(BCM27_NUM) ) < 0) {
		printk("%s : GPIO to IRQ mapping failure:%s\n", __FUNCTION__, "BCM27");
		return;
	}   

	printk("%s : Mapped interrupt 25 : %d, 27 : %d\n"
			, __FUNCTION__, bcm25_irqnum, bcm27_irqnum );

	if( request_irq(
			bcm25_irqnum , 
			(irq_handler_t)keypad_irq_handler, 
			IRQF_TRIGGER_FALLING, 
			"BCM25 ITR", 
			"BCM25IRQ") ) {

		printk("%s : BCM25 irq Request failure\n", __FUNCTION__);
	}   

	if( request_irq(
			bcm27_irqnum, 
			(irq_handler_t)keypad_irq_handler,
			IRQF_TRIGGER_FALLING,
			"BCM27 ITR",
			"BCM27IRQ") ) {

		printk("%s : BCM25 irq Request failure\n", __FUNCTION__);
	}   
}


/**
	Free interrupt(BCM 25, 27)
*/
static void
interrupt_free(void){
	printk(KERN_EMERG "interrupt free opened\n");
	free_irq(bcm25_irqnum , "BCM25IRQ");
	free_irq(bcm27_irqnum , "BCM27IRQ");
}


/**
	Doorlock open
*/
static int 
doorlock_open(struct inode *inode, struct file *file)
{
	// init & set output or input 
	//GPFSEL_2 = 0x1248;// 0 001 001 001 001 000 To set output as BCM 21,22,23,24
	GPFSEL_2 = 0x1249;	// 0 001 001 001 001 001 To set output as BCM 21,22,23,24
	// 0407 ADD set bcm 20 output for keypad LED
	printk(KERN_EMERG "doorlock_open() opened\n");// HW intialization
	return 0;
}


/**
	Doorlock release
*/
static int 
doorlock_release(struct inode *inode, struct file *file)
{
	printk(KERN_NOTICE "doorloc_release() opened\n");// HW intialization
	return 0;
}

/**
	Doorlock read
*/
static ssize_t 
doorlock_read (struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	printk(KERN_NOTICE "doorloc_read() opened\n");// HW intialization

	int flag = 0;
	int ret = 0;
	// output only BCM 24
	CLR_GPIO(21);
	CLR_GPIO(22);
	CLR_GPIO(23);
	SET_GPIO(24);

	exeIrqNum = 0;
	ev_press = 0;
	
	wait_event_interruptible(key_waitqueue, ev_press);
	
	if(exeIrqNum == bcm25_irqnum ){
		flag = 10;
		ret = copy_to_user(buf,&flag,count);
	}
	if( exeIrqNum == bcm27_irqnum ){
		flag = 12;
		ret = copy_to_user(buf,&flag,count);
	}	
	
	printk(KERN_WARNING "KERNEL SEND DATA IS >> %d\n",exeIrqNum);
	printk(KERN_WARNING "KERNEL RECEIVE COUNT IS >> %d\n",count);

	return(count);
}

static long 
doorlock_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
	//for unlocekd_ioctl
{
	switch(arg){
		case 1:{
				   //printk("\n");
				   int i;	// iterator
				   int num;	// pushed number

				   for(i = 0; i < 4; i++) {

					   //printk(KERN_NOTICE "i = %d\n",i);// HW intialization
					   num = 99;
					   GPLEV_0 = 0x00;
					   CLR_GPIO(21);
					   CLR_GPIO(22);
					   CLR_GPIO(23);
					   CLR_GPIO(24);
					   if( i == 0){    //output vcc on one by one
						   SET_GPIO(21);
					   }
					   if( i == 1){
						   SET_GPIO(22);
					   }
					   if( i == 2){
						   SET_GPIO(23);
					   }
					   if( i == 3){
						   SET_GPIO(24);
					   }

					   //printk(KERN_NOTICE "GPLEV = %x\n",GPLEV_0);
					   if(GPLEV_0 & 0x02000000){ //if input BCM 25 bit 1
						   if(i ==0) num = 1;
						   if(i ==1) num = 4;
						   if(i ==2) num = 7;
						   if(i ==3) num = 10;
						   //printk("i:%d,j:%d,num:%d",i+1,1,num);
						   return num;
					   }
					   if( GPLEV_0 & 0x04000000){
						   if(i ==0) num = 2;
						   if(i ==1) num = 5;
						   if(i ==2) num = 8;
						   if(i ==3) num = 0;
						   //printk("i:%d,j:%d,num:%d",i+1,2,num);
						   return num;
					   }
					   if( GPLEV_0 & 0x08000000){
						   if(i ==0) num = 3;
						   if(i ==1) num = 6;
						   if(i ==2) num = 9;
						   if(i ==3) num = 12;
						   //printk("i:%d,j:%d,num:%d",i+1,3,num);
						   return num;
					   }
				   
				   }
				   return num;
				   break;
			   }

		case 2:{	
					//KEYPAD LED ON
					 printk(KERN_NOTICE "ioctl() 2 open keypad led on");
					 SET_GPIO(20);
				   break;
				}
		case 3:{
					//KEYPAD LED OFF
					 printk(KERN_NOTICE "ioctl() 3 open keypad led off");
					CLR_GPIO(20);
				   break;
				}

		default: return 0;
	}

	return 0;
}


struct file_operations doorlock_fops = {
	open			:	doorlock_open,
	read			:	doorlock_read,
	unlocked_ioctl	:	doorlock_ioctl,   
	release			:	doorlock_release
};


/**
	Initialize device driver(Doorlock)
----------------------------------------
	1. register character device
	2. remap base address
	3. initialize interrupt
*/
static int __init doorlock_init(void)
{
	printk(KERN_ALERT "Doorlock : init()\n" );

	MAJOR_NUMBER = register_chrdev(MAJOR_NUMBER, DEVICE_NAME, &doorlock_fops);
	printk(KERN_ALERT "Major number = %d\n", MAJOR_NUMBER);

	// mapping
	GPIO_BASE_VIR	= ioremap(GPIO_BASE_PHY,0x1000);
	GPFSEL2_VIR     = GPIO_BASE_VIR + 0x08;
	GPSET0_VIR		= GPIO_BASE_VIR + 0x1C;
	GPCLR0_VIR      = GPIO_BASE_VIR + 0x28;
	GPLEV0_VIR      = GPIO_BASE_VIR + 0x34;

	interrupt_init();
	return 0;
}


/**
	Exit device driver(Doorlock)
--------------------------------------
	1. unmap base address
	2. unregister character device
	3. free interrupt
*/ 
static void __exit doorlock_exit(void)
{
	printk (KERN_ALERT "Doorlock : exit()\n");
	interrupt_free();
	iounmap(GPIO_BASE_VIR);
	unregister_chrdev(MAJOR_NUMBER, DEVICE_NAME);

}


module_init(doorlock_init);
module_exit(doorlock_exit);
