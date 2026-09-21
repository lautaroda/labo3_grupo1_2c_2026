GRUPO 1

--- ACTIVIDAD ESTACIONAMIENTO ---

El programa simula el funcionamiento de un estacionamiento con una cantidad limitada de lugares utilizando memoria compartida y semaforos.

Se implementan dos procesos:
- entrada: actua como productor, genera autos con un numero de ticket y una patente de forma aleatoria y los deposita en un buffer circular.

- monitor: actua como consumidor, retira los autos del buffer en orden de llegada y simula el registro de su salida.

El buffer circular vive en memoria compartida (struct BufferCompartido) y es el recurso que se reparten los dos procesos. Cada acceso al buffer (escribir en entrada, leer en monitor) es la seccion critica, protegida con el semaforo mutex para que ningun proceso pise al otro.

Los procesos se sincronizan mediante los semaforos nombrados mutex, vacios y llenos: vacios cuenta los lugares libres del estacionamiento, llenos cuenta los autos esperando salida, y mutex protege la seccion critica sobre el buffer.

Esos dos contadores (vacios y llenos) son los que generan el back pressure: cuando el estacionamiento esta lleno, vacios llega a 0 y entrada se bloquea en su sem_wait hasta que monitor libere un lugar. Si no hay autos cargados, llenos esta en 0 y monitor se bloquea hasta que entrada deposite uno. Ninguno de los dos procesos se pasa de la capacidad ni consume algo que no existe.

Al finalizar, monitor es el encargado de eliminar los recursos IPC utilizados (semaforos y memoria compartida).

--- COMPILACION ---

Para compilar el programa, ejecutar: make 
esto genera los ejecutables entrada y monitor.

Para eliminar los archivos objeto y los ejecutables generados: make clean 

--- EJECUCION ---

El programa recibe por parametro la cantidad de autos a procesar.

Para ejecutar el productor: ./entrada <cantidad>
Ejemplo: ./entrada 10

Para ejecutar el consumidor: ./monitor <cantidad>
Ejemplo: ./monitor 10

Si no se proporciona una cantidad valida, o si el valor ingresado es menor o igual a cero, el programa utiliza la cantidad definida por defecto.

Importante: entrada y monitor tienen que correrse con la misma cantidad. Como monitor es quien elimina los semaforos y la memoria compartida al terminar, si le pasan cantidades distintas el proceso que sigue corriendo se puede quedar esperando sobre recursos que ya no existen.

--- ARCHIVOS ---
- estacionamiento.h: definiciones, estructuras y prototipos compartidos.
- estacionamiento.c: implementacion de las funciones compartidas.
- entrada.c: implementacion del proceso productor.
- monitor.c: implementacion del proceso consumidor.
- Makefile: compilacion del proyecto.
- README.md: documentacion del programa.
