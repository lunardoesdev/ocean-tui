#include "types.h"
#include <string.h>
#include <strings.h>

int idx(int x, int y) { return y * MAP_W + x; }
bool in_range(int val, int min, int max) { return val >= min && val < max; }
bool on_map(int x, int y) { return in_range(x, 0, MAP_W) && in_range(y, 0, MAP_H); }
int rand_range(int min, int max) { return min + rand() % (max - min + 1); }
float rand_float() { return (float)rand() / RAND_MAX; }

int count_neighbors(Game& g, int x, int y, int owner) {
    int cnt = 0;
    for (int dy = -1; dy <= 1; dy++)
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx, ny = y + dy;
            if (on_map(nx, ny) && g.map[idx(nx, ny)].owner == owner) cnt++;
        }
    return cnt;
}

void init_countries(Game& g) {
    for (int c = 1; c <= 2; c++) {
        g.countries[c].gold = 50;
        g.countries[c].income = 0;
        g.countries[c].alive = 1;
        for (int i = 0; i < 3; i++) {
            g.countries[c].unit_counts[i] = 0;
            g.countries[c].dev_bonus[i] = 0;
        }
    }
}

void init_unit(Unit& u, UnitType type) {
    u.type = type;
    u.has_moved = false;
    u.moves_left = 0;
    switch (type) {
        case U_GROUND:
            u.max_hp = 120; u.hp = 120;
            u.atk = 10; u.def = 12;
            u.range = 1; u.speed = 2;
            break;
        case U_NAVAL:
            u.max_hp = 100; u.hp = 100;
            u.atk = 14; u.def = 8;
            u.range = 2; u.speed = 3;
            break;
        case U_AIR:
            u.max_hp = 70; u.hp = 70;
            u.atk = 18; u.def = 5;
            u.range = 3; u.speed = 4;
            break;
    }
    u.moves_left = u.speed;
}

int unit_cost(UnitType t) {
    switch (t) {
        case U_GROUND: return 20;
        case U_NAVAL:  return 35;
        case U_AIR:    return 50;
    }
    return 20;
}

bool can_place_unit(Game& g, int x, int y, UnitType t) {
    if (!on_map(x, y)) return false;
    Province& p = g.map[y * MAP_W + x];
    if (p.owner == 0) return false;
    if (p.has_unit) return false;
    if (t == U_NAVAL && p.terrain != WATER) return false;
    if (t == U_GROUND && p.terrain == WATER) return false;
    if (t == U_AIR) return true;
    return true;
}

static float calc_damage(int atk, int def) {
    float base = atk * (1.0f - def / (def + 50.0f));
    float variance = 0.8f + rand_float() * 0.4f;
    return base * variance;
}

void resolve_combat(Game& g, int ax, int ay, int bx, int by) {
    log_action(g, "COMBAT", "initiated");
    Province& a = g.map[ay * MAP_W + ax];
    Province& b = g.map[by * MAP_W + bx];
    if (!a.has_unit || !b.has_unit) return;
    if (a.owner == b.owner) return;

    float a_def_bonus = 1.0f;
    float b_def_bonus = 1.0f;
    switch (a.terrain) {
        case FOREST:   a_def_bonus = 0.8f; break;
        case MOUNTAIN: a_def_bonus = 0.6f; break;
        case CITY:     a_def_bonus = 0.7f; break;
        default: break;
    }
    switch (b.terrain) {
        case FOREST:   b_def_bonus = 0.8f; break;
        case MOUNTAIN: b_def_bonus = 0.6f; break;
        case CITY:     b_def_bonus = 0.7f; break;
        default: break;
    }

    float a_atk_bonus = 1.0f, b_atk_bonus = 1.0f;
    if ((a.unit.type == U_GROUND && b.unit.type == U_AIR) ||
        (a.unit.type == U_AIR && b.unit.type == U_NAVAL) ||
        (a.unit.type == U_NAVAL && b.unit.type == U_GROUND))
        a_atk_bonus = 1.5f;
    if ((b.unit.type == U_GROUND && a.unit.type == U_AIR) ||
        (b.unit.type == U_AIR && a.unit.type == U_NAVAL) ||
        (b.unit.type == U_NAVAL && a.unit.type == U_GROUND))
        b_atk_bonus = 1.5f;

    int a_country = a.owner;
    int b_country = b.owner;
    a_atk_bonus += g.countries[a_country].dev_bonus[a.unit.type] * 0.1f;
    b_atk_bonus += g.countries[b_country].dev_bonus[b.unit.type] * 0.1f;

    float dmg_a = calc_damage(a.unit.atk * a_atk_bonus, b.unit.def * b_def_bonus);
    float dmg_b = calc_damage(b.unit.atk * b_atk_bonus, a.unit.def * a_def_bonus);

    a.unit.hp -= (int)dmg_b;
    b.unit.hp -= (int)dmg_a;

    if (a.unit.hp <= 0) {
        a.has_unit = false;
        g.countries[a_country].unit_counts[a.unit.type]--;
    }
    if (b.unit.hp <= 0) {
        b.has_unit = false;
        g.countries[b_country].unit_counts[b.unit.type]--;
    }
}

