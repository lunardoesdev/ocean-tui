#include "types.h"
#include <string.h>
#include <cmath>

const char* unit_chars = "GNA";
const char* terrain_names[] = {"Water", "Plains", "Forest", "Mountain", "City"};
const char* unit_names[] = {"Ground", "Naval", "Air"};

void init_colors(GameConfig& cfg) {
    start_color();
    use_default_colors();

    init_pair(CP_WATER,    COLOR_WHITE,   COLOR_BLUE);
    init_pair(CP_PLAINS,   COLOR_WHITE,   COLOR_GREEN);
    init_pair(CP_FOREST,   COLOR_GREEN,   COLOR_GREEN);
    init_pair(CP_MOUNTAIN, COLOR_WHITE,   COLOR_YELLOW);
    init_pair(CP_CITY,     COLOR_YELLOW,  COLOR_WHITE);

    init_pair(CP_C1,       COLOR_RED,     COLOR_RED);
    init_pair(CP_C2,       COLOR_CYAN,    COLOR_CYAN);

    init_pair(CP_C1_UNIT,  COLOR_RED,     COLOR_WHITE);
    init_pair(CP_C2_UNIT,  COLOR_CYAN,    COLOR_WHITE);

    init_pair(CP_UI,       cfg.primary_color, COLOR_BLACK);
    init_pair(CP_SEL,      COLOR_BLACK,   cfg.secondary_color);
    init_pair(CP_HP_G,     COLOR_GREEN,   COLOR_BLACK);
    init_pair(CP_HP_Y,     COLOR_YELLOW,  COLOR_BLACK);
    init_pair(CP_HP_R,     COLOR_RED,     COLOR_BLACK);
}

// --- Menu helpers ---

static void draw_menu_item(int y, int x, const char* text, bool selected) {
    if (selected) {
        attron(COLOR_PAIR(CP_SEL) | A_BOLD);
        mvaddstr(y, x - 1, " ");
        mvaddstr(y, x, text);
        attroff(COLOR_PAIR(CP_SEL) | A_BOLD);
    } else {
        attron(COLOR_PAIR(CP_UI));
        mvaddstr(y, x, text);
        attroff(COLOR_PAIR(CP_UI));
    }
}

// --- Pixel Shader ---

static void shader_dots(Game& g, int frame) {
    int colors[] = {CP_C1, CP_C2, CP_MOUNTAIN, CP_UI, CP_HP_Y, CP_WATER};
    char dots[] = {'.', '*', '+', '#', ':', 'o'};
    for (int i = 0; i < 60; i++) {
        int x = (frame * 7 + i * 13 + frame / 3) % g.term_w;
        int y = (frame * 11 + i * 17 + frame / 5) % g.term_h;
        int ci = (frame + i * 3) % 6;
        int di = (frame / 2 + i * 5) % 6;
        attron(COLOR_PAIR(colors[ci]) | A_DIM);
        mvaddch(y, x, dots[di]);
        attroff(COLOR_PAIR(colors[ci]) | A_DIM);
    }
    for (int i = 0; i < 20; i++) {
        int x = (frame * 13 + i * 7 + i * 11) % g.term_w;
        int y = (frame * 3 + i * 23 + i * 5) % g.term_h;
        int ci = (frame / 3 + i * 2) % 6;
        attron(COLOR_PAIR(colors[ci]) | A_BOLD);
        mvaddch(y, x, '.');
        attroff(COLOR_PAIR(colors[ci]) | A_BOLD);
    }
}

static void shader_matrix(Game& g, int frame) {
    for (int i = 0; i < 80; i++) {
        int x = i * 3 % g.term_w;
        int y = (frame * 2 + i * 7 + rand() % 3) % g.term_h;
        char c = "0123456789ABCDEF"[rand() % 16];
        int bright = (y + frame / 2) % 3;
        attron(COLOR_PAIR(CP_HP_G) | (bright == 0 ? A_BOLD : A_DIM));
        mvaddch(y, x, c);
        attroff(COLOR_PAIR(CP_HP_G) | (bright == 0 ? A_BOLD : A_DIM));
    }
}

static void shader_stars(Game& g, int frame) {
    for (int i = 0; i < 80; i++) {
        int x = (frame * 3 + i * 17) % g.term_w;
        int y = (frame * 5 + i * 11) % g.term_h;
        int bright = (frame + i) % 4;
        if (bright == 0) continue;
        attron(COLOR_PAIR(CP_UI) | (bright > 2 ? A_BOLD : A_DIM));
        mvaddch(y, x, '.');
        attroff(COLOR_PAIR(CP_UI) | (bright > 2 ? A_BOLD : A_DIM));
    }
}

