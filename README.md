# Trabajo Práctico Sistemas Operativos: Episodio III: Revenge of the Cth

Este proyecto, realizado por Jeremias Olschansky, Federico Cancelleri, Sofía Baudo y Camila Hernández Greco. busca simular un sistema operativo distribuido, desarrollado en C. Fue realizado el primer cuatrimestre del año 2025 para la materia: "Sistemas Operativos" de la carrera de Ingeniería en Sistemas de Información en la UTN. 

## 📖 Enunciado

Si querés profundizar más en los requerimientos y requisitos del TP, podés acceder al [enunciado](https://docs.google.com/document/d/1HC9Zi-kpn8jI_egJGEZe77wUCbSkwSw9Ygqqs7m_-is/edit?tab=t.0) donde se explica cada componente y las funcionalidades pedidas de los mismos. 

## 🎯 Objetivo del TP
Los objetivos del proyecto eran los siguientes: 
- Entender como funciona un sistema operativo por dentro.
- Aplicar conceptos como planificación de procesos, sincronización(semaforos) y algoritmos de reemplazo.
- Dominar el lenguaje C
- Adquirir experiencia en la programación en entorno LINUX.

## ▶️ Compilación y ejecución

Cada módulo del proyecto se compila de forma independiente a través de un
archivo `makefile`. Para compilar un módulo, es necesario ejecutar el comando
`make` desde la carpeta correspondiente.

El ejecutable resultante de la compilación se guardará en la carpeta `bin` del
módulo. Ejemplo:

```sh
cd kernel
make
./bin/kernel
```
## 🖥️ Tecnologías Utilizadas
- Sockets para las comunicaciones entre los diferentes módulos.
- Tests para verificar el comportamiento del sistema.
- Concurrencia y sincronización.
- Archivos, logs, scripts.

## 📩 Consultas
Por cualquier consulta, por favor contactar a cualquiera de los integrantes: 
- [Jeremías Olschansky](https://github.com/JeremiasOlschansky) y [Federico Cancelleri](https://github.com/fcancelleri) (Kernel & IO):
- [Camilia Hernández Greco](https://github.com/chernandezgreco) (Memoria):
- [Sofía Baudo](https://github.com/SofiaBaudo) (CPU): 