void process_combat(Game& g) {
    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++) {
            Province& p = g.map[y * MAP_W + x];
            if (!p.has_unit) continue;
            if (p.unit.has_moved) continue;

            for (int dy = -p.unit.range; dy <= p.unit.range; dy++)
                for (int dx = -p.unit.range; dx <= p.unit.range; dx++) {
                    if (dx == 0 && dy == 0) continue;
                    int nx = x + dx, ny = y + dy;
                    if (!on_map(nx, ny)) continue;
                    Province& target = g.map[ny * MAP_W + nx];
                    if (!target.has_unit || target.owner == p.owner) continue;
                    resolve_combat(g, x, y, nx, ny);
                    if (!p.has_unit) goto next_unit;
                }
            next_unit:;
        }
}

void ai_tick(Game& g) {
    int c = 2;
    if (!g.countries[c].alive) return;

    for (int attempt = 0; attempt < 5; attempt++) {
        UnitType ut = (UnitType)(rand() % 3);
        int cost = unit_cost(ut);
        if (g.countries[c].gold < cost) continue;

        for (int search = 0; search < 50; search++) {
            int x = rand_range(0, MAP_W - 1);
            int y = rand_range(0, MAP_H - 1);
            if (can_place_unit(g, x, y, ut) && g.map[y * MAP_W + x].owner == c) {
                init_unit(g.map[y * MAP_W + x].unit, ut);
                g.map[y * MAP_W + x].has_unit = true;
                g.countries[c].gold -= cost;
                g.countries[c].unit_counts[ut]++;
                break;
            }
        }
    }

    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++) {
            Province& p = g.map[y * MAP_W + x];
            if (!p.has_unit || p.owner != c) continue;
            if (p.unit.has_moved) continue;

            int best_x = -1, best_y = -1, best_dist = 999999;
            for (int ey = 0; ey < MAP_H; ey++)
                for (int ex = 0; ex < MAP_W; ex++) {
                    if (g.map[ey * MAP_W + ex].owner != 1) continue;
                    int dist = abs(ex - x) + abs(ey - y);
                    if (dist < best_dist && dist > 0) {
                        if (g.map[ey * MAP_W + ex].has_unit) dist /= 2;
                        best_dist = dist;
                        best_x = ex;
                        best_y = ey;
                    }
                }

            if (best_x < 0) continue;

            int dx = (best_x > x) ? 1 : (best_x < x) ? -1 : 0;
            int dy = (best_y > y) ? 1 : (best_y < y) ? -1 : 0;

            for (int step = 0; step < p.unit.speed; step++) {
                int nx = x + dx, ny = y + dy;
                if (!on_map(nx, ny)) break;
                Province& target = g.map[ny * MAP_W + nx];

                if (target.owner != c && target.has_unit) break;

                if (target.owner == 0 || target.owner == c) {
                    if (p.unit.type != U_NAVAL && target.terrain == WATER) break;
                    if (target.has_unit) break;
                    target.has_unit = true;
                    target.unit = p.unit;
                    target.owner = p.owner;
                    g.map[y * MAP_W + x].has_unit = false;
                    x = nx; y = ny;
                }
            }
            p.unit.has_moved = true;
        }
}

void update_economy(Game& g) {
    for (int c = 1; c <= 2; c++) {
        g.countries[c].income = 0;
        g.countries[c].alive = 0;
    }

    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++) {
            Province& p = g.map[y * MAP_W + x];
            if (p.owner == 0) continue;
            g.countries[p.owner].alive = 1;

            int base = 0;
            switch (p.terrain) {
                case WATER:    base = 0; break;
                case PLAINS:   base = 1; break;
                case FOREST:   base = 1; break;
                case MOUNTAIN: base = 1; break;
                case CITY:     base = 3; break;
            }
            int total = base + p.dev;
            g.countries[p.owner].income += total;
            g.countries[p.owner].gold += total;
        }

    for (int c = 1; c <= 2; c++) {
        int maint = 0;
        for (int i = 0; i < 3; i++)
            maint += g.countries[c].unit_counts[i];
        g.countries[c].gold -= maint;
        if (g.countries[c].gold < 0) g.countries[c].gold = 0;
    }
}

void update(Game& g) {
    if (g.pause) return;

    g.tick++;

    if (g.tick % 10 == 0) {
        g.month++;
        if (g.month > 12) {
            g.month = 1;
            g.year++;
        }
    }

    if (g.tick % 5 == 0)
        update_economy(g);

    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++) {
            if (g.map[y * MAP_W + x].has_unit)
                g.map[y * MAP_W + x].unit.has_moved = false;
        }

    process_combat(g);

    if (g.tick % 5 == 0)
        ai_tick(g);

    if (!g.countries[1].alive || !g.countries[2].alive)
        g.mode = MODE_GAMEOVER;
}

// --- Plugin management ---