static void shader_wave(Game& g, int frame) {
    int colors[] = {CP_C1, CP_HP_Y, CP_C2, CP_MOUNTAIN, CP_WATER};
    for (int x = 0; x < g.term_w; x += 2) {
        int y = g.term_h / 2 + (int)(sinf((x + frame) * 0.1f) * (g.term_h / 4));
        if (y < 0 || y >= g.term_h) continue;
        int ci = (x / 2 + frame / 10) % 5;
        attron(COLOR_PAIR(colors[ci]) | A_BOLD);
        mvaddch(y, x, '#');
        attroff(COLOR_PAIR(colors[ci]) | A_BOLD);
    }
}

static void shader_fire(Game& g, int frame) {
    int fire_colors[] = {CP_HP_R, CP_HP_Y, CP_MOUNTAIN, CP_UI};
    for (int i = 0; i < 50; i++) {
        int x = (frame * 5 + i * 11) % g.term_w;
        int y = g.term_h - 1 - ((frame * 3 + i * 7) % (g.term_h / 2 + 5));
        int ci = (y + frame / 5) % 4;
        attron(COLOR_PAIR(fire_colors[ci]) | A_BOLD);
        mvaddch(y, x, '^');
        attroff(COLOR_PAIR(fire_colors[ci]) | A_BOLD);
    }
}

static void shader_plasma(Game& g, int frame) {
    int colors[] = {CP_C1, CP_C2, CP_HP_Y, CP_HP_G, CP_MOUNTAIN};
    for (int x = 0; x < g.term_w; x += 3) {
        for (int y = 0; y < g.term_h; y += 2) {
            float v = sinf(x * 0.05f + frame * 0.02f) + sinf(y * 0.05f + frame * 0.03f) + sinf((x + y + frame) * 0.04f);
            int ci = ((int)(v * 2 + 10)) % 5;
            attron(COLOR_PAIR(colors[ci]) | A_DIM);
            mvaddch(y, x, '.');
            attroff(COLOR_PAIR(colors[ci]) | A_DIM);
        }
    }
}

static void render_pixel_shader(Game& g) {
    g.menu_page++;
    int frame = g.menu_page;
    switch (g.config.shader_style % 6) {
        case 0: shader_dots(g, frame); break;
        case 1: shader_matrix(g, frame); break;
        case 2: shader_stars(g, frame); break;
        case 3: shader_wave(g, frame); break;
        case 4: shader_fire(g, frame); break;
        case 5: shader_plasma(g, frame); break;
    }
}

// --- Main Menu ---

void render_main_menu(Game& g) {
    getmaxyx(stdscr, g.term_h, g.term_w);
    int cx = g.term_w / 2;
    int cy = g.term_h / 2;

    erase();

    render_pixel_shader(g);

    attron(COLOR_PAIR(CP_UI) | A_BOLD);
    mvaddstr(cy - 7, cx - 10, "REALLYFRONTHOLE");
    attroff(COLOR_PAIR(CP_UI) | A_BOLD);

    attron(COLOR_PAIR(CP_UI));
    mvaddstr(cy - 5, cx - 10, "Terminal RTS Game");
    attroff(COLOR_PAIR(CP_UI));

    const char* items[] = {"Start Game", "Settings", "Plugins", "About", "Quit"};
    for (int i = 0; i < 5; i++) {
        draw_menu_item(cy - 2 + i * 2, cx - 6, items[i], g.menu_cursor == i);
    }

    attron(COLOR_PAIR(CP_UI));
    mvaddstr(cy + 9, cx - 15, "Arrows: Navigate  Enter: Select  Q: Quit");
    attroff(COLOR_PAIR(CP_UI));

    refresh();
}

