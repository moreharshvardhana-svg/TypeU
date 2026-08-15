#include <emscripten/bind.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <sstream>

// Main Datastructure behind the model
std::unordered_map<std::string, std::unordered_map<char, float>> moddata;
const float maxWeight = 100.0f;
const float smoothingVal = 50.0f;

void trainModel(const std::string& inputData) {
    long len = inputData.length();
    for (int i = 0; i < len; i++) {
        for (int a = 1; a < 5; a++) {
            if (i + a >= len) break;
            std::string keyString = inputData.substr(i, a);
            char keyChar = inputData[i + a];
            if (moddata.contains(keyString) && moddata[keyString].contains(keyChar)) {
                float w = moddata[keyString][keyChar];
                moddata[keyString][keyChar] = w + ((maxWeight - w) / smoothingVal);
            } else {
                moddata[keyString][keyChar] = 1.0f;
            }
        }
    }
}

// Predict the next N words based on the prev word
std::vector<std::string> predictNext(const std::string& inputDat, int n) {
    std::vector<std::string> suggestions;
    if (inputDat.empty() || moddata.empty()) return suggestions;

    std::string hist = "";
    for (int len = std::min((int)inputDat.length(), 4); len >= 1; len--) {
        std::string test = inputDat.substr(inputDat.length() - len);
        if (moddata.contains(test)) { hist = test; break; }
    }
    if (hist.empty()) return suggestions;

    auto& pool = moddata[hist];
    std::vector<std::pair<char, float>> sorted(pool.begin(), pool.end());
    std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) { return a.second > b.second; });

    for (size_t i = 0; i < sorted.size() && (int)suggestions.size() < n; i++) {
        std::string word = "";
        word += sorted[i].first;
        std::string roll_seed = inputDat + sorted[i].first;

        for (int step = 0; step < 12; step++) {
            std::string sub_hist = "";
            for (int s = std::min((int)roll_seed.length(), 4); s >= 1; s--) {
                std::string test = roll_seed.substr(roll_seed.length() - s);
                if (moddata.contains(test)) { sub_hist = test; break; }
            }
            if (sub_hist.empty() || moddata[sub_hist].empty()) break;

            char best_char = ' ';
            float max_w = -1.0f;
            for (const auto& p : moddata[sub_hist]) {
                if (p.second > max_w) { max_w = p.second; best_char = p.first; }
            }
            if (best_char == ' ' || best_char == '\n') break;
            word += best_char;
            roll_seed += best_char;
        }

        if (!word.empty() && std::find(suggestions.begin(), suggestions.end(), word) == suggestions.end()) {
            suggestions.push_back(word);
        }
    }
    return suggestions;
}

//Binary Serialization in memory
std::string exportBinaryState() {
    std::ostringstream ss(std::ios::binary);
    size_t map_size = moddata.size();
    ss.write(reinterpret_cast<const char*>(&map_size), sizeof(map_size));

    for (const auto& [key, submap] : moddata) {
        size_t key_len = key.length();
        ss.write(reinterpret_cast<const char*>(&key_len), sizeof(key_len));
        ss.write(key.data(), key_len);

        size_t submap_size = submap.size();
        ss.write(reinterpret_cast<const char*>(&submap_size), sizeof(submap_size));

        for (const auto& [next_char, weight] : submap) {
            ss.write(&next_char, sizeof(next_char));
            ss.write(reinterpret_cast<const char*>(&weight), sizeof(weight));
        }
    }
    return ss.str();
}

void importBinaryState(const std::string& data) {
    if (data.empty()) return;
    std::istringstream ss(data, std::ios::binary);
    moddata.clear();

    size_t map_size = 0;
    ss.read(reinterpret_cast<char*>(&map_size), sizeof(map_size));

    for (size_t i = 0; i < map_size; ++i) {
        size_t key_len = 0;
        ss.read(reinterpret_cast<char*>(&key_len), sizeof(key_len));
        std::string key(key_len, '\0');
        ss.read(&key[0], key_len);

        size_t submap_size = 0;
        ss.read(reinterpret_cast<char*>(&submap_size), sizeof(submap_size));

        for (size_t j = 0; j < submap_size; ++j) {
            char next_char;
            float weight;
            ss.read(&next_char, sizeof(next_char));
            ss.read(reinterpret_cast<char*>(&weight), sizeof(weight));
            moddata[key][next_char] = weight;
        }
    }
}

//Starting default parameters/text
void initDefaults() {
    trainModel("The quick brown fox jumps over the lazy dog ");
    trainModel("May the force be with you ");
    trainModel("Nightbear is the best! ");
    trainModel("The cake is a lie ");
    trainModel("Hello World! ");
}
EMSCRIPTEN_BINDINGS(typeu_wasm) {
    emscripten::register_vector<std::string>("VectorString");
    emscripten::function("trainModel", &trainModel);
    emscripten::function("predictNext", &predictNext);
    emscripten::function("initDefaults", &initDefaults);
    emscripten::function("exportBinaryState", &exportBinaryState);
    emscripten::function("importBinaryState", &importBinaryState);
}