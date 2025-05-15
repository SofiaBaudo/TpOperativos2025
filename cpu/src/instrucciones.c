
//Incluimos las librerias.

#include <instrucciones.h>

//Me comunico con el Kernel para obtener el PC/IP y el PID.

void ejecutar_instrucciones(void){
    t_instruccion instru;
    char *instruccionEntera;
    obtenerDelKernelPcPid(pid, pc);
    instruccionEntera = fetch(pc);
    instru = decode(instruccionEntera);
    execute(instru);
    check_interrupt();
}

void obtenerDelKernelPcPid(int pid, int pc){
    //Serializar
    recv(fd_conexion_kernel_dispatch, &pid, sizeof(pid),0);
    recv(fd_conexion_kernel_dispatch, &pc, sizeof(pc),0);
    if(pc < 0){
        log_error(logger, "El program Counter no puede ser negativo");
    }
}

//Fase fetch (Buscar proxima instruccion a realizar)(Primer Fase del Ciclo de Instruccion).

char* fetch(int pc,int pid){
    //Mando confirmacion de cpu a memoria, espero la instruccion a realizar de memoria.
    //LOG OBLIGATORIO
    log_info(logger, "## PID: <PID> - FETCH - Program Counter: <%d>", pc);
    send(fd_conexion_dispatch_memoria,&cpu,sizeof(int),0);
    send(fd_conexion_dispatch_memoria,&pc,sizeof(int),0);
    send(fd_conexion_dispatch_memoria,&pid,sizeof(int),0);
    //Ver con Sofi Mandar PID y CPU
    recv(fd_conexion_dispatch_memoria,&instruccion_recibida,sizeof(t_instruccion),0);
    return instruccion_recibida;
}

//Fase Decode (Interpretar proxima ejecucion a ejecutar)(Segunda Fase del Ciclo de Instruccion)

t_instruccion decode(char* instruccion_recibida){
    //Decodifico las instrucciones
    t_instruccion instruccion;
    obtenerInsPartes = string_split(instruccion_recibida, " "); //te recibe el string tal como es si no lo encuentra
    instruccion.opcode = obtenerInsPartes[0];
    instruccion.param1 = obtenerInsPartes[1];
    instruccion.param2 = obtenerInsPartes[2];
    if(strcmp(instruccion.opcode, "WRITE") == 0 || strcmp(instruccion.opcode, "READ") == 0){ //hacer un if en vez array 
    //Llamar a la MMMU para que lo traduzca.
    //hardcodeemos un valor ahora para la prueba, porque se hace en el prox checkpoint
    } 
    return instruccion;  
} 

//Fase Execute (Ejecutar la instruccion)(Tercera Fase del Ciclo de Instruccion).

void execute(t_instruccion instruccion){
    char *nombre_instruccion = instruccion.opcode;
    char *param1 = instruccion.param1;
    char *param2 = instruccion.param2;
    if(strcmp(nombre_instruccion, "NOOP") == 0){
        instruccion_noop();
        pc++;
    }
    if(strcmp(nombre_instruccion, "WRITE") == 0){
        instruccion_write(param1, param2);
        pc++;
    }
    if(strcmp(nombre_instruccion, "READ") == 0){
        instruccion_read(param1, param2);
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
        //ERROR
        log_error(logger, "Error en la Sintaxis o en el ingreso de la Instruccion");
    }
}

//MMU la cantidad de niveles es variable

//Ejecucion Noop.

void instruccion_noop(void){
    log_info(logger,"## PID: %d - Ejecutando: <NOOP>",pid);
}

//Ejecucion Write.

void instruccion_write(char* param1, char* param2){
    log_info(logger,"## PID: %d - Ejecutando: <WRITE>",pid);
    log_info(logger,"PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%s> - Valor: <%s>",pid,param1, param2);
}

//Ejecucion Read.

void instruccion_read(char* param1, char* param2){
    log_info(logger,"## PID: %d - Ejecutando: <READ>",pid);
    log_info(logger,"PID: <%d> - Acción: <LEER> - Dirección Física: <%s> - Valor: <%s>",pid,param1,param2);
}

//Ejecucion Go to.
//Hay un loop infinito con los go to

void instruccion_goto(char *parametro){
   //Accedo a Memoria y Kernel para actualizar el PC
    int valorACambiar = atoi(parametro);
    pc = valorACambiar;
    log_info(logger,"## PID: %d - Ejecutando: <GOTO> - <%s>",pid, parametro);
    send(fd_conexion_dispatch_memoria, &pc, sizeof(int), 0);
    send(fd_conexion_kernel_dispatch, &pc,sizeof(int),0);
}

//Ejecucion Syscalls

void mandar_syscall(t_instruccion instruccion){
    //Hay que serializar
    send(fd_conexion_kernel_dispatch, &instruccion, sizeof(t_instruccion),0);
}

//Chequear Interrupcion

void check_interrupt(void){
    int pid_interrupcion=0;
    recv(fd_conexion_kernel_interrupt, &pid_interrupcion,sizeof(int),0);
    log_info(logger," ## Llega interrupción al puerto Interrupt <%d>", pid_interrupcion);
    if(pid_interrupcion != 0){ //recibio una instruccion
        //Hay que Serializar 
        send(fd_conexion_kernel_interrupt, &pid, sizeof(int),0);
        send(fd_conexion_kernel_interrupt, &pc, sizeof(int),0);
        log_info(logger, "SI hay interrupcion");
    }
    else{
        log_info(logger, "NO hay interrupcion");
    }
}
