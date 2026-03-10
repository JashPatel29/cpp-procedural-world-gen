#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <random>
#include <ctime>
#include <cmath>
using namespace std;

enum block_type { air, grass, dirt, stone, sand, water, wood, leaves };
enum biome_type { plains, forest, desert, ocean };

class block {
public:
    block_type type;
    char display_char;

    block(block_type t = air) : type(t) {
        switch (t) {
            case grass:  display_char = '#'; break;
            case dirt:   display_char = ':'; break;
            case stone:  display_char = 'O'; break;
            case sand:   display_char = '~'; break;
            case water:  display_char = 'W'; break;
            case wood:   display_char = 'Y'; break;
            case leaves: display_char = '%'; break;
            default:     display_char = ' '; break;
        }
    }
};

class world {
private:
    vector<vector<block>> grid;
    int width, height;

public:
    world(int w, int h) : width(w), height(h) {
        grid.resize(height, vector<block>(width, block(air)));
    }

    void set_block(int x, int y, block_type type) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            grid[y][x] = block(type);
        }
    }

    block get_block(int x, int y) const {
        if (x >= 0 && x < width && y >= 0 && y < height)
            return grid[y][x];
        return block(air);
    }

    int get_width() const { return width; }
    int get_height() const { return height; }

    void display() const {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                switch (grid[y][x].type) {
                    case grass:  cout << "\033[1;32m"; break;
                    case dirt:   cout << "\033[0;33m"; break;
                    case stone:  cout << "\033[1;30m"; break;
                    case sand:   cout << "\033[1;33m"; break;
                    case water:  cout << "\033[1;34m"; break;
                    case wood:   cout << "\033[0;33m"; break;
                    case leaves: cout << "\033[0;32m"; break;
                    default:     break;
                }
                cout << grid[y][x].display_char << "\033[0m";
            }
            cout << endl;
        }
    }
};

class world_generator {
private:
    map<biome_type, vector<block_type>> biome_palettes;
    mt19937 rng;
    double terrain_frequency;
    double terrain_amplitude;
    int terrain_base_height;

public:
    world_generator() {
        rng.seed(static_cast<unsigned int>(time(0)));
        biome_palettes[plains] = { grass, dirt, stone };
        biome_palettes[forest] = { grass, dirt, stone, wood, leaves };
        biome_palettes[desert] = { sand, stone };
        biome_palettes[ocean]  = { water, sand, stone };

        uniform_real_distribution<double> freq_dist(15.0, 25.0);
        uniform_real_distribution<double> amp_dist(4.0, 8.0);
        uniform_int_distribution<int> base_height_dist(18, 22);

        terrain_frequency = freq_dist(rng);
        terrain_amplitude = amp_dist(rng);
        terrain_base_height = base_height_dist(rng);
    }

    void generate_world(world& w) {
        vector<biome_type> biome_map(w.get_width());
        int x = 0;
        biome_type last_biome = ocean;

        uniform_int_distribution<int> biome_length_dist(25, 50);
        uniform_int_distribution<int> biome_type_dist(0, 3);

        while (x < w.get_width()) {
            int biome_length = biome_length_dist(rng);
            biome_type next_biome;
            do {
                next_biome = static_cast<biome_type>(biome_type_dist(rng));
            } while (next_biome == last_biome);

            for (int i = 0; i < biome_length && x < w.get_width(); ++i, ++x) {
                biome_map[x] = next_biome;
            }
            last_biome = next_biome;
        }

        for (int x = 0; x < w.get_width(); ++x) {
            biome_type current_biome = biome_map[x];
            int terrain_height = terrain_base_height + static_cast<int>(terrain_amplitude * sin(x / terrain_frequency));
            if (current_biome == ocean) terrain_height = 10;

            for (int y = 0; y < w.get_height(); ++y) {
                if (y > terrain_height) {
                    if (y == terrain_height + 1) {
                        if (current_biome == plains || current_biome == forest) w.set_block(x, y, grass);
                        if (current_biome == desert || current_biome == ocean) w.set_block(x, y, sand);
                    } else if (y < terrain_height + 5) {
                        if (current_biome == plains || current_biome == forest) w.set_block(x, y, dirt);
                        if (current_biome == desert || current_biome == ocean) w.set_block(x, y, sand);
                    } else {
                        w.set_block(x, y, stone);
                    }
                } else if (y <= terrain_height && current_biome == ocean) {
                    w.set_block(x, y, water);
                }
            }

            if (current_biome == forest) {
                uniform_int_distribution<int> tree_chance(0, 7);
                if (tree_chance(rng) == 0) {
                    int tree_height = 5;
                    for (int i = 0; i < tree_height; ++i)
                        w.set_block(x, terrain_height - i, wood);

                    w.set_block(x, terrain_height - tree_height, leaves);
                    w.set_block(x - 1, terrain_height - tree_height, leaves);
                    w.set_block(x + 1, terrain_height - tree_height, leaves);
                    w.set_block(x, terrain_height - tree_height - 1, leaves);
                }
            }
        }
    }
};

int main() {
    const int world_width = 120;
    const int world_height = 40;

    world w(world_width, world_height);
    world_generator gen;

    cout << "generating new random world...\n";
    gen.generate_world(w);
    cout << "generation complete!\n\n";

    w.display();
    return 0;
}
