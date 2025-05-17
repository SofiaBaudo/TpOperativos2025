#include <semaforos.h>

pthread_mutex_t mx_identificador_del_proceso;
pthread_mutex_t mx_avisar_que_entro_el_primero;
pthread_mutex_t mx_usar_cola_new;
pthread_mutex_t mx_usar_cola_ready;
sem_t INGRESO_DEL_PRIMERO;
sem_t CANTIDAD_DE_PROCESOS_EN_NEW;
sem_t CANTIDAD_DE_PROCESOS_EN_READY;
sem_t INGRESO_DEL_PRIMERO_READY;
void inicializar_sincronizacion() {
    pthread_mutex_init(&mx_identificador_del_proceso, NULL);
    pthread_mutex_init(&mx_avisar_que_entro_el_primero,NULL);
    pthread_mutex_init(&mx_usar_cola_new,NULL);
    pthread_mutex_init(&mx_usar_cola_ready,NULL);
    sem_init(&INGRESO_DEL_PRIMERO, 0, 1); // El segundo parámetro es 0 para semáforo binario
    sem_init(&CANTIDAD_DE_PROCESOS_EN_NEW,0,0);
    sem_init(&CANTIDAD_DE_PROCESOS_EN_READY,0,0);
    sem_init(&INGRESO_DEL_PRIMERO_READY,0,1);
}

