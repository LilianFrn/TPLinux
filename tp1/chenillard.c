#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

// Contenant pour le nom des fichiers
static const char* LEDS[9] = {
    "/sys/class/leds/fpga_led1/brightness",
    "/sys/class/leds/fpga_led2/brightness",
    "/sys/class/leds/fpga_led3/brightness",
    "/sys/class/leds/fpga_led4/brightness",
    "/sys/class/leds/fpga_led5/brightness",
    "/sys/class/leds/fpga_led6/brightness",
    "/sys/class/leds/fpga_led7/brightness",
    "/sys/class/leds/fpga_led8/brightness",
    "/sys/class/leds/fpga_led9/brightness"
};

#define LED_ON  "1"
#define LED_OFF "0"

int main(void) {
    int sel = 0; // Selecteur pour changer de fichier à modifier
    int fd = 0;
    printf("Partying\n");

    while(1) {
        fd = open(LEDS[sel], O_RDWR | O_CREAT);
        if (fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        write(fd, LED_ON, 1);
        fsync(fd);
        usleep(100000); // Délais du chenillard
        write(fd, LED_OFF, 1);
        fsync(fd);
        close(fd);

        sel++;
        if (sel == 9) {
            sel = 0;
        } // Retour à 0 du sélecteur
    }
    return 0;
}