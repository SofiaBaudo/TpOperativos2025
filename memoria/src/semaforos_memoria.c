#include <semaforos_memoria.h>

pthread_mutex_t mutex_procesos_en_memoria;
pthread_mutex_t memoria_usuario_mutex;

// Inicializa todos los mutex/semaforos globales del sistema de memoria
bool inicializar_mutex(void) {
    if (pthread_mutex_init(&mutex_procesos_en_memoria, NULL) != 0) {
        return false;
    }
    if (pthread_mutex_init(&memoria_usuario_mutex, NULL) != 0) {  // MUTEX para memoria de usuario y marcos pues estan fuertemente ligados
        return false;
    }
    return true;
}