static void edit_config_field(Game& g) {
    const char* hints[] = {
        "Enter color name (red, green, yellow, blue, magenta, cyan, white, black)",
        "Enter color name (red, green, yellow, blue, magenta, cyan, white, black)",
        "Enter update rate (1-60 ticks per second)",
        "Enter author description text",
        "Enter shader style (0=Dots, 1=Matrix, 2=Stars, 3=Wave, 4=Fire, 5=Plasma)",
    };
    curs_set(1);
    echo();
    timeout(-1);

    mvhline(g.term_h - 1, 0, ' ', g.term_w);
    attron(COLOR_PAIR(CP_UI) | A_BOLD);
    mvaddstr(g.term_h - 1, 0, "> ");
    attroff(COLOR_PAIR(CP_UI) | A_BOLD);
    mvaddstr(g.term_h - 1, 2, hints[g.edit_field]);
    move(g.term_h - 1, 2 + strlen(hints[g.edit_field]) + 1);

    char val[512] = {0};
    if (g.edit_field < 2) {
        const char* cname = color_name(g.edit_field == 0 ? g.config.primary_color : g.config.secondary_color);
        strncpy(val, cname, 511);
    } else if (g.edit_field == 2) {
        snprintf(val, sizeof(val), "%d", g.config.update_rate);
    } else if (g.edit_field == 4) {
        snprintf(val, sizeof(val), "%d", g.config.shader_style);
    } else {
        memcpy(val, g.config.author_info, 511);
        val[511] = '\0';
    }
    wgetnstr(stdscr, val, 510);

    noecho();
    curs_set(0);
    timeout(100);

    if (val[0]) {
        switch (g.edit_field) {
            case 0: g.config.primary_color = parse_color(val); break;
            case 1: g.config.secondary_color = parse_color(val); break;
            case 2: {
                int v = atoi(val);
                if (v < 1) v = 1;
                if (v > 60) v = 60;
                g.config.update_rate = v;
                timeout(1000 / g.config.update_rate);
                break;
            }
            case 3:
                memcpy(g.config.author_info, val, 511);
                g.config.author_info[511] = '\0';
                break;
            case 4: {
                int v = atoi(val);
                if (v < 0) v = 0;
                if (v > 5) v = 5;
                g.config.shader_style = v;
                break;
            }
        }
        save_config("rts.cfg", g.config);
        init_colors(g.config);
    }

    g.edit_mode = false;
}

// --- Settings Menu ---

void render_settings_menu(Game& g) {
    if (g.edit_mode) {
        edit_config_field(g);
        return;
    }

    getmaxyx(stdscr, g.term_h, g.term_w);
    int cx = g.term_w / 2;

    erase();

    attron(COLOR_PAIR(CP_UI) | A_BOLD);
    mvaddstr(2, cx - 5, "SETTINGS");
    attroff(COLOR_PAIR(CP_UI) | A_BOLD);

    char buf[256];
    int y = 5;

    struct Setting { const char* label; const char* hint; const char* value; };
    auto vs = [&](int idx, Setting s) {
        snprintf(buf, sizeof(buf), "%s: %s", s.label, s.value);
        draw_menu_item(y, cx - 20, buf, g.menu_cursor == idx);
        y++;
        attron(COLOR_PAIR(CP_UI));
        mvaddstr(y, cx - 18, s.hint);
        attroff(COLOR_PAIR(CP_UI));
        y += 2;
    };

    char rate_str[16], shader_str[32];
    snprintf(rate_str, sizeof(rate_str), "%d", g.config.update_rate);
    const char* shader_names[] = {"Dots", "Matrix", "Stars", "Wave", "Fire", "Plasma"};
    snprintf(shader_str, sizeof(shader_str), "%d (%s)", g.config.shader_style % 6,
        shader_names[g.config.shader_style % 6]);

    Setting settings[] = {
        {"Primary Color",   "UI main colour (red, green, blue, ...)",        color_name(g.config.primary_color)},
        {"Secondary Color", "Selection/highlight colour",                    color_name(g.config.secondary_color)},
        {"Update Rate",     "Game speed (1-60 ticks/sec)",                   rate_str},
        {"Author Info",     "Text shown on About screen",                    g.config.author_info},
        {"Shader Style",    "0:Dots 1:Matrix 2:Stars 3:Wave 4:Fire 5:Plasma", shader_str},
    };

    for (int i = 0; i < 5; i++)
        vs(i, settings[i]);

    draw_menu_item(y, cx - 3, "Back", g.menu_cursor == 5);

    attron(COLOR_PAIR(CP_UI));
    mvaddstr(g.term_h - 1, cx - 16, "Arrows: Navigate  Enter: Edit  Esc: Back");
    attroff(COLOR_PAIR(CP_UI));

    refresh();
}

// --- Plugins Menu ---

