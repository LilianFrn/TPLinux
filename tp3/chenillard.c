#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/timer.h>

/*
 * Direction et Vitesse fonctionneles côté Kernel et Client
 * Manque la gestion du pattern
 */

// Prototypes
static int leds_probe(struct platform_device *pdev);
static int leds_remove(struct platform_device *pdev);
static ssize_t leds_read(struct file *file, char *buffer, size_t len, loff_t *offset);
static ssize_t leds_write(struct file *file, const char *buffer, size_t len, loff_t *offset);
static ssize_t proc_dir_read(struct file *file, char *buffer, size_t len, loff_t *offset);
static ssize_t proc_dir_write(struct file *file, const char *buffer, size_t len, loff_t *offset);
static ssize_t proc_speed_read(struct file *file, char *buffer, size_t len, loff_t *offset);
static char loc_buf[32];
static uint8_t param_vit = 0;
static uint8_t param_dir = 0;
static uint8_t param_pat = 0x01;

struct platform_device * pdev2;

// File Operations_________________________________________________________________
static const struct file_operations speed_fops = {
    .owner=THIS_MODULE,
    .read = proc_speed_read,
};


static const struct file_operations dir_fops = {
    .owner=THIS_MODULE,
    .read = proc_dir_read,
    .write = proc_dir_write,
};

// Unused pour l'instant
static const struct file_operations ensea_leds_fops = {
    .owner = THIS_MODULE,
//    .read = leds_read,
//    .write = leds_write,
};

//___________________________________________________________________________


// An instance of this structure will be created for every ensea_led IP in the system
struct ensea_leds_dev {
    struct miscdevice miscdev;
    void __iomem *regs;
    u8 leds_value;
};

// Délai non utilisé à présent
#define TIM1_INT		200
static struct timer_list tim1;
static int led_c = 0x01;

// Ce callback gère un chenillard avec le pattern de base d'une LED et change
// En fonction de la valeur de dir
static void tim1_callback(unsigned long data) {
    // Récupération de la strcutre dev par un cast
    struct ensea_leds_dev *dev = (struct ensea_led_dev*) platform_get_drvdata(pdev2);
	mod_timer(&tim1, jiffies + param_vit);

    // Boucle de sélection de la LED à allumer
    if ((int)param_dir == 0){
        if (led_c <= 0x80) {
		    led_c = led_c << 1;
	    }
	    else {
		    led_c = 0x01;
	    }
    }

    if ((int)param_dir == 1){
        if (led_c >= 0x00) {
		    led_c = led_c >> 1;
	    }
	    else {
		    led_c = 0x80;
	    }
    }
	
    dev->leds_value = led_c;
    // Ecriture de la valeur des LEDs
    iowrite32(dev->leds_value, dev->regs);
}

// Specify which device tree devices this driver supports
static struct of_device_id ensea_leds_dt_ids[] = {
    {
        .compatible = "dev,ensea"
    },
    { /* end of table */ }
};

// Inform the kernel about the devices this driver supports
MODULE_DEVICE_TABLE(of, ensea_leds_dt_ids);

// Data structure that links the probe and remove functions with our driver
static struct platform_driver leds_platform = {
    .probe = leds_probe,
    .remove = leds_remove,
    .driver = {
        .name = "Ensea LEDs Driver",
        .owner = THIS_MODULE,
        .of_match_table = ensea_leds_dt_ids
    }
};

struct proc_dir_entry *ensea = 0;
struct proc_dir_entry *child = 0;
struct proc_dir_entry *childbis = 0;

// Called when the driver is installed
static int leds_init(void)
{
    int ret_val = 0;
    pr_info("Initializing the Ensea LEDs module\n");

    // Register our driver with the "Platform Driver" bus
    ret_val = platform_driver_register(&leds_platform);
    if(ret_val != 0) {
        pr_err("platform_driver_register returned %d\n", ret_val);
        return ret_val;
    }

    pr_info("Ensea LEDs module successfully initialized!\n");
    printk(KERN_DEBUG "la vitesse de balayage est %d\n ",param_vit);

// Création de l'arborescence_________________________________________________________________
    printk(KERN_DEBUG "on cree les fichiers\n");
    // creation des deux fichier dans proc pour la direction et la vitesse
    ensea = proc_mkdir("ensea", NULL);
    child = proc_create("speed", 0, ensea, &speed_fops);
    childbis = proc_create("dir", 0, ensea, &dir_fops);
    printk(KERN_DEBUG "FICHIERS CREES\n");
//__________________________________________________________________________
    return 0;
}

