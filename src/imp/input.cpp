#include "types.h"
#include <strings.h>

void handle_command(Game& g) {
    char cmd[256] = {0};

    curs_set(1);
    echo();
    timeout(-1);

    mvhline(g.term_h - 1, 0, ' ', g.term_w);
    attron(COLOR_PAIR(CP_UI) | A_BOLD);
    mvaddstr(g.term_h - 1, 0, ":");
    attroff(COLOR_PAIR(CP_UI) | A_BOLD);
    move(g.term_h - 1, 1);
    wgetnstr(stdscr, cmd, 254);

    noecho();
    curs_set(0);
    timeout(100);

    if (cmd[0] == '\0') return;

    char copy[256];
    strncpy(copy, cmd, 255);
    copy[255] = '\0';

    char* token = strtok(copy, " ");
    if (!token) return;

    if (strcmp(token, "attack") == 0) {
        char* type_str = strtok(NULL, " ");
        if (type_str) {
            if (strcmp(type_str, "ground") == 0)
                g.sel_unit = U_GROUND;
            else if (strcmp(type_str, "naval") == 0)
                g.sel_unit = U_NAVAL;
            else if (strcmp(type_str, "air") == 0)
                g.sel_unit = U_AIR;
        }
    } else if (strcmp(token, "economy") == 0) {
        g.mode = MODE_ECONOMY;
        render_economy(g);
        g.mode = MODE_NORMAL;
    } else if (strcmp(token, "help") == 0) {
        g.mode = MODE_INFO;
        render_info(g);
        g.mode = MODE_NORMAL;
    } else if (strcmp(token, "menu") == 0) {
        g.mode = MODE_MAIN_MENU;
        g.menu_cursor = 0;
    } else if (strcmp(token, "logger") == 0) {
        char* action = strtok(NULL, " ");
        if (action && strcmp(action, "start") == 0) {
            g.logging_enabled = true;
        } else if (action && strcmp(action, "stop") == 0) {
            g.logging_enabled = false;
        }
    } else {
        int idx = plugin_find_by_name(g, token);
        if (idx >= 0) {
            char args[256] = {0};
            char* rest = strtok(NULL, "");
            if (rest) { strncpy(args, rest, 255); args[255] = '\0'; }
            plugin_run(g, idx, args);
        }
    }
}

