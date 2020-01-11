#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/ktime.h>

static struct hrtimer htimer;
static ktime_t kt_periode;
static unsigned int count = 0;
static enum hrtimer_restart timer_function(struct hrtimer * timer)
{
    // @Do your work here.
    printk(KERN_INFO "Timer Callback function Called [%d]\n",count++);
    hrtimer_forward_now(timer, kt_periode);

    return HRTIMER_RESTART;
}

static int __init timer_init(void)
{
    kt_periode = ktime_set(0, 104167); //seconds,nanoseconds
    hrtimer_init (& htimer, CLOCK_REALTIME, HRTIMER_MODE_REL);
    htimer.function = &timer_function;
    hrtimer_start(& htimer, kt_periode, HRTIMER_MODE_REL);
    return 0;
}

static void timer_cleanup(void)
{
    hrtimer_cancel(& htimer);
}
#if 0
static enum hrtimer_restart timer_function(struct hrtimer * timer)
{
    // @Do your work here. 
    printk(KERN_INFO "Timer Callback function Called [%d]\n",count++);
    hrtimer_forward_now(timer, kt_periode);

    return HRTIMER_RESTART;
}
#endif
module_init(timer_init);
module_exit(timer_cleanup);
