#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/moduleparam.h>

#include <linux/sched/signal.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Mehmet Umut Boran & Mehmet Taha Bastug");

struct task_struct* traverse(struct task_struct*);
void traversePrint(struct task_struct*);

int pid;
static int oldestchild_init(void)
{
	
	struct task_struct* proc;
	printk(KERN_ALERT "Program started with input PID: %d ...\n",pid);
	proc = traverse(&init_task);
	if(proc != NULL)
	{
	  if(!list_empty(&proc->children))
	    traversePrint(proc);
	  else
	    printk(KERN_ALERT "Process %s doesn't have any subtrees!!",proc->comm);
	}
	else
	  printk(KERN_ALERT "Could not found process with PID %d\n",pid);
	return 0;
}

struct task_struct* traverse(struct task_struct* task){
		struct task_struct *child;
    		struct list_head *list;
		struct task_struct *ret = NULL;
		struct task_struct *temp;
    list_for_each(list, &task->children) {
        child = list_entry(list, struct task_struct, sibling);
	if(child->pid == pid) ret = child;
        else{ 
		temp = traverse(child);
		if(temp != NULL) ret = traverse(child);
	}
    }
	return ret;
}

void traversePrint(struct task_struct* task){
	int st = -99;
	struct task_struct *child;
	struct list_head *list;
	struct task_struct* lasts = NULL;
	int child_st;
	if(task == NULL || list_empty(&task->children))
	  return;
	list_for_each(list, &task->children) {
			child = list_entry(list, struct task_struct, sibling);
			child_st = child->start_time;
			if(st == -99) {
				st = child_st;
			}
			if(child_st <= st) {
				st = child_st;
				lasts = child;
			}	
			if(!list_empty(&child->children))
				traversePrint(child);
	}
	if(lasts != NULL){
		printk(KERN_ALERT "Oldest process in this subtree: %s, pid: [%d]\n", lasts->comm, lasts->pid);
	}else printk(KERN_ALERT "Error\n");
}

static void oldestchild_cleanup(void)
{
	printk(KERN_WARNING "Oldest finder finished!!!\n");
}

module_init(oldestchild_init);
module_exit(oldestchild_cleanup);
module_param(pid, int, 0);