void process_menu_input(Game& g) {
    int ch = getch();
    if (ch == ERR) return;

    switch (g.mode) {
        case MODE_MAIN_MENU: {
            const int items = 5;
            switch (ch) {
                case KEY_UP:    case 'k': g.menu_cursor = (g.menu_cursor - 1 + items) % items; break;
                case KEY_DOWN:  case 'j': g.menu_cursor = (g.menu_cursor + 1) % items; break;
                case '\n': case KEY_ENTER:
                    switch (g.menu_cursor) {
                        case 0: // Start Game
                            g.mode = MODE_NORMAL;
                            erase();
                            break;
                        case 1: // Settings
                            g.mode = MODE_SETTINGS;
                            g.menu_cursor = 0;
                            break;
                        case 2: // Plugins
                            g.mode = MODE_PLUGINS;
                            g.menu_cursor = 0;
                            break;
                        case 3: // About
                            g.mode = MODE_AUTHOR_INFO;
                            break;
                        case 4: // Quit
                            g.running = false;
                            break;
                    }
                    break;
                case 'q': case 'Q':
                    g.running = false;
                    break;
            }
            break;
        }

        case MODE_SETTINGS: {
            const int items = 6;
            switch (ch) {
                case KEY_UP:    case 'k': g.menu_cursor = (g.menu_cursor - 1 + items) % items; break;
                case KEY_DOWN:  case 'j': g.menu_cursor = (g.menu_cursor + 1) % items; break;
                case '\n': case KEY_ENTER:
                    if (g.menu_cursor == 5) { // Back
                        g.mode = MODE_MAIN_MENU;
                        g.menu_cursor = 0;
                        break;
                    }
                    // Edit field
                    g.edit_mode = true;
                    g.edit_field = g.menu_cursor;
                    break;
                case 27: case 'q': case 'Q': // Escape / Q
                    g.mode = MODE_MAIN_MENU;
                    g.menu_cursor = 0;
                    break;
            }
            break;
        }

        case MODE_PLUGINS: {
            int items = g.plugin_count + 2; // plugins + "Add custom" + "Back"
            switch (ch) {
                case KEY_UP:    case 'k': g.menu_cursor = (g.menu_cursor - 1 + items) % items; break;
                case KEY_DOWN:  case 'j': g.menu_cursor = (g.menu_cursor + 1) % items; break;
                case ' ': case '\n': case KEY_ENTER:
                    if (g.menu_cursor == g.plugin_count) { // Run all
                        plugin_run_all(g);
                    } else if (g.menu_cursor == g.plugin_count + 1) { // Back
                        g.mode = MODE_MAIN_MENU;
                        g.menu_cursor = 0;
                    } else if (ch == ' ') {
                        plugin_toggle(g, g.menu_cursor);
                    } else {
                        plugin_run(g, g.menu_cursor);
                    }
                    break;
                case 'a': case 'A': { // Add custom
                    curs_set(1);
                    echo();
                    timeout(-1);
                    mvhline(g.term_h - 1, 0, ' ', g.term_w);
                    attron(COLOR_PAIR(CP_UI) | A_BOLD);
                    mvaddstr(g.term_h - 1, 0, "Path: ");
                    char p[256] = {0};
                    move(g.term_h - 1, 6);
                    wgetnstr(stdscr, p, 254);
                    noecho();
                    curs_set(0);
                    timeout(100);
                    if (p[0]) plugin_add_custom(g, p);
                    break;
                }
                case 27: case 'q': case 'Q':
                    g.mode = MODE_MAIN_MENU;
                    g.menu_cursor = 0;
                    break;
            }
            break;
        }

        case MODE_AUTHOR_INFO: {
            if (ch != ERR) {
                g.mode = MODE_MAIN_MENU;
                g.menu_cursor = 3;
            }
            break;
        }

        default:
            break;
    }
}

