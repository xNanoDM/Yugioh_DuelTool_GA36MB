/**
 * Yu-Gi-Oh! Duel Calculator (1 a 5 Jugadores) - SDL2 Nativo para goodluckOS / kennel
 *
 * Controles (Gamepad / D-Pad / Teclado):
 * - Izquierda / Derecha : Navegar entre los 4 bloques de dígitos (y saltar de jugador en extremos)
 * - Arriba / Abajo       : Incrementar / Decrementar dígito seleccionado (Bloque 0 crece > 9: 8, 9, 10, 11...)
 * - Botón A (Enter/Z)   : Confirmar / Avanzar al siguiente jugador activo (P1 -> P2 -> ... -> PN)
 * - Botón B (Esc/X)     : Tirar un dado (D6)
 * - Botón Norte / Y (C) : Dividir a la mitad (Halve) los LP del jugador activo (redondeo arriba)
 * - Botón Este / X (V)  : Tirar una moneda (Cara / Cruz)
 * - Select + Izq / Der  : Cambiar cantidad de jugadores (1 a 5 jugadores; 5 = diseño de dado)
 * - Select + Start      : Reiniciar duelo (todos los jugadores a 8000 LP)
 * - Start + B (o Q)     : Salir de la aplicación al lanzador
 */

#include <SDL2/SDL.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>
#include <string>