void render_plugins_menu(Game& g) {
    getmaxyx(stdscr, g.term_h, g.term_w);
    int cx = g.term_w / 2;

    erase();

    attron(COLOR_PAIR(CP_UI) | A_BOLD);
    mvaddstr(2, cx - 8, "PLUGIN MANAGER");
    attroff(COLOR_PAIR(CP_UI) | A_BOLD);

    int y = 4;
    for (int i = 0; i < g.plugin_count && y < g.term_h - 6; i++) {
        PluginEntry& p = g.plugins[i];
        char line[128];
        snprintf(line, sizeof(line), "%c %s  --  %s",
            p.enabled ? 'X' : ' ',
            p.name,
            p.description);

        if (g.menu_cursor == i) {
            attron(COLOR_PAIR(CP_SEL) | A_BOLD);
            mvaddstr(y, cx - 30, " ");
            mvaddstr(y, cx - 28, line);
            attroff(COLOR_PAIR(CP_SEL) | A_BOLD);
        } else {
            attron(COLOR_PAIR(p.enabled ? CP_C1 : CP_UI));
            mvaddstr(y, cx - 28, line);
            attroff(COLOR_PAIR(p.enabled ? CP_C1 : CP_UI));
        }
        y++;
    }

    y++;
    // Run all enabled
    if (g.menu_cursor == g.plugin_count) {
        attron(COLOR_PAIR(CP_SEL) | A_BOLD);
        mvaddstr(y, cx - 3, " ");
        mvaddstr(y + 0, cx - 1, "Run All Enabled");
        attroff(COLOR_PAIR(CP_SEL) | A_BOLD);
    } else {
        attron(COLOR_PAIR(CP_UI));
        mvaddstr(y, cx - 1, "Run All Enabled");
        attroff(COLOR_PAIR(CP_UI));
    }
    y++;
    if (g.menu_cursor == g.plugin_count + 1) {
        attron(COLOR_PAIR(CP_SEL) | A_BOLD);
        mvaddstr(y, cx - 3, " ");
        mvaddstr(y + 0, cx - 1, "Back");
        attroff(COLOR_PAIR(CP_SEL) | A_BOLD);
    } else {
        attron(COLOR_PAIR(CP_UI));
        mvaddstr(y, cx - 1, "Back");
        attroff(COLOR_PAIR(CP_UI));
    }

    attron(COLOR_PAIR(CP_UI));
    mvaddstr(g.term_h - 1, cx - 28,
        "Arrows: Navigate  Space: Toggle  Enter: Run  A: Add custom  Esc: Back");
    attroff(COLOR_PAIR(CP_UI));

    refresh();
}

// --- Author Info ---

void render_author_info(Game& g) {
    getmaxyx(stdscr, g.term_h, g.term_w);
    int cx = g.term_w / 2;
    int cy = g.term_h / 2;

    erase();

    attron(COLOR_PAIR(CP_UI) | A_BOLD);
    mvaddstr(cy - 6, cx - 4, "ABOUT");
    attroff(COLOR_PAIR(CP_UI) | A_BOLD);

    attron(COLOR_PAIR(CP_UI));
    mvaddstr(cy - 4, cx - 10, "REALLYFRONTHOLE");
    mvaddstr(cy - 3, cx - 10, "Terminal RTS Game");

    int y = cy;
    char* line = strtok(g.config.author_info, "\n");
    while (line && y < cy + 10) {
        // Word-wrap at ~80 chars
        int len = strlen(line);
        int pos = 0;
        while (pos < len) {
            int chunk = (len - pos > 78) ? 78 : (len - pos);
            char buf[256];
            strncpy(buf, line + pos, chunk);
            buf[chunk] = '\0';
            mvaddstr(y++, cx - chunk / 2, buf);
            if (y > cy + 8) break;
            pos += chunk;
        }
        line = strtok(NULL, "\n");
    }
    if (y == cy) {
        mvaddstr(y, cx - strlen(g.config.author_info) / 2, g.config.author_info);
    }
    attroff(COLOR_PAIR(CP_UI));

    attron(COLOR_PAIR(CP_UI));
    mvaddstr(cy + 10, cx - 15, "Press any key to return...");
    attroff(COLOR_PAIR(CP_UI));

    refresh();
}

// --- Legacy renders ---

