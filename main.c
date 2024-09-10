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
#define MAX_FAV 10         // Número máximo de comandos guardados como favoritos

char cmd[Max_Caracteres];
char DirectorioAnterior[Max_Caracteres];
char* pathSavePath;

FILE *fp;
FILE *fRutap;

void parsearComando(char *cmd, char **args,char *token) {
    for (int i = 0; i < Max_Argumentos; i++) {
        args[i] = strsep(&cmd, token);
        if (args[i] == NULL) break;
        if (strlen(args[i]) == 0) i--;  // Manejar múltiples espacios
    }
}

typedef struct _fav{
    int id; // -1 significa no activo
    char comando[Max_Caracteres];
} Fav;

void clearFav(Fav favs[]){
    for (int i = 0; i < MAX_FAV; i++) {
	favs[i].id = -1;	
    }
}

int isFavRepeated(char *comando, Fav favs[]){
    for (int i = 0; i < MAX_FAV; i++) {
	if(favs[i].id == -1)
	    break;

	if(strcmp(favs[i].comando,comando) == 0)
	    return 1;
    }
    return 0;
}

int looking = 0;
void addFav(char *comando,Fav favs[]){
    if(isFavRepeated(comando,favs) == 1)
	return;

    favs[looking].id = looking + 1;
    strcpy(favs[looking].comando,comando);
    looking++;
}

void eliminarFav(int id, Fav favs[]){
    int aux;
    int eliminado = 0;
    for (int i = 0; i < MAX_FAV; i++) {
	if(id == -1 || id == MAX_FAV)
	    return;

	if(id == favs[i].id){
	    favs[i].id = -1;
	    strcpy(favs[i].comando,"");
	    looking--;
	    aux = i;
	    eliminado = 1;
	    break;
	}
    }
    if(eliminado == 0)
	return;
    for (int i = aux; i < MAX_FAV-1; i++) {
	if(favs[i+1].id == -1){
	    favs[i].id = -1;
	    strcpy(favs[i].comando,"");
	    return;
	}

	favs[i].id = favs[i+1].id-1;
	strcpy(favs[i].comando,favs[i+1].comando);
    }
}

