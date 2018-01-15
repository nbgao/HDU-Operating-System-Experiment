#include <linux/sched.h>
#include <linux/init_task.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/list.h>
#include <linux/pid.h>

static int pid = 1;
module_param(pid, int, S_IRUGO);

static int func_init(void){
	struct task_struct *t1,*t2,*t3,*t4;
	struct list_head *list;

	t1 = pid_task(find_get_pid(pid), PIDTYPE_PID);
	if(t1){
		// 输出该进程自身信息
		printk("\nOrigin process\ncomm: %s\tpid: %d\tstate: %ld\n", t1->comm, t1->pid, t1->state);
		
		t2 = t1->parent;
		// 输出该进程的父进程信息
		printk("\nParent process\ncomm: %s\tpid: %d\tstate: %ld\n", t2->comm, t2->pid, t2->state);
		
		list_for_each(list, &t1->children){
			t3 = list_entry(list, struct task_struct, sibling);
			// 输出该进程的子进程信息
			printk("\nChild process\ncomm: %s\tpid: %d\tstate: %ld\n", t3->comm, t3->pid, t3->state);
		}

		list_for_each(list, &t2->children){
			t4 = list_entry(list, struct task_struct, sibling);
			// 输出该进程的兄弟进程信息
			printk("\nBrother process\ncomm: %s\tpid: %d\tstate: %ld\n", t4->comm, t4->pid, t4->state);
		}
		
		/*
		list_for_each_entry(t2, &t1->real_parent->children, sibling);
		if(t2)
			printk("\nSibling process\ncomm: %s\tpid: %d\tstate: %ld\n", t2->comm, t2->pid, t2->state  );
		
		list_for_each_entry(t2, &t1->children, sibling);
		if(t2)
			printk("\nChild process\ncomm: %s\tpid: %d\tstate: %ld\n", t2->comm, t2->pid, t2->state);
		*/
		
	}else{
		printk("Can't find the process %d !\n", pid); 
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
