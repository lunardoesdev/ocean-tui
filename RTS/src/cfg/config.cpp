#include "types.h"
#include <strings.h>
#include <ctype.h>

static void trim(char* s) {
    int len = strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t' || s[len-1] == '\r' || s[len-1] == '\n'))
        s[--len] = '\0';
    while (*s && (*s == ' ' || *s == '\t')) s++;
}

int parse_color(const char* name) {
    if (strcasecmp(name, "red") == 0) return COLOR_RED;
    if (strcasecmp(name, "green") == 0) return COLOR_GREEN;
    if (strcasecmp(name, "yellow") == 0) return COLOR_YELLOW;
    if (strcasecmp(name, "blue") == 0) return COLOR_BLUE;
    if (strcasecmp(name, "magenta") == 0) return COLOR_MAGENTA;
    if (strcasecmp(name, "cyan") == 0) return COLOR_CYAN;
    if (strcasecmp(name, "white") == 0) return COLOR_WHITE;
    if (strcasecmp(name, "black") == 0) return COLOR_BLACK;
    return COLOR_RED;
}

const char* color_name(int c) {
    switch (c) {
        case COLOR_RED:     return "red";
        case COLOR_GREEN:   return "green";
        case COLOR_YELLOW:  return "yellow";
        case COLOR_BLUE:    return "blue";
        case COLOR_MAGENTA: return "magenta";
        case COLOR_CYAN:    return "cyan";
        case COLOR_WHITE:   return "white";
        case COLOR_BLACK:   return "black";
        default:            return "unknown";
    }
}

void load_config(const char* path, GameConfig& cfg) {
    cfg.primary_color = COLOR_RED;
    cfg.secondary_color = COLOR_CYAN;
    cfg.update_rate = 10;
    strcpy(cfg.author_info, "REALLYFRONTHOLE — Terminal RTS Game");
    cfg.tip_count = 0;

    FILE* f = fopen(path, "r");
    if (!f) return;

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        trim(line);
        if (line[0] == '#' || line[0] == '\0') continue;

        char* eq = strchr(line, '=');
        if (!eq) continue;
        *eq = '\0';
        char* key = line;
        char* val = eq + 1;
        trim(key);
        trim(val);

        if (strcmp(key, "primary_color") == 0) {
            cfg.primary_color = parse_color(val);
        } else if (strcmp(key, "secondary_color") == 0) {
            cfg.secondary_color = parse_color(val);
        } else if (strcmp(key, "shader_style") == 0) {
            cfg.shader_style = atoi(val);
            if (cfg.shader_style < 0) cfg.shader_style = 0;
            if (cfg.shader_style > 5) cfg.shader_style = 5;
        } else if (strcmp(key, "update_rate") == 0) {
            cfg.update_rate = atoi(val);
            if (cfg.update_rate < 1) cfg.update_rate = 1;
            if (cfg.update_rate > 60) cfg.update_rate = 60;
        } else if (strcmp(key, "author_menu_button") == 0) {
            strncpy(cfg.author_info, val, 511);
            cfg.author_info[511] = '\0';
        } else if (strncmp(key, "tip_", 4) == 0) {
            if (cfg.tip_count < 20) {
                strncpy(cfg.tips[cfg.tip_count], val, 255);
                cfg.tips[cfg.tip_count][255] = '\0';
                cfg.tip_count++;
            }
        }
    }
    fclose(f);
}

void save_config(const char* path, GameConfig& cfg) {
    FILE* f = fopen(path, "w");
    if (!f) return;

    fprintf(f, "# RTS Configuration File\n");
    fprintf(f, "# ═══════════════════════════════════════════════════════\n\n");
    fprintf(f, "primary_color=%s\n", color_name(cfg.primary_color));
    fprintf(f, "secondary_color=%s\n", color_name(cfg.secondary_color));
    fprintf(f, "update_rate=%d\n", cfg.update_rate);
    fprintf(f, "shader_style=%d\n", cfg.shader_style);
    fprintf(f, "author_menu_button=%s\n", cfg.author_info);
    fprintf(f, "\n# ═══════════════════════════════════════════════════════\n");
    fprintf(f, "# СОВЕТЫ\n");
    fprintf(f, "# ═══════════════════════════════════════════════════════\n\n");
    for (int i = 0; i < cfg.tip_count; i++)
        fprintf(f, "tip_%d=%s\n", i + 1, cfg.tips[i]);

    fclose(f);
}
