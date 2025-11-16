#pragma once

#ifdef USE_NEURAL

#include "board.hpp"
#include <string>
#include <unordered_map>
#include <mutex>

namespace fianchetto {

class NeuralClient {
public:
    NeuralClient(const std::string& url = "http://neural:8000/evaluate");
    
    // Evaluate position and return centipawn score
    int evaluate(const Board& board);
    
    // Clear cache
    void clear_cache();

private:
    std::string url_;
    std::unordered_map<uint64_t, int> cache_;
    std::mutex cache_mutex_;
    
    int http_evaluate(const std::string& fen);
};

} // namespace fianchetto

#endif // USE_NEURAL

