#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <nlohmann/json.hpp>
#include <iostream>
#include <map>
#include <random>
#include <vector>
#include <chrono>

// Random engine (seeded with current time)
std::mt19937& global_rng() {
    static std::mt19937 rng(static_cast<unsigned long>(
        std::chrono::system_clock::now().time_since_epoch().count()));
    return rng;
}

// --- Sample from normalized map<int, double> ---
int sample_from_map(const std::map<int, double>& dist) {
    std::vector<int> keys;
    std::vector<double> weights;

    for (auto& [k, w] : dist) {
        keys.push_back(k);
        weights.push_back(w);
    }

    std::discrete_distribution<> d(weights.begin(), weights.end());
    return keys[d(global_rng())];
}

// --- Sample from normalized map<double, double> ---
double sample_from_map_double(const std::map<double, double>& dist) {
    std::vector<double> keys;
    std::vector<double> weights;

    for (auto& [k, w] : dist) {
        keys.push_back(k);
        weights.push_back(w);
    }

    std::discrete_distribution<> d(weights.begin(), weights.end());
    return keys[d(global_rng())];
}


using json = nlohmann::json;

// Struct for one pattern
struct Pattern {
    std::map<int, double> comm_partners;                     // normalized
    std::map<int, double> buffer_size;                    // fill-forward + normalized
    std::map<int, std::map<int, double>> dist_to_neighbors;  // each inner map normalized
};

// Conversion from JSON → struct
void from_json(const json& j, Pattern& p) {
    // --- comm_partners ---
    {
        std::map<int, int> temp;
        for (auto& [k, v] : j.at("comm_partners").items()) {
            temp[std::stoi(k)] = v.get<int>();
        }
        double total = 0.0;
        for (auto& [k, v] : temp) total += v;
        for (auto& [k, v] : temp) {
            p.comm_partners[k] = (total > 0) ? (double)v / total : 0.0;
        }
    }

    // --- buffer_size ---
    {
        std::map<int, int> temp;
        for (auto& [k, v] : j.at("buffer_size").items()) {
            temp[std::stoi(k)] = v.get<int>();
        }

        // fill-forward
        int last_nonzero = 0;
        for (auto& [key, value] : temp) {
            if (value != 0) {
                last_nonzero = value;
            } else if (last_nonzero != 0) {
                value = last_nonzero;
            }
        }

        // normalize
        double total = 0.0;
        for (auto& [k, v] : temp) total += v;
        for (auto& [k, v] : temp) {
            p.buffer_size[k] = (total > 0) ? (double)v / total : 0.0;
        }
    }

    // --- dist_to_neighbors ---
    for (auto& [outer_k, inner_obj] : j.at("dist_to_neighbors").items()) {
        int outer_key = std::stoi(outer_k);
        std::map<int, int> temp;
        for (auto& [inner_k, inner_v] : inner_obj.items()) {
            temp[std::stoi(inner_k)] = inner_v.get<int>();
        }

        double total = 0.0;
        for (auto& [k, v] : temp) total += v;

        std::map<int, double> norm_inner;
        for (auto& [k, v] : temp) {
            norm_inner[k] = (total > 0) ? (double)v / total : 0.0;
        }

        p.dist_to_neighbors[outer_key] = std::move(norm_inner);
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

    // Print normalized results
    for (auto& [name, pattern] : patterns) {
        std::cout << "Pattern: " << name << "\n";
	auto a = sample_from_map(pattern.comm_partners);
	auto b= sample_from_map(pattern.buffer_size);
	auto c = sample_from_map(pattern.dist_to_neighbors[a]);

    std::cout << "Sampled comm_partner: " <<a<< "\n";
    std::cout << "Sampled buffer_size: " << b << "\n";
    std::cout << "Sampled dist_to_neighbors: " <<c << "\n";

    }


}
