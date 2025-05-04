
//Incluimos las librerias.

#include <instrucciones.h>


//Me comunico con el Kernel para obtenerr el PC/IP y el PID.

void obtenerDelKernelPcPid(t_log* log, int pid, int pc){
    recv(fd_conexion_kernel_dispatch, &pid, sizeof(pid),0);
    recv(fd_conexion_kernel_dispatch, &pc, sizeof(pc),0);
    if(pc < 0){
        log_error(log, "El program Counter no puede ser negativo");
    }
    log_info(log, "## PID: <PID> - FETCH - Program Counter: <%d>", pc);
}
//Fase fecth (Buscar proxima instruccion a realizar)(Primer Fase del Ciclo de Instruccion).

char* fetch(int pc){
    //Mando confirmacion de cpu a memoria, espero la instruccion a realizar de memoria.
    send(fd_conexion_dispatch_memoria,&pc,sizeof(int),0);
    recv(fd_conexion_dispatch_memoria,&instruccion_recibida,sizeof(t_instruccion),0);
    return instruccion_recibida;
}

//Fase Decode (Interpretar proxima ejecucion a ejecutar)(Segunda Fase del Ciclo de Instruccion)

void decode(char* instruccion_recibida){
    //Decodifico las instrucciones
    t_instruccion instruccion;
    obtenerInsPartes = string_split(instruccion_recibida, " "); //te recibe el string tal como es si no lo encuentra
    instruccion.opcode = obtenerInsPartes[0];
    instruccion.param1 = obtenerInsPartes[1];
    instruccion.param2 = obtenerInsPartes[2];
    
    if(strcmp(instruccion.opcode, "WRITE") == 0 || strcmp(instruccion.opcode, "READ") == 0){ //hacer un if en vez array 
    //Llamar a la MMMU para que lo traduzca.
    //hardcodeemos un valor ahora para la pruba, porque se hace en el prox checkpoint
    }   
} 

//Fase Execute (Ejecutar la instruccion)(Tercera Fase del Ciclo de Instruccion).

void execute(t_instruccion instruccion){
    char *nombre_instruccion = instruccion.opcode;
    char *param1 = instruccion.param1;
    char *param2 = instruccion.param1;
    
    if(strcmp(nombre_instruccion, "NOOP") == 0){
        instruccion_noop();
    }
    else if(strcmp(nombre_instruccion, "WRITE") == 0){
        instruccion_write();
    }
    else if(strcmp(nombre_instruccion, "READ") == 0){
        instruccion_read();
    } 
    else if(strcmp(nombre_instruccion, "GOTO") == 0){   
        instruccion_goto();
    }
    else if(strcmp(nombre_instruccion, "IO") == 0){
        mandar_syscall(instruccion);
    }
    else if(strcmp(nombre_instruccion, "INIC_PROC") == 0){
        mandar_syscall(instruccion);
    }
    else if(strcmp(nombre_instruccion, "DUMP_MEMORY") == 0){
        mandar_syscall(instruccion);
    }
    else if(strcmp(nombre_instruccion, "EXIT") == 0){
        mandar_syscall(instruccion);
    }
    else{
        //ERROR
    }
}

//MMU la cantidad de niveles es variable
//Hay un loop infinito con los go to

//Ejecucion Noop.

void instruccion_noop(void){
    //No hace nada, no se debe poner nada aca (Solo tiempo para dirrecionar memoria).
}

//Ejecucion Write.

void instruccion_write(){
    //Acceder a memoria y escribir.
}

//Ejecucion Read.

void instruccion_read(){
    //Acceder a memoria y leer.
}

//Ejecucion Go to.

void instruccion_goto(){
    //Actualizar por parametro el IP/PC go to.
}

//Ejecucion Syscalls

void mandar_syscall(t_instruccion instruccion){
    send(fd_conexion_kernel_dispatch, &instruccion, sizeof(t_instruccion),0);
}

//Chequear Interrupcion

void check_interrupt(){
    //
}

