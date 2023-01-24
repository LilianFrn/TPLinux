# TPLinux | Lilian Fournier - Corentin Fraysse - Solène Altaber

## TP1

### 1.4.1 :
— /sys/class/leds/fpga_ledX/ : Contient brightness étant set sur 1 ou 0 étant la valeur active de la LED

— /proc/ioport : Renvoit une liste des périphérique permettant de comunniquer en input/output

— /proc/iomem : Renvoit le mapping de la mémoire physique

### Codes dans tp1 avec les commentaires

## TP2

### Codes dans tp2 avec les commentaires

## TP3

### 3.1 :
Probe : Lancé à l'isntaciation, il créé la zone mémoire I/O pour le driver et rempli la structure ensea_led_dev contant les infos du driver dev/ qui est rendu disponlible grace à un pointeur pdev.

Remove : éteint les LEDs et enlève le mapping ud driver.

Read : appelée dès qu'une action read est opérée sur le fichier du driver, écrit la valeur dans les LEDs.

Write : appelée dès qu'une action write est opérée sur le fichier du driver, renvoit la valeur des LEDs.

### 3.2 : 

### Codes dans tp3 avec les commentaires, led_client est le programme user