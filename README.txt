YU-GI-OH! DUEL CALCULATOR - NINTENDO DS (.NDS)
=============================================

Cómo compilar automáticamente en GitHub Actions:
1. Sube el contenido de esta carpeta (incluyendo `.github/workflows/build-nds.yml`) a tu repositorio de GitHub.
2. Ve a la pestaña "Actions" y ejecuta "Build Yu-Gi-Oh! Calc NDS" (se lanza automáticamente al subir).
3. Descarga el artefacto `ygo_calc_nds` y copia `ygo_calc.nds` a la carpeta de ROMs de NDS de tu consola.

Configuración de pantalla:
- Utiliza únicamente la pantalla superior (`256x192`), sin pantalla táctil (puedes configurar tu emulador NDS en modo "Single Screen / Pantalla única" para aprovechar toda la pantalla).

Controles en NDS:
- Flechas (Izq / Der / Arr / Abj) : Seleccionar dígito y modificar LP (bloque 0 sube > 9: 8, 9, 10, 11...)
- A     : Cambiar de jugador activo (P1 -> P2 -> ... -> PN)
- B     : Halvear (dividir a la mitad) los LP del jugador activo
- X     : Tirar un dado (1-6)
- Y     : Tirar una moneda (Cara / Cruz)
- L     : Restar un jugador (Mínimo 1)
- R     : Añadir un jugador (Máximo 6; diseño de dado horizontal de 3x2)
- Start : Reiniciar duelo (todos los jugadores a 8000 LP)
