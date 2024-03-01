# Lab: shell

### Búsqueda en $PATH

1) Los wrappers proporcionados por la librería estándar de C son de la forma execXX donde todas cumplen la función de
cargar un programa nuevo en el proceso actual y proveerle argumentos y variables de entorno. 
Las diferencias entre las funciones son: como se encuentra al programa, como se especifican 
los argumentos y de donde viene el entorno.

Los wrappers con 'v' en el nombre reciben un array especificando el argv[] del nuevo programa.
Los wrappers con 'l' en el nombre reciben los argumentos en forma de lista.
Los wrappers con 'e' en el nombre recibe un argumento extra que provee el entorno del nuevo programa.
Los wrappers con 'p' en el nombre usan la variable PATH para ubicar al programa (no hay que especificar pathname, solo el nombre del binario)

Para el caso de execve:
Recibe un pathname, es decir que carga el programa que se encuentra en ese path ('p' no presente).
Recibe los argumentos como un array ('v' presente).
Recibe el entorno envp[] ('e' presente).

2) Todas las funciones de la familia exec pueden fallar y setear el errno para los distintos tipos de errores.
En mi shell, en el caso de error se imprime un mensaje de error y se retorna del programa con codigo (-1)

### Comandos built-in

1) El cd no podria ser ejectuado sin ser built in porque se quiere cambiar el directorio de la shell y no del proceso hijo.
El pwd podría ser ejecutado sin necesidad de ser un comando ya que no afecta el directorio, solamente imprime el cwd.
Al hacerlo de forma built-in nos ahorramos los recursos que implicaría tener que hacer un fork para ejecutar el comando.

### Variables de entorno adicionales

1) Es necesario hacerlo luego de la llamada a fork porque esto nos permite que las variables de entorno definidas solo existan durante la ejecución del comando que acompania su deficion y que no afecten a los siguientes comandos que se llamen en la shell.

2) La diferencia es que el entorno del programa que se llama con execle o execvpe es recibido por parámetro.
Cuando se usa setenv(3), se definen nuevas variables de entorno que se agregan al entorno existente (environ)
mientras que si paso por parámetro las variables "USER=nadie" y "ENTORNO=nada", el entorno del nuevo proceso solo va a 
contener esas 2 variables.

### Procesos en segundo plano

1) Para ejecutar procesos en segundo plano lo que hice es esperar (con waitpid) unicamente a los procesos que no sean del tipo BACK. En el caso de que sean del tipo BACK se usa la función print_back_info para imprimir el pid y luego se los espera oportunamente antes de parsear el siguiente comando usando waitpid(-1, &status, WNOHANG) Flag WNOHANG para que el wait no sea bloqueante.

### Flujo estándar

1) El syntax >& se usa para redirigir un stream a otro file descriptor. En el caso de "2>&1" sé esta redirigiendo
el STDERR al filedescriptor 1 STDOUT.

En el ejemplo, se llama al comando: ls -C /home /noexiste >out.txt 2>&1 y luego cat out.txt. Se obtiene la siguiente salida:
ls: cannot access '/noexiste': No such file or directory
/home:
lucas

Si invierto el orden, obtengo la misma salida.

### Tuberías simples (pipes)

1) Ejemplo de fallo en comando en pipe usando bash:  
ls noexiste | grep a | wc
ls: cannot access 'noexiste': No such file or directory
	0       0       0

echo $?
0

A pesar de que el primer comando dio error, el resto se ejecutaron. Luego, el exit code fue 0.
En la shell del lab ocurre lo mismo pero el exit code devuelto fue 512.
./sh
 (/home/lucas) 
$ ls noexiste | grep a | wc
ls: cannot access 'noexiste': No such file or directory
      0       0       0

 (/home/lucas) 
$ echo $?
512

### Pseudo-variables

1) '$' El PID de la shell actual.
echo $$
2412

2) '!' El PID del ultimo proceso background.
sleep 5 &
[1] 2477
echo $!
2477

3) '0' El filename del script actual.
echo $0
bash


