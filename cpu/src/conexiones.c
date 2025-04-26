//Incluir las librerias

#include <conexiones.h>


// Funcion Iniciar Conexion Kernel
int valor_id;

void iniciar_conexion_kernel_dispatch(int identificador_cpu, t_log* log){
    int fd_conexion_kernel_dispatch = crear_conexion(IP_KERNEL,PUERTO_KERNEL_DISPATCH);
    enviar_op_code(fd_conexion_kernel_dispatch, HANDSHAKE_CPU);                    //avisa que es CPU.
    op_code respuesta = recibir_op_code(fd_conexion_kernel_dispatch);              //recibe un entero que devuelve el kernel cuandola conexion esta hecha.
    if (respuesta == HANDSHAKE_ACCEPTED){
        log_info(log, "Conexion con el kernel dispatch establecida correctamente");
        enviar_id(fd_conexion_kernel_dispatch, identificador_cpu);
    }
    else{
        log_error(log, "Error en la conexion con el kernel dispatch");
        exit(EXIT_FAILURE);
    }
    close(fd_conexion_kernel_dispatch);
}

//void iniciar_conexion_kernel_interrupt(int identificador_cpu){
//    int fd_conexion_kernel_interrupt = crear_conexion(IP_KERNEL,PUERTO_KERNEL_interrupt);
//    enviar_op_code(fd_conexion_kernel_interrupt, HANDSHAKE_CPU);                    //avisa que es CPU.
//    op_code respuesta = recibir_op_code(fd_conexion_kernel_interrupt);              //recibe un entero que devuelve el kernel cuandola conexion esta hecha.
//    if (respuesta == HANDSHAKE_ACCEPTED){
//        log_info(cpu_logger, "Conexion con el kernel dispatch establecida correctamente");
//    }
//    else{
//        log_error(cpu_logger, "Error en la conexion con el kernel dispatch");
//        exit(EXIT_FAILURE);
//    }
//}


// Funcion Iniciar Conexion Memoria

void iniciar_conexion_memoria_dispatch(int identificador_cpu){
    int fd_conexion_dispatch_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
    enviar_op_code(fd_conexion_dispatch_memoria, HANDSHAKE_CPU);                  //avisa que es CPU.
    op_code respuesta = recibir_op_code(fd_conexion_dispatch_memoria);            //recibe un entero que devuelve el kernel cuando la conexion esta hecha.
    if (respuesta == HANDSHAKE_ACCEPTED){
        log_info(cpu_logger ,"Conexion con la memoria establecida correctamente");
    }
    else{
        log_error(cpu_logger, "Error en la conexion con memoria");
        exit(EXIT_FAILURE);
    }
}

//Hacer la funcion de conexion con el kernel interrupt

//cada cpu seria el proceso
void* crear_identificadores(void *arg){
    int id = *(int*)arg;
    printf("el id es ");
    printf("%d", id);
    printf("el valor del id es ");
    printf("%d", id);
    
    t_log* logger;
    //por cada proceso de cpu, se crea un log
    char archivo_log_cpu[50];
    sprintf(archivo_log_cpu, "cpu_%d.log", id);

    char nombre_cpu[200];
    sprintf(nombre_cpu, "cpu_%d", id);
    logger = log_create(archivo_log_cpu, nombre_cpu, true, LOG_LEVEL_INFO);
    if (logger == NULL) {
        printf("No se pudo crear el archivo de log para la CPU %d\n", id);
        exit(1);
    }

    log_info(logger, "Iniciando CPU %d", id);

    //le envia el id al kernel
    iniciar_conexion_kernel_dispatch(id, logger);
    log_destroy(logger);
    pthread_exit(NULL);
}

void enviar_id(int fd_conexion, int identificador_cpu){
    send(fd_conexion, &identificador_cpu, sizeof(identificador_cpu),0);
}
void inicializar_cpus(){
    //valor_id = conseguir_id_ultimo();
    pthread_t hilo_cpu;
    int* id = malloc(sizeof(int));
    *id = valor_id+1;
    valor_id = *id;
    //guardar_id(valor_id);
    pthread_create(&hilo_cpu, NULL, crear_identificadores, id);
    pthread_join(hilo_cpu, NULL);
    pthread_detach(hilo_cpu); 
}
/*
void guardar_id(int id){
    FILE *arc = fopen("archivoGuardarId.txt","w");
    fprintf(arc,"%d", id); //fwrite lo hace en bytes (creo)
    fclose(arc);
}
int conseguir_id_ultimo(){
    FILE *arc = fopen("archivoGuardarId.txt", "r");
    int id;
    fscanf(arc, "%d", &id);
    fclose(arc);
    return id;    
}
*/

