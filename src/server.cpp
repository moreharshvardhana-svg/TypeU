#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>

// --- Core N-Gram Engine ---
std::unordered_map<std::string, std::unordered_map<char, float>> moddata;
const float MAX_WEIGHT = 100.0f;
const float SMOOTHING = 50.0f;

// Load the Weights/Parameters from the file. brain.bin
void loadModel(const std::string& filename = "brain.bin") {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return;
    moddata.clear();
    size_t map_size = 0;
    file.read(reinterpret_cast<char*>(&map_size), sizeof(map_size));
    for (size_t i = 0; i < map_size; ++i) {
        size_t key_len = 0;
        file.read(reinterpret_cast<char*>(&key_len), sizeof(key_len));
        std::string key(key_len, '\0');
        file.read(&key[0], key_len);
        size_t submap_size = 0;
        file.read(reinterpret_cast<char*>(&submap_size), sizeof(submap_size));
        for (size_t j = 0; j < submap_size; ++j) {
            char next_char;
            float weight;
            file.read(&next_char, sizeof(next_char));
            file.read(reinterpret_cast<char*>(&weight), sizeof(weight));
            moddata[key][next_char] = weight;
        }
    }
}

// Put the newly learnt word/weights into the file. brain.bin
void saveModel(const std::string& filename = "brain.bin") {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return;
    size_t map_size = moddata.size();
    file.write(reinterpret_cast<const char*>(&map_size), sizeof(map_size));
    for (const auto& [key, submap] : moddata) {
        size_t key_len = key.length();
        file.write(reinterpret_cast<const char*>(&key_len), sizeof(key_len));
        file.write(key.data(), key_len);
        size_t submap_size = submap.size();
        file.write(reinterpret_cast<const char*>(&submap_size), sizeof(submap_size));
        for (const auto& [next_char, weight] : submap) {
            file.write(&next_char, sizeof(next_char));
            file.write(reinterpret_cast<const char*>(&weight), sizeof(weight));
        }
    }
}

// Learn the relations/possibility of characters bw words.
void trainModel(const std::string& inputData) {
    long len = inputData.length();
    for (int i = 0; i < len; i++) {
        for (int a = 1; a < 5; a++) {
            if (i + a >= len) break;
            std::string keyString = inputData.substr(i, a);
            char keyChar = inputData[i + a];
            if (moddata.contains(keyString) && moddata[keyString].contains(keyChar)) {
                float w = moddata[keyString][keyChar];
                moddata[keyString][keyChar] = w + ((MAX_WEIGHT - w) / SMOOTHING);
            } else {
                moddata[keyString][keyChar] = 1.0f;
            }
        }
    }
}

// Predict next N words after the previous word.
std::vector<std::string> predictNext(const std::string& inputDat, int n = 3) {
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
        if (!word.empty() && find(suggestions.begin(), suggestions.end(), word) == suggestions.end()) {
            suggestions.push_back(word);
        }
    }
    return suggestions;
}

std::string urlDecode(const std::string& src) {
    std::string ret;
    char ch;
    int i, ii;
    for (i = 0; i < (int)src.length(); i++) {
        if (src[i] == '%') {
            sscanf(src.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            ret += ch;
            i += 2;
        } else if (src[i] == '+') {
            ret += ' ';
        } else {
            ret += src[i];
        }
    }
    return ret;
}

int main() {
    //load whatever it learnt from you if possible.
    loadModel("brain.bin");
    if (moddata.empty()) {
        std::cout << "[TypeU Server] No brain.bin found. Seeding initial vocabulary...\n";
        trainModel("The quick brown fox jumps over the lazy dog "); 
        trainModel("May the force be with you ");
        trainModel("Nightbear is the best! ");
        trainModel("The cake is a lie ");
        trainModel("Hello World! ");
        saveModel("brain.bin");
        std::cout << "[TypeU Server] Created fresh brain.bin on disk.\n";
    }

    // setup http socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "[TypeU Server] Error: Port 8080 is busy. Kill existing instance first.\n";
        return 1;
    }

    listen(server_fd, 10);
    std::cout << "[TypeU Server] Listening on http://localhost:8080...\n";

    // infinite loop.
    while (true) {
        int client_socket = accept(server_fd, nullptr, nullptr);
        if (client_socket < 0) continue;

        char buffer[2048] = {0};
        read(client_socket, buffer, sizeof(buffer) - 1);

        std::string request(buffer);
        std::string responseBody = "[]";

        // get seed word/character to predict next word
        if (request.find("GET /predict?q=") != std::string::npos) {
            size_t start = request.find("?q=") + 3;
            size_t end = request.find(" ", start);
            std::string query = urlDecode(request.substr(start, end - start));

            auto suggestions = predictNext(query, 3);
            responseBody = "[";
            for (size_t i = 0; i < suggestions.size(); ++i) {
                responseBody += "\"" + suggestions[i] + "\"";
                if (i + 1 < suggestions.size()) responseBody += ",";
            }
            responseBody += "]";
        }
        // get train
        else if (request.find("GET /train?q=") != std::string::npos) {
            size_t start = request.find("?q=") + 3;
            size_t end = request.find(" ", start);
            std::string query = urlDecode(request.substr(start, end - start));
            trainModel(query);
            saveModel("brain.bin");
            responseBody = "{\"status\":\"trained\"}";
        }

        std::string httpResponse = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: " + std::to_string(responseBody.length()) + "\r\n"
            "\r\n" + responseBody;

        write(client_socket, httpResponse.c_str(), httpResponse.length());
        close(client_socket);
    }

    close(server_fd);
    return 0;
}