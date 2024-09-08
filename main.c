#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#define Max_Caracteres 1024  // Tamaño máximo de la línea de comando
#define Max_Argumentos 64    // Número máximo de argumentos
#define Max_pipes 10         // Número maximo de argumentos concatenados con pipes
#define MAX_FAV 1000         // Número máximo de comandos guardados como favoritos

char DirectorioAnterior[Max_Caracteres];
char *favoritos[MAX_FAV];

FILE *fp;

void executeCommand(char **args) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Error en fork");
    } else if (pid == 0) {
        // Proceso hijo
        if (execvp(args[0], args) < 0) {
            perror("Error en exec");
        }
        exit(EXIT_FAILURE);
    } else {
        // Proceso padre
        waitpid(pid,NULL,0);
    }
}

void favsCmd(char **args) {
    if (strcmp(args[1], "crear") == 0) {
        fp = fopen(args[2], "w+");
        printf("favcrear\n");
        
    } else if (strcmp(args[1], "mostrar") == 0) {

        printf("favmostrar\n");

    } else if (strcmp(args[1], "eliminar") == 0) {

        printf("faveliminar\n");

    } else if (strcmp(args[1], "buscar") == 0) {

        printf("favbuscar\n");

    } else if (strcmp(args[1], "borrar") == 0) {

        printf("favborrar\n");

    } else if (args[1] != NULL && args[2] != NULL && strcmp(args[2], "ejecutar") == 0) {

        int num = atoi(args[1]);
        if(num > 0 && num <= MAX_FAV && favoritos[num - 1] != NULL){
            char *Compatible[] = {favoritos[num - 1], NULL};
            executeCommand(Compatible);
            printf("favejecutar\n");
        }
        else{
            printf("Error: Número favorito fuera de rango o no valido");
        }
    } else if (strcmp(args[1], "cargar") == 0) {

        printf("favcargar\n");

    } else if (strcmp(args[1], "guardar") == 0) {
        fclose(fp);
        fp = fopen(args[2], "w+");
        printf("favguardar\n");

    } else {
        printf("Error, no se ha proporcionado un argumento valido para el comando 'favs'\n");
    }
}

void cd(char *ruta){
    char DirectorioActual[Max_Caracteres];

    getcwd(DirectorioActual, sizeof(DirectorioActual));

    if (ruta == NULL) {
        if(strlen(DirectorioAnterior)>0){
            if(chdir(DirectorioAnterior)!=0){
                perror("Error al cambiar al directorio anterior");
            }
        }
    } else {
        if (chdir(ruta) != 0) {
            perror("Error al cambiar de directorio");
        }
    }
    strcpy(DirectorioAnterior,DirectorioActual);
}

void printSetComands(){
    printf("\n  Comandos disponibles:\nset recordatorio segundos mensaje\n\n");
} 

void sig_usr(int signo){
    if(signo == SIGINT)
    printf("Signal caught!");
    return;
}
void recordatorio(char **action){
    if(action[2] == NULL || action[3] == NULL){
	printSetComands();
	return;
    }

    int segundos = atoi(action[2]);
    char *mensaje = action[3];

    if(segundos < 0){
	printf("\n No se permiten segundos negativos\n");
	return;
    }

    // PROCESO
    pid_t parentId = getpid();
    pid_t pid = fork();

    if (pid < 0) {
        perror("Error en fork");
    } else if (pid == 0) {
        // Proceso hijo
	sleep(segundos);

	printf("\n\n-------------------------\n\n  * RECUERDE: %s\n\n-------------------------\nENTER PARA CONTINUAR\n\n",mensaje);
	exit(0);

    } else {
        // Proceso padre
	printf("\n| Nuevo recordatorio: %s en %d segundos. | \n",mensaje,segundos);
	return;
    }
}

void set(char **action){
    if(action[1] == NULL){
	printSetComands();
	return;
    }

    if(strcmp(action[1],"recordatorio") == 0){
	recordatorio(action);
	return;
    }
    
    printSetComands();
}

void parseCommand(char *cmd, char **args) {
    for (int i = 0; i < Max_Argumentos; i++) {
        args[i] = strsep(&cmd, " ");
        if (args[i] == NULL) break;
    }
}



int main() {
    char cmd[Max_Caracteres];
    char *args[Max_Argumentos];

    while (1) {
        char DirectorioActual[Max_Caracteres];
        getcwd(DirectorioActual, sizeof(DirectorioActual));
        printf("mishell:\%s$ ",DirectorioActual);
        fgets(cmd, Max_Caracteres, stdin);

        // Eliminar el salto de línea al final de la entrada
        cmd[strcspn(cmd, "\n")] = 0;

        // Si la línea está vacía, continuar
        if (strlen(cmd) == 0) continue;
        
        // Comprobar si el comando es "exit"
        if (strcmp(cmd, "exit") == 0) break;

        // Parsear la entrada
        parseCommand(cmd, args);

        // Ejecutar el comando
        if(strcmp(cmd,"cd") == 0) {
            cd(args[1]);
        }
        else if(strcmp(cmd,"set") == 0) {
            set(args);
        }
        else if(strcmp(cmd,"favs") == 0){
            favsCmd(args);
        }
        else{
            executeCommand(args);
        }
        
    }
    return 0;
}
