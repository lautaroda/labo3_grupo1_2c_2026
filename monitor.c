
#define _XOPEN_SOURCE 600 

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<semaphore.h>
#include<sys/mman.h>
#include<errno.h>
#include"estacionamiento.h"

#define CANTIDAD_DEFAULT 10

static struct BufferCompartido *buffer = NULL;
static sem_t *vacios = NULL;
static sem_t *llenos = NULL;
static sem_t *mutex = NULL;

int inicializar_recursos(void){
    int intentos = 0;
    int fd;

    fd = -1;
    while(intentos < 10){
        /*Abro memoria compartida*/
        fd = shm_open(SHM_NOMBRE, O_RDWR, 0);
        if(fd != -1){
            break;
        }

        if(errno == ENOENT){
            printf("[Monitor] Esperando a entrada.. \n");
            intentos++;
            sleep(1);
        }else{
            perror("[Monitor] Error shm_open");
            return -1;
        }
    }

    if(fd == -1){
        fprintf(stderr, "[Monitor] La entrada no esta corriendo... \n");
        return -1;
    }

    /*Mapeo memoria*/
    buffer = mmap(NULL, sizeof(struct BufferCompartido), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(buffer == MAP_FAILED){
        perror("[Monitor] Error mmap");
        close(fd);
        return -1;
    }

    /*Ya no necesito fd*/
    close(fd);

    /*Abro semaforos sin O_CREAT*/
    vacios = sem_open(VACIOS, 0);
    if(vacios == SEM_FAILED){
        perror("[Monitor] Error sem_open vacios");
        return -1;
    }

    llenos = sem_open(LLENOS, 0);
    if(llenos == SEM_FAILED){
        perror("[Monitor] Error sem_open llenos");
        return -1;
    }

    mutex = sem_open(MUTEX, 0);
    if(mutex == SEM_FAILED){
        perror("[Monitor] Error sem_open mutex");
        return -1;
    }

    return 0;
}

void eliminar_recursos(void){

    if(buffer != NULL && buffer != MAP_FAILED){
        if(munmap(buffer,sizeof(struct BufferCompartido)) == -1){
            perror("[Monitor] Error munmap");
        }
    }

    if(vacios != NULL && vacios != SEM_FAILED){
        if(sem_close(vacios) == -1){
            perror("[Monitor] Error sem_close vacios");
        }
        if(sem_unlink(VACIOS) == -1){
            perror("[Monitor] Error sem_unlink vacios");
        }
    }

    if(llenos != NULL && llenos != SEM_FAILED){
        if(sem_close(llenos) == -1){
            perror("[Monitor] Error sem_close llenos");
        }
        if(sem_unlink(LLENOS) == -1){
            perror("[Monitor] Error sem_unlink llenos");
        }
    }

    if(mutex != NULL && mutex != SEM_FAILED){
        if(sem_close(mutex) == -1){
            perror("[Monitor] Error sem_close mutex");
        }
        if(sem_unlink(MUTEX) == -1){
            perror("[Monitor] Error sem_unlink mutex");
        }
    }

    if(shm_unlink(SHM_NOMBRE) == -1){
        perror("[Monitor] Error shm_unlink");
    }

    
}


int main(int argc, char *argv[]){
    int cantidad;
    int i;
    struct Estacionamiento est;

    if(argc > 1){
        cantidad = atoi(argv[1]);
        if(cantidad <= 0){
            printf("[Monitor] Cantidad ingresada por parametro invalida, asignando cantidad por default \n ");
            cantidad = CANTIDAD_DEFAULT;
        }
    }else{
        printf("[Monitor] Uso incorrecto, asignando cantidad por default\n");
        cantidad = CANTIDAD_DEFAULT;
    }

    if(inicializar_recursos() == -1){
        perror("[Monitor] Error inicializando recursos");
        exit(EXIT_FAILURE);
    }


    for(i=0; i<cantidad; i++){

        if(sem_wait(llenos) == -1){
            perror("[Monitor] Error sem_wait llenos");
            eliminar_recursos();
            exit(EXIT_FAILURE);
        }

        if(sem_wait(mutex) == -1){
            perror("[Monitor] Error sem_wait mutex");
            sem_post(llenos); /*Devuelvo el lugar por fallo*/
            eliminar_recursos();
            exit(EXIT_FAILURE);
        }

        /*Inicio zona critica*/

        est = buffer->estacionamiento[buffer->cola];
        buffer->cola = (buffer->cola +1)%CAPACIDAD;

        /*Fin zona critica*/

        if(sem_post(mutex) == -1){
            perror("[Monitor] Error sem_post mutex");
            eliminar_recursos();
            exit(EXIT_FAILURE);
        }

        if(sem_post(vacios) == -1){
            perror("[Monitor] Error sem_post vacios");
            eliminar_recursos();
            exit(EXIT_FAILURE);
        }

        printf("Salio del estacionamiento: Patente %s - Ticket %d \n", est.patente, est.ticket);
    }

    eliminar_recursos();

    printf("Recursos liberados y eliminados del sistema.. \n");

    return EXIT_SUCCESS;
}