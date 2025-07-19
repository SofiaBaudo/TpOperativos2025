
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
    log_debug(cpu_log_debug, "EL VALOR DE TENGO QUE SOLICITAR PID Y PC ES: %i", tengo_que_solicitar_pid_y_pc);
    if(tengo_que_solicitar_pid_y_pc){
        obtenerDelKernelPcPid(); 
        log_debug(cpu_log_debug, "EL PC DEL KERNEL ES %i", pc);
    } 
    log_debug(cpu_log_debug, "POR HACER EL FETCH CON PC: %i", pc);
    instruccionEntera = fetch(pid,pc); 
    instru = decode(instruccionEntera);
    execute(instru, pid);
    check_interrupt(); //ponerlo en hilo.    
    return NULL;
}

void obtenerDelKernelPcPid(){
    //t_paquete *paquete = malloc(sizeof(t_paquete)); 
    tengo_que_solicitar_pid_y_pc = false;
    enviar_op_code(fd_conexion_kernel_dispatch,CPU_LISTA);
    t_paquete *paquete = recibir_paquete(fd_conexion_kernel_dispatch);
    deserializar_pid_y_pc(paquete,&pid,&pc);
    if(pc < 0){
    log_error(cpu_logger, "El PC no puede ser negativo");
    }
}

//Fase fetch (Buscar proxima instruccion a realizar)(Primer Fase del Ciclo de Instruccion).

char* fetch(int pid,int pc){
    log_info(cpu_logger,"## PID: <%d> - FETCH - Program Counter: <%d>",pid, pc);
    t_buffer *buffer = crear_buffer_cpu(pid, pc);
    crear_paquete(FETCH_INSTRUCCION, buffer, fd_conexion_dispatch_memoria);
    log_debug(cpu_log_debug, "ENVIE EL PAQUETE CON pc %i", pc);
    t_paquete *paquete = recibir_paquete(fd_conexion_dispatch_memoria);
    char *instruccion_recibida = deserializar_nombre_instruccion(paquete);
    log_debug(cpu_log_debug, "la instrucciones %s", instruccion_recibida);
    return instruccion_recibida;
}


//Fase Decode (Interpretar proxima ejecucion a ejecutar)(Segunda Fase del Ciclo de Instruccion)

t_instruccion decode(char* instruccion_recibida){  
    t_instruccion instruccion;
    obtenerInsPartes = string_split(instruccion_recibida, " "); //te recibe el string tal como es si no lo encuentra
    instruccion.opcode = obtenerInsPartes[0];
    instruccion.param1 = obtenerInsPartes[1];
    if(strcmp(instruccion.opcode, "GOTO") == 0 || strcmp(instruccion.opcode, "EXIT") == 0 || strcmp(instruccion.opcode, "DUMP_MEMORY") == 0 || strcmp(instruccion.opcode, "NOOP") == 0 ){
        instruccion.param2 = " ";
    }  
    else{
        instruccion.param2 = obtenerInsPartes[2];
    }
    
    return instruccion;  
} 
//Fase Execute (Ejecutar la instruccion)(Tercera Fase del Ciclo de Instruccion).

void execute(t_instruccion instruccion, int pid){
    char *nombre_instruccion = instruccion.opcode;
    char *param1Char = instruccion.param1;
    int param1;
    if(param1Char){ // que sea distinto de NULL
    param1 = atoi(param1Char);
    }
    char *param2 = instruccion.param2;
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
    } 
    else if(strcmp(nombre_instruccion, "GOTO") == 0){   
        instruccion_goto(param1);
    }
    else if(strcmp(nombre_instruccion, "INIT_PROC") == 0 || strcmp(nombre_instruccion, "EXIT") == 0 || strcmp(nombre_instruccion, "DUMP_MEMORY") == 0 || strcmp(nombre_instruccion, "IO") == 0){
        
        mandar_syscall(instruccion);
        pc++;
    }
    else{
        log_error(cpu_logger, "Error en la Sintaxis o en el ingreso de la Instruccion");
    }
}


//Ejecucion Noop.

