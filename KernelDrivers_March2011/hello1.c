/*                                                     
 * $Id: hello.c,v 1.5 2004/10/26 03:32:21 corbet Exp $ 
 */                                                    
#include <linux/init.h>
#include <linux/module.h>

//int new_symbol;
extern int new_symbol;
int  new_symbol1 =5;
int *var1 = NULL;

static int __init hello_init(void)
{
	printk(KERN_ALERT "Hello, world1\n");
        new_symbol1 = new_symbol;
        return 0;
}

static void __exit hello_exit(void)
{
          
        *var1 = 0x55aa;
	printk(KERN_ALERT "Goodbye, cruel world1\n");
}

module_init(hello_init);
module_exit(hello_exit);


MODULE_LICENSE("GPL");
