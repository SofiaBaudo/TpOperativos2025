#include <semaforos.h>

pthread_mutex_t mx_identificador_del_proceso;
pthread_mutex_t mx_agregar_a_cola_new;
pthread_mutex_t mx_avisar_que_entro_el_primero;
sem_t INGRESO_DEL_PRIMERO;
sem_t CANTIDAD_DE_PROCESOS_EN_NEW;
sem_t USAR_COLA_NEW;
void inicializar_sincronizacion() {
    pthread_mutex_init(&mx_identificador_del_proceso, NULL);
    pthread_mutex_init(&mx_agregar_a_cola_new,NULL);
    pthread_mutex_init(&mx_avisar_que_entro_el_primero,NULL);
    sem_init(&INGRESO_DEL_PRIMERO, 0, 1); // El segundo parámetro es 0 para semáforo binario
    sem_init(&CANTIDAD_DE_PROCESOS_EN_NEW,0,0);
    sem_init(&USAR_COLA_NEW,0,1);
}

