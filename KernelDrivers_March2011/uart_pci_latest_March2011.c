#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/pci.h>
#include<linux/fs.h>
#include<linux/string.h>
#include<linux/ioport.h>
#include<linux/module.h>
#include<linux/list.h>
#include<linux/wait.h>
#include<linux/kfifo.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<linux/interrupt.h>
#include<linux/types.h>
#include<linux/errno.h>
#include<linux/slab.h>
#include<linux/sched.h>
#include<linux/kdev_t.h>
#include<linux/version.h>
#include<asm/uaccess.h>
#include<asm/unistd.h>
#include<asm/fcntl.h>
#include<asm/io.h>
#include<asm/irq.h>
#include<asm/atomic.h>
#include<asm/errno.h>

#define VENDOR_ID 0x5372
#define DEVICE_ID 0x6872
#define PORTS 2
#define MAX_BUFFER 2048
#define DRIVER_NAME "PCI_DUAL_UART"

typedef struct pci_dev_str
{
	struct list_head list;
	struct cdev cdev;
	unsigned int base_addr;
	unsigned char* name;
	unsigned int irq_num;
	spinlock_t lock1;
	spinlock_t lock2;
	struct kfifo *write_kfifo;
	struct kfifo *read_kfifo;
	unsigned char *rd_buff;
	unsigned char *wr_buff;
	wait_queue_head_t read_queue;
	wait_queue_head_t write_queue;
	unsigned int read_flag;
	unsigned int write_flag;
	atomic_t r_count,rw_count,w_count;
}P_DEV;

static struct pci_device_id dev_ids[] = {
	{PCI_DEVICE(VENDOR_ID,DEVICE_ID)},
	{0}
};

static int __init init_pci(void);
static void __exit exit_pci(void);
static int __devinit pci_probe(struct pci_dev *dev,struct pci_device_id *ids);
static void __devexit pci_remove(struct pci_dev *dev);
void device_diagnos_config(unsigned int);
void device_nondiagnos_config(unsigned int);
int device_diagnos_start(unsigned int);
int pci_dev_init(P_DEV *dev,unsigned addr,char *name);

static int uart_open(struct inode *inode,struct file *file);
static int uart_release(struct inode *inode,struct file *file);
static ssize_t uart_read(struct file *file,char __user *buf,size_t count,loff_t *pos);
static ssize_t uart_write(struct file *file,char __user *buf,size_t count,loff_t *pos);


static struct pci_driver driver = {
	.name     = DRIVER_NAME,
	.id_table = dev_ids,
	.probe    = pci_probe,
	.remove   = pci_remove,
};


static struct file_operations uart_fops = {
	.open = uart_open,
	.read = uart_read,
	.write = uart_write,
	.release = uart_release,
	.owner = THIS_MODULE,
};



P_DEV my_dev_1;
P_DEV my_dev_2;
dev_t uart_dev;
static struct pci_dev *pdev;
static unsigned int pci_bar[2];
unsigned int irq_line;

void device_diagnos_config(unsigned int addr)
{
	printk("device configurating.....\n");
	outb(0x80 , addr + 3);		//LCR reg : DLAB set
	outb(0x00 , addr + 1);		//DLM reg : baud rate higher byte
	outb(0x0c , addr + 0);		//DLL reg : baud rate lower byte
	outb(0x03 , addr + 3);		//LCR reg : no parity , 1 stop bit
	outb(0xc7 , addr + 2);		//FCR reg : fifo enabling
	outb(0x00 , addr + 1);		//IER reg : disabling interrupt
	outb(0x10 , addr + 4);		//MCR reg : loopback mode enabled
}

