/**
 * Yu-Gi-Oh! Duel Calculator (1 a 6 Jugadores) para Nintendo DS (.nds)
 * Compilado con devkitARM + libnds (Modo FB0 RGB15 256x192, una sola pantalla)
 *
 * Controles NDS:
 * - Izquierda / Derecha : Seleccionar bloque de dígito (salta de jugador al llegar al extremo)
 * - Arriba / Abajo       : Sumar / Restar al dígito activo (Bloque 0 crece > 9: 8, 9, 10, 11...)
 * - Botón A             : Confirmar / Siguiente jugador activo (P1 -> P2 -> ... -> PN)
 * - Botón B             : Halvear (dividir a la mitad) los LP del jugador activo (redondeo arriba)
 * - Botón X             : Tirar un dado (D6)
 * - Botón Y             : Tirar una moneda (Cara / Cruz)
 * - Botón L             : Restar un jugador (Mínimo 1)
 * - Botón R             : Añadir un jugador (Máximo 6; diseño de dado horizontal)
 * - Botón START         : Reiniciar duelo (todos los jugadores a 8000 LP)
 */

#include <nds.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>

// ==========================================
// UTILIDADES GRÁFICAS PANTALLA ÚNICA (256x192)
// ==========================================
#define SCREEN_W 256
#define SCREEN_H 192

static u16* vramBuf = nullptr;

static inline u16 rgb15(int r, int g, int b) {
    return (u16)(RGB15(r, g, b) | BIT(15));
}

static void fillScreen(u16 color) {
    if (!vramBuf) return;
    u32 c32 = ((u32)color << 16) | color;
    u32* dst = (u32*)vramBuf;
    for (int i = 0; i < (SCREEN_W * SCREEN_H) / 2; ++i) {
        dst[i] = c32;
    }
}

static void fillRect(int x, int y, int w, int h, u16 color) {
    if (!vramBuf) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > SCREEN_W) w = SCREEN_W - x;
    if (y + h > SCREEN_H) h = SCREEN_H - y;
    if (w <= 0 || h <= 0) return;

    for (int row = y; row < y + h; ++row) {
        u16* line = &vramBuf[row * SCREEN_W + x];
        for (int col = 0; col < w; ++col) {
            line[col] = color;
        }
    }
}

static void drawRect(int x, int y, int w, int h, u16 color) {
    fillRect(x, y, w, 1, color);
    fillRect(x, y + h - 1, w, 1, color);
    fillRect(x, y, 1, h, color);
    fillRect(x + w - 1, y, 1, h, color);
}

static void drawTriangleUp(int cx, int tipY, int size, u16 color) {
    for (int dy = 0; dy <= size; ++dy) {
        fillRect(cx - dy, tipY + dy, dy * 2 + 1, 1, color);
    }
}

static void drawTriangleDown(int cx, int tipY, int size, u16 color) {
    for (int dy = 0; dy <= size; ++dy) {
        fillRect(cx - dy, tipY - dy, dy * 2 + 1, 1, color);
    }
}

// ==========================================
// FUENTE BITMAP 5x7 EMBEBIDA
// ==========================================
struct Glyph5x7 {
    char ch;
    u8 rows[7];
};

