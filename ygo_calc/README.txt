YU-GI-OH! DUEL CALCULATOR (SDL2 - goodluckOS / kennel)
========================================================

Estructura del paquete:
- run.sh       : Lanzador configurado con kmsdrm, LD_LIBRARY_PATH y eventos de mando en segundo plano.
- src/main.cpp : Código fuente completo en C++ / SDL2 (sin dependencias externas de fuentes TTF).
- Makefile     : Compilación y empaquetado automático de librerías en ./lib.
- lib/         : Carpeta donde copiar libSDL2-2.0.so.0 de tu consola (o con `make bundle-libs`).

NOTA IMPORTANTE SOBRE EL BINARIO Y LIB/:
Este entorno de generación corre en arquitectura x86_64, mientras que tu consola con goodluckOS
corre sobre ARM (armhf / aarch64). Por seguridad, no incluimos un binario x86_64 incompatible:
1) Compila el proyecto en tu entorno/toolchain de la consola (o directamente en la consola si tiene g++):
   make && make bundle-libs
2) Si ya tienes libSDL2-2.0.so.0 en el sistema o en la carpeta lib/ de otro port (como Balatro),
   puedes copiar libSDL2-2.0.so.0 dentro de esta carpeta `lib/` (o dejar que use la del sistema).

Controles:
- Izquierda / Derecha : Seleccionar bloque de dígito (salta de jugador al llegar al extremo)
- Arriba / Abajo       : Sumar / Restar al dígito activo (el primer dígito sube > 9: 8, 9, 10, 11...)
- Botón A             : Confirmar / Siguiente jugador activo
- Botón B             : Tirar dado (D6)
- Botón Norte (Y)     : Dividir LP a la mitad (Halve) del jugador activo
- Botón Este (X)      : Tirar moneda (Cara / Cruz)
- Select + Izq / Der  : Cambiar cantidad de jugadores (1 a 5; con 5 usa diseño de dado)
- Select + Start      : Reiniciar duelo a 8000 LP
- Start + B           : Salir al lanzador (kennel)
