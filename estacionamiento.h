#ifndef ESTACIONAMIENTO_H
#define ESTACIONAMIENTO_H

#define CAPACIDAD 10


#define VACIOS "/sem_vacios"
#define LLENOS "/sem_llenos"
#define MUTEX "/sem_mutex"
#define SHM_NOMBRE "/mi_buffer"

struct Estacionamiento{
    int ticket;
    char patente[10];
};

struct BufferCompartido{
    struct Estacionamiento estacionamiento[CAPACIDAD];
    int cabeza;
    int cola;
};


struct Estacionamiento obtener_estacionamiento(void);

#endif