void device_nondiagnos_config(unsigned int addr)
{
	outb(0x80 , addr + 3);		//LCR reg : DLAB set
	outb(0x00 , addr + 1);		//DLM reg : baud rate higher byte
	outb(0x0c , addr + 0);		//DLL reg : baud rate lower byte
	outb(0x03 , addr + 3);		//LCR reg : no parity , 1 stop bit
	outb(0xc7 , addr + 2);		//FCR reg : fifo enabling
	outb(0x03 , addr + 1);		//IER reg : disabling interrupt
	outb(0x08 , addr + 4);		//MCR reg : normal mode enabled
}

int device_diagnos_start(unsigned int addr)
{
	int i=0,w=0,r=0;
	unsigned char a;
	printk("Diagnos starting......\n");
	while(i < 1)
	{
		a = inb(addr + 5);
		if((a=a & 0x20))
		{
			printk("Diagnos writing : %x\n",a);
			while(w < 16)
			{
				outb(0xff,addr);
				w++;
			}
		}
		else
		{
			msleep(50);
			continue;
		}

		while(r < 16)
		{
			char ch;
			a =inb(addr + 5);
			ch = a;
			if((a = a & 0x01))
			{
				a = inb(addr);
				printk("Diagnosis reading : %x\n",a);
				if(ch & 0x0e)
				{
					return -1;
				}
				r++;
			}
			else
			{
				msleep(50);
			}
		}
		i++;
	}
	printk("device diagnosis completed\n");
	return 0;
}




static int uart_open(struct inode *inode,struct file *file)
{
	int ret;
	P_DEV *dev;
	printk("Inside open....\n");
	dev = container_of(inode->i_cdev,P_DEV,cdev);
	file->private_data = dev;

	if(file->f_flags & O_RDWR)
	{
		if((atomic_read(&dev->rw_count)==0)&&(atomic_read(&dev->r_count)==0)&&(atomic_read(&dev->w_count)==0))
		{
			atomic_inc(&dev->rw_count);
			atomic_inc(&dev->r_count);
			atomic_inc(&dev->w_count);
			return 0;
		}
		else
		{
			printk("Device already opened\n");
			return -EBUSY;
		}
	}
	
	if(file->f_flags & O_RDONLY)
	{
		if((atomic_read(&dev->r_count)==0))
		{
			atomic_inc(&dev->r_count);
			return 0;
		}
		else
		{
			printk("Device already opened\n");
			return -EBUSY;
		}
	}
	if(file->f_flags & O_WRONLY)
	{
		if((atomic_read(&dev->w_count)==0))
		{
			atomic_inc(&dev->w_count);
			return 0;
		}
		else
		{
			printk("Device already opened\n");
			return -EBUSY;
		}
	}
}

static int uart_release(struct inode *inode,struct file *file)
{
	P_DEV *dev;
	file->private_data = dev;
	printk("In uart release\n");
	if(file->f_flags & O_RDWR)
	{
		atomic_dec(&dev->rw_count);
		atomic_dec(&dev->r_count);
		atomic_dec(&dev->w_count);
		return 0;
	}
	if(file->f_flags & O_RDONLY)
	{
		atomic_dec(&dev->r_count);
		return 0;
	}
	if(file->f_flags & O_WRONLY)
	{
		atomic_dec(&dev->W_count);
		return 0;
	}

}


static ssize_t uart_read(struct file *file,char __user *buf,size_t count,loff_t *pos)
{
}


static ssize_t uart_write(struct file *file,char __user *buf,size_t count,loff_t *pos)
{
}



