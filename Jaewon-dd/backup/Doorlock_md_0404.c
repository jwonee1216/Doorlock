#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/unistd.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/timer.h>


//README
//BCM(broadcom)
//output BCM 21,22,23,24
//input BCM 25,26,27

// base address
#define GPIO_BASE_PHY 0x3F200000

// registers to control raspberryPi
void	*GPIO_BASE_VIR, 
		*GPFSEL2_VIR, 
		*GPSET0_VIR, 
		*GPCLR0_VIR, 
		*GPLEV0_VIR;

#define GPFSEL_2	( *(volatile unsigned long*)GPFSEL2_VIR )
#define GPSET_0		( *(volatile unsigned long*)GPSET0_VIR )
#define GPCLR_0		( *(volatile unsigned long*)GPCLR0_VIR )
#define GPLEV_0		( *(volatile unsigned long*)GPLEV0_VIR )

// device's name & major number
#define DEVICE_NAME "doorlock"
int MAJOR_NUMBER = 0;

int doorlock_open(struct inode *inode, struct file *file)
{
	// init & set output or input 
	GPFSEL_2 = 0x1248;	// 0 001 001 001 001 000 To set output as BCM 21,22,23,24

	printk(KERN_EMERG "doorlock Device has been opened\n");// HW intialization
	return 0;
}

int doorlock_release(struct inode *inode, struct file *file)
{
	printk(KERN_NOTICE "doorlock Device has been releaseed\n");// HW intialization
	return 0;
}

#define SET_GPIO(num) (GPSET_0 = 0x00000001 << num)
#define CLR_GPIO(num) (GPCLR_0 = 0x00000001 << num)

//int doorlock_ioctl(struct inode *inode, struct file *file,unsigned int cmd,unsigned long arg)
//for ioctl
static long doorlock_ioctl(struct file *file,unsigned int cmd, unsigned long arg)
	//for unlocekd_ioctl
{
	switch(arg){
		case 1:{
				   printk("\n");
				   printk(KERN_WARNING "doorlock iotcl cmd =1\n");
				   int i, j;	// iterator
				   int	num;	// pushed number


				   for(i = 0; i < 4; i++) {

					   printk(KERN_NOTICE "i = %d\n",i);// HW intialization
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

					   printk(KERN_NOTICE "GPLEV = %x\n",GPLEV_0);// HW intialization
					   if(GPLEV_0 & 0x02000000){
						   if(i ==0) num = 1;
						   if(i ==1) num = 4;
						   if(i ==2) num = 7;
						   if(i ==3) num = 10;
						   printk("i:%d,j:%d,num:%d",i+1,1,num);
						   return 0;
					   }
					   if( GPLEV_0 & 0x04000000){
						   if(i ==0) num = 2;
						   if(i ==1) num = 5;
						   if(i ==2) num = 8;
						   if(i ==3) num = 0;
						   printk("i:%d,j:%d,num:%d",i+1,2,num);
						   return 0;
					   }
					   if( GPLEV_0 & 0x08000000){
						   if(i ==0) num = 3;
						   if(i ==1) num = 6;
						   if(i ==2) num = 9;
						   if(i ==3) num = 11;
						   printk("i:%d,j:%d,num:%d",i+1,3,num);
						   return 0; 
					   }
				   }
				   
				   break;
			   }
		
		default: return 0;
	}

	return 0;
}


struct file_operations doorlock_fops = {
					open			:	doorlock_open,
					unlocked_ioctl	:	doorlock_ioctl,   
					release			:	doorlock_release
};

static int __init doorlock_init(void)
{
	printk(KERN_ALERT "Hello this is doorlock's world\n" );

	MAJOR_NUMBER = register_chrdev(MAJOR_NUMBER, DEVICE_NAME, &doorlock_fops);
	printk(KERN_ALERT "Major number = %d\n", MAJOR_NUMBER);

	// mapping
	GPIO_BASE_VIR	= ioremap(GPIO_BASE_PHY,0x1000);
	GPFSEL2_VIR     = GPIO_BASE_VIR + 0x08;
	GPSET0_VIR		= GPIO_BASE_VIR + 0x1C;
	GPCLR0_VIR      = GPIO_BASE_VIR + 0x28;
	GPLEV0_VIR      = GPIO_BASE_VIR + 0x34;

	return 0;
}

static void __exit doorlock_exit(void)
{
	printk (KERN_ALERT "Goodbye ~\n" );
	iounmap(GPIO_BASE_VIR);
	unregister_chrdev(MAJOR_NUMBER, DEVICE_NAME);
}

module_init(doorlock_init);
module_exit(doorlock_exit);
