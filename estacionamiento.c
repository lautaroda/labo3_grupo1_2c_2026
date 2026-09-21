#include "estacionamiento.h"
#include <stdio.h>
#include <stdlib.h>

struct Estacionamiento obtener_estacionamiento(void){
    struct Estacionamiento est;

    /*Ticket entre 1000 y 9999*/
    est.ticket = rand() % 9000 + 1000;

    /*Patente con formato AB123CD*/
    sprintf(est.patente, "%c%c%03d%c%c", 'A'+rand()%26, 'A'+rand()%26, rand()%1000,'A'+rand()%26,'A'+rand()%26);

    return est;
}