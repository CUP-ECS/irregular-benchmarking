#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Struct for one pattern
struct Pattern {
    std::map<int, int> comm_partners;
    std::map<double, int> buffer_size;
    std::map<int, std::map<int, int>> dist_to_neighbors;
};

// Conversion from JSON → struct
void from_json(const json& j, Pattern& p) {
    // comm_partners
    for (auto& [k, v] : j.at("comm_partners").items()) {
        p.comm_partners[std::stoi(k)] = v.get<int>();
    }

    // buffer_size
    {
        std::map<double, int> temp;
        for (auto& [k, v] : j.at("buffer_size").items()) {
            temp[std::stod(k)] = v.get<int>();
        }

        // fill-forward: replace 0 with last non-zero
        int last_nonzero = 0;
        for (auto& [key, value] : temp) {
            if (value != 0) {
                last_nonzero = value;
            } else if (last_nonzero != 0) {
                value = last_nonzero;
            }
        }

        p.buffer_size = std::move(temp);
    }

    // dist_to_neighbors
    for (auto& [outer_k, inner_obj] : j.at("dist_to_neighbors").items()) {
        int outer_key = std::stoi(outer_k);
        std::map<int, int> inner_map;
        for (auto& [inner_k, inner_v] : inner_obj.items()) {
            inner_map[std::stoi(inner_k)] = inner_v.get<int>();
        }
        p.dist_to_neighbors[outer_key] = inner_map;
    }
}

int main() {
    std::ifstream file("x.json");
    if (!file.is_open()) {
        std::cerr << "Error: Could not open x.json\n";
        return 1;
    }

    json j;
    file >> j;

    std::map<std::string, Pattern> patterns;
    for (auto& [pattern_name, pattern_json] : j.items()) {
        patterns[pattern_name] = pattern_json.get<Pattern>();
    }

    // Example: print buffer_size with fill-forward applied

    // Example: print one pattern’s data
    for (auto& [name, pattern] : patterns) {
        std::cout << "Pattern: " << name << "\n";

        std::cout << "  comm_partners:\n";
        for (auto& [k, v] : pattern.comm_partners) {
            std::cout << "    " << k << " : " << v << "\n";
        }

        std::cout << "  buffer_size:\n";
        for (auto& [k, v] : pattern.buffer_size) {
            std::cout << "    " << k << " : " << v << "\n";
        }

        std::cout << "  dist_to_neighbors:\n";
        for (auto& [outer, inner] : pattern.dist_to_neighbors) {
            std::cout << "    " << outer << ":\n";
            for (auto& [ik, iv] : inner) {
                std::cout << "      " << ik << " : " << iv << "\n";
            }
        }
    }
}
