#include "board.hpp"
#include "search.hpp"
#include "movegen.hpp"
#include <iostream>
#include <sstream>
#include <string>

#ifdef USE_NEURAL
#include "neural_client.hpp"
#endif

int main() {
    fianchetto::Board board;
    fianchetto::SearchParams params;
    std::string line;

#ifdef USE_NEURAL
    fianchetto::NeuralClient neural_client;
    params.use_neural = true;
#endif

    std::cout << "Fianchetto Engine v1.0" << std::endl;

    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "uci") {
            std::cout << "id name Fianchetto Engine" << std::endl;
            std::cout << "id author Fianchetto Team" << std::endl;
            std::cout << "uciok" << std::endl;
        }
        else if (cmd == "isready") {
            std::cout << "readyok" << std::endl;
        }
        else if (cmd == "ucinewgame") {
            board.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        }
        else if (cmd == "position") {
            std::string type;
            iss >> type;
            if (type == "startpos") {
                board.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            } else if (type == "fen") {
                std::string fen;
                std::string part;
                for (int i = 0; i < 6 && iss >> part; i++) {
                    if (i > 0) fen += " ";
                    fen += part;
                }
                board.set_fen(fen);
            }
            
            std::string moves_cmd;
            if (iss >> moves_cmd && moves_cmd == "moves") {
                std::string move_str;
                while (iss >> move_str) {
                    // Parse move (simplified - assumes UCI format like "e2e4")
                    if (move_str.length() >= 4) {
                        int from_file = move_str[0] - 'a';
                        int from_rank = move_str[1] - '1';
                        int to_file = move_str[2] - 'a';
                        int to_rank = move_str[3] - '1';
                        
                        fianchetto::Square from = fianchetto::square(from_file, from_rank);
                        fianchetto::Square to = fianchetto::square(to_file, to_rank);
                        
                        // Find matching move
                        auto moves = fianchetto::movegen::generate_legal_moves(board);
                        for (fianchetto::Move m : moves) {
                            if (m.from() == from && m.to() == to) {
                                if (move_str.length() > 4) {
                                    // Promotion
                                    char promo = move_str[4];
                                    if (m.is_promotion() && 
                                        ((promo == 'q' && m.promotion() == fianchetto::PieceType::QUEEN) ||
                                         (promo == 'r' && m.promotion() == fianchetto::PieceType::ROOK) ||
                                         (promo == 'b' && m.promotion() == fianchetto::PieceType::BISHOP) ||
                                         (promo == 'n' && m.promotion() == fianchetto::PieceType::KNIGHT))) {
                                        board.make_move(m);
                                        break;
                                    }
                                } else {
                                    board.make_move(m);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (cmd == "go") {
            std::string subcmd;
            while (iss >> subcmd) {
                if (subcmd == "depth") {
                    int d;
                    if (iss >> d) params.depth = d;
                } else if (subcmd == "movetime") {
                    int ms;
                    if (iss >> ms) params.time_limit_ms = ms;
                }
            }

            fianchetto::SearchStats stats;
            fianchetto::Move best = fianchetto::search_root(board, params, stats);

            std::cout << "bestmove ";
            std::cout << static_cast<char>('a' + fianchetto::file_of(best.from()));
            std::cout << static_cast<char>('1' + fianchetto::rank_of(best.from()));
            std::cout << static_cast<char>('a' + fianchetto::file_of(best.to()));
            std::cout << static_cast<char>('1' + fianchetto::rank_of(best.to()));
            if (best.is_promotion()) {
                char promo_chars[] = {' ', ' ', 'n', 'b', 'r', 'q', ' '};
                std::cout << static_cast<char>(std::tolower(promo_chars[static_cast<int>(best.promotion())]));
            }
            std::cout << std::endl;
        }
        else if (cmd == "stop") {
            // Stop search (simplified - would need threading for real implementation)
        }
        else if (cmd == "quit") {
            break;
        }
    }

    return 0;
}