void render_tile(Game& g, int mx, int my, int scr_x, int scr_y) {
    if (!on_map(mx, my)) return;
    Province& p = g.map[my * MAP_W + mx];

    int pair = CP_WATER;
    char ch = '~';
    bool show_unit = false;
    char unit_ch = 'G';
    int unit_pair = CP_C1_UNIT;

    switch (p.terrain) {
        case WATER:
            ch = '~'; pair = CP_WATER;
            break;
        case PLAINS:
            ch = '.'; pair = CP_PLAINS;
            if ((mx + my) % 3 == 0) ch = '.';
            else ch = ',';
            break;
        case FOREST:
            ch = '&'; pair = CP_FOREST;
            break;
        case MOUNTAIN:
            ch = '^'; pair = CP_MOUNTAIN;
            break;
        case CITY:
            ch = '#'; pair = CP_CITY;
            if (p.owner == 1) pair = CP_C1;
            else if (p.owner == 2) pair = CP_C2;
            break;
    }

    if (p.has_unit) {
        show_unit = true;
        unit_ch = unit_chars[p.unit.type];
        unit_pair = (p.owner == 1) ? CP_C1_UNIT : CP_C2_UNIT;
        float hp_pct = (float)p.unit.hp / p.unit.max_hp;
        if (hp_pct < 0.3f) unit_pair = CP_HP_R;
        else if (hp_pct < 0.6f) unit_pair = CP_HP_Y;
    }

    if (g.has_selection && mx == g.selected_province_x && my == g.selected_province_y) {
        pair = CP_SEL;
        ch = (show_unit) ? unit_ch : ch;
        attron(COLOR_PAIR(pair) | A_BOLD);
        mvaddch(scr_y, scr_x, ch);
        attroff(COLOR_PAIR(pair) | A_BOLD);
        return;
    }

    if (show_unit) {
        attron(COLOR_PAIR(unit_pair) | A_BOLD);
        mvaddch(scr_y, scr_x, unit_ch);
        attroff(COLOR_PAIR(unit_pair) | A_BOLD);
    } else {
        int display_pair = pair;
        if (p.owner == 1 && p.terrain != CITY && p.terrain != WATER) {
            display_pair = CP_C1;
            ch = (p.terrain == MOUNTAIN) ? '^' : (p.terrain == FOREST) ? '&' : '.';
        } else if (p.owner == 2 && p.terrain != CITY && p.terrain != WATER) {
            display_pair = CP_C2;
            ch = (p.terrain == MOUNTAIN) ? '^' : (p.terrain == FOREST) ? '&' : '.';
        }
        attron(COLOR_PAIR(display_pair));
        mvaddch(scr_y, scr_x, ch);
        attroff(COLOR_PAIR(display_pair));
    }
}

void render_minimap(Game& g, int mx, int my, int mw, int mh) {
    attron(COLOR_PAIR(CP_UI));
    for (int x = 0; x <= mw; x++) {
        mvaddch(my, mx + x, '-');
        mvaddch(my + mh, mx + x, '-');
    }
    for (int y = 0; y <= mh; y++) {
        mvaddch(my + y, mx, '|');
        mvaddch(my + y, mx + mw, '|');
    }
    attroff(COLOR_PAIR(CP_UI));

    for (int y = 0; y < mh - 1; y++)
        for (int x = 0; x < mw - 1; x++) {
            int map_x = x * MAP_W / (mw - 1);
            int map_y = y * MAP_H / (mh - 1);
            Province& p = g.map[map_y * MAP_W + map_x];

            int pair = CP_WATER;
            char ch = ' ';
            if (p.owner == 1) { pair = CP_C1; ch = '#'; }
            else if (p.owner == 2) { pair = CP_C2; ch = '#'; }
            else if (p.terrain != WATER) { pair = CP_PLAINS; ch = '.'; }
            else { pair = CP_WATER; ch = '~'; }

            attron(COLOR_PAIR(pair));
            mvaddch(my + 1 + y, mx + 1 + x, ch);
            attroff(COLOR_PAIR(pair));
        }

    int vx = g.cam_x * (mw - 1) / MAP_W;
    int vy = g.cam_y * (mh - 1) / MAP_H;
    int vw = g.view_w * (mw - 1) / MAP_W;
    int vh = g.view_h * (mh - 1) / MAP_H;
    attron(COLOR_PAIR(CP_SEL));
    mvaddch(my + 1 + vy, mx + 1 + vx, '+');
    mvaddch(my + 1 + vy, mx + 1 + std::min(vx + vw, mw - 2), '+');
    mvaddch(my + 1 + std::min(vy + vh, mh - 2), mx + 1 + vx, '+');
    mvaddch(my + 1 + std::min(vy + vh, mh - 2), mx + 1 + std::min(vx + vw, mw - 2), '+');
    attroff(COLOR_PAIR(CP_SEL));
}

