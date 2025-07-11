
//Incluimos las librerias.

#include <instrucciones.h>
#include <cache.h>
#include <traduccion.h>
//Inicializacion de los Loggers


//Declaracion Variables Globales

char** obtenerInsPartes;
char* parametros;
char* nombre_instruccion;
int pid;
int pc;
//bool mande_syscall;
//Me comunico con el Kernel para obtener el PC/IP y el PID.

void* ejecutar_instrucciones(void* arg){
    //hacer un semaforo donde se verifica que llega un proceso. espera que llegue un proceso --> (pensar bien donde)
    //int cpu_id = *(int *)arg;
    log_debug(cpu_log_debug,"INICIANDO CICLO");
    t_instruccion instru;
    char *instruccionEntera;
    if(tengo_que_solicitar_pid_y_pc){
        obtenerDelKernelPcPid();
        log_debug(cpu_log_debug,"ESTOY ACA ADENTRO DEL IF DE OBTENER PID Y PC");
        log_debug(cpu_log_debug, "obtuve el pid y el pc");  
    } 
    log_debug(cpu_log_debug, "el pc es: %i", pc);
    instruccionEntera = fetch(pid,pc);
    log_debug(cpu_log_debug,"La instruccion que llego es: %s",instruccionEntera);
    log_debug(cpu_log_debug, "finalizo el fetch");    
    instru = decode(instruccionEntera);
    log_debug(cpu_log_debug,"La instruccion que devolvio: %s",instru.opcode);
    log_debug(cpu_log_debug,"terminado el split de decode");
    execute(instru, pid);
    log_debug(cpu_log_debug, "mande la syscall?");
    check_interrupt(); //ponerlo en hilo.    
    return NULL;
}

void obtenerDelKernelPcPid(){
    //t_paquete *paquete = malloc(sizeof(t_paquete)); 
    tengo_que_solicitar_pid_y_pc = false;
    enviar_op_code(fd_conexion_kernel_dispatch,CPU_LISTA);
    t_paquete *paquete = recibir_paquete(fd_conexion_kernel_dispatch);
    deserializar_pid_y_pc(paquete,&pid,&pc);
    log_debug(cpu_log_debug,"EL pid es %i", pid);
    log_debug(cpu_log_debug, "El pc es %i", pc);
    if(pc < 0){
    log_error(cpu_logger, "El PC no puede ser negativo");
    }
}

//Fase fetch (Buscar proxima instruccion a realizar)(Primer Fase del Ciclo de Instruccion).

char* fetch(int pid,int pc){
    log_info(cpu_logger,"## PID: <%d> - FETCH - Program Counter: <%d>",pid, pc);
    //enviar_op_code(FETCH_INSTRUCCION,fd_conexion_dispatch_memoria);
    //recibir_op_code(fd_conexion_dispatch_memoria);
    t_buffer *buffer = crear_buffer_cpu(pid, pc);
    crear_paquete(FETCH_INSTRUCCION, buffer, fd_conexion_dispatch_memoria);
    log_debug(cpu_log_debug, "el pid es %i",pid);
    log_debug(cpu_log_debug, "el pc es %i",pc);
    log_debug(cpu_log_debug, "por recibir instru");
    t_paquete *paquete = recibir_paquete(fd_conexion_dispatch_memoria);
    char *instruccion_recibida = deserializar_nombre_instruccion(paquete);
    log_debug(cpu_log_debug, "instruccion recibida %s", instruccion_recibida);
    usleep(10000000);
    return instruccion_recibida;
}


//Fase Decode (Interpretar proxima ejecucion a ejecutar)(Segunda Fase del Ciclo de Instruccion)

t_instruccion decode(char* instruccion_recibida){  
    t_instruccion instruccion;
    obtenerInsPartes = string_split(instruccion_recibida, " "); //te recibe el string tal como es si no lo encuentra
    instruccion.opcode = obtenerInsPartes[0];
    instruccion.param1 = obtenerInsPartes[1];
    log_debug(cpu_log_debug, "primer parametro de decode : %s", instruccion.param1);
    if(strcmp(instruccion.opcode, "GOTO") == 0 || strcmp(instruccion.opcode, "EXIT") == 0 || strcmp(instruccion.opcode, "DUMP MEMORY") == 0 || strcmp(instruccion.opcode, "NOOP") == 0 ){
        instruccion.param2 = " ";
        log_debug(cpu_log_debug, "adentro del if de decode");
    }  
    else{
        instruccion.param2 = obtenerInsPartes[2];
        log_debug(cpu_log_debug, "el segundo parametro del decode es : %s", instruccion.param2);
    }
    
    return instruccion;  
} 
//Fase Execute (Ejecutar la instruccion)(Tercera Fase del Ciclo de Instruccion).

