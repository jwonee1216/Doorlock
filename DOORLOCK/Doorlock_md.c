/*
 * doorlock_dd.c - 도어락 디바이스 드라이버.
 *
 * 이 파일은 Raspberry PI 3의 디지털 제어핀을 사용하여 중고 도어락을 제어하기 위한 소스 코드이다.
 * 
 * 지원하는 기능은 다음과 같다.
 *  1. 4X3 키패드 입력 이벤트 처리.
 *  2. LED 점등/소등.
 *  3. 도어락 잠금/열림(모터 이용).
 *
 * 핀 구성은 BCM 기준으로 다음과 같다.
 *	출력 --> 21	[1] [2] [3]
 *			 22	[4] [5] [6] 
 *			 23	[7] [8] [9] 
 *			 24	[*]	[0] [#]
 *				20	26	27	<-- 입력 
 *
 *  키패드 행(출력)	: 21, 22, 23, 24
 *  키패드 열(입력)	: 20, 26, 27	
 *  키패드 LED	: 25(출력), physical 3(GND)
 *  도어락 모터	: { 5,  6, 12, 13}(출력),	--> 적은 전력량으로 인한 병렬 처리
 *				  {16, 17, 18, 19}(입력)
 *  잠금해제 버튼(도어락 뒷면) : 4
 */


#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/unistd.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <asm/io.h>

#define DEVICE_NAME "doorlock"
#define CLASS_NAME	"doorlock"
static int				major;		// 주 번호
static struct class		*class;	
static struct device	*device;	

#define GPIO_BASE_PHY	0x3F200000			// GPIO base address.
void	*GPIO_BASE_VIR,					
						*GPFSEL2_VIR,		// DOORLOCK SET 
						*GPFSEL1_VIR,		// MOTOR SET
						*GPFSEL0_VIR,			
						*GPSET0_VIR, 
						*GPCLR0_VIR, 
						*GPLEV0_VIR;

#define GPFSEL_2	( *(volatile unsigned long *)GPFSEL2_VIR )
#define GPFSEL_1	( *(volatile unsigned long *)GPFSEL1_VIR )
#define GPFSEL_0	( *(volatile unsigned long *)GPFSEL0_VIR )
#define GPSET_0		( *(volatile unsigned long *)GPSET0_VIR )
#define GPCLR_0		( *(volatile unsigned long *)GPCLR0_VIR )
#define GPLEV_0		( *(volatile unsigned long *)GPLEV0_VIR )

/* led	motor	in  out
	0	0		000 000 

#define OUT_1	0x1
#define OUT_2	0x2
#define OUT_3	0x3
#deinfe OUT_4	0x4

#define MOTOR	0x40
#define LED		0x80	

#define IN_1	0x8
#define IN_2	0x10
#define IN_3	0x20

unsigned char status = 0x0; */

#define NUM_OF_OUTS	4	
#define NUM_OF_INS	3

static char keypad[NUM_OF_OUTS][NUM_OF_INS] = { 
	{'1', '2', '3'},
	{'4', '5', '6'},
	{'7', '8', '9'},
	{'*', '0', '#'} 
};

static int outpins[NUM_OF_OUTS]	= {	21,	22,	23, 24};
static int inpins[NUM_OF_INS]	= {	20,	26,	27	};

#define UNLOCK_PIN	4
#define LED_PIN		25

#define BCM20_NUM	20		// BCM20(Key *)
#define BCM27_NUM	27		// BCM27(Key #)
#define BCM4_NUM	4		// BCM4 (Inside btn)

#define SET_GPIO(num) (GPSET_0 = 0x00000001 << num)
#define CLR_GPIO(num) (GPCLR_0 = 0x00000001 << num)
#define GET_GPIO(num) ( (GPLEV_0 >> num) & 1)

short int	bcm20_irqnum	= 0;
short int	bcm27_irqnum	= 0;
short int	bcm4_irqnum		= 0;

int			exeIrqNum	= 0;
static int	ev_press	= 0;
static		DECLARE_WAIT_QUEUE_HEAD(key_waitqueue);

/**
 * Free interrupt(BCM 20, 27, 4)
 */
static void interrupt_free(void){
	printk(KERN_NOTICE "Doorlock : interrupt free()\n");
	free_irq(bcm20_irqnum,	"BCM25IRQ");
	free_irq(bcm27_irqnum,	"BCM27IRQ");
	free_irq(bcm4_irqnum,	"BCM4IRQ");
}