void render(Game& g) {
    if (g.mode == MODE_MAIN_MENU) { render_main_menu(g); return; }
    if (g.mode == MODE_SETTINGS)  { render_settings_menu(g); return; }
    if (g.mode == MODE_PLUGINS)   { render_plugins_menu(g); return; }
    if (g.mode == MODE_AUTHOR_INFO) { render_author_info(g); return; }

    getmaxyx(stdscr, g.term_h, g.term_w);
    g.view_h = (int)(g.term_h * VIEWPORT_FACTOR) - 1;
    g.view_w = g.term_w - 2;

    if (g.cam_x < 0) g.cam_x = 0;
    if (g.cam_y < 0) g.cam_y = 0;
    if (g.cam_x > MAP_W - g.view_w) g.cam_x = MAP_W - g.view_w;
    if (g.cam_y > MAP_H - g.view_h) g.cam_y = MAP_H - g.view_h;

    attron(COLOR_PAIR(CP_UI) | A_BOLD);
    mvhline(0, 0, ' ', g.term_w);
    char date_str[64];
    snprintf(date_str, sizeof(date_str), " REALLYFRONTHOLE  |  %d-%02d  |  Tick: %d  |  View: %d,%d",
        g.year, g.month, g.tick, g.cam_x, g.cam_y);
    mvaddstr(0, 0, date_str);

    if (g.pause)
        mvaddstr(0, g.term_w - 10, " [PAUSED]");

    mvaddstr(0, g.term_w - 25, g.attack_mode ? " [ATK MODE]" : "           ");
    attroff(COLOR_PAIR(CP_UI) | A_BOLD);

    attron(COLOR_PAIR(CP_UI));
    mvhline(1, 0, ' ', g.term_w);
    char stats[256];
    for (int c = 1; c <= 2; c++) {
        attron(COLOR_PAIR(c == 1 ? CP_C1 : CP_C2) | A_BOLD);
        mvaddch(1, (c-1) * g.term_w/2 + 1, ' ');
        attroff(COLOR_PAIR(c == 1 ? CP_C1 : CP_C2) | A_BOLD);
        attron(COLOR_PAIR(CP_UI));
        snprintf(stats, sizeof(stats), "%s | G:%d I:%d U:%d,%d,%d",
            g.countries[c].name, g.countries[c].gold, g.countries[c].income,
            g.countries[c].unit_counts[0], g.countries[c].unit_counts[1], g.countries[c].unit_counts[2]);
        mvaddstr(1, (c-1) * g.term_w/2 + 3, stats);
    }
    attroff(COLOR_PAIR(CP_UI));

    for (int sy = 0; sy < g.view_h && sy < g.term_h - 4; sy++)
        for (int sx = 0; sx < g.view_w; sx++)
            render_tile(g, g.cam_x + sx, g.cam_y + sy, sx, sy + 2);

    int mm_w = 20, mm_h = 10;
    int mm_x = g.term_w - mm_w - 2;
    int mm_y = 2;
    render_minimap(g, mm_x, mm_y, mm_w, mm_h);

    attron(COLOR_PAIR(CP_UI));
    mvhline(g.term_h - 3, 0, ' ', g.term_w);
    mvaddstr(g.term_h - 3, 1,
        "Arrows/WASD/ЦФЫВ:Scroll 1-3:Unit B:Build M:Move A:Atk E:Economy I:Info :Cmd SPACE:P Q:Quit Menu");

    mvhline(g.term_h - 2, 0, ' ', g.term_w);
    if (g.has_selection) {
        int sx = g.selected_province_x, sy = g.selected_province_y;
        Province& p = g.map[sy * MAP_W + sx];
        char info[128];
        if (p.has_unit) {
            float hp_pct = (float)p.unit.hp / p.unit.max_hp * 100;
            snprintf(info, sizeof(info), " [%d,%d] %s | %s | Owner:%d | Dev:%d | HP:%.0f%% ATK:%d DEF:%d",
                sx, sy, terrain_names[p.terrain],
                unit_names[p.unit.type], p.owner, p.dev, hp_pct, p.unit.atk, p.unit.def);
        } else {
            snprintf(info, sizeof(info), " [%d,%d] %s %s | Owner:%d | Dev:%d | Income:%d",
                sx, sy, terrain_names[p.terrain],
                p.city_name[0] ? p.city_name : "",
                p.owner, p.dev,
                (p.terrain == WATER ? 1 : p.terrain == PLAINS ? 5 : p.terrain == FOREST ? 3 : p.terrain == MOUNTAIN ? 4 : 12) * (1 + p.dev));
        }
        mvaddstr(g.term_h - 2, 1, info);
    }
    attroff(COLOR_PAIR(CP_UI));

    refresh();
}