// Called whenever the kernel finds a new device that our driver can handle
// (In our case, this should only get called for the one instantiation of the Ensea LEDs module)
static int leds_probe(struct platform_device *pdev)
{
    int ret_val = -EBUSY;
    struct ensea_leds_dev *dev;
    struct resource *r = 0;

    pr_info("leds_probe enter\n");

    // Get the memory resources for this LED device
    r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if(r == NULL) {
        pr_err("IORESOURCE_MEM (register space) does not exist\n");
        goto bad_exit_return;
    }

    // Create structure to hold device-specific information (like the registers)
    dev = devm_kzalloc(&pdev->dev, sizeof(struct ensea_leds_dev), GFP_KERNEL);
    pdev2 = pdev;

    // Both request and ioremap a memory region
    // This makes sure nobody else can grab this memory region
    // as well as moving it into our address space so we can actually use it
    dev->regs = devm_ioremap_resource(&pdev->dev, r);
    if(IS_ERR(dev->regs))
        goto bad_ioremap;

    // Turn the LEDs on (access the 0th register in the ensea LEDs module)
    dev->leds_value = 0xFF;
    iowrite32(dev->leds_value, dev->regs);

    // Initialize the misc device (this is used to create a character file in userspace)
    dev->miscdev.minor = MISC_DYNAMIC_MINOR;    // Dynamically choose a minor number
    dev->miscdev.name = "ensea_leds";
    dev->miscdev.fops = &ensea_leds_fops;

    ret_val = misc_register(&dev->miscdev);
    if(ret_val != 0) {
        pr_info("Couldn't register misc device :(");
        goto bad_exit_return;
    }

    // Give a pointer to the instance-specific data to the generic platform_device structure
    // so we can access this data later on (for instance, in the read and write functions)
    platform_set_drvdata(pdev, (void*)dev);

	setup_timer(&tim1, tim1_callback, 0);
    // Lancement du timer avec param_vit
	mod_timer(&tim1, jiffies + param_vit);

    pr_info("leds_probe exit\n");

    return 0;


bad_ioremap:
   ret_val = PTR_ERR(dev->regs);
bad_exit_return:
    pr_info("leds_probe bad exit :(\n");
    return ret_val;
}


// This function gets called whenever a read operation occurs on one of the character files
static ssize_t leds_read(struct file *file, char *buffer, size_t len, loff_t *offset)
{
    int success = 0;

    /*
    * Get the ensea_leds_dev structure out of the miscdevice structure.
    *
    * Remember, the Misc subsystem has a default "open" function that will set
    * "file"s private data to the appropriate miscdevice structure. We then use the
    * container_of macro to get the structure that miscdevice is stored inside of (which
    * is our ensea_leds_dev structure that has the current led value).
    *
    * For more info on how container_of works, check out:
    * http://linuxwell.com/2012/11/10/magical-container_of-macro/
    */
    struct ensea_leds_dev *dev = container_of(file->private_data, struct ensea_leds_dev, miscdev);

    // Give the user the current led value
    success = copy_to_user(buffer, &dev->leds_value, sizeof(dev->leds_value));

    // If we failed to copy the value to userspace, display an error message
    if(success != 0) {
        pr_info("Failed to return current led value to userspace\n");
        return -EFAULT; // Bad address error value. It's likely that "buffer" doesn't point to a good address
    }

    return 0; // "0" indicates End of File, aka, it tells the user process to stop reading
}

