/**
 * Yu-Gi-Oh! Duel Calculator (1 a 6 Jugadores) para PlayStation Portable (PSP - EBOOT.PBP)
 * Compilado con PSPSDK (pspdev/pspdev) usando framebuffer 32-bit 480x272 (stride 512)
 *
 * Controles PSP (mapeo equivalente al esquema Nintendo):
 * - Cruceta (Izq / Der) : Seleccionar bloque de dígito (salta de jugador al llegar al extremo)
 * - Cruceta (Arr / Abj) : Sumar / Restar al dígito activo (Bloque 0 crece > 9: 8, 9, 10, 11...)
 * - Círculo (Este / A)  : Confirmar / Siguiente jugador activo (P1 -> P2 -> ... -> PN)
 * - Cruz (Sur / B)      : Halvear (dividir a la mitad) los LP del jugador activo (redondeo arriba)
 * - Triángulo (Norte/X) : Tirar un dado (D6)
 * - Cuadrado (Oeste/Y)  : Tirar una moneda (Cara / Cruz)
 * - Gatillo L           : Restar un jugador (Mínimo 1)
 * - Gatillo R           : Añadir un jugador (Máximo 6; diseño de dado horizontal 3x2)
 * - START               : Reiniciar duelo (todos los jugadores a 8000 LP)
 */

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>

PSP_MODULE_INFO("YGO_DUEL_CALC", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

// ==========================================
// EXIT CALLBACK (para salir limpio con HOME)
// ==========================================
static bool running = true;

int exit_callback(int arg1, int arg2, void* common) {
    running = false;
    return 0;
}

int callback_thread(SceSize args, void* argp) {
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, nullptr);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

void setupCallbacks() {
    int thid = sceKernelCreateThread("update_thread", callback_thread, 0x11, 0xFA0, 0, nullptr);
    if (thid >= 0) {
        sceKernelStartThread(thid, 0, nullptr);
    }
}

// ==========================================
// GRÁFICOS FRAMEBUFFER 480x272 (STRIDE 512)
// ==========================================
#define SCREEN_W 480
#define SCREEN_H 272
#define BUF_WIDTH 512
#define PIXEL_SIZE 4
#define FRAME_SIZE (BUF_WIDTH * SCREEN_H * PIXEL_SIZE)

// VRAM uncached base en PSP
static u32* const VRAM_BASE = (u32*)0x44000000;
static int currentDrawBuf = 0;

static inline u32* getDrawBuffer() {
    return VRAM_BASE + (currentDrawBuf * BUF_WIDTH * SCREEN_H);
}

static inline u32 rgba(u8 r, u8 g, u8 b) {
    return (0xFFu << 24) | ((u32)b << 16) | ((u32)g << 8) | (u32)r;
}

static void fillScreen(u32 color) {
    u32* buf = getDrawBuffer();
    for (int y = 0; y < SCREEN_H; ++y) {
        u32* row = buf + y * BUF_WIDTH;
        for (int x = 0; x < SCREEN_W; ++x) {
            row[x] = color;
        }
    }
}

static void fillRect(int x, int y, int w, int h, u32 color) {
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > SCREEN_W) w = SCREEN_W - x;
    if (y + h > SCREEN_H) h = SCREEN_H - y;
    if (w <= 0 || h <= 0) return;

    u32* buf = getDrawBuffer();
    for (int row = y; row < y + h; ++row) {
        u32* line = buf + row * BUF_WIDTH + x;
        for (int col = 0; col < w; ++col) {
            line[col] = color;
        }
    }
}

static void drawRect(int x, int y, int w, int h, u32 color) {
    fillRect(x, y, w, 1, color);
    fillRect(x, y + h - 1, w, 1, color);
    fillRect(x, y, 1, h, color);
    fillRect(x + w - 1, y, 1, h, color);
}

static void drawTriangleUp(int cx, int tipY, int size, u32 color) {
    for (int dy = 0; dy <= size; ++dy) {
        fillRect(cx - dy, tipY + dy, dy * 2 + 1, 1, color);
    }
}