void render_economy(Game& g) {
    attron(COLOR_PAIR(CP_UI) | A_BOLD);
    mvhline(0, 0, ' ', g.term_w);
    mvaddstr(0, g.term_w/2 - 8, "=== ECONOMY OVERVIEW ===");
    attroff(COLOR_PAIR(CP_UI) | A_BOLD);

    int y = 2;
    for (int c = 1; c <= 2; c++) {
        attron(COLOR_PAIR(c == 1 ? CP_C1 : CP_C2) | A_BOLD);
        mvaddch(y, 1, ' ');
        char header[64];
        snprintf(header, sizeof(header), " %s ", g.countries[c].name);
        mvaddstr(y, 3, header);
        attroff(COLOR_PAIR(c == 1 ? CP_C1 : CP_C2) | A_BOLD);
        y += 2;
        char line[128];
        snprintf(line, sizeof(line), "  Gold: %d     Income/tick: %d",
            g.countries[c].gold, g.countries[c].income);
        mvaddstr(y++, 2, line);
        snprintf(line, sizeof(line), "  Units: Ground:%d  Naval:%d  Air:%d",
            g.countries[c].unit_counts[0], g.countries[c].unit_counts[1], g.countries[c].unit_counts[2]);
        mvaddstr(y++, 2, line);
        snprintf(line, sizeof(line), "  Research: Ground+%d  Naval+%d  Air+%d",
            g.countries[c].dev_bonus[0], g.countries[c].dev_bonus[1], g.countries[c].dev_bonus[2]);
        mvaddstr(y++, 2, line);
        y++;
    }

    int prov_counts[3] = {0,0,0};
    for (int i = 0; i < MAP_W * MAP_H; i++)
        if (g.map[i].owner >= 0 && g.map[i].owner <= 2)
            prov_counts[g.map[i].owner]++;

    y++;
    attron(COLOR_PAIR(CP_UI) | A_BOLD);
    mvaddstr(y++, 2, "Province Control:");
    attroff(COLOR_PAIR(CP_UI) | A_BOLD);
    char pc[64];
    snprintf(pc, sizeof(pc), "  Neutral: %d  ReallyFrontHole: %d  NotReallyBackDoor: %d",
        prov_counts[0], prov_counts[1], prov_counts[2]);
    mvaddstr(y++, 2, pc);
    y++;

    attron(COLOR_PAIR(CP_UI) | A_BOLD);
    mvaddstr(y++, 2, "Unit Costs:");
    attroff(COLOR_PAIR(CP_UI) | A_BOLD);
    mvaddstr(y++, 3, "Ground: 20G  |  Naval: 35G  |  Air: 50G");
    mvaddstr(y++, 3, "Develop province (D): 25G  |  Research (R): 40G");

    attron(COLOR_PAIR(CP_UI));
    mvaddstr(g.term_h - 1, g.term_w/2 - 10, "Press any key to return...");
    attroff(COLOR_PAIR(CP_UI));
    refresh();

    timeout(-1);
    getch();
    timeout(100);
}