// ==========================================
// FUENTE BITMAP 5x7 EMBEBIDA (SIN DEPENDER DE SDL2_ttf)
// ==========================================
struct Glyph5x7 {
    char ch;
    uint8_t rows[7];
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

static const uint8_t* getGlyph(char c) {
    if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
    for (size_t i = 0; i < sizeof(FONT_5X7) / sizeof(FONT_5X7[0]); ++i) {
        if (FONT_5X7[i].ch == c) return FONT_5X7[i].rows;
    }
    return FONT_5X7[0].rows;
}

static void drawText(SDL_Renderer* ren, const char* text, int x, int y, int scale, SDL_Color color) {
    SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
    int curX = x;
    for (size_t i = 0; text[i] != '\0'; ++i) {
        const uint8_t* rows = getGlyph(text[i]);
        for (int r = 0; r < 7; ++r) {
            for (int c = 0; c < 5; ++c) {
                if (rows[r] & (1 << (4 - c))) {
                    SDL_Rect px = {curX + c * scale, y + r * scale, scale, scale};
                    SDL_RenderFillRect(ren, &px);
                }
            }
        }
        curX += 6 * scale;
    }
}

static void drawTriangleUp(SDL_Renderer* ren, int cx, int tipY, int size, SDL_Color color) {
    SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
    for (int dy = 0; dy <= size; ++dy) {
        SDL_RenderDrawLine(ren, cx - dy, tipY + dy, cx + dy, tipY + dy);
    }
}

static void drawTriangleDown(SDL_Renderer* ren, int cx, int tipY, int size, SDL_Color color) {
    SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
    for (int dy = 0; dy <= size; ++dy) {
        SDL_RenderDrawLine(ren, cx - dy, tipY - dy, cx + dy, tipY - dy);
    }
}

// ==========================================
// LÓGICA DEL DUELO (1 A 5 JUGADORES)
// ==========================================
#define MAX_PLAYERS 5

struct PlayerLP {
    int blocks[4]; // [0]: Millares (0..N), [1]: Centenas (0-9), [2]: Decenas (0-9), [3]: Unidades (0-9)
};

enum ToolType { TOOL_NONE, TOOL_COIN, TOOL_DICE };

struct DuelState {
    PlayerLP players[MAX_PLAYERS];
    int numPlayers = 2;
    int activePlayer = 0;
    int cursorIdx = 0;
    ToolType lastTool = TOOL_NONE;
    int toolResult = 0;
    Uint32 toolDisplayTimer = 0;
    bool selectHeld = false;
    bool startHeld = false;
};

static const Uint32 TOOL_DISPLAY_DURATION = 3500;

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

static void setPlayerCount(DuelState& st, int count) {
    if (count < 1) count = MAX_PLAYERS;
    if (count > MAX_PLAYERS) count = 1;
    st.numPlayers = count;
    if (st.activePlayer >= st.numPlayers) {
        st.activePlayer = 0;
        st.cursorIdx = 0;
    }
}

static void flipCoin(DuelState& st) {
    st.toolResult = std::rand() % 2;
    st.lastTool = TOOL_COIN;
    st.toolDisplayTimer = SDL_GetTicks();
}

static void rollDice(DuelState& st) {
    st.toolResult = (std::rand() % 6) + 1;
    st.lastTool = TOOL_DICE;
    st.toolDisplayTimer = SDL_GetTicks();
}

// ==========================================
// RENDERIZADO SDL2
// ==========================================
static void drawPlayerPanel(SDL_Renderer* ren, const DuelState& st, int p, int x, int y, int w, int h) {
    bool isSelected = (st.activePlayer == p);

    SDL_Color bg = isSelected ? SDL_Color{12, 34, 48, 255} : SDL_Color{18, 18, 22, 255};
    SDL_Color border = isSelected ? SDL_Color{0, 230, 255, 255} : SDL_Color{85, 85, 95, 255};

    SDL_Rect box = {x, y, w, h};
    SDL_SetRenderDrawColor(ren, bg.r, bg.g, bg.b, 255);
    SDL_RenderFillRect(ren, &box);

    SDL_SetRenderDrawColor(ren, border.r, border.g, border.b, 255);
    SDL_RenderDrawRect(ren, &box);
    if (isSelected) {
        SDL_Rect inner = {x + 1, y + 1, w - 2, h - 2};
        SDL_RenderDrawRect(ren, &inner);
    }

    // Cabecera jugador
    char header[32];
    if (w < 190) {
        std::snprintf(header, sizeof(header), "P%d%s", p + 1, isSelected ? " *" : "");
    } else {
        std::snprintf(header, sizeof(header), "JUGADOR %d%s", p + 1, isSelected ? " [ACT]" : "");
    }
    SDL_Color hdrCol = isSelected ? SDL_Color{0, 230, 255, 255} : SDL_Color{170, 175, 185, 255};
    drawText(ren, header, x + 8, y + 8, 2, hdrCol);

    // Bloques numéricos
    char blockStr[4][12];
    std::snprintf(blockStr[0], sizeof(blockStr[0]), "%d", st.players[p].blocks[0]);
    std::snprintf(blockStr[1], sizeof(blockStr[1]), "%d", st.players[p].blocks[1]);
    std::snprintf(blockStr[2], sizeof(blockStr[2]), "%d", st.players[p].blocks[2]);
    std::snprintf(blockStr[3], sizeof(blockStr[3]), "%d", st.players[p].blocks[3]);

    int totalChars = (int)std::strlen(blockStr[0]) + 3;
    int scale = 5;
    if (st.numPlayers == 1) {
        scale = 8;
    } else if (w < 215 || (w < 250 && totalChars >= 5)) {
        scale = 4;
    } else if (st.numPlayers == 2) {
        scale = 6;
    }

    int charW = 6 * scale;
    int charH = 7 * scale;
    int spacing = (scale >= 6) ? 6 : 4;
    int totalTextW = totalChars * charW + (3 * spacing);

    int curX = x + (w - totalTextW) / 2;
    int curY = y + (h - charH) / 2 + 8;

    long totalLP = getTotalLP(st, p);
    SDL_Color normalCol = {245, 245, 250, 255};
    if (totalLP == 0) normalCol = {255, 70, 70, 255};
    else if (totalLP <= 2000) normalCol = {255, 175, 50, 255};

    for (int i = 0; i < 4; ++i) {
        int bLen = (int)std::strlen(blockStr[i]);
        int bWidth = bLen * charW;
        bool isCursor = (isSelected && st.cursorIdx == i);

        if (isCursor) {
            SDL_Rect hi = {curX - 4, curY - 5, bWidth + 6, charH + 10};
            SDL_SetRenderDrawColor(ren, 255, 215, 0, 255);
            SDL_RenderFillRect(ren, &hi);

            int midX = curX + (bWidth - scale) / 2;
            int triSize = (scale >= 5) ? 6 : 4;
            drawTriangleUp(ren, midX, curY - 10 - triSize, triSize, SDL_Color{255, 215, 0, 255});
            drawTriangleDown(ren, midX, curY + charH + 10 + triSize, triSize, SDL_Color{255, 215, 0, 255});

            drawText(ren, blockStr[i], curX, curY, scale, SDL_Color{10, 10, 12, 255});
        } else {
            drawText(ren, blockStr[i], curX, curY, scale, normalCol);
        }

        curX += bWidth + spacing;
    }
}

static void renderFrame(SDL_Renderer* ren, DuelState& st, int screenW, int screenH) {
    SDL_SetRenderDrawColor(ren, 8, 8, 12, 255);
    SDL_RenderClear(ren);

    // Barra superior
    int topBarH = 32;
    SDL_Rect topBar = {0, 0, screenW, topBarH};
    SDL_SetRenderDrawColor(ren, 24, 28, 38, 255);
    SDL_RenderFillRect(ren, &topBar);

    char title[48];
    std::snprintf(title, sizeof(title), "YU-GI-OH! DUEL [%dP]", st.numPlayers);
    drawText(ren, title, 10, 9, 2, SDL_Color{240, 240, 245, 255});

    // Badge de Dado o Moneda
    Uint32 now = SDL_GetTicks();
    if (st.lastTool != TOOL_NONE && (now - st.toolDisplayTimer < TOOL_DISPLAY_DURATION)) {
        int badgeW = 210;
        int badgeX = screenW - badgeW - 8;
        SDL_Rect badge = {badgeX, 4, badgeW, topBarH - 8};
        SDL_Color bCol = (st.lastTool == TOOL_COIN) ? SDL_Color{255, 200, 40, 255} : SDL_Color{225, 80, 235, 255};
        SDL_SetRenderDrawColor(ren, bCol.r, bCol.g, bCol.b, 255);
        SDL_RenderFillRect(ren, &badge);

        char toolTxt[32];
        if (st.lastTool == TOOL_COIN) {
            std::snprintf(toolTxt, sizeof(toolTxt), "MONEDA: %s", st.toolResult == 0 ? "CARA" : "CRUZ");
        } else {
            std::snprintf(toolTxt, sizeof(toolTxt), "DADO: [ %d ]", st.toolResult);
        }
        drawText(ren, toolTxt, badgeX + 10, 9, 2, SDL_Color{10, 10, 12, 255});
    } else {
        st.lastTool = TOOL_NONE;
    }

    // Área de paneles de jugadores
    int m = 8;
    int topY = topBarH + m;
    int botBarH = 26;
    int botY = screenH - botBarH - m;
    int W = screenW;
    int H = botY - topY;

    if (st.numPlayers == 1) {
        drawPlayerPanel(ren, st, 0, m, topY, W - m * 2, H);
    } else if (st.numPlayers == 2) {
        int pW = (W - m * 3) / 2;
        drawPlayerPanel(ren, st, 0, m, topY, pW, H);
        drawPlayerPanel(ren, st, 1, m * 2 + pW, topY, pW, H);
    } else if (st.numPlayers == 3) {
        int pW = (W - m * 3) / 2;
        int pH = (H - m) / 2;
        drawPlayerPanel(ren, st, 0, m, topY, pW, pH);
        drawPlayerPanel(ren, st, 1, m * 2 + pW, topY, pW, pH);
        drawPlayerPanel(ren, st, 2, (W - pW) / 2, topY + pH + m, pW, pH);
    } else if (st.numPlayers == 4) {
        int pW = (W - m * 3) / 2;
        int pH = (H - m) / 2;
        drawPlayerPanel(ren, st, 0, m, topY, pW, pH);
        drawPlayerPanel(ren, st, 1, m * 2 + pW, topY, pW, pH);
        drawPlayerPanel(ren, st, 2, m, topY + pH + m, pW, pH);
        drawPlayerPanel(ren, st, 3, m * 2 + pW, topY + pH + m, pW, pH);
    } else if (st.numPlayers == 5) {
        // Disposición en dado (4 esquinas + 1 en el centro exacto)
        int pW = (W - m * 4) / 3;
        int pH = (H - m) / 2;

        int col0X = m;
        int col1X = m * 2 + pW;
        int col2X = m * 3 + pW * 2;

        int row0Y = topY;
        int row1Y = topY + pH + m;
        int midY  = topY + (H - pH) / 2;

        drawPlayerPanel(ren, st, 0, col0X, row0Y, pW, pH); // Sup. Izquierda (P1)
        drawPlayerPanel(ren, st, 1, col2X, row0Y, pW, pH); // Sup. Derecha   (P2)
        drawPlayerPanel(ren, st, 2, col0X, row1Y, pW, pH); // Inf. Izquierda (P3)
        drawPlayerPanel(ren, st, 3, col2X, row1Y, pW, pH); // Inf. Derecha   (P4)
        drawPlayerPanel(ren, st, 4, col1X, midY,  pW, pH); // Centro         (P5)
    }

    // Barra inferior de ayuda
    SDL_Rect botBar = {0, screenH - botBarH, screenW, botBarH};
    SDL_SetRenderDrawColor(ren, 20, 22, 30, 255);
    SDL_RenderFillRect(ren, &botBar);
    drawText(ren, "[A]SIG [B]DADO [N]1/2 [E]MONEDA [SEL+<>]JUG [SEL+ST]RESET", 8, screenH - botBarH + 6, 1, SDL_Color{170, 175, 185, 255});

    SDL_RenderPresent(ren);
}

// ==========================================
// GESTIÓN DE ENTRADA (GAMEPAD / JOYSTICK / TECLADO)
// ==========================================
enum Action {
    ACT_NONE,
    ACT_UP,
    ACT_DOWN,
    ACT_LEFT,
    ACT_RIGHT,
    ACT_A,
    ACT_B,
    ACT_NORTH,
    ACT_EAST,
    ACT_SELECT_DOWN,
    ACT_SELECT_UP,
    ACT_START_DOWN,
    ACT_START_UP,
    ACT_QUIT
};

static void handleAction(DuelState& st, Action act, bool& running) {
    if (act == ACT_QUIT) {
        running = false;
        return;
    }
    if (act == ACT_SELECT_DOWN) { st.selectHeld = true; return; }
    if (act == ACT_SELECT_UP)   { st.selectHeld = false; return; }
    if (act == ACT_START_DOWN)  {
        st.startHeld = true;
        if (st.selectHeld) resetDuel(st);
        return;
    }
    if (act == ACT_START_UP)    { st.startHeld = false; return; }

    // Salir con Start + B
    if (st.startHeld && act == ACT_B) {
        running = false;
        return;
    }

    // Cambiar número de jugadores con Select + Izq/Der
    if (st.selectHeld) {
        if (act == ACT_LEFT)  setPlayerCount(st, st.numPlayers - 1);
        if (act == ACT_RIGHT) setPlayerCount(st, st.numPlayers + 1);
        return;
    }

    switch (act) {
        case ACT_LEFT:
            if (st.cursorIdx > 0) {
                st.cursorIdx--;
            } else if (st.numPlayers > 1) {
                st.activePlayer = (st.activePlayer - 1 + st.numPlayers) % st.numPlayers;
                st.cursorIdx = 3;
            }
            break;
        case ACT_RIGHT:
            if (st.cursorIdx < 3) {
                st.cursorIdx++;
            } else if (st.numPlayers > 1) {
                st.activePlayer = (st.activePlayer + 1) % st.numPlayers;
                st.cursorIdx = 0;
            }
            break;
        case ACT_UP:
            modifyDigit(st, 1);
            break;
        case ACT_DOWN:
            modifyDigit(st, -1);
            break;
        case ACT_A:
            if (st.numPlayers > 1) {
                st.activePlayer = (st.activePlayer + 1) % st.numPlayers;
            }
            st.cursorIdx = 0;
            break;
        case ACT_B:
            rollDice(st);
            break;
        case ACT_NORTH:
            halveActivePlayerLP(st);
            break;
        case ACT_EAST:
            flipCoin(st);
            break;
        default:
            break;
    }
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    std::srand((unsigned int)std::time(nullptr));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK) != 0) {
        std::fprintf(stderr, "Error SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    SDL_ShowCursor(SDL_DISABLE);

    SDL_DisplayMode dm;
    int winW = 640, winH = 480;
    if (SDL_GetCurrentDisplayMode(0, &dm) == 0 && dm.w > 0 && dm.h > 0) {
        winW = dm.w;
        winH = dm.h;
    }

    SDL_Window* win = SDL_CreateWindow(
        "Yu-Gi-Oh! Duel Calculator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        winW, winH,
        SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_SHOWN
    );
    if (!win) {
        std::fprintf(stderr, "Error SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) {
        ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    }

    // Abrir mandos / joysticks conectados
    std::vector<SDL_GameController*> controllers;
    std::vector<SDL_Joystick*> joysticks;
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            SDL_GameController* gc = SDL_GameControllerOpen(i);
            if (gc) controllers.push_back(gc);
        } else {
            SDL_Joystick* js = SDL_JoystickOpen(i);
            if (js) joysticks.push_back(js);
        }
    }

    DuelState state;
    resetDuel(state);

    bool running = true;
    int lastHat = SDL_HAT_CENTERED;
    bool axisXActive = false, axisYActive = false;

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            // Soporte SDL_GameController (mapeo estándar SDL)
            else if (e.type == SDL_CONTROLLERBUTTONDOWN) {
                switch (e.cbutton.button) {
                    case SDL_CONTROLLER_BUTTON_DPAD_UP:    handleAction(state, ACT_UP, running); break;
                    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:  handleAction(state, ACT_DOWN, running); break;
                    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:  handleAction(state, ACT_LEFT, running); break;
                    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: handleAction(state, ACT_RIGHT, running); break;
                    case SDL_CONTROLLER_BUTTON_A:          handleAction(state, ACT_A, running); break;
                    case SDL_CONTROLLER_BUTTON_B:          handleAction(state, ACT_B, running); break;
                    case SDL_CONTROLLER_BUTTON_Y:          handleAction(state, ACT_NORTH, running); break; // Norte
                    case SDL_CONTROLLER_BUTTON_X:          handleAction(state, ACT_EAST, running); break;  // Este
                    case SDL_CONTROLLER_BUTTON_BACK:       handleAction(state, ACT_SELECT_DOWN, running); break;
                    case SDL_CONTROLLER_BUTTON_START:      handleAction(state, ACT_START_DOWN, running); break;
                    default: break;
                }
            }
            else if (e.type == SDL_CONTROLLERBUTTONUP) {
                if (e.cbutton.button == SDL_CONTROLLER_BUTTON_BACK)  handleAction(state, ACT_SELECT_UP, running);
                if (e.cbutton.button == SDL_CONTROLLER_BUTTON_START) handleAction(state, ACT_START_UP, running);
            }
            // Fallback SDL_Joystick crudo (para mandos sin entrada en gamecontrollerdb)
            else if (e.type == SDL_JOYHATMOTION) {
                uint8_t v = e.jhat.value;
                if ((v & SDL_HAT_UP) && !(lastHat & SDL_HAT_UP))       handleAction(state, ACT_UP, running);
                if ((v & SDL_HAT_DOWN) && !(lastHat & SDL_HAT_DOWN))   handleAction(state, ACT_DOWN, running);
                if ((v & SDL_HAT_LEFT) && !(lastHat & SDL_HAT_LEFT))   handleAction(state, ACT_LEFT, running);
                if ((v & SDL_HAT_RIGHT) && !(lastHat & SDL_HAT_RIGHT)) handleAction(state, ACT_RIGHT, running);
                lastHat = v;
            }
            else if (e.type == SDL_JOYAXISMOTION) {
                if (e.jaxis.axis == 0) {
                    if (e.jaxis.value < -16000 && !axisXActive) { axisXActive = true; handleAction(state, ACT_LEFT, running); }
                    else if (e.jaxis.value > 16000 && !axisXActive) { axisXActive = true; handleAction(state, ACT_RIGHT, running); }
                    else if (e.jaxis.value > -8000 && e.jaxis.value < 8000) axisXActive = false;
                } else if (e.jaxis.axis == 1) {
                    if (e.jaxis.value < -16000 && !axisYActive) { axisYActive = true; handleAction(state, ACT_UP, running); }
                    else if (e.jaxis.value > 16000 && !axisYActive) { axisYActive = true; handleAction(state, ACT_DOWN, running); }
                    else if (e.jaxis.value > -8000 && e.jaxis.value < 8000) axisYActive = false;
                }
            }
            else if (e.type == SDL_JOYBUTTONDOWN) {
                switch (e.jbutton.button) {
                    case 0: handleAction(state, ACT_A, running); break;
                    case 1: handleAction(state, ACT_B, running); break;
                    case 2: handleAction(state, ACT_EAST, running); break;
                    case 3: handleAction(state, ACT_NORTH, running); break;
                    case 6:
                    case 8: handleAction(state, ACT_SELECT_DOWN, running); break;
                    case 7:
                    case 9: handleAction(state, ACT_START_DOWN, running); break;
                    default: break;
                }
            }
            else if (e.type == SDL_JOYBUTTONUP) {
                if (e.jbutton.button == 6 || e.jbutton.button == 8) handleAction(state, ACT_SELECT_UP, running);
                if (e.jbutton.button == 7 || e.jbutton.button == 9) handleAction(state, ACT_START_UP, running);
            }
            // Soporte teclado (para pruebas en PC o teclados USB)
            else if (e.type == SDL_KEYDOWN && !e.key.repeat) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP:        handleAction(state, ACT_UP, running); break;
                    case SDLK_DOWN:      handleAction(state, ACT_DOWN, running); break;
                    case SDLK_LEFT:      handleAction(state, ACT_LEFT, running); break;
                    case SDLK_RIGHT:     handleAction(state, ACT_RIGHT, running); break;
                    case SDLK_RETURN:
                    case SDLK_z:         handleAction(state, ACT_A, running); break;
                    case SDLK_x:         handleAction(state, ACT_B, running); break;
                    case SDLK_c:         handleAction(state, ACT_NORTH, running); break;
                    case SDLK_v:         handleAction(state, ACT_EAST, running); break;
                    case SDLK_RSHIFT:
                    case SDLK_LSHIFT:    handleAction(state, ACT_SELECT_DOWN, running); break;
                    case SDLK_SPACE:     handleAction(state, ACT_START_DOWN, running); break;
                    case SDLK_ESCAPE:
                    case SDLK_q:         handleAction(state, ACT_QUIT, running); break;
                    default: break;
                }
            }
            else if (e.type == SDL_KEYUP) {
                if (e.key.keysym.sym == SDLK_RSHIFT || e.key.keysym.sym == SDLK_LSHIFT) handleAction(state, ACT_SELECT_UP, running);
                if (e.key.keysym.sym == SDLK_SPACE) handleAction(state, ACT_START_UP, running);
            }
        }

        int curW = winW, curH = winH;
        SDL_GetRendererOutputSize(ren, &curW, &curH);
        renderFrame(ren, state, curW, curH);
        SDL_Delay(16); // ~60 FPS
    }

    for (auto* gc : controllers) SDL_GameControllerClose(gc);
    for (auto* js : joysticks) SDL_JoystickClose(js);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