int ejecutandoFavs = 0;
void favsCmd(char **args, Fav favsRam[]) {
    if(args[1] == NULL){
	printf("\nFalta los argumentos...\n");
	return;
    }
    if (strcmp(args[1], "crear") == 0) {
	fp = fopen(args[2], "w+");

	char *path = realpath(args[2],NULL);
	fRutap = fopen(pathSavePath,"w+");
	fputs(path,fRutap);
	fclose(fRutap);
	fclose(fp);
	printf("\nArchivo creado satisfactoriamente!\n");
    } 
    else if (strcmp(args[1], "mostrar") == 0) {
	printf("\n| Favs en ram |\n");
	for (int i = 0; i < MAX_FAV; i++) {
	    if(favsRam[i].id == -1)
		break;
	    printf("%d. %s\n",favsRam[i].id,favsRam[i].comando);
	}
	printf("\n");
    } 
    else if (strcmp(args[1], "eliminar") == 0) {
	parsearComando(args[2],args,",");

	int primer = atoi(args[0]);
	int segundo = atoi(args[1]);

	if(primer > segundo){
	    eliminarFav(primer,favsRam);
	    eliminarFav(segundo,favsRam);
	}
	else if(segundo > primer){

	    eliminarFav(primer,favsRam);
	    eliminarFav(segundo-1,favsRam);
	}
	else{
	    eliminarFav(primer,favsRam);
	    printf("Solo se eliminó un comando\n");
	    return;
	}

        printf("Comandos eliminados\n");
    } 
    else if (strcmp(args[1], "buscar") == 0) {
	printf("\n| Favs en ram con prefijo %s |\n",args[2]);
	for (int i = 0; i < MAX_FAV; i++) {
	    if(favsRam[i].id == -1)
		break;

	    int diferencias = 0;
	    for (int y = 0; y < strlen(args[2]); y++) {
		if(strlen(favsRam[i].comando) < strlen(args[2])){
		    if(y == strlen(args[2])-1)
			break;
		}
		if(args[2][y] != favsRam[i].comando[y]){
		    diferencias = 1;
		    break;
		}
	    }
	    if(diferencias == 1)
		continue;

	    printf("%d. %s\n",favsRam[i].id,favsRam[i].comando);
	}
	printf("\n");
    }
    else if (strcmp(args[1], "borrar") == 0) {
	fRutap = fopen(pathSavePath, "r");
	if(pathSavePath == NULL || fRutap == NULL){
	    printf("No se ha creado el archivo para favs, porfavor use el comando \"favs crear ruta/nombrearchivo.txt\"\n");
	    return;
	}
	char info[Max_Caracteres];
	fgets(info,Max_Caracteres,fRutap);
	fclose(fRutap);

	fp = fopen(info, "w+");
	fputs("",fp);
	fclose(fp);
        printf("Se han borrado todos los comandos de los favs en disco, path: %s\n",info);
    } 
    else if (args[1] != NULL && args[2] != NULL && strcmp(args[2], "ejecutar") == 0) {
        int num = atoi(args[1]);
        if(num > 0 && num <= MAX_FAV){
	    if(favsRam[num-1].id == -1){
		printf("\nNo hay comando asignado a este número\n");
		return;
	    }

	    strcpy(cmd,favsRam[num-1].comando);
	    ejecutandoFavs = 1;
            printf("Ejecutando...\n");
	    return;
        }
        else{
            printf("Error: Número favorito fuera de rango o no valido");
        }
    }
    else if (strcmp(args[1], "cargar") == 0) {
	fRutap = fopen(pathSavePath, "r");
	char info[Max_Caracteres];
	char getting[Max_Caracteres];
	char *argumentos[2];
	fgets(info,Max_Caracteres,fRutap);
	fclose(fRutap);

	fp = fopen(info, "r");
	if(pathSavePath == NULL || fRutap == NULL || fp == NULL){
	    printf("No se ha creado el archivo para favs, porfavor use el comando \"favs crear ruta/nombrearchivo.txt\"\n");
	    return;
	}
	clearFav(favsRam);
	looking = 0;
	for (int i = 0; i < MAX_FAV; ++i) {
	    if(fgets(getting,Max_Caracteres,fp) == NULL && i == 0)
		break;
	    parsearComando(getting,argumentos,"\n");
	    if(getting == NULL)
		break;
	    addFav(argumentos[0],favsRam);
	}

	fclose(fp);
        printf("\nSe han cargado todos los comandos de los favs en disco de path: %s\n(Al cargar se sustituyo los favs de la RAM con los guardados en disco)\n",info);

	printf("\n| Favs en ram |\n");
	for (int i = 0; i < MAX_FAV; i++) {
	    if(favsRam[i].id == -1)
		break;
	    printf("%d. %s\n",favsRam[i].id,favsRam[i].comando);
	}
	printf("\n");
    } 
    else if (strcmp(args[1], "guardar") == 0) {
	fRutap = fopen(pathSavePath, "r");
	if(pathSavePath == NULL || fRutap == NULL){
	    printf("No se ha creado el archivo para favs, porfavor use el comando \"favs crear ruta/nombrearchivo.txt\"\n");
	    return;
	}
	char info[Max_Caracteres];
	fgets(info,Max_Caracteres,fRutap);
	fclose(fRutap);

	fp = fopen(info, "w+");
	for (int i = 0; i < MAX_FAV; ++i) {
	    if(favsRam[i].id == -1)
		break;
	    fputs(favsRam[i].comando,fp);
	    fputs("\n",fp);
	}
	fclose(fp);
        printf("Se han guardado todos los comandos de los favs en disco, path: %s\n",info);
    } 
    else {
        printf("Error, no se ha proporcionado un argumento valido para el comando 'favs'\n");
    }
}

void EjecutarComando(char **args,char cmdNoSplit[],Fav favsRam[]) {
    if (args[0] == NULL) return;  // Si no hay comando, salir
    pid_t pid = fork();
    int childStatus = 0;

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
        waitpid(pid,&childStatus,0);

	// Funcionó el hijo, fue un comando correcto
	if(childStatus == 0)
	    addFav(cmdNoSplit,favsRam);
    }
}

int contarPipes(char *cmd, char **cmds) {
    int i = 0;
        while ((cmds[i] = strsep(&cmd, "|")) != NULL) {
        i++;
    }
    return i;  // Retorna el número de comandos separados por pipes
}

