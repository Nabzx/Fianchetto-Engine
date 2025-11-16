#include "board.hpp"
#include "types.hpp"
#include "movegen.hpp"
#include <random>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <cstdlib>
#include <vector>

namespace fianchetto {

// Static Zobrist initialization
std::array<std::array<std::array<uint64_t, 64>, 7>, 2> Board::zobrist_pieces_;
std::array<uint64_t, 4> Board::zobrist_castling_;
std::array<uint64_t, 8> Board::zobrist_en_passant_;
uint64_t Board::zobrist_side_;
bool Board::zobrist_initialized_ = false;

void Board::init_zobrist() {
    if (zobrist_initialized_) return;

    std::mt19937_64 rng(12345); // Fixed seed for reproducibility

    for (int color = 0; color < 2; color++) {
        for (int piece = 0; piece < 7; piece++) {
            for (int sq = 0; sq < 64; sq++) {
                zobrist_pieces_[color][piece][sq] = rng();
            }
        }
    }

    for (int i = 0; i < 4; i++) {
        zobrist_castling_[i] = rng();
    }

    for (int i = 0; i < 8; i++) {
        zobrist_en_passant_[i] = rng();
    }

    zobrist_side_ = rng();
    zobrist_initialized_ = true;
}

Board::Board() {
    init_zobrist();
    bitboards_.fill({});
    pieces_.fill(PieceType::NONE);
    colors_.fill(Color::WHITE);
    stm_ = Color::WHITE;
    castling_.fill(false);
    ep_square_ = 64; // Invalid square
    halfmove_clock_ = 0;
    fullmove_number_ = 1;
    hash_key_ = 0;
    set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

Board::Board(const std::string& fen) : Board() {
    set_fen(fen);
}

void Board::set_fen(const std::string& fen) {
    // Clear board
    for (int i = 0; i < 64; i++) {
        pieces_[i] = PieceType::NONE;
    }
    bitboards_.fill({});

    std::istringstream iss(fen);
    std::string token;

    // Parse piece placement
    std::getline(iss, token, ' ');
    int rank = 7, file = 0;
    for (char c : token) {
        if (c == '/') {
            rank--;
            file = 0;
        } else if (c >= '1' && c <= '8') {
            file += c - '0';
        } else {
            Square sq = square(file, rank);
            Color color = (c >= 'A' && c <= 'Z') ? Color::WHITE : Color::BLACK;
            PieceType piece;
            switch (std::tolower(c)) {
                case 'p': piece = PieceType::PAWN; break;
                case 'n': piece = PieceType::KNIGHT; break;
                case 'b': piece = PieceType::BISHOP; break;
                case 'r': piece = PieceType::ROOK; break;
                case 'q': piece = PieceType::QUEEN; break;
                case 'k': piece = PieceType::KING; break;
                default: continue;
            }
            place_piece(sq, piece, color);
            file++;
        }
    }

    // Parse side to move
    std::getline(iss, token, ' ');
    stm_ = (token == "w") ? Color::WHITE : Color::BLACK;

    // Parse castling
    std::getline(iss, token, ' ');
    castling_.fill(false);
    if (token != "-") {
        if (token.find('K') != std::string::npos) castling_[0] = true;
        if (token.find('Q') != std::string::npos) castling_[1] = true;
        if (token.find('k') != std::string::npos) castling_[2] = true;
        if (token.find('q') != std::string::npos) castling_[3] = true;
    }

    // Parse en passant
    std::getline(iss, token, ' ');
    if (token == "-") {
        ep_square_ = 64;
    } else {
        file = token[0] - 'a';
        rank = token[1] - '1';
        ep_square_ = square(file, rank);
    }

    // Parse halfmove clock
    std::getline(iss, token, ' ');
    halfmove_clock_ = token.empty() ? 0 : std::stoi(token);

    // Parse fullmove number
    std::getline(iss, token, ' ');
    fullmove_number_ = token.empty() ? 1 : std::stoi(token);

    update_hash();
}

std::string Board::get_fen() const {
    std::ostringstream oss;
    
    // Piece placement
    for (int rank = 7; rank >= 0; rank--) {
        int empty = 0;
        for (int file = 0; file < 8; file++) {
            Square sq = square(file, rank);
            PieceType piece = pieces_[sq];
            if (piece == PieceType::NONE) {
                empty++;
            } else {
                if (empty > 0) {
                    oss << empty;
                    empty = 0;
                }
                char c;
                switch (piece) {
                    case PieceType::PAWN: c = 'p'; break;
                    case PieceType::KNIGHT: c = 'n'; break;
                    case PieceType::BISHOP: c = 'b'; break;
                    case PieceType::ROOK: c = 'r'; break;
                    case PieceType::QUEEN: c = 'q'; break;
                    case PieceType::KING: c = 'k'; break;
                    default: c = '?'; break;
                }
                if (colors_[sq] == Color::WHITE) c = std::toupper(c);
                oss << c;
            }
        }
        if (empty > 0) oss << empty;
        if (rank > 0) oss << '/';
    }

    oss << ' ' << (stm_ == Color::WHITE ? 'w' : 'b') << ' ';

    // Castling
    bool any_castle = false;
    if (castling_[0]) { oss << 'K'; any_castle = true; }
    if (castling_[1]) { oss << 'Q'; any_castle = true; }
    if (castling_[2]) { oss << 'k'; any_castle = true; }
    if (castling_[3]) { oss << 'q'; any_castle = true; }
    if (!any_castle) oss << '-';

    oss << ' ';

    // En passant
    if (ep_square_ < 64) {
        oss << static_cast<char>('a' + file_of(ep_square_));
        oss << static_cast<char>('1' + rank_of(ep_square_));
    } else {
        oss << '-';
    }

    oss << ' ' << halfmove_clock_ << ' ' << fullmove_number_;

    return oss.str();
}

PieceType Board::piece_on(Square sq) const {
    return pieces_[sq];
}

Color Board::color_on(Square sq) const {
    return colors_[sq];
}

void Board::place_piece(Square sq, PieceType piece, Color color) {
    pieces_[sq] = piece;
    colors_[sq] = color;
    Bitboard bb = 1ULL << sq;
    bitboards_[static_cast<int>(color)][static_cast<int>(piece)] |= bb;
}

void Board::remove_piece(Square sq) {
    PieceType piece = pieces_[sq];
    Color color = colors_[sq];
    if (piece != PieceType::NONE) {
        Bitboard bb = ~(1ULL << sq);
        bitboards_[static_cast<int>(color)][static_cast<int>(piece)] &= bb;
        pieces_[sq] = PieceType::NONE;
    }
}

Bitboard Board::pieces(PieceType piece, Color color) const {
    return bitboards_[static_cast<int>(color)][static_cast<int>(piece)];
}

Bitboard Board::all_pieces(Color color) const {
    Bitboard bb = 0;
    for (int pt = 1; pt < 7; pt++) {
        bb |= bitboards_[static_cast<int>(color)][pt];
    }
    return bb;
}

Bitboard Board::all_pieces() const {
    return all_pieces(Color::WHITE) | all_pieces(Color::BLACK);
}

void Board::make_move(Move move) {
    MoveInfo info;
    info.move = move;
    info.captured = piece_on(move.to());
    info.castle_kingside[0] = castling_[0];
    info.castle_kingside[1] = castling_[2];
    info.castle_queenside[0] = castling_[1];
    info.castle_queenside[1] = castling_[3];
    info.ep_square = ep_square_;
    info.halfmove_clock = halfmove_clock_;
    info.hash_key = hash_key_;

    Square from = move.from();
    Square to = move.to();
    PieceType piece = move.piece();
    Color color = stm_;

    // Remove captured piece
    if (info.captured != PieceType::NONE) {
        remove_piece(to);
    }

    // Handle en passant
    if (move.is_en_passant()) {
        Square ep_capture = (color == Color::WHITE) ? to - 8 : to + 8;
        remove_piece(ep_capture);
    }

    // Move piece
    remove_piece(from);
    if (move.is_promotion()) {
        place_piece(to, move.promotion(), color);
    } else {
        place_piece(to, piece, color);
    }

    // Handle castling
    if (move.is_castling()) {
        if (to == square(6, rank_of(from))) { // Kingside
            Square rook_from = square(7, rank_of(from));
            Square rook_to = square(5, rank_of(from));
            remove_piece(rook_from);
            place_piece(rook_to, PieceType::ROOK, color);
        } else if (to == square(2, rank_of(from))) { // Queenside
            Square rook_from = square(0, rank_of(from));
            Square rook_to = square(3, rank_of(from));
            remove_piece(rook_from);
            place_piece(rook_to, PieceType::ROOK, color);
        }
    }

    // Update castling rights
    if (piece == PieceType::KING) {
        castling_[static_cast<int>(color) * 2] = false;
        castling_[static_cast<int>(color) * 2 + 1] = false;
    }
    if (piece == PieceType::ROOK) {
        if (from == square(0, rank_of(from))) castling_[static_cast<int>(color) * 2 + 1] = false;
        if (from == square(7, rank_of(from))) castling_[static_cast<int>(color) * 2] = false;
    }
    if (to == square(0, rank_of(to)) && piece_on(to) == PieceType::ROOK) {
        castling_[static_cast<int>(1 - static_cast<int>(color)) * 2 + 1] = false;
    }
    if (to == square(7, rank_of(to)) && piece_on(to) == PieceType::ROOK) {
        castling_[static_cast<int>(1 - static_cast<int>(color)) * 2] = false;
    }

    // Update en passant
    ep_square_ = 64;
    if (piece == PieceType::PAWN && abs(rank_of(to) - rank_of(from)) == 2) {
        ep_square_ = (color == Color::WHITE) ? to - 8 : to + 8;
    }

    // Update counters
    if (piece == PieceType::PAWN || info.captured != PieceType::NONE) {
        halfmove_clock_ = 0;
    } else {
        halfmove_clock_++;
    }
    if (color == Color::BLACK) {
        fullmove_number_++;
    }

    stm_ = (color == Color::WHITE) ? Color::BLACK : Color::WHITE;
    history_.push_back(info);
    update_hash();
}

void Board::unmake_move(Move move) {
    if (history_.empty()) return;

    MoveInfo info = history_.back();
    history_.pop_back();

    Square from = move.from();
    Square to = move.to();
    PieceType piece = move.piece();
    Color color = (stm_ == Color::WHITE) ? Color::BLACK : Color::WHITE;

    // Restore side to move
    stm_ = color;

    // Restore piece
    remove_piece(to);
    if (move.is_promotion()) {
        place_piece(from, PieceType::PAWN, color);
    } else {
        place_piece(from, piece, color);
    }

    // Restore captured piece
    if (info.captured != PieceType::NONE) {
        place_piece(to, info.captured, (color == Color::WHITE) ? Color::BLACK : Color::WHITE);
    }

    // Restore en passant
    if (move.is_en_passant()) {
        Square ep_capture = (color == Color::WHITE) ? to - 8 : to + 8;
        place_piece(ep_capture, PieceType::PAWN, (color == Color::WHITE) ? Color::BLACK : Color::WHITE);
    }

    // Restore castling
    if (move.is_castling()) {
        if (to == square(6, rank_of(from))) {
            Square rook_from = square(5, rank_of(from));
            Square rook_to = square(7, rank_of(from));
            remove_piece(rook_from);
            place_piece(rook_to, PieceType::ROOK, color);
        } else if (to == square(2, rank_of(from))) {
            Square rook_from = square(3, rank_of(from));
            Square rook_to = square(0, rank_of(from));
            remove_piece(rook_from);
            place_piece(rook_to, PieceType::ROOK, color);
        }
    }

    // Restore state
    castling_ = {info.castle_kingside[0], info.castle_queenside[0],
                 info.castle_kingside[1], info.castle_queenside[1]};
    ep_square_ = info.ep_square;
    halfmove_clock_ = info.halfmove_clock;
    if (color == Color::BLACK) {
        fullmove_number_--;
    }
    hash_key_ = info.hash_key;
}

bool Board::in_check(Color color) const {
    Square king_sq = 64;
    Bitboard king_bb = pieces(PieceType::KING, color);
    if (king_bb == 0) return false;
    
    // Find king square
    king_sq = __builtin_ctzll(king_bb);

    Color enemy = (color == Color::WHITE) ? Color::BLACK : Color::WHITE;
    
    // Check for attacks
    Bitboard attacks = 0;
    attacks |= movegen::pawn_attacks(king_sq, color);
    attacks |= movegen::knight_attacks(king_sq);
    attacks |= movegen::bishop_attacks(king_sq, all_pieces());
    attacks |= movegen::rook_attacks(king_sq, all_pieces());
    attacks |= movegen::queen_attacks(king_sq, all_pieces());
    attacks |= movegen::king_attacks(king_sq);

    return (attacks & all_pieces(enemy)) != 0;
}

bool Board::is_legal_move(Move move) const {
    // Make move and check if king is in check
    Board temp = *this;
    temp.make_move(move);
    return !temp.in_check(stm_);
}

void Board::update_hash() {
    hash_key_ = 0;

    // Hash pieces
    for (int sq = 0; sq < 64; sq++) {
        PieceType piece = pieces_[sq];
        if (piece != PieceType::NONE) {
            Color color = colors_[sq];
            hash_key_ ^= zobrist_pieces_[static_cast<int>(color)][static_cast<int>(piece)][sq];
        }
    }

    // Hash castling
    for (int i = 0; i < 4; i++) {
        if (castling_[i]) {
            hash_key_ ^= zobrist_castling_[i];
        }
    }

    // Hash en passant
    if (ep_square_ < 64) {
        int file = file_of(ep_square_);
        hash_key_ ^= zobrist_en_passant_[file];
    }

    // Hash side to move
    if (stm_ == Color::BLACK) {
        hash_key_ ^= zobrist_side_;
    }
}

} // namespace fianchetto

