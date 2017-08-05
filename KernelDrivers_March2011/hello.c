/*                                                     
 * $Id: hello.c,v 1.5 2004/10/26 03:32:21 corbet Exp $ 
 */                                                    
//these headers are from the kernel - source tree ??
//
#include <linux/init.h>
#include <linux/module.h>

//you may declare the global variables 
//the memory is allocated using non-contiguous memory allocator
//of the kernel
 static int new_symbol = 5; //this will allocated in the data region of the module 
 char *str1 = NULL;
 int *var1 = NULL;

//the routine that will be executed once when the module is added
//to the kernel space-during module loading

static int hello_init(void)
{

        int local_var1 =0;//this will be allocated on the system stack 

        //here, trivial printks are used
        //ideally, some tangible coding will be added 
	printk(KERN_ALERT "Hello, world\n");
      
        //printk(KERN_ALERT "var1 is %d\n", *var1); 
	printk(KERN_ALERT "Hello, world\n");
	printk(KERN_ALERT "Hello, world\n");
	printk(KERN_ALERT "Hello, world\n");
	printk(KERN_ALERT "Hello, world\n");
        printk(KERN_ALERT "string is %s\n", str1); 
	return 0;
}


//this routine is called only once when the module is unloaded 
//
//
static void hello_exit(void)
{
        //any local variable will allocated on the kernel stack 

        //this may be used to free some of the dynamically allocated
        //resources during the init() function above

	printk(KERN_ALERT "Goodbye, cruel world\n");
}

//refer to page nos 31-32
//module_xxx() is mandatory 
//these macros add special info. in a special section of the module's
//object file

module_init(hello_init);
module_exit(hello_exit);

//you may export symbols from a module - like you do in the kernel 
//symbols exported from a module are stored in the module's symbol 
//table in the kernel-space


//EXPORT_SYMBOL_GPL(new_symbol);
EXPORT_SYMBOL(new_symbol);

//MODULE_LICENSE("Dual BSD/GPL");
MODULE_LICENSE("GPL");











