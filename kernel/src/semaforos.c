#include <semaforos.h>
//MUTEX

pthread_mutex_t mx_usar_cola_estado[7];
pthread_mutex_t mx_identificador_del_proceso;
pthread_mutex_t mx_avisar_que_entro_el_primero;
pthread_mutex_t mx_proximo_a_consultar;
pthread_mutex_t mx_usar_recurso[2];

//SEMAFOROS

//sem_t INGRESO_DEL_PRIMERO; //POR SI ES NECESARIO.
sem_t INTENTAR_INICIAR;
sem_t CANTIDAD_DE_PROCESOS_EN_NEW;
sem_t CANTIDAD_DE_PROCESOS_EN_READY;
sem_t INGRESO_DEL_PRIMERO_READY;
sem_t CPUS_LIBRES;
sem_t REPLANIFICAR;

//INICIALIZACION DE SEMAFOROS

void inicializar_sincronizacion() {
    pthread_mutex_init(&mx_identificador_del_proceso, NULL);
    pthread_mutex_init(&mx_avisar_que_entro_el_primero,NULL);
    pthread_mutex_init(&mx_proximo_a_consultar,NULL);
     for(int i=0; i<2; i++){
        pthread_mutex_init(&mx_usar_recurso[i],NULL);
    }
    for(int i=0; i<7; i++){
        pthread_mutex_init(&mx_usar_cola_estado[i],NULL);
    }
    //sem_init(&INGRESO_DEL_PRIMERO, 0, 1); // El segundo parámetro es 0 para semáforo binario
    sem_init(&INTENTAR_INICIAR, 0, 0); 
    sem_init(&CANTIDAD_DE_PROCESOS_EN_NEW,0,0);
    sem_init(&CANTIDAD_DE_PROCESOS_EN_READY,0,0);
    sem_init(&INGRESO_DEL_PRIMERO_READY,0,1);
    sem_init(&CPUS_LIBRES,0,0);
    sem_init(&REPLANIFICAR,0,0);
}

