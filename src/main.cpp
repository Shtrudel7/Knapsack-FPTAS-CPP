#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

struct Item { int weight; int value; };
struct Result { long long value; std::string method; std::size_t states; };

std::vector<Item> readItems(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Cannot open input: " + path);
    std::vector<Item> items;
    long long w, v;
    while (in >> w >> v) {
        if (w <= 0 || v < 0 || w > std::numeric_limits<int>::max() || v > std::numeric_limits<int>::max())
            throw std::runtime_error("Invalid item weight/value in " + path);
        items.push_back({static_cast<int>(w), static_cast<int>(v)});
    }
    if (!in.eof()) throw std::runtime_error("Malformed item data: " + path);
    return items;
}

std::vector<Item> eligible(const std::vector<Item>& items, int capacity) {
    std::vector<Item> out;
    for (const auto& item : items)
        if (item.weight <= capacity && item.value > 0) out.push_back(item);
    return out;
}

Result exactByWeight(const std::vector<Item>& items, int capacity) {
    std::vector<long long> dp(static_cast<std::size_t>(capacity) + 1, 0);
    for (const auto& item : items)
        for (int w = capacity; w >= item.weight; --w)
            dp[w] = std::max(dp[w], dp[w - item.weight] + item.value);
    return {dp[capacity], "exact capacity DP", dp.size()};
}

// Classic value-scaling FPTAS; dp[s] stores the minimum weight obtaining
// scaled profit s; actual[s] stores the actual profit for that witness.
Result fptasByValue(const std::vector<Item>& items, int capacity, double epsilon,
                    std::size_t maxStates = 4000000) {
    if (!(epsilon > 0.0 && epsilon < 1.0) || !std::isfinite(epsilon))
        throw std::invalid_argument("epsilon must satisfy 0 < epsilon < 1");
    const auto a = eligible(items, capacity);
    if (a.empty()) return {0, "value-scaling FPTAS", 1};
    int maxValue = 0;
    for (const auto& item : a) maxValue = std::max(maxValue, item.value);
    const double K = epsilon * static_cast<double>(maxValue) / a.size();
    std::vector<std::size_t> scaled;
    std::size_t total = 0;
    for (const auto& item : a) {
        double s = std::floor(static_cast<double>(item.value) / K);
        if (!std::isfinite(s) || s > static_cast<double>(maxStates) ||
            total > maxStates - static_cast<std::size_t>(s))
            throw std::runtime_error("Scaled DP exceeds safe state limit; use --method auto or exact");
        auto q = static_cast<std::size_t>(s);
        scaled.push_back(q);
        total += q;
    }
    const long long INF = std::numeric_limits<long long>::max() / 4;
    std::vector<long long> weight(total + 1, INF), actual(total + 1, 0);
    weight[0] = 0;
    for (std::size_t i = 0; i < a.size(); ++i) {
        const auto gain = scaled[i];
        // Reverse traversal guarantees 0/1 semantics; zero-scaled items are
        // ignored because they cannot improve scaled profit for positive weight.
        if (gain == 0) continue;
        for (std::size_t s = total + 1; s-- > gain;) {
            if (weight[s - gain] == INF) continue;
            auto candidate = weight[s - gain] + a[i].weight;
            auto candidateValue = actual[s - gain] + a[i].value;
            if (candidate < weight[s] || (candidate == weight[s] && candidateValue > actual[s])) {
                weight[s] = candidate;
                actual[s] = candidateValue;
            }
        }
    }
    long long best = 0;
    for (std::size_t s = 0; s <= total; ++s)
        if (weight[s] <= capacity) best = std::max(best, actual[s]);
    return {best, "value-scaling FPTAS", total + 1};
}

int main(int argc, char** argv) {
    try {
        if (argc < 3) {
            std::cerr << "Usage: knapsack <items.txt> <capacity> [epsilon=0.1] [--method auto|exact|fptas]\n";
            return 2;
        }
        const std::string path = argv[1];
        long long inputCapacity = std::stoll(argv[2]);
        if (inputCapacity < 0 || inputCapacity > std::numeric_limits<int>::max())
            throw std::invalid_argument("capacity must be between 0 and INT_MAX");
        const int capacity = static_cast<int>(inputCapacity);
        double epsilon = (argc >= 4 && std::string(argv[3]).rfind("--", 0) != 0)
            ? std::stod(argv[3]) : 0.1;
        std::string method = "auto";
        for (int i = 3; i < argc; ++i) {
            if (std::string(argv[i]) == "--method" && i + 1 < argc) method = argv[++i];
        }
        if (!(epsilon > 0.0 && epsilon < 1.0) || !std::isfinite(epsilon))
            throw std::invalid_argument("epsilon must satisfy 0 < epsilon < 1");
        auto items = readItems(path);
        auto start = std::chrono::steady_clock::now();
        Result result;
        if (method == "exact" || (method == "auto" && capacity <= 100000))
            result = exactByWeight(eligible(items, capacity), capacity);
        else if (method == "fptas" || method == "auto")
            result = fptasByValue(items, capacity, epsilon);
        else throw std::invalid_argument("Unknown method: " + method);
        auto finish = std::chrono::steady_clock::now();
        std::cout << "Items: " << items.size() << "\nCapacity: " << capacity
                  << "\nEpsilon: " << epsilon << "\nMethod: " << result.method
                  << "\nObjective value: " << result.value << "\nDP states: " << result.states
                  << "\nElapsed (s): " << std::fixed << std::setprecision(6)
                  << std::chrono::duration<double>(finish - start).count() << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
