
//Incluimos las librerias.

#include <instrucciones.h>
//Inicializacion de los Loggers


//Declaracion Variables Globales

char* instruccion_recibida;
char** obtenerInsPartes;
char* parametros;
char* nombre_instruccion;
int pid;
int pc;

//Me comunico con el Kernel para obtener el PC/IP y el PID.

void* ejecutar_instrucciones(void* arg){
    //int cpu_id = *(int *)arg;
    instru instru;
    char *instruccionEntera;
    obtenerDelKernelPcPid(&pid, &pc);
    instruccionEntera = fetch(pc,pid);
    instru = decode(instruccionEntera);
    execute(instru, pid);
    check_interrupt(); //ponerlo en hilo.
    return NULL;
}

void obtenerDelKernelPcPid(int *pid, int *pc){
    t_paquete *paquete = recibir_paquete(fd_conexion_kernel_dispatch);
    *pid = deserializar_pid(paquete); 
    *pc = deserializar_pc(paquete);
    if(pc < 0){
    log_error(cpu_logger, "El program Counter no puede ser negativo");
    }
}

//Fase fetch (Buscar proxima instruccion a realizar)(Primer Fase del Ciclo de Instruccion).

char* fetch(int pid,int pc){
    log_info(cpu_logger,"## PID: <PID> - FETCH - Program Counter: <%d>", pc);
    t_buffer *buffer = crear_buffer_cpu(pc, pid);
    crear_paquete(ENVIO_PID_Y_PC, buffer, fd_conexion_dispatch_memoria);
    recv(fd_conexion_dispatch_memoria,&instruccion_recibida,sizeof(instru),0);
    return instruccion_recibida;
}

//Fase Decode (Interpretar proxima ejecucion a ejecutar)(Segunda Fase del Ciclo de Instruccion)

instru decode(char* instruccion_recibida){
    instru instruccion;
    obtenerInsPartes = string_split(instruccion_recibida, " "); //te recibe el string tal como es si no lo encuentra
    instruccion.opcode = obtenerInsPartes[0];
    instruccion.param1 = obtenerInsPartes[1];
    instruccion.param2 = obtenerInsPartes[2];
    return instruccion;  
} 

//Fase Execute (Ejecutar la instruccion)(Tercera Fase del Ciclo de Instruccion).

void execute(instru instruccion, int pid){
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
    int direccionFisica = traduccion(direccion, pid);
    t_buffer *buffer = crear_buffer_pid_dirFis_datos(pid, direccion,param2);
    crear_paquete(ENVIO_PID_DIRFIS_DAT, buffer, fd_conexion_dispatch_memoria);
    log_info(cpu_logger,"## PID: %d - Ejecutando: <WRITE>",pid);
    log_info(cpu_logger,"PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%s>",pid,direccionFisica, param2);
}

//Ejecucion Read.

void instruccion_read(int direccion, char* param2, int pid){
    int direccionFisica = traduccion(direccion, pid);
    t_buffer *buffer = crear_buffer_pid_dirFis_datos(pid, direccion,param2);
    crear_paquete(ENVIO_PID_DIRFIS_DAT, buffer, fd_conexion_dispatch_memoria);
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

//Ejecucion Syscalls

void mandar_syscall(instru instruccion){
    //Hay que serializar
    
    send(fd_conexion_kernel_dispatch, &instruccion, sizeof(instru),0);
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
        //tendria que mandarle a kernel una syscall que se llame desalojo aceptado.
    }
    else{
        log_info(cpu_logger, "NO hay interrupcion");
    }
}
