#define _XOPEN_SOURCE 600 

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<fcntl.h>
#include<semaphore.h>
#include<sys/mman.h>
#include"estacionamiento.h"



#define CANTIDAD_DEFAULT 10
#define INTERVALO 500000

static struct BufferCompartido *buffer = NULL;
static sem_t *vacios = NULL;
static sem_t *llenos = NULL;
static sem_t *mutex = NULL;

int inicializar_recursos(void){
    int fd;

    /*Creo memoria compartida*/
    fd = shm_open(SHM_NOMBRE, O_CREAT | O_RDWR, 0600);
    if(fd == -1){
        perror("[Entrada] Error shm_open");
        return -1;
    }

    /*Defino tamanio*/
    if(ftruncate(fd, sizeof(struct BufferCompartido)) == -1){
        perror("[Entrada] Error ftruncate");
        close(fd);
        return -1;
    }

    /*Mapeo memoria*/
    buffer = mmap(NULL, sizeof(struct BufferCompartido), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(buffer == MAP_FAILED){
        perror("[Entrada] Error mmap");
        close(fd);
        return -1;
    }

    /*Ya no necesito fd*/
    close(fd);

    /*Inicializo buffer*/
    buffer->cabeza = 0;
    buffer->cola = 0;

    /*Creo semaforos*/
    vacios = sem_open(VACIOS, O_CREAT, 0600, CAPACIDAD);
    if(vacios == SEM_FAILED){
        perror("[Entrada] Error sem_open vacios");
        return -1;
    }

    llenos = sem_open(LLENOS, O_CREAT, 0600, 0);
    if(llenos == SEM_FAILED){
        perror("[Entrada] Error sem_open llenos");
        return -1;
    }

    mutex = sem_open(MUTEX, O_CREAT, 0600, 1);
    if(mutex == SEM_FAILED){
        perror("[Entrada] Error sem_open mutex");
        return -1;
    }

    return 0;
}


void liberar_recursos(void){
    if(buffer != NULL && buffer != MAP_FAILED){
        if(munmap(buffer,sizeof(struct BufferCompartido)) == -1){
            perror("[Entrada] Error munmap");
        }
    }

    if(vacios != NULL && vacios != SEM_FAILED){
        if(sem_close(vacios) == -1){
            perror("[Entrada] Error sem_close vacios");
        }
    }

    if(llenos != NULL &&  llenos != SEM_FAILED){
        if(sem_close(llenos) == -1){
            perror("[Entrada] Error sem_close llenos");
        }
    }

    if(mutex != NULL && mutex != SEM_FAILED){
        if(sem_close(mutex) == -1){
            perror("[Entrada] Error sem_close mutex");
        }
    }
}


int main(int argc, char *argv[]){

    int cantidad;
    int i;
    struct Estacionamiento est;

    if(argc > 1){
        cantidad = atoi(argv[1]);
        if(cantidad <= 0){
            printf("[Entrada] Cantidad ingresada por parametro invalida, asignando cantidad por default \n");
            cantidad = CANTIDAD_DEFAULT;
        }
    }else{
        printf("[Entrada] Uso incorrecto, asignando cantidad por default \n");
        cantidad = CANTIDAD_DEFAULT;
    }

    srand(time(NULL));

    if(inicializar_recursos() == -1){
        perror("[Entrada] Error inicializando recursos");
        exit(EXIT_FAILURE);
    }

    printf("[Entrada] Ingresando al estacionamiento %d vehiculos \n", cantidad);

    for(i=0; i<cantidad; i++){

        est = obtener_estacionamiento();

        if(sem_wait(vacios) == -1){
            perror("[Entrada] Error sem_wait vacios");
            liberar_recursos();
            exit(EXIT_FAILURE);
        }

        if(sem_wait(mutex) == -1){
            perror("[Entrada] Error sem_wait mutex");
            sem_post(vacios); /*Devuelvo el lugar por fallo*/
            liberar_recursos();
            exit(EXIT_FAILURE);
        }

        /*Inicio seccion critica*/

        buffer->estacionamiento[buffer->cabeza]= est;
        buffer->cabeza=(buffer->cabeza +1)% CAPACIDAD;

        /*Fin seccion critica*/

        if(sem_post(mutex) == -1){
            perror("[Entrada] Error sem_post mutex");
            liberar_recursos();
            exit(EXIT_FAILURE);
        }

        if(sem_post(llenos) == -1){
            perror("[Entrada] Error sem_post llenos");
            liberar_recursos();
            exit(EXIT_FAILURE);
        }

        printf("Ingreso al estacionamiento: Patente %s - Ticket %d \n", est.patente, est.ticket);

        usleep(INTERVALO);

    }

    printf("Ingresaron al estacionamiento %d vehiculos \n", cantidad);

    liberar_recursos();

    return EXIT_SUCCESS;
}
