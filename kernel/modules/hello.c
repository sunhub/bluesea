#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

MODULE_LICENSE("Dual BSD/GPL");

static char *whom = "world";
static int howmany = 1;
module_param(whom, charp, 0);
module_param(howmany, int, 0);

static int hello_init(void)
{
    int i;
    for(i=0; i < howmany; ++i)
        printk(KERN_ALERT "(%d)Hello,%s!\n", i, whom);
    return 0;
}

static void hello_exit(void)
{
    printk(KERN_ALERT "Goodbye!\n");
}

module_init(hello_init);
module_exit(hello_exit);