// This function gets called whenever a write operation occurs on one of the character files
static ssize_t leds_write(struct file *file, const char *buffer, size_t len, loff_t *offset)
{
    int success = 0;

    /*
    * Get the ensea_leds_dev structure out of the miscdevice structure.
    *
    * Remember, the Misc subsystem has a default "open" function that will set
    * "file"s private data to the appropriate miscdevice structure. We then use the
    * container_of macro to get the structure that miscdevice is stored inside of (which
    * is our ensea_leds_dev structure that has the current led value).
    *
    * For more info on how container_of works, check out:
    * http://linuxwell.com/2012/11/10/magical-container_of-macro/
    */
    struct ensea_leds_dev *dev = container_of(file->private_data, struct ensea_leds_dev, miscdev);

    // Get the new led value (this is just the first byte of the given data)
    success = copy_from_user(&dev->leds_value, buffer, sizeof(dev->leds_value));

    // If we failed to copy the value from userspace, display an error message
    if(success != 0) {
        pr_info("Failed to read led value from userspace\n");
        return -EFAULT; // Bad address error value. It's likely that "buffer" doesn't point to a good address
    } else {
        // We read the data correctly, so update the LEDs
        iowrite32(dev->leds_value, dev->regs);
    }

    // Tell the user process that we wrote every byte they sent
    // (even if we only wrote the first value, this will ensure they don't try to re-write their data)
    return len;
}

// Gets called whenever a device this driver handles is removed.
// This will also get called for each device being handled when
// our driver gets removed from the system (using the rmmod command).
static int leds_remove(struct platform_device *pdev)
{
    // Grab the instance-specific information out of the platform device
    struct ensea_leds_dev *dev = (struct ensea_leds_dev*)platform_get_drvdata(pdev);

    pr_info("leds_remove enter\n");

    // Turn the LEDs off
    iowrite32(0x00, dev->regs);

    // Unregister the character file (remove it from /dev)
    misc_deregister(&dev->miscdev);

    pr_info("leds_remove exit\n");

    return 0;
}

// Called when the driver is removed
static void leds_exit(void)
{
    pr_info("Ensea LEDs module exit\n");

    remove_proc_entry("speed", ensea);
    remove_proc_entry("dir", ensea);
    remove_proc_entry("ensea", NULL);

    // Unregister our driver from the "Platform Driver" bus
    // This will cause "leds_remove" to be called for each connected device
    del_timer(&tim1);
    platform_driver_unregister(&leds_platform);

    pr_info("Ensea LEDs module successfully unregistered\n");
}


//=============================================================================================================================================
//=============================================================================================================================================
//=============================================================================================================================================

// Lecture de la direction, donc récupération de la valeur actuelle côté client
static ssize_t proc_dir_read(struct file *file, char *buffer, size_t len, loff_t *offset)
{
    printk(KERN_DEBUG "proc_dir_read\n");
    int success = copy_to_user(buffer, &param_dir, sizeof(param_dir));
    if(success != 0) {
        pr_info("Failed to return current led value to userspace\n");
        return -EFAULT;
    }

    return 0;
}

// Ecriture de la direction, une conversion ASCII donne que 1=49, on ne check que cette valeur ou on met 0
static ssize_t proc_dir_write(struct file *file, const char *buffer, size_t len, loff_t *offset)
{
    printk(KERN_DEBUG "proc_dir_write");
    int success = copy_from_user(loc_buf, buffer, sizeof(param_dir));
    if (loc_buf[0] == 49){
        param_dir = 1;
    }
    else{
        param_dir = 0;
    }
    if(success != 0) {
        pr_info("Failed to read led value from userspace\n");
        return -EFAULT;
    }

    return len;
}

// Speed ne se règle qu'à l'execution, on a donc juste une fonction de lecture
static ssize_t proc_speed_read(struct file *file, char *buffer, size_t len, loff_t *offset)
{
    printk(KERN_DEBUG "proc_speed_read\n");
    int success = copy_to_user(buffer, &param_vit, sizeof(param_vit));
    if(success != 0) {
        pr_info("Failed to return current led value to userspace\n");
        return -EFAULT;
    }

    return 0;
}

// Tell the kernel which functions are the initialization and exit functions
module_init(leds_init);
module_exit(leds_exit);

// Define information about this kernel module
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Devon Andrade <devon.andrade@oit.edu>");
MODULE_DESCRIPTION("Exposes a character device to user space that lets users turn LEDs on and off");
MODULE_VERSION("1.0");
// Initialisation du paramètre de vitesse, 200 de base.
module_param(param_vit, int, 200);
MODULE_PARM_DESC(param_vit,"vitesse de balayage");