void process_input(Game& g) {
    if (g.mode == MODE_GAMEOVER) {
        render_gameover(g);
        return;
    }

    if (g.mode == MODE_MAIN_MENU || g.mode == MODE_SETTINGS ||
        g.mode == MODE_PLUGINS || g.mode == MODE_AUTHOR_INFO) {
        process_menu_input(g);
        return;
    }

    int ch = getch();
    if (ch == ERR) return;

    switch (ch) {
        case 'q': case 'Q':
            g.running = false;
            break;

        case ':':
            handle_command(g);
            break;

        case ' ':
            g.pause = !g.pause;
            break;

        case KEY_UP:    g.cam_y -= 3; break;
        case KEY_DOWN:  g.cam_y += 3; break;
        case KEY_LEFT:  g.cam_x -= 3; break;
        case KEY_RIGHT: g.cam_x += 3; break;

        case 'w': g.cam_y -= 3; break;
        case 's': g.cam_y += 3; break;
        case 'a': g.cam_x -= 3; break;
        case 'd': g.cam_x += 3; break;

        case 0x0446: case 0x0426: g.cam_y -= 3; break;
        case 0x044B: case 0x042B: g.cam_y += 3; break;
        case 0x0444: case 0x0424: g.cam_x -= 3; break;
        case 0x0432: case 0x0412: g.cam_x += 3; break;

        case 'h': g.cam_x -= 3; break;
        case 'l': g.cam_x += 3; break;
        case 'j': g.cam_y += 3; break;
        case 'k': g.cam_y -= 3; break;

        case '1': g.sel_unit = U_GROUND; break;
        case '2': g.sel_unit = U_NAVAL; break;
        case '3': g.sel_unit = U_AIR; break;

        case '\n':
        case KEY_ENTER: {
            int mx = g.cam_x + g.view_w / 2;
            int my = g.cam_y + g.view_h / 2;
            g.selected_province_x = mx;
            g.selected_province_y = my;
            g.has_selection = true;
            break;
        }

                case 'b': case 'B':
            if (g.has_selection) {
                int mx = g.selected_province_x;
                int my = g.selected_province_y;
                Province& p = g.map[my * MAP_W + mx];
                int owner = p.owner;
                if (owner > 0 && g.countries[owner].gold >= unit_cost((UnitType)g.sel_unit)) {
                    if (can_place_unit(g, mx, my, (UnitType)g.sel_unit)) {
                        init_unit(p.unit, (UnitType)g.sel_unit);
                        p.has_unit = true;
                        g.countries[owner].gold -= unit_cost((UnitType)g.sel_unit);
                        g.countries[owner].unit_counts[g.sel_unit]++;
                        const char* utypes[] = {"ground", "naval", "air"};
                        log_action(g, "BUILD", utypes[g.sel_unit]);
                    }
                }
            }
            break;

        case 'A':
            g.attack_mode = !g.attack_mode;
            log_action(g, "ATTACK_MODE", g.attack_mode ? "on" : "off");
            break;

        case 'e': case 'E':
            if (!g.attack_mode) {
                g.mode = MODE_ECONOMY;
                render_economy(g);
                g.mode = MODE_NORMAL;
            }
            break;

        case 'i': case 'I':
            g.mode = MODE_INFO;
            render_info(g);
            g.mode = MODE_NORMAL;
            break;

        case 'D':
            if (g.has_selection) {
                int mx = g.selected_province_x;
                int my = g.selected_province_y;
                Province& p = g.map[my * MAP_W + mx];
                if (p.owner > 0 && p.dev < MAX_DEV && g.countries[p.owner].gold >= 25) {
                    p.dev++;
                    g.countries[p.owner].gold -= 25;
                    char d[32];
                    snprintf(d, sizeof(d), "province (%d,%d) dev=%d", mx, my, p.dev);
                    log_action(g, "DEVELOP", d);
                }
            }
            break;

        case 'r': case 'R':
            if (g.has_selection) {
                int mx = g.selected_province_x;
                int my = g.selected_province_y;
                Province& p = g.map[my * MAP_W + mx];
                if (p.owner > 0 && p.has_unit) {
                    int c = p.owner;
                    int ut = p.unit.type;
                    if (g.countries[c].dev_bonus[ut] < 5 && g.countries[c].gold >= 40) {
                        g.countries[c].dev_bonus[ut]++;
                        g.countries[c].gold -= 40;
                        const char* utypes[] = {"ground", "naval", "air"};
                        char r[32];
                        snprintf(r, sizeof(r), "%s bonus=%d", utypes[ut], g.countries[c].dev_bonus[ut]);
                        log_action(g, "RESEARCH", r);
                    }
                }
            }
            break;

        case 'm': case 'M':
            if (g.has_selection) {
                int mx = g.selected_province_x;
                int my = g.selected_province_y;
                Province& p = g.map[my * MAP_W + mx];
                if (p.has_unit && p.owner == 1) {
                    int tx = g.cam_x + g.view_w / 2;
                    int ty = g.cam_y + g.view_h / 2;
                    int dx = (tx > mx) ? 1 : (tx < mx) ? -1 : 0;
                    int dy = (ty > my) ? 1 : (ty < my) ? -1 : 0;
                    int nx = mx + dx, ny = my + dy;
                    if (on_map(nx, ny)) {
                        Province& target = g.map[ny * MAP_W + nx];
                        if (!target.has_unit && (target.owner == 1 || target.owner == 0)) {
                            if (p.unit.type != U_NAVAL && target.terrain == WATER) break;
                            target.has_unit = true;
                            target.unit = p.unit;
                            target.owner = p.owner;
                            p.has_unit = false;
                            g.selected_province_x = nx;
                            g.selected_province_y = ny;
                            char m[64];
                            snprintf(m, sizeof(m), "from (%d,%d) to (%d,%d)", mx, my, nx, ny);
                            log_action(g, "MOVE", m);
                        }
                    }
                }
            }
            break;
    }

    if (g.cam_x < 0) g.cam_x = 0;
    if (g.cam_y < 0) g.cam_y = 0;
    if (g.cam_x > MAP_W - 1) g.cam_x = MAP_W - 1;
    if (g.cam_y > MAP_H - 1) g.cam_y = MAP_H - 1;
}
