#include <semaforos.h>

pthread_mutex_t mx_identificador_del_proceso;
sem_t INGRESO_DEL_PRIMERO;
sem_t CANTIDAD_DE_PROCESOS_EN_NEW;
void inicializar_sincronizacion() {
    pthread_mutex_init(&mx_identificador_del_proceso, NULL);
    sem_init(&INGRESO_DEL_PRIMERO, 0, 0); // El segundo parámetro es 0 para semáforo binario
    sem_init(&CANTIDAD_DE_PROCESOS_EN_NEW,0,0);
}

