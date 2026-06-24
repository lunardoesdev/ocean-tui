#include "types.h"

static void generate_terrain(Game& g) {
    int* terrain = new int[MAP_W * MAP_H];

    for (int i = 0; i < MAP_W * MAP_H; i++)
        terrain[i] = (rand_float() < 0.42f) ? 1 : 0;

    for (int iter = 0; iter < 5; iter++) {
        int* next = new int[MAP_W * MAP_H];
        for (int y = 0; y < MAP_H; y++)
            for (int x = 0; x < MAP_W; x++) {
                int wall_count = 0;
                for (int dy = -1; dy <= 1; dy++)
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        int nx = x + dx, ny = y + dy;
                        if (!on_map(nx, ny)) wall_count++;
                        else wall_count += terrain[ny * MAP_W + nx];
                    }
                next[y * MAP_W + x] = (terrain[y * MAP_W + x] == 1)
                    ? (wall_count >= 4 ? 1 : 0)
                    : (wall_count >= 5 ? 1 : 0);
            }
        delete[] terrain;
        terrain = next;
    }

    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++) {
            int i = y * MAP_W + x;
            g.map[i].terrain = terrain[i] ? PLAINS : WATER;
            g.map[i].owner = 0;
            g.map[i].dev = 0;
            g.map[i].has_unit = false;
            g.map[i].city_name[0] = 0;
        }
    delete[] terrain;
}

static void add_detail(Game& g) {
    for (int y = 2; y < MAP_H - 2; y++)
        for (int x = 2; x < MAP_W - 2; x++) {
            int i = y * MAP_W + x;
            if (g.map[i].terrain != PLAINS) continue;
            int land_count = 0;
            for (int dy = -2; dy <= 2; dy++)
                for (int dx = -2; dx <= 2; dx++)
                    if (g.map[(y+dy)*MAP_W + (x+dx)].terrain != WATER) land_count++;
            if (land_count >= 23 && rand_float() < 0.15f)
                g.map[i].terrain = FOREST;
        }

    for (int y = 3; y < MAP_H - 3; y++)
        for (int x = 3; x < MAP_W - 3; x++) {
            int i = y * MAP_W + x;
            if (g.map[i].terrain != PLAINS) continue;
            int land_count = 0;
            for (int dy = -3; dy <= 3; dy++)
                for (int dx = -3; dx <= 3; dx++)
                    if (g.map[(y+dy)*MAP_W + (x+dx)].terrain != WATER) land_count++;
            if (land_count >= 40 && rand_float() < 0.08f)
                g.map[i].terrain = MOUNTAIN;
        }

    for (int m = 0; m < 8; m++) {
        int sx = rand_range(10, MAP_W - 10);
        int sy = rand_range(10, MAP_H - 10);
        int dx = rand_range(-1, 1);
        int dy = rand_range(-1, 1);
        if (dx == 0 && dy == 0) dx = 1;
        int len = rand_range(15, 60);
        for (int step = 0; step < len; step++) {
            int cx = sx + dx * step + rand_range(-2, 2);
            int cy = sy + dy * step + rand_range(-2, 2);
            if (!on_map(cx, cy)) continue;
            for (int py = -1; py <= 1; py++)
                for (int px = -1; px <= 1; px++) {
                    int nx = cx + px, ny = cy + py;
                    if (on_map(nx, ny) && g.map[ny*MAP_W+nx].terrain == PLAINS && rand_float() < 0.5f)
                        g.map[ny*MAP_W+nx].terrain = MOUNTAIN;
                }
        }
    }
}

