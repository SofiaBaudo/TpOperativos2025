
//Incluimos las librerias.

#include <instrucciones.h>
#include <cache.h>
#include <traduccion.h>
//Inicializacion de los Loggers


//Declaracion Variables Globales

char* instruccion_recibida;
char** obtenerInsPartes;
char* parametros;
char* nombre_instruccion;
int pid = 1;
int pc = 2;

//Me comunico con el Kernel para obtener el PC/IP y el PID.

void* ejecutar_instrucciones(void* arg){
    //hacer un semaforo donde se verifica que llega un proceso. espera que llegue un proceso --> (pensar bien donde)
    //int cpu_id = *(int *)arg;
    t_instruccion instru;
    char *instruccionEntera;
    //obtenerDelKernelPcPid();
    //log_debug(cpu_log_debug,"el pid es %i", pid);
    //log_debug(cpu_log_debug, "el pc es %i", pc);
    /*
    instru.opcode = "INIC_PROC";
    instru.param1 = "holahola";
    instru.param2 = "4";

    mandar_syscall(instru);
    */
       
    instruccionEntera = fetch(pc,pid);
    /*
    instru = decode(instruccionEntera);
    execute(instru, pid);
    check_interrupt(); //ponerlo en hilo.
    return NULL;
    */
    
    return NULL;
}

void obtenerDelKernelPcPid(){
    t_paquete *paquete = malloc(sizeof(t_paquete)); 
    paquete = recibir_paquete(fd_conexion_kernel_dispatch);
    deserializar_pid_y_pc(paquete,&pid,&pc);
    if(pc < 0){
    log_error(cpu_logger, "El PC no puede ser negativo");
    }
}

//Fase fetch (Buscar proxima instruccion a realizar)(Primer Fase del Ciclo de Instruccion).

char* fetch(int pid,int pc){
    log_info(cpu_logger,"## PID: <%d> - FETCH - Program Counter: <%d>",pid, pc);
    t_buffer *buffer = crear_buffer_cpu(pc, pid);
    crear_paquete(FETCH_INSTRUCCION, buffer, fd_conexion_dispatch_memoria);
    recv(fd_conexion_dispatch_memoria,&instruccion_recibida,sizeof(char*),0);
    log_debug(cpu_log_debug, "instruccion recibida %c", instruccion_recibida);
    return instruccion_recibida;
}

//Fase Decode (Interpretar proxima ejecucion a ejecutar)(Segunda Fase del Ciclo de Instruccion)

t_instruccion decode(char* instruccion_recibida){  
    t_instruccion instruccion;
    obtenerInsPartes = string_split(instruccion_recibida, " "); //te recibe el string tal como es si no lo encuentra
    instruccion.opcode = obtenerInsPartes[0];
    instruccion.param1 = obtenerInsPartes[1];
    instruccion.param2 = obtenerInsPartes[2];
    return instruccion;  
} 

//Fase Execute (Ejecutar la instruccion)(Tercera Fase del Ciclo de Instruccion).

void execute(t_instruccion instruccion, int pid){
    char *nombre_instruccion = instruccion.opcode;
    char *param1Char = instruccion.param1;
    int param1 = atoi(param1Char);
    char *param2 = instruccion.param2;
    if(strcmp(nombre_instruccion, "NOOP") == 0){
        instruccion_noop();
        pc++;
    }
    if(strcmp(nombre_instruccion, "WRITE") == 0){
        instruccion_write(param1, param2, pid);
        pc++;
    }
    if(strcmp(nombre_instruccion, "READ") == 0){
        instruccion_read(param1, param2, pid);
        pc++;
    } 
    if(strcmp(nombre_instruccion, "GOTO") == 0){   
        instruccion_goto(param1);
    }
    if(strcmp(nombre_instruccion, "INIC_PROC") == 0 || strcmp(nombre_instruccion, "EXIT") == 0 || strcmp(nombre_instruccion, "DUMP_MEMORY") == 0 || strcmp(nombre_instruccion, "IO") == 0){
        mandar_syscall(instruccion);
        pc++;
    }
    else{
        log_error(cpu_logger, "Error en la Sintaxis o en el ingreso de la Instruccion");
    }
}


