#include "types.hpp"
#include <sstream>

namespace fianchetto {

std::string square_to_string(Square sq) {
    if (sq >= 64) return "??";
    int file = file_of(sq);
    int rank = rank_of(sq);
    std::ostringstream oss;
    oss << static_cast<char>('a' + file) << (rank + 1);
    return oss.str();
}

std::string move_to_string(Move move) {
    std::ostringstream oss;
    oss << square_to_string(move.from()) << square_to_string(move.to());
    if (move.is_promotion()) {
        char promo_chars[] = {' ', ' ', 'n', 'b', 'r', 'q', ' '};
        oss << static_cast<char>(std::tolower(promo_chars[static_cast<int>(move.promotion())]));
    }
    return oss.str();
}

Move string_to_move(const std::string& str) {
    if (str.length() < 4) return Move();
    int from_file = str[0] - 'a';
    int from_rank = str[1] - '1';
    int to_file = str[2] - 'a';
    int to_rank = str[3] - '1';
    
    Square from = square(from_file, from_rank);
    Square to = square(to_file, to_rank);
    
    PieceType promo = PieceType::NONE;
    if (str.length() > 4) {
        switch (str[4]) {
            case 'q': promo = PieceType::QUEEN; break;
            case 'r': promo = PieceType::ROOK; break;
            case 'b': promo = PieceType::BISHOP; break;
            case 'n': promo = PieceType::KNIGHT; break;
        }
    }
    
    return Move(from, to, PieceType::NONE, PieceType::NONE, promo);
}

} // namespace fianchetto