/**
 * Interrupt handler 
 */
static irqreturn_t 
keypad_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	//interrupt handler should be short cause if long, it could make system error
	//local_irq_disable(irqNum), local_irq_enable(irqNum) are
	//normal funcs to control interrupt
	printk(KERN_NOTICE "Doorlock : irq_handler()\n irq = %d\n", irq);

	exeIrqNum	= irq;
	ev_press	= 1;

	CLR_GPIO(24);
	wake_up_interruptible(&key_waitqueue);

	return IRQ_HANDLED;	// Success
}


/**
  Initialize Interrupt(BCM 20, 27, 4)
 */
	static void 
interrupt_init(void)
{
	printk(KERN_NOTICE "interrupt_init()\n");
	if( (bcm20_irqnum = gpio_to_irq(BCM20_NUM) ) < 0) {
		printk("%s : GPIO to IRQ mapping failure:%s\n", __FUNCTION__, "BCM25");
		return; 
	}   

	if( (bcm27_irqnum = gpio_to_irq(BCM27_NUM) ) < 0) {
		printk("%s : GPIO to IRQ mapping failure:%s\n", __FUNCTION__, "BCM27");
		return;
	}   

	if( (bcm4_irqnum = gpio_to_irq(BCM4_NUM) ) < 0) {
		printk("%s : GPIO to IRQ mapping failure:%s\n", __FUNCTION__
				, "BCM4");
		return;
	}

	printk("%s : Mapped interrupt 20 : %d, 27 : %d, 4 : %d\n"
			, __FUNCTION__, bcm20_irqnum, bcm27_irqnum, bcm4_irqnum);

	if( request_irq(
				bcm20_irqnum , 
				(irq_handler_t)keypad_irq_handler, 
				//IRQF_TRIGGER_RISING , 
				IRQF_TRIGGER_RISING , 
				"BCM20 ITR", 
				"BCM20IRQ") ) {

		printk("%s : BCM20 irq Request failure\n", __FUNCTION__);
	}   

	if( request_irq(
				bcm27_irqnum, 
				(irq_handler_t)keypad_irq_handler,
				IRQF_TRIGGER_RISING , 
				"BCM27 ITR",
				"BCM27IRQ") ) {

		printk("%s : BCM27 irq Request failure\n", __FUNCTION__);
	}   

	if( request_irq(
				bcm4_irqnum ,
				(irq_handler_t)keypad_irq_handler,
				IRQF_TRIGGER_FALLING,
				"BCM4 ITR",
				"BCM4IRQ") ) {

		printk("%s : BCM4 irq Request failure\n", __FUNCTION__);
	}
}


/**
  Doorlock open
 */
static int 
doorlock_open(struct inode *inode, struct file *file)
{
	// init & set output or input 
	GPFSEL_2 = 0x9248;	// 001 001 001 001 001 000 To set output as BCM 21,22,23,24,25 input 20 26 27
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

	char flag;
	int ret = 0;
	// output only BCM 24
	CLR_GPIO(21);
	CLR_GPIO(22);
	CLR_GPIO(23);
	SET_GPIO(24);

	mdelay(5);
	exeIrqNum = 0;

	ev_press = 0;

	enable_irq(bcm20_irqnum);
	enable_irq(bcm27_irqnum);
	enable_irq(bcm4_irqnum);

	mdelay(5);

	wait_event_interruptible(key_waitqueue, ev_press);

	disable_irq(bcm20_irqnum);
	disable_irq(bcm27_irqnum);
	disable_irq(bcm4_irqnum);

	mdelay(5);

	if(exeIrqNum == bcm20_irqnum ){
		flag = '*';
		ret = copy_to_user(buf,&flag,count);
	}
	if( exeIrqNum == bcm27_irqnum ){
		flag = '#';
		ret = copy_to_user(buf,&flag,count);
	}
	if( exeIrqNum == bcm4_irqnum ){
		flag = 'B';
		ret = copy_to_user(buf,&flag,count);
	}
	printk(KERN_WARNING "KERNEL SEND DATA IS >> %c\n",flag);
	printk(KERN_WARNING "KERNEL RECEIVE COUNT IS >> %d\n",count);

	return(count);
}

