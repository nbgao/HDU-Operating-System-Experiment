#include <linux/sched.h>
#include <linux/init_task.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int func_init(void){
	struct task_struct *p;
	printk("Process name:\tPID:\tState:\tPriority:\n");
	for_each_process(p){
		if(p->mm == NULL){
			printk("%s\t%d\t%ld\t%d\n", p->comm, p->pid, p->state, p->prio);
		}
	}
	return 0;
}

static void func_exit(void){
	printk(KERN_ALERT"goodbye\n");
}

module_init(func_init);
module_exit(func_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gao Pengbing");