static void drawTriangleDown(int cx, int tipY, int size, u32 color) {
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
    {'(', {0x02,0x04,0x08,0x08,0x08,0x04,0x02}},
    {')', {0x08,0x04,0x02,0x02,0x02,0x04,0x08}},
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

static void drawText(const char* text, int x, int y, int scale, u32 color) {
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
    setTotalLP(st, st.activePlayer, (cur + 1) / 2); // Redondeo hacia arriba
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
// RENDERIZADO EN PSP (480x272)
// ==========================================
static void drawPlayerPanel(const DuelState& st, int p, int x, int y, int w, int h) {
    bool isSelected = (st.activePlayer == p);

    u32 bgCol     = isSelected ? rgba(18, 48, 82)   : rgba(24, 24, 32);
    u32 borderCol = isSelected ? rgba(0, 232, 255)  : rgba(88, 88, 104);

    fillRect(x, y, w, h, bgCol);
    drawRect(x, y, w, h, borderCol);
    if (isSelected) {
        drawRect(x + 1, y + 1, w - 2, h - 2, borderCol);
    }

    // Cabecera jugador
    char hdr[24];
    int hdrScale = (w >= 200) ? 2 : 1;
    if (w < 170) {
        std::snprintf(hdr, sizeof(hdr), "P%d%s", p + 1, isSelected ? " [ACT]" : "");
    } else {
        std::snprintf(hdr, sizeof(hdr), "JUG %d%s", p + 1, isSelected ? " [ACT]" : "");
    }
    u32 hdrCol = isSelected ? rgba(0, 232, 255) : rgba(175, 180, 195);
    drawText(hdr, x + 8, y + 6, hdrScale, hdrCol);

    // Bloques de dígitos
    char blockStr[4][12];
    std::snprintf(blockStr[0], sizeof(blockStr[0]), "%d", st.players[p].blocks[0]);
    std::snprintf(blockStr[1], sizeof(blockStr[1]), "%d", st.players[p].blocks[1]);
    std::snprintf(blockStr[2], sizeof(blockStr[2]), "%d", st.players[p].blocks[2]);
    std::snprintf(blockStr[3], sizeof(blockStr[3]), "%d", st.players[p].blocks[3]);

    int totalChars = (int)std::strlen(blockStr[0]) + 3;
    int scale = 4;
    if (st.numPlayers == 1) {
        scale = 7;
    } else if (st.numPlayers == 2) {
        scale = (totalChars <= 5) ? 5 : 4;
    } else if (st.numPlayers <= 4) {
        scale = (totalChars <= 5) ? 4 : 3;
    } else {
        // 5 o 6 jugadores (ancho ~150px)
        scale = (totalChars <= 5) ? 3 : 2;
    }

    int charW = 6 * scale;
    int charH = 7 * scale;
    int spacing = (scale >= 4) ? 5 : 3;
    int totalTextW = totalChars * charW + (3 * spacing);

    int curX = x + (w - totalTextW) / 2;
    int curY = y + (h - charH) / 2 + 8;

    long totalLP = getTotalLP(st, p);
    u32 normalCol = rgba(255, 255, 255);
    if (totalLP == 0) normalCol = rgba(255, 65, 65);
    else if (totalLP <= 2000) normalCol = rgba(255, 175, 45);

    u32 yellowCol = rgba(255, 220, 0);
    u32 blackCol  = rgba(10, 10, 15);

    for (int i = 0; i < 4; ++i) {
        int bLen = (int)std::strlen(blockStr[i]);
        int bWidth = bLen * charW;
        bool isCursor = (isSelected && st.cursorIdx == i);

        if (isCursor) {
            fillRect(curX - 3, curY - 3, bWidth + 5, charH + 6, yellowCol);
            int midX = curX + (bWidth - scale) / 2;
            int triSize = (scale >= 3) ? 5 : 3;
            drawTriangleUp(midX, curY - 6 - triSize, triSize, yellowCol);
            drawTriangleDown(midX, curY + charH + 6 + triSize, triSize, yellowCol);
            drawText(blockStr[i], curX, curY, scale, blackCol);
        } else {
            drawText(blockStr[i], curX, curY, scale, normalCol);
        }
        curX += bWidth + spacing;
    }
}

static void renderFrame(const DuelState& st) {
    fillScreen(rgba(10, 10, 16));

    // Barra superior
    int topH = 24;
    fillRect(0, 0, SCREEN_W, topH, rgba(30, 34, 48));
    char title[32];
    std::snprintf(title, sizeof(title), "YGO DUEL CALC [%d JUG]", st.numPlayers);
    drawText(title, 8, 5, 2, rgba(240, 240, 245));

    // Badge de Moneda / Dado
    if (st.lastTool != TOOL_NONE && st.toolFramesLeft > 0) {
        int badgeW = 165;
        int badgeX = SCREEN_W - badgeW - 6;
        u32 bCol = (st.lastTool == TOOL_COIN) ? rgba(255, 200, 35) : rgba(225, 80, 245);
        fillRect(badgeX, 3, badgeW, topH - 6, bCol);

        char toolTxt[24];
        if (st.lastTool == TOOL_COIN) {
            std::snprintf(toolTxt, sizeof(toolTxt), "MONEDA:%s", st.toolResult == 0 ? "CARA" : "CRUZ");
        } else {
            std::snprintf(toolTxt, sizeof(toolTxt), "DADO: [ %d ]", st.toolResult);
        }
        drawText(toolTxt, badgeX + 8, 5, 2, rgba(10, 10, 15));
    }

    // Área central
    int m = 6;
    int topY = topH + m;
    int botH = 20;
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
    fillRect(0, SCREEN_H - botH, SCREEN_W, botH, rgba(22, 24, 36));
    drawText("O(A):SIG X(B):1/2 TRI(X):DADO CUA(Y):MON L/R:-/+P ST:RST", 8, SCREEN_H - botH + 6, 1, rgba(190, 195, 210));
}

int main() {
    setupCallbacks();

    sceDisplaySetMode(0, SCREEN_W, SCREEN_H);
    sceDisplaySetFrameBuf(VRAM_BASE, BUF_WIDTH, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_IMMEDIATE);

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

    DuelState state;
    state.numPlayers = 2;
    state.rngState = 0x12345678u;
    resetDuel(state);

    SceCtrlData pad;
    u32 prevButtons = 0;
    u32 frameCounter = 0;

    // Dibujar ambos buffers inicialmente
    currentDrawBuf = 0;
    renderFrame(state);
    currentDrawBuf = 1;
    renderFrame(state);

    while (running) {
        sceDisplayWaitVblankStart();
        frameCounter++;
        state.rngState ^= frameCounter * 2654435761u;

        sceCtrlReadBufferPositive(&pad, 1);
        u32 down = pad.Buttons & ~prevButtons;
        prevButtons = pad.Buttons;

        if (down != 0) {
            state.rngState += down * 747796405u;
        }

        // Flechas modifican LP
        if (down & PSP_CTRL_LEFT) {
            if (state.cursorIdx > 0) {
                state.cursorIdx--;
            } else if (state.numPlayers > 1) {
                state.activePlayer = (state.activePlayer - 1 + state.numPlayers) % state.numPlayers;
                state.cursorIdx = 3;
            }
        }
        if (down & PSP_CTRL_RIGHT) {
            if (state.cursorIdx < 3) {
                state.cursorIdx++;
            } else if (state.numPlayers > 1) {
                state.activePlayer = (state.activePlayer + 1) % state.numPlayers;
                state.cursorIdx = 0;
            }
        }
        if (down & PSP_CTRL_UP) {
            modifyDigit(state, 1);
        }
        if (down & PSP_CTRL_DOWN) {
            modifyDigit(state, -1);
        }

        // Círculo (Este / botón A en layout Nintendo) -> cambia de jugador
        if (down & PSP_CTRL_CIRCLE) {
            if (state.numPlayers > 1) {
                state.activePlayer = (state.activePlayer + 1) % state.numPlayers;
            }
            state.cursorIdx = 0;
        }

        // Cruz (Sur / botón B en layout Nintendo) -> halvea vida del jugador
        if (down & PSP_CTRL_CROSS) {
            halveActivePlayerLP(state);
        }

        // Triángulo (Norte / botón X en layout Nintendo) -> tira un dado
        if (down & PSP_CTRL_TRIANGLE) {
            rollDice(state);
        }

        // Cuadrado (Oeste / botón Y en layout Nintendo) -> tira una moneda
        if (down & PSP_CTRL_SQUARE) {
            flipCoin(state);
        }

        // L -> resta un jugador (mínimo 1)
        if (down & PSP_CTRL_LTRIGGER) {
            changePlayerCount(state, -1);
        }

        // R -> añade un jugador (máximo 6)
        if (down & PSP_CTRL_RTRIGGER) {
            changePlayerCount(state, 1);
        }

        // Start -> reinicia duelo
        if (down & PSP_CTRL_START) {
            resetDuel(state);
        }

        if (state.toolFramesLeft > 0) {
            state.toolFramesLeft--;
            if (state.toolFramesLeft == 0) {
                state.lastTool = TOOL_NONE;
            }
        }

        currentDrawBuf ^= 1;
        renderFrame(state);
        sceDisplaySetFrameBuf(getDrawBuffer(), BUF_WIDTH, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_NEXTFRAME);
    }

    sceKernelExitGame();
    return 0;
}
