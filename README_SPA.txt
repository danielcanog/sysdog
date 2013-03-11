==== introduccion ====

'Sysdog' es un sistema de 'watchdog' para sistemas que corren linux, que intenta asegurar la disponibilidad 24/7 del sistema.
este programa es diseñado mas especificamente para correr sobre sistemas embebidos que corran linux, que cuenten con un modulo de 'watchdog', que cumplan una funcion especifica y que deban estar disponibles 24/7.

==== Funcionalidad ====

Mas detalladamente 'sysdog; cumple tres funcionalidades.
1-Revisa que el sistema tenga acceso a una red.
2-Ejecuta un programa especifico y revisa que este proceso.
2.1-Se este ejecutando
2.2-No se bloquee. (Que consuma procesamiento)

Cuando una de estas condiciones falle por cierto tiempo, 'sysdog' reinicia el sistema.

El reinicio se hace de dos formas.
1-Primero se intenta reiniciar el sistema por software usando el comando 'reboot'.
2-Si por algun motivo falla el anterior intento, se deja de alimentar al modulo de 'watchdog' para lograr un reinicio del sistema por hardware.

!!'Sysdog' ademas asegura que el sistema operativo como tal no se estrelle, pues activa y alimenta el sistema de watchdog.

==== Caracteristicas ====

*-'Sysdog' es facilmente adaptable a las necesidades de cada situacion, pues todas las funcionalidades se pueden activar o desactivar individualmente.
*-'Sysdog' es facilmente portable entre diferentes sistemas embebidos, por que en el codigo fuente se tiene claramente definidas las funciones que hay que reimplementar para cada sistema.

Actualmente se encuentra desarrollado y compilado para los siguientes sistemas.
*-'Ts75xx' de "Technologic Systems"
*-'Raspberry Pi'

==== Ejecucion ====

'Sysdog' recive dos argumentos 
1-Direccion web para revisar acceso a la red.
2-Comando a ejecutar y vigilar.

Ejemplo:

 > sysdog "www.udea.edu.co" "ping www.google.com &> log.txt"

En este ejemplo el primer argumento ("www.udea.edu.co") dice que esa es la direccion que sera usada para revisar la conexion (Acceso a la red)
El segundo argumento ("ping www.google.com &> log.txt") es el comando que va  a ser ejecutado y el cual se va a vigilar que siempre este trabajando.

Este programa esta diseñado para correr cuando el sistema operativo esta iniciando y con permisos de super-usuario.
Una forma de hacer esto es agregando la linea de ejecucion al script '/etc/rc.local'

Ejemplo:
(asumiendo que el archivo 'sysdog' (binario) esta ubicado en '/root/sysdog')
Agregue la siguiente linea en el script '/etc/rc.local'
 /root/sysdog "www.udea.edu.co" "ping www.google.com"

!!Nota: En el script '/etc/rc.local' es importante asegurarse de poner la linea que se desea ejecutar antes de la linea 'exit 0'

==== Otros Ejemplos ====

*-Revisar solo el acceso a la red
  > sysdog "www.udea.edu.co" 
*-Revisar solo la ejecucion del programa
 > sysdog 127.0.0.1 "ping www.google.com &> log.txt"
*-Revisar solo acceso a internet (acceso a google.com)
 > sysdog
*-Activar solo modulo de watchdog.
 > sysdog 127.0.0.1

==== Precauciones ====

*-El acceso a la red es revisado usando '/bin/ping', la opcion '-w' es utilizada. Algunas implementaciones de '/bin/ping' no tienen esa opcion.
  Para revisar si su implementacion tiene esta opecion ejecute
   >ping --help
  Y busque esa opcion.
  Si no tiene esa opcion puede intentar solucionarlo actualizando el programa '/bin/ping' ejecutando 
   >sudo apt-get install iputils-ping

*-Para validar el comando que se va a ejecutar se usa el programa 'file'
  Para revisar si su sistema tiene el programa ejecute
   >file
  Y revise si se encontro.
  Si no tiene instalado el programa instalelo ejecutando 
   >sudo apt-get install file

* Si la direccion con la cual se va a revisar el acceso a la red es una direccion que necesita 'dns' para ser resuelta (La direccion por default lo es) es de suma importancia que el dispositivo tenga bien configurado el servidor 'dns', de lo contrario el watchdog por hardware reiniciara el sistema cuando se active (aprox 30 seg despues de iniciado).

==== Desarrollo ====
===== Compilando para nuevos sistemas =====

1-Cree un .h donde implemente en 'c' las funciones para que funcionen con su sistema.
 void enable_wd() //Enable watch dog
 void feed_dog() //feed dog

2-en el archivo './sysdog.c' edite la linea que se parece a esta
	include "wd_ts75xx.h"  //Include header that implement 'enable_wd()' and 'feed_dog()' functions
  y haga que incluya el archivo que implemento en el anterior punto

3-Edite el archivo './makefile' y haga que la variable 'CC' apunte a el compilador (o cross compilador) para su sistema
	CC = /mnt/NFS-RW/cross-compiler-tools/Ts/arm-unknown-linux-gnu/bin/arm-unknown-linux-gnu-gcc

4-Ejecute
   >make

