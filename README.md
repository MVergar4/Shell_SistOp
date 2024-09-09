# Shell_SistOp
En primer lugar para compilar ejecutar el comando: gcc -o shell main.c, posteriormente si se compila bien ejecutar: ./shell
Si se compila y ejecuta bien se deberia ver la shell de la tarea, estando esta lista para ejecutar cualquier comando.

En segundo lugar aquí esta la lista de comandos:

1.- set recordatorio int segundos "mensaje"
    ejemplo: set recordatorio 10 "Hacer una pausa activa"
    En 10 segundos desde ejecutar el comando, se desplegará el mensaje: Hacer una pausa activa y
    se tendrá que apretar enter para continuar.

2.-Tambien se pueden ejecutar la gran mayoria de comandos vistos en clases o usados en cursos anteriores como:
   cd, ls, ps -aux | sort -nr -k 4 | head -20 etc.

3.-favs, estan presentes todos los comandos pedidos de este tipo:
   favs crear ruta/misfavoritos.txt. 
   favs mostrar.
   favs eliminar num1,num2.
   favs buscar cmd.
   favs borrar.
   favs num ejecutar.
   favs cargar.
   favs guardar.