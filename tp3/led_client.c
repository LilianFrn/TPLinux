#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

// Nom des commandes possibles à entrer en paramètre à l'éxecution
#define COM_SPD_GET "speed_get"
#define COM_PAT_SET "pattern_set"
#define COM_PAT_GET "pattern_get"
#define COM_SWP_SET "sweep_set"
#define COM_SWP_GET "sweep_get"
 
// Longueur des réponses
#define SPD_LEN     5
#define PAT_LEN     2
#define SWP_LEN     1

// Fichiers d'interection
#define FILE_SPD    "/proc/ensea/speed"
#define FILE_DRV    "/dev/ensea-leds"
#define FILE_SWP    "/proc/ensea/dir"
 
static char buff[32];
 
int main(int argc, char* argv[]) {
    int fd = 0;

    // Check si le premier argument est une des fonctions
    // Prend le 2e argument à envoyer dans le cas des écritures
    // print la valeur reçu dans le cas des lectures
    if (strcmp(argv[1],COM_SPD_GET) == 0){
        fd = open(FILE_SPD, O_RDWR | O_CREAT);
        if (fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        read(fd, buff, SPD_LEN);
        printf(buff);
    }
 
    else if (strcmp(argv[1],COM_PAT_SET) == 0){
        fd = open(FILE_SPD, O_RDWR | O_CREAT);
        if (fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        write(fd, argv[2], PAT_LEN);
    }
 
    else if (strcmp(argv[1],COM_PAT_GET) == 0){
        fd = open(FILE_SPD, O_RDWR | O_CREAT);
        if (fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        read(fd, buff, PAT_LEN);
        printf(buff);
    }
 
    else if (strcmp(argv[1],COM_SWP_SET) == 0){
        fd = open(FILE_SPD, O_RDWR | O_CREAT);
        if (fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        write(fd, argv[2], SWP_LEN);
    }
 
    else if (strcmp(argv[1],COM_SWP_GET) == 0){
        fd = open(FILE_SPD, O_RDWR | O_CREAT);
        if (fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        read(fd, buff, SWP_LEN);
        printf(buff);
    }
 
    else {
        printf("Command not found\n");
    }
 
    // Fermeture du fichier
    close(fd);
    return 0;
}