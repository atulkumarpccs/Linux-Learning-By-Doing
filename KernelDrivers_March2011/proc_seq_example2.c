//include appropriate headers
//
//

#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/proc_fs.h>
#include<linux/slab.h>
#include<linux/seq_file.h>


/* Private Data structure */
struct _mydrv_struct {
   /* ... */
   struct list_head list; /* Link to the next node */  //this is fixed for any type of struct
   char info[12];         /* Info to pass via the procfs file */ //these elements differ
                          //based on your module/driver
   /* ... */
};


static LIST_HEAD(mydrv_list);  /* List Head */
static struct proc_dir_entry *entry = NULL ; 


/* start() method */
//
//whatever index is requested, we send back the pointer of that object
//
//pos gives the starting element in your list 
//typically, it is 0
//*pos tells us where are we in the list access - 
//                                        the specific node in the list  

static void *
mydrv_seq_start(struct seq_file *seq, loff_t *pos)
{
  struct _mydrv_struct *p;
  loff_t off = 0;
  /* The iterator at the requested offset */
 
  dump_stack(); 

  list_for_each_entry(p, &mydrv_list, list) {
    if (*pos == off++) {
                        printk("in start : success %d\n",*pos);
                        return p;
    }
  }
  printk("in seq_start : over\n");
  return NULL;
}

//new code for assignment 
static void *
mydrv_seq_start(struct seq_file *seq, loff_t *pos)
{
  struct task_struct *p; //= kmalloc(sizeof(struct task_struct, GFP_KERNEL);
  loff_t off = 0;
  /* The iterator at the requested offset */
 
  dump_stack(); 

  //we are using a macro provided by the system to navigate the 
  //master list of pds and getting the ptr of the pd needed
  //as per the *pos - this is how we navigate pd list 
  //and get ptr to a specific pd for *pos

  //we are navigating the list to return ptr to an object 
  //whose position in the list is (*pos)

  //when we navigate the list, if we reach end of the list,
  //we must return NULL 

  for_each_process(p) {
    if (*pos == off++) {
                        printk("in start : success %d\n",*pos);
                        return p;
    }
  }
  printk("in seq_start : over\n");
  return NULL;
}
/* next() method */
//
//advance to the next position and return the pointer to that object
//
static void *
mydrv_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
  //here, our real objective is to increment the *pos 
  //and return ptr to the next object based on what is 
  //passed to us by system 

  //we still must check if the next element is the first element
  //and if so , we have reached end of list and we must 
  //return NULL  


  /* 'v' is a pointer to the iterator returned by start() or
     by the previous invocation of next() */
  struct list_head *n = ((struct _mydrv_struct *)v)->list.next;
  ++*pos; /* Advance position */
  
   /* Return the next iterator, which is the next node in the list */
   printk("in seq_next :%d\n",*pos);
   


  return(n != &mydrv_list) ?
          list_entry(n, struct _mydrv_struct, list) : NULL;
}


/* show() method */
static int
mydrv_seq_show(struct seq_file *seq, void *v)
{
   int ret;
   const struct task_struct *p =v;

   char *info_str = kmalloc(256,GFP_KERNEL);
   /* Interpret the iterator, 'v' */
   printk("in seq_show \n");
   
   //what ever information that you wish to generate on the fly, it 
   //must be generated and passed as below

   //using seq_printf() we can pass info. to user space 
   //

   //collect information from pd and pass it to system space
   //buffer using seq_printf
   sprintf(info_str, "pid is %d ppid is %d\n", p->pid, p->ppid);

   ret = seq_printf(seq,info_str);
   printk(KERN_INFO "the return value of seq_printf is %d\n", ret); 

   kfree(info_str); 
   return 0;
}


/* stop() method */
static void
mydrv_seq_stop(struct seq_file *seq, void *v)
{

   //stop() is normally used for freeing any resource allocated during
   //start() or show()
   /* No cleanup needed in this example */
   printk("in seq_stop:\n");
}


/* Define iterator operations */
static struct seq_operations mydrv_seq_ops = {
   .start = mydrv_seq_start,
   .next   = mydrv_seq_next,
   .stop   = mydrv_seq_stop,
   .show   = mydrv_seq_show,
};



static int
mydrv_seq_open(struct inode *inode, struct file *file)
{
   //printk writes the message in the kernel's log-buffer 
   //using dmesg, you can read from the kernel's log-buffer 
   printk("we are in mydrv_seq_open\n");   //1
   /* Register the operations */

    

   dump_stack();  //used for diagnostic messages  
   return seq_open(file, &mydrv_seq_ops);
}


static struct file_operations mydrv_proc_fops = {
   .owner    = THIS_MODULE,    //this macro will provide the ptr to our module object
   .open     = mydrv_seq_open, /* User supplied */  //passing addresses of functions 
                                                    //to function pointers
   .read     = seq_read,       /* Built-in helper function */
   .llseek   = seq_lseek,       /* Built-in helper function */
   .release  = seq_release,    /* Built-in helper funciton */
};


static int __init
mydrv_init(void)
{
   /* ... */

  int i;
  struct _mydrv_struct *mydrv_new;
  /* ... */
  /* Create /proc/readme */
  //this will create a proc file
  //name 
  //permissions
  //ptr to the parent directory 
  entry = create_proc_entry("readme", S_IRUSR, NULL);//a file is created 
  /* Attach it to readme_proc() */
  //check for error - NULL 
  if (entry) {
   /* Replace the assignment to entry->read_proc in proc_example1.c
      with a more fundamental assignment to entry->proc_fops. So
      instead of doing "entry->read_proc = readme_proc;", do the
      following: */

      //we are replacing an entry in the proc_dir_entry to our convenience

   entry->proc_fops = &mydrv_proc_fops; 
  }
  else 
  {
	return -EINVAL;
  }
  
  /* Handcraft mydrv_list for testing purpose.
     In the real world, device driver logic or kernel's  sub-systems 
     maintains the list and populates the 'info' field */
    for (i=0;i<100;i++) {
       mydrv_new = kmalloc(sizeof(struct _mydrv_struct), GFP_KERNEL);
    //check errors
    sprintf(mydrv_new->info, "Node No: %d\n", i);
    list_add_tail(&mydrv_new->list, &mydrv_list);
  }

  printk("we are in init function of the module\n");  //2
  return 0;
}

static void mydrv_exit(void)
{
  //incomplete
  struct _mydrv_struct *p,*n;
  list_for_each_entry_safe(p,n, &mydrv_list, list) 
      kfree(p);

   remove_proc_entry("readme", NULL);
   printk("mydrv_exit just executed\n");    //3

}

module_init(mydrv_init);
module_exit(mydrv_exit);


//add other macros as needed 


