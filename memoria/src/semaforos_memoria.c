#include <semaforos_memoria.h>

pthread_mutex_t mutex_procesos_en_memoria;
pthread_mutex_t memoria_usuario_mutex;
pthread_mutex_t mutex_lista_metricas_procesos;
pthread_mutex_t mutex_paginas_en_swap;
pthread_mutex_t mutex_lista_procesos_swap;
pthread_mutex_t mutex_marcos;
pthread_mutex_t mutex_swap;

// Inicializa todos los mutex/semaforos globales del sistema de memoria

bool inicializar_mutex(void) {
    if (pthread_mutex_init(&mutex_procesos_en_memoria, NULL) != 0) {
        return false;
    }
    if (pthread_mutex_init(&memoria_usuario_mutex, NULL) != 0) {  // MUTEX para memoria de usuario y marcos pues estan fuertemente ligados
        return false;
    }
    if (pthread_mutex_init(&mutex_lista_metricas_procesos, NULL) != 0) {
        return false;
    }
    if (pthread_mutex_init(&mutex_paginas_en_swap, NULL) != 0) {
        return false;
    }
    if (pthread_mutex_init(&mutex_lista_procesos_swap, NULL) != 0) {
        return false;
    }
    if (pthread_mutex_init(&mutex_marcos, NULL) != 0) {
        return false;
    }
    if (pthread_mutex_init(&mutex_swap, NULL) != 0) {
        return false;
    }
    return true;
}