void EjecutarPipes(char **cmds, int num_cmds, char* cmdNoSplit, Fav favsRam[]) {
    int pipefd[2];
    pid_t pid;
    int fd_in = 0;  // Inicialmente la entrada es stdin
    int waitStatus = 0;

    for (int i = 0; i < num_cmds; i++) {
        pipe(pipefd);
        if ((pid = fork()) == -1) {
            perror("Error en fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Proceso hijo
            dup2(fd_in, 0);  // Reemplaza la entrada estándar por la entrada anterior
            if (i < num_cmds - 1) {
                dup2(pipefd[1], 1);  // Reemplaza la salida estándar por la salida del pipe
            }
            close(pipefd[0]);
            close(pipefd[1]);

            char *args[Max_Argumentos];
            parsearComando(cmds[i], args," ");
            if (execvp(args[0], args) < 0) {
                perror("Error en exec");
                exit(EXIT_FAILURE);
            }
        } else {
            // Proceso padre
            wait(&waitStatus);
            close(pipefd[1]);
            fd_in = pipefd[0];  // La entrada ahora es la salida del pipe anterior

        }
    }
    // Funcionó hasta el último pipe 
    if(waitStatus == 0)
	addFav(cmdNoSplit,favsRam);

    close(fd_in); // Cerrar el último descriptor de archivo abierto
}

void cd(char *ruta, char* cmdNoSplit, Fav favsRam[]){
    char DirectorioActual[Max_Caracteres];

    getcwd(DirectorioActual, sizeof(DirectorioActual));

    if (ruta == NULL) {
	if(chdir(getenv("HOME"))!=0){
	    perror("Error al cambiar al directorio anterior");
	}
	else{
	    addFav(cmdNoSplit,favsRam);
	}
    } 
    else {

        if (chdir(ruta) != 0) {
            perror("Error al cambiar de directorio");
        }
	else{
	    addFav(cmdNoSplit,favsRam);
	}
    }
    strcpy(DirectorioAnterior,DirectorioActual);
}

void printSetComands(){
    printf("\n  Comandos disponibles:\nset recordatorio segundos \"mensaje\"\n\n");
} 

char *argsTitle[2];
void recordatorio(char **action, char* cmdNoSplit, Fav favsRam[]){
    if(action[2] == NULL || action[3] == NULL || argsTitle[1] == NULL){
	printSetComands();
	return;
    }

    int segundos = atoi(action[2]);
    char *mensaje = argsTitle[1];

    if(segundos < 0){
	printf("\n No se permiten segundos negativos\n");
	return;
    }

    // Pasó argumentos correctos 
    addFav(cmdNoSplit,favsRam);

    // PROCESO
    pid_t parentId = getpid();
    pid_t pid = fork();

    if (pid < 0) {
        perror("Error en fork");
    } else if (pid == 0) {
        // Proceso hijo
	sleep(segundos);

	printf("\n\n-------------------------\n\n  * RECUERDE: %s\n\n-------------------------\nENTER PARA CONTINUAR\n\n",mensaje);
        exit(EXIT_SUCCESS);
    } else {
        // Proceso padre
	printf("\n| Nuevo recordatorio: %s en %d segundos. | \n",mensaje,segundos);
	return;
    }
}

void set(char **action,char* cmdNoSplit, Fav favsRam[]){
    if(action[1] == NULL){
	printSetComands();
	return;
    }

    if(strcmp(action[1],"recordatorio") == 0){
	recordatorio(action,cmdNoSplit,favsRam);
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

void getFavfilePath(){
    pathSavePath = realpath("GuardarRuta.txt", NULL);
    if(pathSavePath == NULL){
	printf("\nNo se encuentra GuardarRuta.txt, no eliminar porfavor.\n");
    }
}

int main() {
    Fav favsRam[MAX_FAV];
    clearFav(favsRam);
    getFavfilePath();

    char cmdNoSplit[Max_Caracteres];
    char cmdNoSplit2[Max_Caracteres];
    char *cmds[Max_pipes];
    char *args[Max_Argumentos];

    while (1) {
        char DirectorioActual[Max_Caracteres];
        getcwd(DirectorioActual, sizeof(DirectorioActual));

	if(ejecutandoFavs != 1){
	    printf("mishell:\%s$ ",DirectorioActual);
	    fgets(cmd, Max_Caracteres, stdin);
	}
	else
	    ejecutandoFavs = 0;

        // Eliminar el salto de línea al final de la entrada
        cmd[strcspn(cmd, "\n")] = 0;

        // Si la línea está vacía, continuar
        if (strlen(cmd) == 0) continue;
        
        // Comprobar si el comando es "exit"
        if (strcmp(cmd, "exit") == 0) break;

	// GUARDAR COPIAS COMANDO SIN SPLIT
	strcpy(cmdNoSplit,cmd);
	strcpy(cmdNoSplit2,cmd);

	// Parsear comillas
	parsearComando(cmdNoSplit2,argsTitle,"\"");

        // Parsear comandos con pipes
        int num_cmds = contarPipes(cmd, cmds);

        // Ejecutar el comando
        if(num_cmds>1){
            EjecutarPipes(cmds,num_cmds,cmdNoSplit,favsRam);
        } else{
            // Parsear la entrada
            parsearComando(cmd, args," ");
            if(strcmp(cmd,"cd") == 0) {
                cd(args[1], cmdNoSplit,favsRam);
            }
            else if(strcmp(cmd,"set") == 0) {
                set(args,cmdNoSplit,favsRam);
            }
            else if(strcmp(cmd,"favs") == 0){
                favsCmd(args,favsRam);
            }
            else{
                EjecutarComando(args,cmdNoSplit,favsRam);
            }
        }
    }

    free(pathSavePath);
    return 0;
}
