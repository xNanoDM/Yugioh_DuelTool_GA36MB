YU-GI-OH! DUEL CALCULATOR - GAME BOY ADVANCE (.GBA)
====================================================

Cómo compilar automáticamente en GitHub Actions (sin instalar nada en tu PC):
1. Sube todo el contenido de esta carpeta (incluyendo la carpeta oculta `.github`) a un repositorio nuevo de GitHub.
2. Ve a la pestaña "Actions" de tu repositorio en GitHub.
3. Entra en la ejecución "Build Yu-Gi-Oh! Calc GBA" (o pulsa "Run workflow").
4. Cuando termine (tarda ~15 segundos), descarga el artefacto `ygo_calc_gba` y descomprímelo para obtener `ygo_calc.gba`.
5. Copia `ygo_calc.gba` a la carpeta de ROMs de GBA de tu consola y ábrelo como cualquier juego de GBA.

Controles en GBA:
- Izquierda / Derecha : Seleccionar dígito (salta de jugador al llegar al extremo)
- Arriba / Abajo       : Sumar / Restar al dígito seleccionado (el primer dígito sube > 9: 8, 9, 10, 11...)
- Botón A             : Confirmar / Siguiente jugador activo
- Botón B             : Tirar dado (1-6)
- Botón L             : Dividir LP a la mitad (Halve) del jugador activo
- Botón R             : Tirar moneda (Cara / Cruz)
- Select + Izq / Der  : Cambiar cantidad de jugadores (1 a 5; con 5 usa diseño de dado)
- Select + Start      : Reiniciar duelo (todos a 8000 LP)