void instruccion_noop(void){
    log_info(cpu_logger,"## PID: %d - Ejecutando: <NOOP>",pid);
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
    char* valor_leido;
    if(direccionFisica == -1){
        //entro por cache
    }
    else{
        t_buffer *buffer = crear_buffer_pid_dirFis_datos(pid, direccion,param2);
        crear_paquete(ENVIO_PID_DIRFIS_DAT, buffer, fd_conexion_dispatch_memoria);
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
}

void mandar_syscall(t_instruccion instruccion){
    tengo_que_solicitar_pid_y_pc = true;
    log_debug(cpu_log_debug, "la instruccion es %s", instruccion.opcode);
    if(strcmp(instruccion.opcode, "INIT_PROC") == 0){
        log_debug(cpu_log_debug, "entre al if");
        int tamanio = atoi(instruccion.param2);
        t_buffer *buffer = crear_buffer_instruccion_init_proc(instruccion.param1, tamanio, &pid, &pc);
        crear_paquete(INIT_PROC,buffer,fd_conexion_kernel_dispatch);
        log_debug(cpu_log_debug, "termine de mandar el paquete");
        log_debug(cpu_log_debug, "el opcode es %s", instruccion.param1);
        pc++;
        recibir_op_code(fd_conexion_kernel_dispatch);
        return;
    }
    else if(strcmp(instruccion.opcode, "EXIT") == 0){
        //t_buffer *buffer = crear_buffer_vacio();
        t_buffer* buffer = crear_buffer_cpu(pid, pc);
        crear_paquete(EXIT, buffer, fd_conexion_kernel_dispatch);
        pc++;
        ultima_instruccion_fue_syscall_bloqueante = true;
        desalojarProcesoTLB();
        desalojarProcesoCache(pid);
        recibir_op_code(fd_conexion_kernel_dispatch); //este opcode avisa que memoria ya autorizó que el proceso termine
        return;
    }
    else if(strcmp(instruccion.opcode, "IO") == 0){
        int milisegundos = atoi(instruccion.param2);
        t_buffer *buffer = crear_buffer_instruccion_io(instruccion.param1,milisegundos,&pid,&pc);
        crear_paquete(IO, buffer, fd_conexion_kernel_dispatch);
        pc++;
        ultima_instruccion_fue_syscall_bloqueante = true;
        return;
    }
    else if(strcmp(instruccion.opcode, "DUMP_MEMORY") == 0){
        t_buffer *buffer = crear_buffer_cpu(pid,pc);
        crear_paquete(DUMP_MEMORY, buffer, fd_conexion_kernel_dispatch);
        pc++;
        ultima_instruccion_fue_syscall_bloqueante = true;
        recibir_op_code(fd_conexion_kernel_dispatch);
        return;
    }
}

//Chequear Interrupcion
void check_interrupt(void){
    if(hayInterrupcion && !ultima_instruccion_fue_syscall_bloqueante){ //recibio una interrupcion
        t_buffer *buffer = crear_buffer_cpu(pid, pc);
        crear_paquete(DESALOJO_ACEPTADO, buffer,fd_conexion_kernel_dispatch); 
        tengo_que_solicitar_pid_y_pc = true;
        pthread_mutex_lock(&mx_interrupcion);
        hayInterrupcion = false;
        pthread_mutex_unlock(&mx_interrupcion);
        ultima_instruccion_fue_syscall_bloqueante = false;
        op_code respuesta = recibir_op_code(fd_conexion_kernel_dispatch);
    }
    else if(hayInterrupcion){
        pthread_mutex_lock(&mx_interrupcion);
        hayInterrupcion = false;
        pthread_mutex_unlock(&mx_interrupcion);
    }
    else if(ultima_instruccion_fue_syscall_bloqueante){
        ultima_instruccion_fue_syscall_bloqueante = false;
    }
    else{
        log_info(cpu_logger, "No hay interrupcion");
    }
}

void* esperar_interrupcion(){
    while(1){
    op_code solicitud_de_desalojo = recibir_op_code(fd_conexion_kernel_interrupt);
    pthread_mutex_lock(&mx_interrupcion);
    hayInterrupcion = true;
    pthread_mutex_unlock(&mx_interrupcion);
    log_info(cpu_logger," ## Llega interrupción al puerto Interrupt <por desalojo>");
    }
    return NULL;
}

