YU-GI-OH! DUEL CALCULATOR - PLAYSTATION PORTABLE (.PBP)
=======================================================

Cómo compilar automáticamente en GitHub Actions:
1. Sube todo el contenido de esta carpeta (incluyendo `.github/workflows/build-psp.yml`) a tu repositorio de GitHub.
2. Ve a la pestaña "Actions" y espera a que termine "Build Yu-Gi-Oh! Calc PSP" (usa la imagen oficial pspdev/pspdev:latest).
3. Descarga el artefacto `ygo_calc_psp`, descomprímelo y copia `ygo_calc.pbp` a la carpeta de ROMs de PSP en tu consola GA-36MB.

Resolución y pantalla:
- Resolución nativa 480x272 (16:9 panorámico, sin doble pantalla ni táctil), ideal para la pantalla de la GA-36MB bajo PPSSPP.

Controles (equivalencia exacta de posición física en tu consola):
- Flechas (Izq / Der / Arr / Abj) : Seleccionar dígito y modificar LP (bloque 0 sube > 9: 8, 9, 10, 11...)
- Círculo (botón Este / A)        : Cambiar de jugador activo (P1 -> P2 -> ... -> PN)
- Cruz (botón Sur / B)            : Halvear (dividir a la mitad) los LP del jugador activo
- Triángulo (botón Norte / X)     : Tirar un dado (1-6)
- Cuadrado (botón Oeste / Y)      : Tirar una moneda (Cara / Cruz)
- L                               : Restar un jugador (Mínimo 1)
- R                               : Añadir un jugador (Máximo 6; diseño de dado horizontal 3x2)
- Start                           : Reiniciar duelo (todos los jugadores a 8000 LP)
