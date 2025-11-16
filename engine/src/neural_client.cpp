#ifdef USE_NEURAL

#include "neural_client.hpp"
#include <curl/curl.h>
#include <sstream>
#include <iostream>

namespace fianchetto {

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    size_t total_size = size * nmemb;
    data->append((char*)contents, total_size);
    return total_size;
}

NeuralClient::NeuralClient(const std::string& url) : url_(url) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

int NeuralClient::http_evaluate(const std::string& fen) {
    CURL* curl = curl_easy_init();
    if (!curl) return 0;

    std::string response;
    std::ostringstream json;
    json << "{\"fen\":\"" << fen << "\"}";

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url_.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.str().c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return 0;
    }

    // Parse JSON response (simplified)
    // Expected: {"score": 42}
    size_t pos = response.find("\"score\":");
    if (pos != std::string::npos) {
        pos += 8;
        while (pos < response.length() && (response[pos] == ' ' || response[pos] == ':')) pos++;
        return std::stoi(response.substr(pos));
    }

    return 0;
}

int NeuralClient::evaluate(const Board& board) {
    uint64_t hash = board.hash();
    
    // Check cache
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        auto it = cache_.find(hash);
        if (it != cache_.end()) {
            return it->second;
        }
    }

    // Call neural service
    std::string fen = board.get_fen();
    int score = http_evaluate(fen);

    // Cache result
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        cache_[hash] = score;
    }

    return score;
}

void NeuralClient::clear_cache() {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    cache_.clear();
}

} // namespace fianchetto

#endif // USE_NEURAL

