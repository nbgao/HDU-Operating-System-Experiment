#include <linux/fs.h>       //定义文件表结构（ file 结构,buffer_head,m_inode 等）
#include <linux/types.h>    //对一些特殊的系统数据类型的定义，例如 dev_t, off_t, pid_t
#include <linux/cdev.h>     //包含了 cdev 结构及相关函数的定义。
#include <linux/uaccess.h>    //包含 copy_to_user(),copy_from_user()的定义
#include <linux/module.h>   //模块编程相关函数
#include <linux/init.h>     //模块编程相关函数
#include <linux/kernel.h>   //包含了printk()函数
#include <linux/slab.h>     //包含内核的内存分配相关函数，如 kmalloc()/kfree()等

#define SIZE 512

static int major;
static int minor = 0;
static dev_t devnum;

struct mymem_dev
{
    struct cdev cdev;
    unsigned char mem[SIZE];
};

static struct mymem_dev mycdev;

static int char_dev_open(struct inode *node, struct file *flip)
{
    struct mymem_dev *d;
    d = list_entry(node->i_cdev, struct mymem_dev, cdev);
    flip->private_data = d;
    printk("成功打开设备文件！\n");
    return 0;
}

static ssize_t char_dev_read(struct file *file, char __user *buff, size_t count, loff_t *offp)
{
    struct mymem_dev *d = file->private_data;
    char *kernel_buff;
    kernel_buff = d->mem;
    if(copy_to_user(buff, kernel_buff, count) != 0)
    {
        printk("读取内核态数据失败！\n");
    }else{
        printk("设备文件成功读取：%s\n", kernel_buff);
    }
    return count;
}

static ssize_t char_dev_write(struct file *file, const char __user *buff, size_t count, loff_t *offp)
{
    struct mymem_dev *d = file->private_data;
    char *kernel_buff;
    kernel_buff = d->mem;
    if(count > SIZE)
    {
        printk("写入字节数超出上限！\n");
    }
    if(copy_from_user(kernel_buff, buff, count) != 0)
    {
        printk("写入内核态数据失败！\n");
    }else{
        printk("设备文件成功写入字符串：%s\n", kernel_buff);
    }
    return count;
}

static long char_dev_ioctl(struct file *flip, unsigned int cmd, unsigned long arg)
{
    struct mymem_dev *d = flip->private_data;
    switch(cmd)
    {
        case 2017:
            printk("主设备号：%d\n", major);
            break;
        case 2018:
            printk("次设备号：%d\n", minor);
            break;
        case 2019:
            printk("清空设备空间\n");
            memset(d->mem, 0, sizeof(d->mem));
            break;
        default:
            return -EFAULT;
            break;
    }
    return 0;
}

static int char_dev_release(struct inode *node, struct file *flip)
{
    printk("关闭设备文件成功！\n");
    return 0;
}

static struct file_operations char_dev_fops=
{
    .owner = THIS_MODULE,
    .open = char_dev_open,
    .release = char_dev_release,
    .read = char_dev_read,
    .write = char_dev_write,
    .unlocked_ioctl = char_dev_ioctl,
};


static int Module_init(void)
{
    printk("模块初始化\n");
    devnum = MKDEV(major, minor);

    if(major)   // 静态分配设备号
    {
        if(register_chrdev_region(devnum, 1, "char_dev_module") < 0)
        {
            printk("注册静态设备号失败！\n");
        }else{
            printk("注册静态设备号成功！\n");
            printk("静态分配主主设备号: %d\n", major);
        }
    }else{      // 动态分配设备号
        if(alloc_chrdev_region(&devnum, 0, 1, "char_dev_module") < 0)
        {
            printk("注册动态设备号失败！\n");
        }else{
            major = MAJOR(devnum);
            printk("注册动态设备号成功！\n");
            printk("动态分配主设备号: %d\n", major);
        }
    }

    // Init cdev
    cdev_init(&mycdev.cdev, &char_dev_fops);
    // Add cdev;
    if(cdev_add(&mycdev.cdev, devnum, 1) < 0)
    {
        printk("添加驱动设备失败！\n");
    }else{
        printk("添加驱动设备成功！\n");
    }
    return 0;
}

static void Module_exit(void)
{
    cdev_del(&mycdev.cdev);
    unregister_chrdev_region(devnum,1);
    printk("模块已经卸载！\n\n");
}

module_param(major, int, S_IRUGO);
module_init(Module_init);
module_exit(Module_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gao Pengbing");