enum COMMAND {
	KEYPAD = 0x00,
	LED	   = 0x01,
	MOTOR  = 0x03,
};

#define OPEN	0
#define CLOSE	1 
int motorpins[2][4] = { 
	[OPEN]	=	{ 5,  6, 18, 19},
	[CLOSE]	=	{12, 13, 16, 17} 
};

/**
 * ioctl
 */
static long 
doorlock_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int oi;	// 출력핀 핸들러.
	int ii; // 입력핀 핸들러.

	switch(cmd) {
		// 키패드 값 읽기.
		case KEYPAD:
			// 키보드 출력 핀을 초기화.
			for(oi = 0; oi < NUM_OF_OUTS; oi++) {
				CLR_GPIO( outpins[oi] );
			}
			mdelay(5);
			// 각 행을 순서대로 출력한다.
			// 출력된 행과 입력된 열에 위치한 키 값을 반환.	
			// 입력이 없을 시, 공백 문자 반환.
			for(oi = 0; oi < NUM_OF_OUTS; oi++) {
				SET_GPIO( outpins[oi] );
				mdelay(5);
			
				for(ii = 0; ii < NUM_OF_INS; ii++) {
					if( GET_GPIO( inpins[ii] ) ) {
						return keypad[oi][ii];
					}
				}
				CLR_GPIO( outpins[oi] );
			}
			return '\0';
		// 키패드 LED 제어.
		case LED:
			printk(KERN_NOTICE "ioctl() 2 open keypad led on");
			arg ? SET_GPIO(25) : CLR_GPIO(25);
			return arg;
		// 도어락 모터 제어.
		case MOTOR:
			//unlock door == reverse way

			printk(KERN_NOTICE "ioctl() 4 open motor on");
			GPFSEL_0 = 0x48000;	//To set output as BCM 5,6
			GPFSEL_1 = 0x240249; //To set output as BCM 12,13

			for(ii = 0; ii < NUM_OF_INS; ii++) {
				CLR_GPIO( motorpins[!arg][ii] );
			}

			for(oi = 0; oi < NUM_OF_OUTS; oi++) {
				SET_GPIO( motorpins[arg][oi] );
			}
			
			msleep(1000);
			
			for(oi = 0; oi < NUM_OF_OUTS; oi++) {
				CLR_GPIO( motorpins[arg][oi] );
			}
			return arg;

		default:
			return 0;
	}
}

struct file_operations fops = {
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
	printk(KERN_INFO "Doorlock : init()\n" );

	major = register_chrdev(0, DEVICE_NAME, &fops);
	if(major < 0) {
		printk(KERN_ALERT "Doorlock : Error --> failed to regitster a major number.\n");
		return major;
	} 
	printk(KERN_INFO "Doorlock : registered major number %d.\n", major);

	class = class_create(THIS_MODULE, CLASS_NAME);
	if( IS_ERR(class) ) {
		unregister_chrdev(major, DEVICE_NAME);
		printk(KERN_ALERT "failed to register device class.\n");
		return PTR_ERR(class);
	}

	device = device_create(class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
	if( IS_ERR(device) ) {
		class_destroy(class);
		unregister_chrdev(major, DEVICE_NAME);
		printk(KERN_ALERT "failed to create device class.\n");
		return PTR_ERR(device);
	}
/*
	MAJOR_NUMBER = register_chrdev(MAJOR_NUMBER, DEVICE_NAME, &doorlock_fops);
	printk(KERN_ALERT "Major number = %d\n", MAJOR_NUMBER);
*/
	// mapping
	GPIO_BASE_VIR	= ioremap(GPIO_BASE_PHY,0x1000);
	GPFSEL0_VIR     = GPIO_BASE_VIR + 0x00;
	GPFSEL1_VIR     = GPIO_BASE_VIR + 0x04; 
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
	printk (KERN_NOTICE "Doorlock : exit()\n");
	interrupt_free();
	iounmap(GPIO_BASE_VIR);

	device_destroy(class, MKDEV(major, 0) );
	class_unregister(class);
	class_destroy(class);

	unregister_chrdev(major, DEVICE_NAME);
}

module_init(doorlock_init);
module_exit(doorlock_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HomIT");
MODULE_DESCRIPTION("Doorlock device driver.");
MODULE_VERSION("1.0");
