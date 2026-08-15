#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <random>
#include <fstream>

using namespace std;

std::unordered_map<std::string, std::unordered_map<char, float>> moddata;

void saveModel(const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for writing!\n";
        return;
    }

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

    file.close();
    std::cout << "Model successfully saved to " << filename << " (" << map_size << " history states)\n";
}

void loadModel(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "No existing brain found (" << filename << "). Starting fresh!\n";
        return;
    }

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

    file.close();
    std::cout << "Model successfully loaded from " << filename << "!\n";
}


float findSC(const string& s,char c){
    //returns weight double value of the string character pair. <!> Do not check for fake vals <!>
    return moddata.at(s).at(c);
}

void applyWeightDecay(double decayFactor = 0.98) {
    for (auto& [history, submap] : moddata) {
        for (auto& [next_char, weight] : submap) {
            // Shrink every weight slightly
            weight *= decayFactor; 
        }
	}
}

int trainModel(const string& inputData){ //test input

    long len=inputData.length();
        for(int i = 0 ; i<len ; i++){

            for(int a = 1 ; a<5 ; a++){

                if(i+a >= len) break;
                string keyString=inputData.substr(i,a);
                char keyChar=inputData[i+a];

                if(moddata.contains(keyString) && moddata[keyString].contains(keyChar)){ //if it already exists

                    float currentWeight=moddata[keyString][keyChar];
                    moddata[keyString][keyChar]=(currentWeight+(100-currentWeight)/50); //smoothing function

                }else{
                    moddata[keyString][keyChar]=1.0;
                }


        }
    }
//     for (const auto& [str_key, inner_map] : moddata) {
    
//     for (const auto& [ch_key, val] : inner_map) {
//         std::cout << str_key << " : {" << ch_key << "," << val << "}\n";
//     }
// }
    return 0;

}

string gen(int tokens,string seed="Hello"){
  string out = seed; 

    random_device rd;
    mt19937 gen(rd());

    while (tokens--) {
        string current_history = "";
        
        for (int size = 4; size >= 1; size--) {
            if (out.length() >= size) {
                string test_history = out.substr(out.length() - size, size);
                if (moddata.contains(test_history)) {
                    current_history = test_history;
                    break;
                }
            }
        }

        if (current_history.empty() || moddata[current_history].empty()) {
            break; 
        }

        auto& choice_pool = moddata[current_history];

        double total_weight = 0;
        for (const auto& pair : choice_pool) {
            total_weight += pair.second;
        }

        uniform_real_distribution<> dis(0.0, total_weight);
        double dice_roll = dis(gen);

        char chosen_char = choice_pool.begin()->first; 
        for (const auto& pair : choice_pool) {
            if (dice_roll <= pair.second) {
                chosen_char = pair.first;
                break;
            }
            dice_roll -= pair.second;
        }

        out += chosen_char;
    }

    return out;
}

int console_main(){
    string filename = "brain.bin";
    
    // 1. Load memory on startup
    loadModel(filename);
    std::string inputData;
	cout<<"temp train check\n";
    int its=0;
    while(true){
        getline(cin,inputData);
        
        if (inputData == "end"){
            saveModel(filename);
            cout << "ENDED\n";
            return 0; break;
        }
        
        trainModel(inputData+" ");
        cout<<gen(150,inputData)<<endl;
        if(its>10){
            its=0;
            applyWeightDecay();
        }
        its++;
    }

}