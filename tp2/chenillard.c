#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>

#define DRIVER_AUTHOR	"L.Fournier C.Fraysse S.Altaber"
#define DRIVER_DESC		"Chenillard Module"
#define DRIVER_LICENSE	"GPL"

#define PROC_NAME		"chenillard"
#define PROC_PAR_NAME	"ensea"
struct file_operations proc_fops = {};
struct proc_dir_entry* proc_parent;

// Intervalle du timer
#define TIM1_INT		200
static struct timer_list tim1;
static int led_c = 0;

// Callback du timer qui se rapelle en boucle en rajoutant le délais au temps actuel
// Pour l'instant, il ne fait que print sur le kernel
static void tim1_callback(unsigned long data) {
	mod_timer(&tim1, jiffies + TIM1_INT);
	if (led_c < 10){
		printk(KERN_INFO "LED %d ON\n", led_c);
		led_c ++;
	}
	else {
		led_c = 0;
	}
}

// param en entrée qui sera print dans le hello world à la création du module
int param = 0;
module_param(param,int,0);

int hello_init(void)
{
	// Création du fichier ensea/chenillard dans proc/
	proc_parent = proc_mkdir(PROC_PAR_NAME, NULL);
	proc_create(PROC_NAME, 0, proc_parent, &proc_fops);

	// Création et lancement du timer
	setup_timer(&tim1, tim1_callback, 0);
	mod_timer(&tim1, jiffies + TIM1_INT);

	printk(KERN_INFO "Hello world! %d\n", param);
	return 0;
}

void hello_exit(void)
{
	// Suppression des fichier procs à la sortie
	remove_proc_entry(PROC_NAME, proc_parent);
	remove_proc_entry(PROC_PAR_NAME, NULL);

	// Suppression des timers
	del_timer(&tim1);

	// Sauter ces étapes mènent à de la corruptiion de mémoire et des problèmes
	// de ségmentation au rechargement du module.

	printk(KERN_ALERT "Bye bye...\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE(DRIVER_LICENSE);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