//Ejecucion Noop.

void instruccion_noop(void){
    log_info(logger,"## PID: %d - Ejecutando: <NOOP>",pid);
}

//Ejecucion Write.

void instruccion_write(int direccion, char* param2, int pid){
    int direccionFisica =  traduccion(direccion,pid, "WRITE", param2);
    if(direccionFisica == -1){
        //entro por cache
    }
    else{
    t_buffer *buffer = crear_buffer_pid_dirFis_datos(pid, direccion,param2);
    crear_paquete(ENVIO_PID_DIRFIS_DAT, buffer, fd_conexion_dispatch_memoria);
    }
    log_info(cpu_logger,"## PID: %d - Ejecutando: <WRITE>",pid);
    log_info(cpu_logger,"PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%s>",pid,direccionFisica, param2);
}

//Ejecucion Read.

void instruccion_read(int direccion, char* param2, int pid){
    int direccionFisica = traduccion(direccion, pid, "READ", param2);
    if(direccionFisica == -1){
        //entro por cache
    }
    else{
    t_buffer *buffer = crear_buffer_pid_dirFis_datos(pid, direccion,param2);
    crear_paquete(ENVIO_PID_DIRFIS_DAT, buffer, fd_conexion_dispatch_memoria);
    }
    log_info(cpu_logger,"## PID: %d - Ejecutando: <READ>",pid);
    log_info(cpu_logger,"PID: <%d> - Acción: <LEER> - Dirección Física: <%d> - Valor: <%s>",pid,direccion,param2);
}

void instruccion_goto(int parametro){
   //Accedo a Memoria y Kernel para actualizar el PC
    pc = parametro;
    log_info(cpu_logger,"## PID: %d - Ejecutando: <GOTO> - <%d>",pid, parametro);
    send(fd_conexion_dispatch_memoria, &pc, sizeof(int), 0);
    send(fd_conexion_kernel_dispatch, &pc,sizeof(int),0);
}

void mandar_syscall(t_instruccion instruccion){
    log_debug(cpu_log_debug,"Estoy por mandar syscall");
    if(strcmp(instruccion.opcode, "INIC_PROC") == 0){
        int tamanio = atoi(instruccion.param2);
        t_buffer *buffer = crear_buffer_instruccion_init_proc(instruccion.param1, tamanio, &pid, &pc);
        crear_paquete(INIT_PROC,buffer,fd_conexion_kernel_dispatch);
        return;
    }
    else if(strcmp(instruccion.opcode, "EXIT") == 0){
    t_buffer *buffer = crear_buffer_vacio();
    crear_paquete(EXIT, buffer, fd_conexion_kernel_dispatch);
    return;
    }
    else if(strcmp(instruccion.opcode, "IO") == 0){
        int milisegundos = atoi(instruccion.param2);
        t_buffer *buffer = crear_buffer_instruccion_io(instruccion.param1,milisegundos,&pid,&pc);
        crear_paquete(IO, buffer, fd_conexion_kernel_dispatch);
        return;
    }
    else if(strcmp(instruccion.opcode, "DUMP_MEMORY") == 0){
        t_buffer *buffer = crear_buffer_vacio();
        crear_paquete(DUMP_MEMORY, buffer, fd_conexion_kernel_dispatch);
        return;
    }
}

//Chequear Interrupcion
void check_interrupt(void){
    int pid_interrupcion=0;
    recv(fd_conexion_kernel_interrupt, &pid_interrupcion,sizeof(int),0);
    log_info(cpu_logger," ## Llega interrupción al puerto Interrupt <%d>", pid_interrupcion);
    if(pid_interrupcion != 0){ //recibio una interrupcion
        //Hay que Serializar
        t_buffer *buffer = crear_buffer_cpu(pc, pid);
        crear_paquete(ENVIO_PID_Y_PC,buffer, fd_conexion_kernel_dispatch); 
        log_info(cpu_logger, "SI hay interrupcion");
        //hay que desalojar el proceso => en el tlb y en cache. 
        //tendria que mandarle a kernel una syscall que se llame desalojo aceptado.
    }
    else{
        log_info(cpu_logger, "NO hay interrupcion");
    }
}
