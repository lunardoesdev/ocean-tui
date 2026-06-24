#pragma once
#include <ncurses.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <vector>
#include <queue>
#include <algorithm>

#define MAP_W 500
#define MAP_H 500
#define MAX_DEV 5
#define VIEWPORT_FACTOR 0.85f
#define MAX_PLUGINS 20

enum Terrain { WATER, PLAINS, FOREST, MOUNTAIN, CITY };
enum UnitType { U_GROUND, U_NAVAL, U_AIR };
enum GameMode {
    MODE_NORMAL, MODE_ECONOMY, MODE_INFO, MODE_GAMEOVER,
    MODE_MAIN_MENU, MODE_SETTINGS, MODE_PLUGINS, MODE_AUTHOR_INFO
};

struct Unit {
    UnitType type;
    int hp, max_hp, atk, def, range, speed;
    int moves_left;
    bool has_moved;
};

struct Province {
    Terrain terrain;
    int owner;
    int dev;
    Unit unit;
    bool has_unit;
    char city_name[32];
};

struct Country {
    char name[32];
    int gold, income;
    int color;
    int alive;
    int unit_counts[3];
    int dev_bonus[3];
};

struct GameConfig {
    int primary_color;
    int secondary_color;
    int update_rate;
    int shader_style;
    char author_info[512];
    char tips[20][256];
    int tip_count;
};

struct PluginEntry {
    char name[64];
    char path[256];
    bool enabled;
    bool builtin;
    char description[128];
};

enum CP {
    CP_WATER=1, CP_PLAINS, CP_FOREST, CP_MOUNTAIN, CP_CITY,
    CP_C1, CP_C2, CP_C1_UNIT, CP_C2_UNIT,
    CP_UI, CP_SEL, CP_HP_G, CP_HP_Y, CP_HP_R,
    CP_SNOW, CP_SAND, CP_BORDER
};

extern const char* unit_chars;
extern const char* terrain_names[];
extern const char* unit_names[];

struct Game {
    Province* map;
    Country countries[3];
    int cam_x, cam_y;
    int sel_unit;
    GameMode mode;
    int pause, tick, year, month;
    bool running;
    int term_w, term_h;
    int view_w, view_h;
    bool attack_mode;
    int selected_province_x, selected_province_y;
    bool has_selection;
    GameConfig config;
    int menu_cursor;
    int menu_page;
    bool edit_mode;
    int edit_field;
    PluginEntry plugins[MAX_PLUGINS];
    int plugin_count;
    bool logging_enabled;
};

int idx(int x, int y);
bool in_range(int val, int min, int max);
bool on_map(int x, int y);
int rand_range(int min, int max);
float rand_float();
int count_neighbors(Game& g, int x, int y, int owner);

void load_config(const char* path, GameConfig& cfg);
void save_config(const char* path, GameConfig& cfg);
int parse_color(const char* name);
const char* color_name(int c);

void generate_map(Game& g);

void init_countries(Game& g);
void init_unit(Unit& u, UnitType type);
int unit_cost(UnitType t);
bool can_place_unit(Game& g, int x, int y, UnitType t);
void resolve_combat(Game& g, int ax, int ay, int bx, int by);
void process_combat(Game& g);
void ai_tick(Game& g);
void update_economy(Game& g);
void update(Game& g);
void setup_game(Game& g);
void destroy_game(Game& g);

void init_plugins(Game& g);
void plugin_toggle(Game& g, int idx);
void plugin_add_custom(Game& g, const char* path);
void plugin_run(Game& g, int idx, const char* args = nullptr);
int plugin_find_by_name(Game& g, const char* name);
void plugin_run_by_name(Game& g, const char* name, const char* args);
void plugin_run_all(Game& g);
void log_action(Game& g, const char* action, const char* details);

void init_colors(GameConfig& cfg);
void render(Game& g);
void render_minimap(Game& g, int mx, int my, int mw, int mh);
void render_tile(Game& g, int mx, int my, int scr_x, int scr_y);
void render_economy(Game& g);
void render_info(Game& g);
void render_gameover(Game& g);
void render_main_menu(Game& g);
void render_settings_menu(Game& g);
void render_plugins_menu(Game& g);
void render_author_info(Game& g);

void process_input(Game& g);
void handle_command(Game& g);