static void assign_countries(Game& g) {
    int* visited = new int[MAP_W * MAP_H]();
    std::vector<std::vector<std::pair<int,int>>> continents;

    for (int y = 10; y < MAP_H - 10; y++)
        for (int x = 10; x < MAP_W - 10; x++) {
            int i = y * MAP_W + x;
            if (visited[i] || g.map[i].terrain == WATER) continue;

            std::vector<std::pair<int,int>> cells;
            std::queue<std::pair<int,int>> q;
            q.push({x, y});
            visited[i] = 1;

            while (!q.empty()) {
                auto [cx, cy] = q.front(); q.pop();
                cells.push_back({cx, cy});
                for (int dy = -1; dy <= 1; dy++)
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        int nx = cx + dx, ny = cy + dy;
                        if (!on_map(nx, ny)) continue;
                        int ni = ny * MAP_W + nx;
                        if (!visited[ni] && g.map[ni].terrain != WATER) {
                            visited[ni] = 1;
                            q.push({nx, ny});
                        }
                    }
            }

            if ((int)cells.size() > 20)
                continents.push_back(cells);
        }

    delete[] visited;

    std::sort(continents.begin(), continents.end(),
        [](const auto& a, const auto& b) { return a.size() > b.size(); });

    if (continents.size() < 2) {
        for (int y = 0; y < MAP_H/2; y++)
            for (int x = 0; x < MAP_W/2; x++)
                if (g.map[y*MAP_W + x].terrain != WATER)
                    g.map[y*MAP_W + x].owner = 1;
        for (int y = MAP_H/2; y < MAP_H; y++)
            for (int x = MAP_W/2; x < MAP_W; x++)
                if (g.map[y*MAP_W + x].terrain != WATER)
                    g.map[y*MAP_W + x].owner = 2;
        strcpy(g.countries[1].name, "ReallyFrontHole");
        strcpy(g.countries[2].name, "NotReallyBackDoor");
        return;
    }

    for (auto& cell : continents[0])
        g.map[cell.second * MAP_W + cell.first].owner = 1;
    for (auto& cell : continents[1])
        g.map[cell.second * MAP_W + cell.first].owner = 2;

    strcpy(g.countries[1].name, "ReallyFrontHole");
    strcpy(g.countries[2].name, "NotReallyBackDoor");

    for (int i = 3; i < (int)continents.size() && i < 6; i++) {
        int owner = (i % 2 == 0) ? 1 : 2;
        for (auto& cell : continents[i])
            g.map[cell.second * MAP_W + cell.first].owner = owner;
    }
}

static void place_cities(Game& g) {
    const char* city_names_c1[] = {"ReallyCity", "FrontTown", "HolePort", "RedKeep", "ScarletPeak"};
    const char* city_names_c2[] = {"NotCity", "BackTown", "DoorPort", "BlueCitadel", "AzureBay"};

    int c1_cities = 0, c2_cities = 0;

    for (int y = 5; y < MAP_H - 5; y++)
        for (int x = 5; x < MAP_W - 5; x++) {
            int i = y * MAP_W + x;
            if (g.map[i].terrain == WATER || g.map[i].terrain == CITY) continue;

            int owner = g.map[i].owner;
            if (owner == 0) continue;

            bool too_close = false;
            for (int dy = -10; dy <= 10 && !too_close; dy++)
                for (int dx = -10; dx <= 10 && !too_close; dx++) {
                    int nx = x + dx, ny = y + dy;
                    if (on_map(nx, ny) && g.map[ny*MAP_W+nx].terrain == CITY)
                        too_close = true;
                }
            if (too_close) continue;

            int same_owner = count_neighbors(g, x, y, owner);
            if (same_owner < 4) continue;

            if ((owner == 1 && c1_cities < 5) || (owner == 2 && c2_cities < 5)) {
                g.map[i].terrain = CITY;
                g.map[i].dev = 3;
                const char* name = (owner == 1)
                    ? city_names_c1[c1_cities++]
                    : city_names_c2[c2_cities++];
                strcpy(g.map[i].city_name, name);
            }
        }
}

void generate_map(Game& g) {
    generate_terrain(g);
    add_detail(g);
    assign_countries(g);
    place_cities(g);
}