static const Glyph5x7 FONT_5X7[] = {
    {' ', {0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
    {'0', {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E}},
    {'1', {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E}},
    {'2', {0x0E,0x11,0x01,0x02,0x04,0x08,0x1F}},
    {'3', {0x1F,0x02,0x04,0x02,0x01,0x11,0x0E}},
    {'4', {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02}},
    {'5', {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E}},
    {'6', {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E}},
    {'7', {0x1F,0x01,0x02,0x04,0x08,0x08,0x08}},
    {'8', {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E}},
    {'9', {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C}},
    {'A', {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11}},
    {'B', {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E}},
    {'C', {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E}},
    {'D', {0x1C,0x12,0x11,0x11,0x11,0x12,0x1C}},
    {'E', {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F}},
    {'F', {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10}},
    {'G', {0x0E,0x11,0x10,0x17,0x11,0x11,0x0F}},
    {'H', {0x11,0x11,0x11,0x1F,0x11,0x11,0x11}},
    {'I', {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E}},
    {'J', {0x07,0x02,0x02,0x02,0x02,0x12,0x0C}},
    {'K', {0x11,0x12,0x14,0x18,0x14,0x12,0x11}},
    {'L', {0x10,0x10,0x10,0x10,0x10,0x10,0x1F}},
    {'M', {0x11,0x1B,0x15,0x15,0x11,0x11,0x11}},
    {'N', {0x11,0x11,0x19,0x15,0x13,0x11,0x11}},
    {'O', {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E}},
    {'P', {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10}},
    {'Q', {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D}},
    {'R', {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11}},
    {'S', {0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E}},
    {'T', {0x1F,0x04,0x04,0x04,0x04,0x04,0x04}},
    {'U', {0x11,0x11,0x11,0x11,0x11,0x11,0x0E}},
    {'V', {0x11,0x11,0x11,0x11,0x11,0x0A,0x04}},
    {'W', {0x11,0x11,0x11,0x15,0x15,0x15,0x0A}},
    {'X', {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11}},
    {'Y', {0x11,0x11,0x0A,0x04,0x04,0x04,0x04}},
    {'Z', {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F}},
    {':', {0x00,0x04,0x04,0x00,0x04,0x04,0x00}},
    {'!', {0x04,0x04,0x04,0x04,0x04,0x00,0x04}},
    {'-', {0x00,0x00,0x00,0x1F,0x00,0x00,0x00}},
    {'+', {0x00,0x04,0x04,0x1F,0x04,0x04,0x00}},
    {'/', {0x01,0x02,0x02,0x04,0x08,0x08,0x10}},
    {'[', {0x0E,0x08,0x08,0x08,0x08,0x08,0x0E}},
    {']', {0x0E,0x02,0x02,0x02,0x02,0x02,0x0E}},
    {'<', {0x02,0x04,0x08,0x10,0x08,0x04,0x02}},
    {'>', {0x08,0x04,0x02,0x01,0x02,0x04,0x08}},
    {'*', {0x00,0x04,0x15,0x0E,0x15,0x04,0x00}},
    {'.', {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C}},
};

static const u8* getGlyph(char c) {
    if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
    for (size_t i = 0; i < sizeof(FONT_5X7) / sizeof(FONT_5X7[0]); ++i) {
        if (FONT_5X7[i].ch == c) return FONT_5X7[i].rows;
    }
    return FONT_5X7[0].rows;
}

static void drawText(const char* text, int x, int y, int scale, u16 color) {
    int curX = x;
    for (size_t i = 0; text[i] != '\0'; ++i) {
        const u8* rows = getGlyph(text[i]);
        for (int r = 0; r < 7; ++r) {
            for (int c = 0; c < 5; ++c) {
                if (rows[r] & (1 << (4 - c))) {
                    fillRect(curX + c * scale, y + r * scale, scale, scale, color);
                }
            }
        }
        curX += 6 * scale;
    }
}

// ==========================================
// LÓGICA DEL DUELO (1 A 6 JUGADORES)
// ==========================================
#define MAX_PLAYERS 6

struct PlayerLP {
    int blocks[4]; // [0]: Millares (0..N), [1]: Centenas (0-9), [2]: Decenas (0-9), [3]: Unidades (0-9)
};

enum ToolType { TOOL_NONE, TOOL_COIN, TOOL_DICE };

struct DuelState {
    PlayerLP players[MAX_PLAYERS];
    int numPlayers;
    int activePlayer;
    int cursorIdx;
    ToolType lastTool;
    int toolResult;
    int toolFramesLeft;
    u32 rngState;
};

static u32 nextRand(DuelState& st) {
    st.rngState = st.rngState * 1664525u + 1013904223u;
    return st.rngState >> 16;
}

static void resetPlayerLP(DuelState& st, int p) {
    st.players[p].blocks[0] = 8;
    st.players[p].blocks[1] = 0;
    st.players[p].blocks[2] = 0;
    st.players[p].blocks[3] = 0;
}

static void resetDuel(DuelState& st) {
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        resetPlayerLP(st, i);
    }
    st.activePlayer = 0;
    st.cursorIdx = 0;
    st.lastTool = TOOL_NONE;
    st.toolFramesLeft = 0;
}

static long getTotalLP(const DuelState& st, int p) {
    return (long)st.players[p].blocks[0] * 1000L +
           (long)st.players[p].blocks[1] * 100L +
           (long)st.players[p].blocks[2] * 10L +
           (long)st.players[p].blocks[3];
}

static void setTotalLP(DuelState& st, int p, long total) {
    if (total < 0) total = 0;
    st.players[p].blocks[0] = (int)(total / 1000);
    st.players[p].blocks[1] = (int)((total % 1000) / 100);
    st.players[p].blocks[2] = (int)((total % 100) / 10);
    st.players[p].blocks[3] = (int)(total % 10);
}

static void halveActivePlayerLP(DuelState& st) {
    long cur = getTotalLP(st, st.activePlayer);
    setTotalLP(st, st.activePlayer, (cur + 1) / 2); // Redondeo hacia arriba (regla oficial)
}

static void modifyDigit(DuelState& st, int delta) {
    int val = st.players[st.activePlayer].blocks[st.cursorIdx] + delta;
    if (st.cursorIdx == 0) {
        if (val < 0) val = 0;
        st.players[st.activePlayer].blocks[0] = val;
    } else {
        if (val > 9) val = 0;
        if (val < 0) val = 9;
        st.players[st.activePlayer].blocks[st.cursorIdx] = val;
    }
}

static void changePlayerCount(DuelState& st, int delta) {
    int next = st.numPlayers + delta;
    if (next < 1) next = 1;
    if (next > MAX_PLAYERS) next = MAX_PLAYERS;
    st.numPlayers = next;
    if (st.activePlayer >= st.numPlayers) {
        st.activePlayer = 0;
        st.cursorIdx = 0;
    }
}

static void flipCoin(DuelState& st) {
    st.toolResult = (int)(nextRand(st) & 1u);
    st.lastTool = TOOL_COIN;
    st.toolFramesLeft = 180; // ~3 segundos a 60fps
}

static void rollDice(DuelState& st) {
    st.toolResult = (int)(nextRand(st) % 6u) + 1;
    st.lastTool = TOOL_DICE;
    st.toolFramesLeft = 180;
}

// ==========================================
// RENDERIZADO EN NDS (256x192 PANTALLA ÚNICA)
// ==========================================
static void drawPlayerPanel(const DuelState& st, int p, int x, int y, int w, int h) {
    bool isSelected = (st.activePlayer == p);

    u16 bgCol     = isSelected ? rgb15(2, 6, 10)  : rgb15(3, 3, 4);
    u16 borderCol = isSelected ? rgb15(0, 29, 31) : rgb15(11, 11, 13);

    fillRect(x, y, w, h, bgCol);
    drawRect(x, y, w, h, borderCol);
    if (isSelected) {
        drawRect(x + 1, y + 1, w - 2, h - 2, borderCol);
    }

    // Cabecera jugador
    char hdr[20];
    if (w < 96) {
        std::snprintf(hdr, sizeof(hdr), "P%d%s", p + 1, isSelected ? "*" : "");
    } else {
        std::snprintf(hdr, sizeof(hdr), "JUG %d%s", p + 1, isSelected ? " [ACT]" : "");
    }
    u16 hdrCol = isSelected ? rgb15(0, 29, 31) : rgb15(21, 22, 24);
    drawText(hdr, x + 4, y + 4, 1, hdrCol);

    // Bloques de dígitos
    char blockStr[4][10];
    std::snprintf(blockStr[0], sizeof(blockStr[0]), "%d", st.players[p].blocks[0]);
    std::snprintf(blockStr[1], sizeof(blockStr[1]), "%d", st.players[p].blocks[1]);
    std::snprintf(blockStr[2], sizeof(blockStr[2]), "%d", st.players[p].blocks[2]);
    std::snprintf(blockStr[3], sizeof(blockStr[3]), "%d", st.players[p].blocks[3]);

    int totalChars = (int)std::strlen(blockStr[0]) + 3;
    int scale = 2;
    if (st.numPlayers == 1) {
        scale = 4;
    } else if (st.numPlayers == 2 && totalChars <= 5) {
        scale = 3;
    } else if (w < 86 && totalChars >= 5) {
        scale = 1;
    }

    int charW = 6 * scale;
    int charH = 7 * scale;
    int spacing = (scale >= 3) ? 3 : 2;
    int totalTextW = totalChars * charW + (3 * spacing);

    int curX = x + (w - totalTextW) / 2;
    int curY = y + (h - charH) / 2 + 5;

    long totalLP = getTotalLP(st, p);
    u16 normalCol = rgb15(31, 31, 31);
    if (totalLP == 0) normalCol = rgb15(31, 7, 7);
    else if (totalLP <= 2000) normalCol = rgb15(31, 21, 5);

    u16 yellowCol = rgb15(31, 27, 0);
    u16 blackCol  = rgb15(1, 1, 2);

    for (int i = 0; i < 4; ++i) {
        int bLen = (int)std::strlen(blockStr[i]);
        int bWidth = bLen * charW;
        bool isCursor = (isSelected && st.cursorIdx == i);

        if (isCursor) {
            fillRect(curX - 2, curY - 2, bWidth + 3, charH + 4, yellowCol);
            int midX = curX + (bWidth - scale) / 2;
            int triSize = (scale >= 2) ? 3 : 2;
            drawTriangleUp(midX, curY - 4 - triSize, triSize, yellowCol);
            drawTriangleDown(midX, curY + charH + 4 + triSize, triSize, yellowCol);
            drawText(blockStr[i], curX, curY, scale, blackCol);
        } else {
            drawText(blockStr[i], curX, curY, scale, normalCol);
        }
        curX += bWidth + spacing;
    }
}

static void renderFrame(const DuelState& st) {
    fillScreen(rgb15(1, 1, 2));

    // Barra superior
    int topH = 16;
    fillRect(0, 0, SCREEN_W, topH, rgb15(4, 5, 7));
    char title[24];
    std::snprintf(title, sizeof(title), "YGO DUEL [%dP]", st.numPlayers);
    drawText(title, 5, 5, 1, rgb15(30, 30, 31));

    // Badge de Moneda / Dado
    if (st.lastTool != TOOL_NONE && st.toolFramesLeft > 0) {
        int badgeW = 92;
        int badgeX = SCREEN_W - badgeW - 4;
        u16 bCol = (st.lastTool == TOOL_COIN) ? rgb15(31, 25, 4) : rgb15(28, 10, 30);
        fillRect(badgeX, 2, badgeW, topH - 4, bCol);

        char toolTxt[20];
        if (st.lastTool == TOOL_COIN) {
            std::snprintf(toolTxt, sizeof(toolTxt), "MONEDA:%s", st.toolResult == 0 ? "CARA" : "CRUZ");
        } else {
            std::snprintf(toolTxt, sizeof(toolTxt), "DADO: [ %d ]", st.toolResult);
        }
        drawText(toolTxt, badgeX + 5, 5, 1, rgb15(1, 1, 2));
    }

    // Área central
    int m = 4;
    int topY = topH + m;
    int botH = 14;
    int botY = SCREEN_H - botH - m;
    int W = SCREEN_W;
    int H = botY - topY;

    if (st.numPlayers == 1) {
        drawPlayerPanel(st, 0, m, topY, W - m * 2, H);
    } else if (st.numPlayers == 2) {
        int pW = (W - m * 3) / 2;
        drawPlayerPanel(st, 0, m, topY, pW, H);
        drawPlayerPanel(st, 1, m * 2 + pW, topY, pW, H);
    } else if (st.numPlayers == 3) {
        int pW = (W - m * 3) / 2;
        int pH = (H - m) / 2;
        drawPlayerPanel(st, 0, m, topY, pW, pH);
        drawPlayerPanel(st, 1, m * 2 + pW, topY, pW, pH);
        drawPlayerPanel(st, 2, (W - pW) / 2, topY + pH + m, pW, pH);
    } else if (st.numPlayers == 4) {
        int pW = (W - m * 3) / 2;
        int pH = (H - m) / 2;
        drawPlayerPanel(st, 0, m, topY, pW, pH);
        drawPlayerPanel(st, 1, m * 2 + pW, topY, pW, pH);
        drawPlayerPanel(st, 2, m, topY + pH + m, pW, pH);
        drawPlayerPanel(st, 3, m * 2 + pW, topY + pH + m, pW, pH);
    } else if (st.numPlayers == 5) {
        // 5 Jugadores: disposición en dado (4 esquinas + 1 al medio)
        int pW = (W - m * 4) / 3;
        int pH = (H - m) / 2;

        int col0X = m;
        int col1X = m * 2 + pW;
        int col2X = m * 3 + pW * 2;

        int row0Y = topY;
        int row1Y = topY + pH + m;
        int midY  = topY + (H - pH) / 2;

        drawPlayerPanel(st, 0, col0X, row0Y, pW, pH); // Sup. Izq (P1)
        drawPlayerPanel(st, 1, col2X, row0Y, pW, pH); // Sup. Der (P2)
        drawPlayerPanel(st, 2, col0X, row1Y, pW, pH); // Inf. Izq (P3)
        drawPlayerPanel(st, 3, col2X, row1Y, pW, pH); // Inf. Der (P4)
        drawPlayerPanel(st, 4, col1X, midY,  pW, pH); // Centro   (P5)
    } else if (st.numPlayers == 6) {
        // 6 Jugadores: dado de 6 en horizontal (3 columnas x 2 filas)
        int pW = (W - m * 4) / 3;
        int pH = (H - m) / 2;

        int col0X = m;
        int col1X = m * 2 + pW;
        int col2X = m * 3 + pW * 2;

        int row0Y = topY;
        int row1Y = topY + pH + m;

        drawPlayerPanel(st, 0, col0X, row0Y, pW, pH); // Fila 1, Col 1 (P1)
        drawPlayerPanel(st, 1, col1X, row0Y, pW, pH); // Fila 1, Col 2 (P2)
        drawPlayerPanel(st, 2, col2X, row0Y, pW, pH); // Fila 1, Col 3 (P3)
        drawPlayerPanel(st, 3, col0X, row1Y, pW, pH); // Fila 2, Col 1 (P4)
        drawPlayerPanel(st, 4, col1X, row1Y, pW, pH); // Fila 2, Col 2 (P5)
        drawPlayerPanel(st, 5, col2X, row1Y, pW, pH); // Fila 2, Col 3 (P6)
    }

    // Barra inferior de ayuda
    fillRect(0, SCREEN_H - botH, SCREEN_W, botH, rgb15(3, 3, 5));
    drawText("A:SIG B:1/2 X:DADO Y:MON L/R:-/+P ST:RST", 4, SCREEN_H - botH + 4, 1, rgb15(21, 22, 24));
}

int main() {
    // Usar solo la pantalla superior (Main Engine en modo FB0 directo), pantalla inferior apagada
    powerOn(POWER_ALL_2D);
    lcdMainOnTop();
    videoSetMode(MODE_FB0);
    videoSetModeSub(MODE_0_2D);
    vramSetBankA(VRAM_A_LCD);
    vramBuf = (u16*)VRAM_A;

    DuelState state;
    state.numPlayers = 2;
    state.rngState = 0x12345678u;
    resetDuel(state);

    bool needsRedraw = true;
    u32 frameCounter = 0;

    while (pmMainLoop()) {
        swiWaitForVBlank();
        frameCounter++;
        state.rngState ^= frameCounter * 2654435761u;

        scanKeys();
        u16 down = keysDown();

        if (down != 0) {
            state.rngState += (u32)down * 747796405u;
        }

        // Flechas modifican LP
        if (down & KEY_LEFT) {
            if (state.cursorIdx > 0) {
                state.cursorIdx--;
            } else if (state.numPlayers > 1) {
                state.activePlayer = (state.activePlayer - 1 + state.numPlayers) % state.numPlayers;
                state.cursorIdx = 3;
            }
            needsRedraw = true;
        }
        if (down & KEY_RIGHT) {
            if (state.cursorIdx < 3) {
                state.cursorIdx++;
            } else if (state.numPlayers > 1) {
                state.activePlayer = (state.activePlayer + 1) % state.numPlayers;
                state.cursorIdx = 0;
            }
            needsRedraw = true;
        }
        if (down & KEY_UP) {
            modifyDigit(state, 1);
            needsRedraw = true;
        }
        if (down & KEY_DOWN) {
            modifyDigit(state, -1);
            needsRedraw = true;
        }

        // A cambia de jugador
        if (down & KEY_A) {
            if (state.numPlayers > 1) {
                state.activePlayer = (state.activePlayer + 1) % state.numPlayers;
            }
            state.cursorIdx = 0;
            needsRedraw = true;
        }

        // B halvea la vida del jugador
        if (down & KEY_B) {
            halveActivePlayerLP(state);
            needsRedraw = true;
        }

        // X tira un dado
        if (down & KEY_X) {
            rollDice(state);
            needsRedraw = true;
        }

        // Y tira una moneda
        if (down & KEY_Y) {
            flipCoin(state);
            needsRedraw = true;
        }

        // L resta un jugador (mínimo 1)
        if (down & KEY_L) {
            changePlayerCount(state, -1);
            needsRedraw = true;
        }

        // R añade un jugador (máximo 6)
        if (down & KEY_R) {
            changePlayerCount(state, 1);
            needsRedraw = true;
        }

        // Start reinicia duelo
        if (down & KEY_START) {
            resetDuel(state);
            needsRedraw = true;
        }

        if (state.toolFramesLeft > 0) {
            state.toolFramesLeft--;
            if (state.toolFramesLeft == 0) {
                state.lastTool = TOOL_NONE;
                needsRedraw = true;
            }
        }

        if (needsRedraw) {
            renderFrame(state);
            needsRedraw = false;
        }
    }

    return 0;
}
