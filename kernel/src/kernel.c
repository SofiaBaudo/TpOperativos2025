#include <kernel.h>
#include <conexiones.h>

int main(int argc, char* argv[]){
   char* archivo = argv[1];
   int tamanio = atoi(argv[2]);
   
 //argc es la cantidad de argumentos y argv que contiene cada argumento
  
   inicializar_kernel(archivo, tamanio);
   inicializar_hilos_de_planificacion();
  


   
}  
  

 