void init_plugins(Game& g) {
    g.plugin_count = 0;

    auto add = [&](const char* name, const char* path, const char* desc) {
        if (g.plugin_count >= MAX_PLUGINS) return;
        PluginEntry& p = g.plugins[g.plugin_count++];
        strncpy(p.name, name, 63);
        strncpy(p.path, path, 255);
        p.enabled = true;
        p.builtin = true;
        strncpy(p.description, desc, 127);
    };

    add("Logger",      "etc/lua/logger.lua",       "Game action logger (writes game.log)");
    add("Calculator",  "etc/lua/calculator.lua",    "Simple calculator (like bc)");
    add("Translator",  "etc/lua/translator.lua",    "EN to ES/RU dictionary translator");
    add("Template",    "etc/lua/template.lua",      "Plugin development template");
    add("Screenshot",  "etc/lua/screenshot.lua",    "Save terminal screenshot to file");
    add("Cleaner",     "etc/lua/cleaner.lua",       "Count lines & remove .o files");
}

void plugin_toggle(Game& g, int idx) {
    if (idx < 0 || idx >= g.plugin_count) return;
    g.plugins[idx].enabled = !g.plugins[idx].enabled;
}

void plugin_add_custom(Game& g, const char* path) {
    if (g.plugin_count >= MAX_PLUGINS) return;
    PluginEntry& p = g.plugins[g.plugin_count++];
    snprintf(p.name, 63, "Custom-%d", g.plugin_count);
    strncpy(p.path, path, 255);
    p.enabled = true;
    p.builtin = false;
    snprintf(p.description, 127, "Custom plugin: %s", path);
}

void plugin_run(Game& g, int idx, const char* args) {
    if (idx < 0 || idx >= g.plugin_count) return;
    PluginEntry& p = g.plugins[idx];
    if (!p.enabled) return;

    g.edit_mode = true;
    curs_set(0);
    timeout(-1);

    erase();

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "lua5.4 etc/lua/runner.lua %s %s 2>&1", p.path, args ? args : "");

    int y = 2;
    attron(COLOR_PAIR(CP_UI) | A_BOLD);
    mvhline(0, 0, ' ', g.term_w);
    mvaddstr(0, g.term_w / 2 - 15, "=== Plugin Output ===");
    attroff(COLOR_PAIR(CP_UI) | A_BOLD);
    mvaddstr(y++, 2, p.name);
    mvaddstr(y++, 2, p.path);
    y++;

    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        mvaddstr(y, 2, "Error: failed to execute plugin");
    } else {
        char line[256];
        while (fgets(line, sizeof(line), pipe)) {
            int len = strlen(line);
            if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
            mvaddstr(y++, 4, line);
            if (y > g.term_h - 4) {
                mvaddstr(y, 4, "... (output truncated)");
                break;
            }
        }
        pclose(pipe);
    }

    attron(COLOR_PAIR(CP_UI));
    mvaddstr(g.term_h - 1, g.term_w / 2 - 12, "Press any key to close");
    attroff(COLOR_PAIR(CP_UI));
    refresh();

    getch();
    timeout(100);
    g.edit_mode = false;
}

void plugin_run_all(Game& g) {
    for (int i = 0; i < g.plugin_count; i++) {
        if (g.plugins[i].enabled) {
            plugin_run(g, i);
        }
    }
}

int plugin_find_by_name(Game& g, const char* name) {
    for (int i = 0; i < g.plugin_count; i++) {
        if (strcasecmp(name, g.plugins[i].name) == 0)
            return i;
    }
    return -1;
}

void plugin_run_by_name(Game& g, const char* name, const char* args) {
    int idx = plugin_find_by_name(g, name);
    if (idx >= 0) plugin_run(g, idx, args);
}

void log_action(Game& g, const char* action, const char* details) {
    if (!g.logging_enabled) return;

    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    char path[256];
    snprintf(path, sizeof(path), "logs/actions_%04d-%02d-%02d.log",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);

    FILE* f = fopen(path, "a");
    if (!f) return;

    fprintf(f, "[%02d:%02d:%02d] %s | %s\n",
        t->tm_hour, t->tm_min, t->tm_sec, action, details);
    fclose(f);
}

// ---

void setup_game(Game& g) {
    g.map = new Province[MAP_W * MAP_H]();
    generate_map(g);
    init_countries(g);
    init_plugins(g);
    g.cam_x = 0; g.cam_y = 0;
    g.sel_unit = U_GROUND;
    g.mode = MODE_NORMAL;
    g.pause = 0; g.tick = 0;
    g.year = 1936; g.month = 1;
    g.running = true;
    g.attack_mode = false;
    g.has_selection = false;
    g.term_w = 0; g.term_h = 0;
    g.view_w = 0; g.view_h = 0;
    g.menu_cursor = 0;
    g.menu_page = 0;
    g.edit_mode = false;
    g.edit_field = 0;
    g.logging_enabled = false;
}

void destroy_game(Game& g) {
    delete[] g.map;
    g.map = nullptr;
}