void render_info(Game& g) {
    attron(COLOR_PAIR(CP_UI) | A_BOLD);
    mvhline(0, 0, ' ', g.term_w);
    mvaddstr(0, g.term_w/2 - 8, "=== GAME INFO ===");
    attroff(COLOR_PAIR(CP_UI) | A_BOLD);

    int y = 2;
    mvaddstr(y++, 2, "REALLYFRONTHOLE - Terminal RTS Game");
    mvaddstr(y++, 2, "Two countries battle for supremacy!");
    y++;
    attron(COLOR_PAIR(CP_UI) | A_BOLD);
    mvaddstr(y++, 2, "TERRAIN:");
    attroff(COLOR_PAIR(CP_UI) | A_BOLD);
    mvaddstr(y++, 4, "~ Water (blue)   . Plains (green)");
    mvaddstr(y++, 4, "& Forest (dark)  ^ Mountain (brown)");
    mvaddstr(y++, 4, "# City  (yellow)");
    y++;
    attron(COLOR_PAIR(CP_UI) | A_BOLD);
    mvaddstr(y++, 2, "UNITS:");
    attroff(COLOR_PAIR(CP_UI) | A_BOLD);
    mvaddstr(y++, 4, "G - Ground (120HP, ATK:10, DEF:12, SPD:2) - costs 20G");
    mvaddstr(y++, 4, "N - Naval   (100HP, ATK:14, DEF:8, SPD:3)  - costs 35G");
    mvaddstr(y++, 4, "A - Air      (70HP, ATK:18, DEF:5, SPD:4)  - costs 50G");
    y++;
    mvaddstr(y++, 4, "Rock-Paper-Scissors: Ground > Air > Naval > Ground (1.5x dmg)");
    y++;
    attron(COLOR_PAIR(CP_UI) | A_BOLD);
    mvaddstr(y++, 2, "CONTROLS:");
    attroff(COLOR_PAIR(CP_UI) | A_BOLD);
    mvaddstr(y++, 4, "Arrows/WASD/ЦФЫВ  - Scroll map");
    mvaddstr(y++, 4, "1/2/3             - Select unit type (Ground/Naval/Air)");
    mvaddstr(y++, 4, "B                 - Build unit on selected province");
    mvaddstr(y++, 4, "A                 - Toggle auto-attack mode");
    mvaddstr(y++, 4, "E                 - Economy overview");
    mvaddstr(y++, 4, "D                 - Develop province (costs 25G, max 5)");
    mvaddstr(y++, 4, "R                 - Research upgrade for selected unit (costs 40G)");
    mvaddstr(y++, 4, "M                 - Move unit towards cursor");
    mvaddstr(y++, 4, "Enter             - Select province under cursor");
    mvaddstr(y++, 4, "Space             - Pause/Resume");
    mvaddstr(y++, 4, ":                 - Command mode (attack/economy/help)");
    mvaddstr(y++, 4, "Q                 - Quit");
    y++;
    attron(COLOR_PAIR(CP_UI) | A_BOLD);
    mvaddstr(y++, 2, "COMMANDS:");
    attroff(COLOR_PAIR(CP_UI) | A_BOLD);
    mvaddstr(y++, 4, ":attack <type>    - Select attack unit type");
    mvaddstr(y++, 4, ":economy          - Show economy overview");
    mvaddstr(y++, 4, ":help             - Show this help + random tip");
    mvaddstr(y++, 4, ":menu             - Return to main menu");
    mvaddstr(y++, 4, ":logger start/stop - Enable/disable action logging");
    mvaddstr(y++, 4, ":<plugin> [args]  - Run a plugin by name");
    y++;
    attron(COLOR_PAIR(CP_UI) | A_BOLD);
    mvaddstr(y++, 2, "VICTORY:");
    attroff(COLOR_PAIR(CP_UI) | A_BOLD);
    mvaddstr(y++, 4, "Conquer all enemy provinces to win!");

    if (g.config.tip_count > 0) {
        y++;
        attron(COLOR_PAIR(CP_UI) | A_BOLD);
        mvaddstr(y++, 2, "RANDOM TIP:");
        attroff(COLOR_PAIR(CP_UI) | A_BOLD);
        int tip_idx = rand() % g.config.tip_count;
        mvaddstr(y++, 4, g.config.tips[tip_idx]);
    }

    attron(COLOR_PAIR(CP_UI));
    mvaddstr(g.term_h - 1, g.term_w/2 - 10, "Press any key to return...");
    attroff(COLOR_PAIR(CP_UI));
    refresh();

    timeout(-1);
    getch();
    timeout(100);
}

void render_gameover(Game& g) {
    erase();
    int winner = 0;
    if (!g.countries[1].alive) winner = 2;
    else if (!g.countries[2].alive) winner = 1;
    if (winner == 0) return;

    attron(COLOR_PAIR(CP_UI) | A_BOLD);
    int cx = g.term_w / 2;
    int cy = g.term_h / 2;
    mvaddstr(cy - 4, cx - 15, "=== GAME OVER ===");
    attroff(COLOR_PAIR(CP_UI) | A_BOLD);

    attron(COLOR_PAIR(winner == 1 ? CP_C1 : CP_C2) | A_BOLD);
    char msg[128];
    snprintf(msg, sizeof(msg), "%s WINS!", g.countries[winner].name);
    mvaddstr(cy - 2, cx - (int)strlen(msg)/2, msg);
    attroff(COLOR_PAIR(winner == 1 ? CP_C1 : CP_C2) | A_BOLD);

    char stats[128];
    snprintf(stats, sizeof(stats), "Ticks: %d  |  Year: %d  |  Month: %02d",
        g.tick, g.year, g.month);
    mvaddstr(cy, cx - (int)strlen(stats)/2, stats);

    snprintf(stats, sizeof(stats), "Units built: %d/%d/%d",
        g.countries[winner].unit_counts[0],
        g.countries[winner].unit_counts[1],
        g.countries[winner].unit_counts[2]);
    mvaddstr(cy + 1, cx - (int)strlen(stats)/2, stats);

    attron(COLOR_PAIR(CP_UI));
    mvaddstr(cy + 4, cx - 15, "Press Q to quit, R to restart...");
    attroff(COLOR_PAIR(CP_UI));
    refresh();

    while (true) {
        int ch = getch();
        if (ch == 'q' || ch == 'Q') { g.running = false; return; }
        if (ch == 'r' || ch == 'R') {
            g.mode = MODE_MAIN_MENU;
            g.menu_cursor = 0;
            return;
        }
    }
}
