#include <instrucciones.h>

void execute(instruccion instruccion_recibida){
switch(instruccion_recibida)
{
    case NOOP:
    instruccion_noop();
    break;
    case WRITE:
    instruccion_write();
    break;
    case READ:
    instruccion_read();
    break;
    case GOTO:
    instruccion_goto();
    break;
    case IO:
    instruccion_io();
    break;
    case INIT_PROC:
    instruccion_initproc();
    break;
    case DUMP_MEMORY:
    instruccion_dumpmemory();
    break;
    case EXIT:
    instruccion_exit();
    break;
default:

    break;
}

}

void instruccion_noop(){

}

void instruccion_write(){

}

void instruccion_read(){

}

void instruccion_goto(){

}

void instruccion_io(){

}

void instruccion_initproc(){

}

void instruccion_dumpmemory(){

}

void instruccion_exit(){

}

// Verificar en el utils Enum de Instrucciones

instruccion fetch(int pc){
    send(fd_conexion_dispatch_memoria,&pc,sizeof(int),0);
    instruccion instruccion_recibida;
    recv(fd_conexion_dispatch_memoria,&instruccion_recibida,sizeof(instruccion),0);
    return instruccion_recibida;  

void obtenerDelKernelPcPid(t_log* log, int pid, int pc){
    recv(fd_kernel, &pid, sizeof(pid),0);
    recv(fd_kernel, &pc, sizeof(pc),0);
    if(pc < 0){
        log_error(log, "El program Counter no puede ser negativo");
    }
    log_info(log, "## PID: <PID> - FETCH - Program Counter: <%d>", pc );
}