void execute(t_instruccion instruccion, int pid){
    char *nombre_instruccion = instruccion.opcode;
    char *param1Char = instruccion.param1;
    int param1 = -5; 
    log_debug(cpu_log_debug, "el primer parametro es %s", param1Char);
    if(strcmp(param1Char," ")!=0){
    param1 = atoi(param1Char);
    }
    char *param2 = instruccion.param2;
    log_debug(cpu_log_debug,"el segundo parametro es %s", param2);
    if(strcmp(nombre_instruccion, "NOOP") == 0){
        instruccion_noop();
        pc++;
    }
    else if(strcmp(nombre_instruccion, "WRITE") == 0){
        instruccion_write(param1, param2, pid);
        pc++;
    }
    else if(strcmp(nombre_instruccion, "READ") == 0){
        instruccion_read(param1, param2, pid);
        pc++;
        log_debug(cpu_log_debug,"EL PC AHORA ES: %i",pc);
    } 
    else if(strcmp(nombre_instruccion, "GOTO") == 0){   
        instruccion_goto(param1);
    }
    else if(strcmp(nombre_instruccion, "INIT_PROC") == 0 || strcmp(nombre_instruccion, "EXIT") == 0 || strcmp(nombre_instruccion, "DUMP_MEMORY") == 0 || strcmp(nombre_instruccion, "IO") == 0){
        log_debug(cpu_log_debug,"La instruccion a enviar es: %s",nombre_instruccion);
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
    crear_paquete(WRITE_MEMORIA, buffer, fd_conexion_dispatch_memoria);
    }
    op_code esperar_a_memoria = recibir_op_code(fd_conexion_dispatch_memoria);
    log_info(cpu_logger,"## PID: %d - Ejecutando: <WRITE>",pid);
    log_info(cpu_logger,"PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%s>",pid,direccionFisica, param2);
}

//Ejecucion Read.

void instruccion_read(int direccion, char* param2, int pid){
    int direccionFisica = traduccion(direccion, pid, "READ", param2);
    log_debug(cpu_log_debug,"Termine la traduccion");
    log_debug(cpu_log_debug,"Direccion Fisica : %i",direccionFisica);
    char* valor_leido;
    if(direccionFisica == -1){
        //entro por cache
    }
    else{
        log_debug(cpu_log_debug,"Estoy en el else definitivo");
        t_buffer *buffer = crear_buffer_pid_dirFis_datos(pid, direccion,param2);
        log_debug(cpu_log_debug, "cre el buffer, ahora voy a enviar el paquete");
        crear_paquete(ENVIO_PID_DIRFIS_DAT, buffer, fd_conexion_dispatch_memoria);
        log_debug(cpu_log_debug,"Ya mande el paquete");
        t_paquete *paquete = recibir_paquete(fd_conexion_dispatch_memoria);
        valor_leido = deserializar_char_asterisco(paquete);
    }
    log_info(cpu_logger,"## PID: %d - Ejecutando: <READ>",pid);
    log_info(cpu_logger,"PID: <%d> - Acción: <LEER> - Dirección Física: <%d> - Valor: <%s>",pid,direccion,valor_leido);
}

void instruccion_goto(int parametro){
   //Accedo a Memoria y Kernel para actualizar el PC
    pc = parametro;
    log_info(cpu_logger,"## PID: %d - Ejecutando: <GOTO> - <%d>",pid, parametro);
    send(fd_conexion_dispatch_memoria, &pc, sizeof(int), 0);
    send(fd_conexion_kernel_dispatch, &pc,sizeof(int),0);
}

void mandar_syscall(t_instruccion instruccion){
    tengo_que_solicitar_pid_y_pc = true;
    log_debug(cpu_log_debug,"Estoy por mandar syscall");
    if(strcmp(instruccion.opcode, "INIT_PROC") == 0){
        int tamanio = atoi(instruccion.param2);
        log_debug(cpu_log_debug,"El pc que estoy por mandar es: %i",pc);
        t_buffer *buffer = crear_buffer_instruccion_init_proc(instruccion.param1, tamanio, &pid, &pc);
        crear_paquete(INIT_PROC,buffer,fd_conexion_kernel_dispatch);
        pc++;
        log_debug(cpu_log_debug,"ESTOY POR ENVIARLE EL INIT PROC A KERNEL");
        recibir_op_code(fd_conexion_kernel_dispatch);
        return;
    }
    else if(strcmp(instruccion.opcode, "EXIT") == 0){
        //t_buffer *buffer = crear_buffer_vacio();
        t_buffer* buffer = crear_buffer_cpu(pid, pc);
        crear_paquete(EXIT, buffer, fd_conexion_kernel_dispatch);
        pc++;
        log_debug(cpu_log_debug,"LE ENVIE EL EXIT A KERNEL");
        recibir_op_code(fd_conexion_kernel_dispatch); //este opcode avisa que memoria ya autorizó que el proceso termine
        desalojarProcesoTLB(pid);
        desalojarProcesoCache(pid);
        return;
    }
    else if(strcmp(instruccion.opcode, "IO") == 0){
        int milisegundos = atoi(instruccion.param2);
        t_buffer *buffer = crear_buffer_instruccion_io(instruccion.param1,milisegundos,&pid,&pc);
        crear_paquete(IO, buffer, fd_conexion_kernel_dispatch);
        pc++;
        return;
    }
    else if(strcmp(instruccion.opcode, "DUMP_MEMORY") == 0){
        t_buffer *buffer = crear_buffer_vacio();
        crear_paquete(DUMP_MEMORY, buffer, fd_conexion_kernel_dispatch);
        pc++;
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
