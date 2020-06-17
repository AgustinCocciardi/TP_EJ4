#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

#define SHELLSCRIPT "\
#/bin/bash \n\
ps aux > procesos.txt\n\
"

#define SHELLSCRIPT2 "\
#/bin/bash \n\
rm procesos.txt\n\
"

int fileDescriptor;
pid_t pid1, pid2;
int status1, status2;
FILE* resultado;

void catchSignal(int signal){
    close(fileDescriptor);
    system(SHELLSCRIPT2);
    waitpid(pid1, &status1, 0);
    waitpid(pid2, &status2, 0);
    fclose(resultado);
    exit(2);
}

int main(int argc, char* argv[]){
    char *ayuda="-Help"; //Uso esta cadena para ver si el usuario quiere ver la ayuda
    if (argc == 2 && strcmp(argv[1],ayuda) == 0) //Muestro la ayuda al usuario
    {
        printf("\nEste programa tiene la funcionalidad de crear 2 procesos nuevos. Uno de ellos se llamará Control y otro Registro");
        printf("\nEl proceso control será el encargado de revisar si los procesos en ejecución superan los valores límite (que el usuario pasará por parámetro)");
        printf("\nEn caso de que ocurra, el proceso Control deberá pasarle el PID, el nombre, el tipo de exceso, y la hora del sistema al proceso Registro");
        printf("\nLos tres procesos deben estar en segundo plano hasta que el usuario envíe la señal SIGUSR1, al ocurrir eso. Los procesos deben finalizar");
        printf("\nEjemplo de ejecución");
        printf("\n./Principal 0.1 0.2");
        printf("\nDonde el primer valor es el límite de uso de CPU y el segundo es el límite de uso de memoria");
        printf("\n");
        exit(3);
    }

    if (argc != 3)   //verifico que me haya pasado los valores limite como parametro
    {
        printf("\nPor favor, pase los valores límite de uso de CPU y de uso de memoria que quiere utilizar");
        printf("\nEjecute './Principal -Help' sin las comillas para obtener ayuda");
        printf("\n");
        exit(1);
    }

    float cpu= atof(argv[1]);
    float memoria= atof(argv[2]);
    float cpuInput;
    float memoriaInput;

    char excedenAmbos[200][6];
    char excedenCPU[200][6];
    char excedenMemoria[200][6];

    strcpy(excedenAmbos[0]," ");
    strcpy(excedenCPU[0]," ");
    strcpy(excedenMemoria[0]," ");

    char buffer[1000000];

    mkfifo("tmp/fifo",0666);

    resultado= fopen("excesos.txt","a");
    if (resultado == NULL)
    {
        printf("\nSe ha producido un error\n");
        exit(2);
    }
    

    if ( (pid1=fork()) == 0 )
    { /* CONTROL */
            fileDescriptor=open("tmp/fifo",O_WRONLY);
            while (1 == 1){
                system(SHELLSCRIPT);
                FILE* archivo = fopen("procesos.txt","r");
                if (archivo == NULL)
                {
                    printf("\nNo se pudo extraer información de los Procesos en ejecucion\n");
                    exit(1);
                }
                char delimitador[]=" \n";
                char delimitadorN[]="\0";
                char palabra[1100];
                char *user;
                char *pid;
                char *cpuUso;
                char *memUso;
                char *vsz;
                char *rss;
                char *tty;
                char *stat;
                char *start;
                char *tiempo;
                char *command;

                time_t t;
                struct tm *tm;
                char hora[100];

                t=time(NULL);
                tm=localtime(&t);
                strftime(hora, 100, "%H:%M:%S", tm);

                int amb=0;
                int cpuArr=0;
                int mem=0;
                int corte;

                while (feof(archivo) == 0)
                {
                    fgets(palabra,1100,archivo);
                    if (strlen(palabra) > 30)
                    {
                        user= strtok(palabra,delimitador);
                        pid= strtok(NULL,delimitador);
                        cpuUso = strtok(NULL,delimitador);
                        memUso = strtok(NULL, delimitador);
                        vsz = strtok(NULL, delimitador);
                        rss = strtok(NULL, delimitador);
                        tty = strtok(NULL, delimitador);
                        stat = strtok(NULL, delimitador);
                        start = strtok(NULL, delimitador);
                        tiempo = strtok(NULL, delimitador);
                        command = strtok(NULL, delimitadorN);
                        cpuInput= atof(cpuUso);
                        memoriaInput = atof(memUso);
                        if (cpuInput > cpu && memoriaInput > memoria)
                        {
                            //printf("\nUser: %s PID: %s CPU: %s Memoria: %s Nombre: %s", user, pid, cpuUso, memUso, command);
                            corte=1;
                            amb=0;
                            while (strcmp(excedenAmbos[amb]," ") != 0 && corte == 1)
                            {
                                if (strcmp(excedenAmbos[amb],pid) == 0)
                                {
                                    corte=0;
                                }
                                else
                                {
                                    amb++;
                                }
                            }
                            if (corte == 1)
                            {
                                strcpy(excedenAmbos[amb],pid);
                                strcat(buffer,pid);
                                strcat(buffer," ");
                                strcat(buffer,command);
                                strcat(buffer," ");
                                strcat(buffer,"Ambos");
                                strcat(buffer," ");
                                strcat(buffer,hora);
                                strcat(buffer,"\n");
                                strcpy(excedenAmbos[amb+1]," ");
                            }
                        }
                        else
                        {
                            if (cpuInput > cpu)
                            {
                                corte=1;
                                cpuArr=0;
                                while (strcmp(excedenCPU[cpuArr]," ") != 0 && corte == 1)
                                {
                                    if (strcmp(excedenCPU[cpuArr],pid) == 0)
                                    {
                                        corte=0;
                                    }
                                    else
                                    {
                                        cpuArr++;
                                    }
                                }
                                if (corte == 1)
                                {
                                    strcpy(excedenCPU[cpuArr],pid);
                                    strcat(buffer,pid);
                                    strcat(buffer," ");
                                    strcat(buffer,command);
                                    strcat(buffer," ");
                                    strcat(buffer,"CPU");
                                    strcat(buffer," ");
                                    strcat(buffer,hora);
                                    strcat(buffer,"\n");
                                    strcpy(excedenCPU[cpuArr+1]," ");
                                }
                            }
                            if (memoriaInput > memoria)
                            {
                                corte=1;
                                mem=0;
                                while (strcmp(excedenMemoria[mem]," ") != 0 && corte == 1)
                                {
                                    if (strcmp(excedenMemoria[mem],pid) == 0)
                                    {
                                        corte=0;
                                    }
                                    else
                                    {
                                        mem++;
                                    }
                                }
                                if (corte == 1)
                                {
                                    strcpy(excedenMemoria[mem],pid);
                                    strcat(buffer,pid);
                                    strcat(buffer," ");
                                    strcat(buffer,command);
                                    strcat(buffer," ");
                                    strcat(buffer,"Memoria");
                                    strcat(buffer," ");
                                    strcat(buffer,hora);
                                    strcat(buffer,"\n");
                                    strcpy(excedenMemoria[mem+1]," ");
                                }
                            }
                        }
                    }
                }
                fclose(archivo);
                strcat(buffer,"\0");
                write(fileDescriptor,buffer,sizeof(buffer));
                sleep(1);
            }
    }
    else
    { /*  PRINCIPAL */
        if ( (pid2=fork()) == 0 )
        { /* REGISTRO  */
            fileDescriptor=open("tmp/fifo",O_RDONLY);
            while (1 == 1){
                read(fileDescriptor,buffer,sizeof(buffer));
                fwrite(buffer,sizeof(char),sizeof(buffer),resultado);
            }
        }
        else
        { /* PRINCIPAL */
            printf("\nLos procesos 'Principal', 'Control' y 'Registro' están ejecutando\nPara detenerlos mande la señal SIGUSR1 al proceso %d\n", getpid());
            signal(SIGUSR1,&catchSignal);
            while (1 == 1){
            }
        }
    }
 
    return 0;
}