int pci_dev_init(P_DEV *dev,unsigned addr,char *name)
{
	dev -> base_addr = addr;
	dev -> name = name;
	dev -> irq_num = irq_line;
	atomic_set(&dev->r_count,0);
	atomic_set(&dev->rw_count,0);
	atomic_set(&dev->w_count,0);
	printk("Initializing wait q \n");
	init_waitqueue_head(&(dev -> read_queue));
	init_waitqueue_head(&(dev -> write_queue));
	dev -> read_flag = 0;
	dev -> write_flag = 0;
	printk("Allcoacting write & read buffer\n");
	dev -> wr_buff = kmalloc(MAX_BUFFER,GFP_KERNEL);
	if(dev -> wr_buff == NULL)
	{
		printk("Error in allocating write buffer\n");
		return -1;
	}
	dev -> rd_buff = kmalloc(MAX_BUFFER,GFP_KERNEL);
	if(dev -> rd_buff == NULL)
	{
		printk("Error in allocating read buffer\n");
		kfree(dev->wr_buff);
		return -1;
	}
	printk("Allcoacting write & read kfifo\n");
	dev->write_kfifo = kfifo_init(dev->wr_buff,MAX_BUFFER,GFP_KERNEL,NULL);
	if(dev->write_kfifo==NULL)
	{
		printk("Error in allocating write kfifo\n");
		kfree(dev->wr_buff);
		kfree(dev->rd_buff);
		return -1;
	}
	dev->read_kfifo = kfifo_init(dev->rd_buff,MAX_BUFFER,GFP_KERNEL,NULL);
	if(dev->read_kfifo==NULL)
	{
		printk("Error in allocating read kfifo\n");
		kfifo_free(dev->write_kfifo);
		kfree(dev->rd_buff);
		return -1;
	}


	return 0;
}

static int __init init_pci(void)
{
	unsigned long ret;
	printk("Inside init of pci\n");
	ret = pci_register_driver(&driver);
	if(ret < 0)
	{
		printk("Unable to register pci driver \n");
		return ret;
	}

	return 0;
}

