#include <locale.h>
#include <string.h>
#include "types.h"

int main() {
    setlocale(LC_ALL, "");
    srand(time(NULL));

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(100);

    if (!has_colors()) {
        endwin();
        printf("Terminal doesn't support colors!\n");
        return 1;
    }

    GameConfig cfg;
    load_config("rts.cfg", cfg);
    timeout(1000 / cfg.update_rate);
    init_colors(cfg);

    Game g;
    memset(&g, 0, sizeof(g));
    g.config = cfg;
    g.mode = MODE_MAIN_MENU;
    g.menu_cursor = 0;
    setup_game(g);
    g.mode = MODE_MAIN_MENU;

    while (g.running) {
        process_input(g);
        if (g.mode == MODE_NORMAL) {
            update(g);
        }
        if (g.mode == MODE_GAMEOVER) {
            render_gameover(g);
            if (!g.running) break;
            destroy_game(g);
            memset(&g, 0, sizeof(g));
            g.config = cfg;
            setup_game(g);
            g.mode = MODE_MAIN_MENU;
            continue;
        }
        render(g);
    }

    destroy_game(g);
    endwin();
    printf("Thanks for playing REALLYFRONTHOLE!\n");
    return 0;
}
