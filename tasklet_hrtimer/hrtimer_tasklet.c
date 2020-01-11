#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/hardirq.h>
#include <linux/slab.h>
#include <linux/ktime.h>
#include <linux/timer.h> 
//Timer Variable
#define TIMEOUT   10000 * 1000000L  //nano seconds
static struct hrtimer etx_hr_timer;
static unsigned int count = 0;
 
dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;
 
static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);
 
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = etx_read,
        .write          = etx_write,
        .open           = etx_open,
        .release        = etx_release,
};
struct cdc_ncm_ctx {
     //   struct hrtimer tx_timer;
	struct tasklet_hrtimer  mtimer;
	struct timer_list       rtimer;
     	struct tasklet_struct bh;
};
void cdc_ncm_txpath_bh(unsigned long param)
{

        printk("Execting the txpath_bh\n");

}

void replay_timer_handler(struct timer_list *t)
{

	printk("Executing the reply timer handler\n");

}
//Timer Callback function. This will be called when timer expires
enum hrtimer_restart timer_callback(struct hrtimer *timer)
{
	printk(KERN_INFO "Timer Callback function Called [%d]\n",count++);
	struct cdc_ncm_ctx *ctx = container_of(timer, struct cdc_ncm_ctx, mtimer);
	tasklet_schedule(&ctx->bh);
	hrtimer_forward_now(timer,ktime_set(0,TIMEOUT));
        return HRTIMER_RESTART;
}
 
static int etx_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device File Opened...!!!\n");
    return 0;
}
 
static int etx_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device File Closed...!!!\n");
    return 0;
}
 
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "Read Function\n");
    return 0;
}
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "Write function\n");
    return 0;
}
 
static int __init etx_driver_init(void)
{
     ktime_t ktime;
    
    /*Allocating Major number*/
    if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0){
            printk(KERN_INFO "Cannot allocate major number\n");
            return -1;
    }
    printk(KERN_INFO "Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
    /*Creating cdev structure*/
    cdev_init(&etx_cdev,&fops);
 
    /*Adding character device to the system*/
    if((cdev_add(&etx_cdev,dev,1)) < 0){
        printk(KERN_INFO "Cannot add the device to the system\n");
        goto r_class;
    }
 
    /*Creating struct class*/
    if((dev_class = class_create(THIS_MODULE,"etx_class")) == NULL){
        printk(KERN_INFO "Cannot create the struct class\n");
        goto r_class;
    }
 
    /*Creating device*/
    if((device_create(dev_class,NULL,dev,NULL,"etx_device")) == NULL){
        printk(KERN_INFO "Cannot create the Device 1\n");
        goto r_device;
    }
   struct cdc_ncm_ctx *ctx;
   ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
   if (!ctx)
   	return -ENOMEM;
 
    ktime = ktime_set(0, TIMEOUT);
    tasklet_hrtimer_init(&ctx->mtimer, timer_callback, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
 //   timer_setup(&ctx->mtimer, replay_timer_handler, 0);
	tasklet_init(&ctx->bh, cdc_ncm_txpath_bh, (unsigned long)dev); 
    tasklet_hrtimer_start(&ctx->mtimer, ktime_set (0, TIMEOUT), HRTIMER_MODE_REL);
    //hrtimer_start(&ctx->tx_timer,ktime_set(0, TIMEOUT), HRTIMER_MODE_REL); 
    printk(KERN_INFO "Device Driver Insert...Done!!!\n");
    return 0;
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
    return -1;
}
 
void __exit etx_driver_exit(void)
{
    //stop the timer
    hrtimer_cancel(&etx_hr_timer);
    device_destroy(dev_class,dev); 
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "Device Driver Remove...Done!!!\n");
}
 
module_init(etx_driver_init);
module_exit(etx_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com>");
MODULE_DESCRIPTION("A simple device driver - High Resolution Timer");
MODULE_VERSION("1.22");