static int __devinit pci_probe(struct pci_dev *dev,struct pci_device_id *ids)
{
	int ret,i;
	pdev = dev;
	printk("Inside probe function\n");
	if(pdev==NULL)
	{
		printk("Device is not accessible\n");
		pci_unregister_driver(&driver);
		return -ENODEV;
	}
	printk("Enabling device \n");
	ret = pci_enable_device(pdev);
	if(ret < 0)
	{
		printk("device can not be enabled\n");
		pci_unregister_driver(&driver);
		return -1;
	}
	printk("Allcoating resource for uart 1\n");
	ret = pci_request_region(pdev,0,"dual_uart_1");
	if(ret < 0)
	{
		printk("unable to reserve resource uart 1\n");
		pci_disable_device(pdev);
		pci_unregister_driver(&driver);
		return -ENOMEM;
	}
	printk("Allcoating resource for uart 2\n");
	ret = pci_request_region(pdev,1,"dual_uart_2");
	if(ret < 0)
	{
		printk("unable to reserve resource uart 2\n");
		pci_disable_device(pdev);
		pci_unregister_driver(&driver);
		return -ENOMEM;
	}
	pci_bar[0] = pci_resource_start(pdev,0);
	printk("Uart 1 start address : %x \n",pci_bar[0]);
	pci_bar[1] = pci_resource_start(pdev,1);
	printk("Uart 2 start address : %x \n",pci_bar[1]);
	
	for(i=0;i<2;i++)
	{
		device_diagnos_config(pci_bar[i]);
		ret = device_diagnos_start(pci_bar[i]);
		if(ret < 0)
		{
			printk("H/W is not working properly\n");
			pci_release_region(pdev,0);
			pci_release_region(pdev,1);
			pci_disable_device(pdev);
			pci_unregister_driver(&driver);
			return -ENODEV;
		}
	}
	printk("Allcating device ids\n");
	ret = alloc_chrdev_region(&uart_dev,0,PORTS,DRIVER_NAME);
	if(ret < 0)
	{
		printk("unable to allocate device region\n");
		pci_release_region(pdev,0);
		pci_release_region(pdev,1);
		pci_disable_device(pdev);
		pci_unregister_driver(&driver);
		return -ENOMEM;
	}

	ret = pci_dev_init(&my_dev_1,pci_bar[0],"uart1");	
	if(ret < 0)
	{
		printk("Error in device intialization \n");
		unregister_chrdev_region(uart_dev,2);
		pci_release_region(pdev,0);
		pci_release_region(pdev,1);
		pci_disable_device(pdev);
		pci_unregister_driver(&driver);
		return -ENOMEM;
	}
	ret = pci_dev_init(&my_dev_2,pci_bar[1],"uart2");	
	if(ret < 0)
	{
		printk("Error in device intialization \n");
		unregister_chrdev_region(uart_dev,2);
		pci_release_region(pdev,0);
		pci_release_region(pdev,1);
		pci_disable_device(pdev);
		pci_unregister_driver(&driver);
		return -ENOMEM;
	}
	
	device_nondiagnos_config(pci_bar[0]);
	device_nondiagnos_config(pci_bar[1]);
	
	ret = request_irq(irq_line,intr_handler,SA_INTERRUPT|SA_SHIRQ,"uart1",(void*)&my_dev_1);
	if(ret < 0)
	{
		printk("Unable to register interrupt line to device\n");
		unregister_chrdev_region(uart_dev,2);
		pci_release_region(pdev,0);
		pci_release_region(pdev,1);
		pci_disable_device(pdev);
		pci_unregister_driver(&driver);
		return -EBUSY;
	}
	
	ret = request_irq(irq_line,intr_handler,SA_INTERRUPT|SA_SHIRQ,"uart2",(void*)&my_dev_2);
	if(ret < 0)
	{
		printk("Unable to register interrupt line to device\n");
		free_irq(irq_line,&my_dev_1);
		unregister_chrdev_region(uart_dev,2);
		pci_release_region(pdev,0);
		pci_release_region(pdev,1);
		pci_disable_device(pdev);
		pci_unregister_driver(&driver);
		return -EBUSY;
	}

	printk("cdev initialising & registering.......\n");
	cdev_init(&my_dev_1.cdev,&uart_fops);
	my_dev_1.cdev.owner = THIS_MODULE;
	if(cdev_add(&my_dev_1,uart_dev,1)< 0)
	{
		printk("Error in cdev registration of uart 1 \n");
		free_irq(irq_line,&my_dev_1);
		free_irq(irq_line,&my_dev_2);
		unregister_chrdev_region(uart_dev,2);
		pci_release_region(pdev,0);
		pci_release_region(pdev,1);
		pci_disable_device(pdev);
		pci_unregister_driver(&driver);
		return -EBUSY;
	}
	
	cdev_init(&my_dev_2.cdev,&uart_fops);
	my_dev_2.cdev.owner = THIS_MODULE;
	if(cdev_add(&my_dev_2,uart_dev+1,1)< 0)
	{
		printk("Error in cdev registration  of uart 2\n");
		free_irq(irq_line,&my_dev_1);
		free_irq(irq_line,&my_dev_2);
		unregister_chrdev_region(uart_dev,2);
		cdev_del(&my_dev_1.cdev);
		pci_release_region(pdev,0);
		pci_release_region(pdev,1);
		pci_disable_device(pdev);
		pci_unregister_driver(&driver);
		return -EBUSY;
	}
	return 1;
}

static void __exit exit_pci(void)
{
	pci_unregister_driver(&driver);
	printk("Exiting driver\n");
}

static void __devexit pci_remove(struct pci_dev *dev)
{
	free_irq(irq_line,&my_dev_1);
	free_irq(irq_line,&my_dev_2);
	unregister_chrdev_region(uart_dev,2);
	cdev_del(&my_dev_1.cdev);
	cdev_del(&my_dev_2.cdev);
	pci_release_region(pdev,0);
	pci_release_region(pdev,1);
	pci_disable_device(pdev);
	printk("Exiting pci_remove\n");
}

MODULE_AUTHOR("THIS_MODULE");
MODULE_LICENSE("GPL");

module_init(init_pci);
module_exit(exit_pci);


