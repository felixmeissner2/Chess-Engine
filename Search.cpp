#include "Search.h"
#include "legalmoves.h"
#include "movement.h"
#include "Evaluation.h"
#include "king_check.h"
#include <vector>
#include <string>
#include <algorithm>
#include <climits>

// Constants for minimax
const int MAX_EVAL = 100000;
const int MIN_EVAL = -100000;

std::string minimax_driver(int target_depth, std::vector<std::vector<char>>& chess_board, int opp_move_start_i, int opp_move_start_j, int opp_move_dest_i, int opp_move_dest_j, char current_turn, bool& _white_king_moved, bool& _white_king_side_rook_moved, bool& _white_queen_side_rook_moved, bool& _black_king_moved, bool& _black_king_side_rook_moved, bool& _black_queen_side_rook_moved)
{
    char ai_color = current_turn;
    int best_eval = MIN_EVAL;
    std::string best_move = "";

    // Generate all legal moves for the AI
    std::vector<std::pair<std::string, std::string>> all_moves; // pair of (start, dest)

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            char piece = chess_board[i][j];

            // Skip empty squares and opponent pieces
            if (piece == '.')
                continue;

            if ((ai_color == 'W' && islower(piece)) || (ai_color == 'B' && isupper(piece)))
                continue;

            // Generate legal moves for this piece
            std::vector<std::string> moves = generate_legal_moves_for_a_piece(
                chess_board, ai_color, i, j,
                opp_move_start_i, opp_move_start_j,
                opp_move_dest_i, opp_move_dest_j,
                _white_king_moved, _white_king_side_rook_moved,
                _white_queen_side_rook_moved, _black_king_moved,
                _black_king_side_rook_moved, _black_queen_side_rook_moved
            );

            for (const auto& move : moves)
            {
                std::string start = std::to_string(i) + "," + std::to_string(j);
                all_moves.push_back({ start, move });
            }
        }
    }

    // Evaluate each move
    for (const auto& move_pair : all_moves)
    {
        std::string start = move_pair.first;
        std::string dest = move_pair.second;

        int start_i = start[0] - '0';
        int start_j = start[2] - '0';
        int dest_i = dest[0] - '0';
        int dest_j = dest[2] - '0';

        // Make the move
        char piece = chess_board[start_i][start_j];
        char captured = chess_board[dest_i][dest_j];

        chess_board[dest_i][dest_j] = piece;
        chess_board[start_i][start_j] = '.';

        // Update castling rights
        bool white_king_moved_backup = _white_king_moved;
        bool white_ks_rook_moved_backup = _white_king_side_rook_moved;
        bool white_qs_rook_moved_backup = _white_queen_side_rook_moved;
        bool black_king_moved_backup = _black_king_moved;
        bool black_ks_rook_moved_backup = _black_king_side_rook_moved;
        bool black_qs_rook_moved_backup = _black_queen_side_rook_moved;

        if (piece == 'K') _white_king_moved = true;
        if (piece == 'k') _black_king_moved = true;
        if (piece == 'R')
        {
            if (start_i == 7 && start_j == 0) _white_queen_side_rook_moved = true;
            if (start_i == 7 && start_j == 7) _white_king_side_rook_moved = true;
        }
        if (piece == 'r')
        {
            if (start_i == 0 && start_j == 0) _black_queen_side_rook_moved = true;
            if (start_i == 0 && start_j == 7) _black_king_side_rook_moved = true;
        }

        char opponent_turn = (ai_color == 'W') ? 'B' : 'W';

        // Evaluate this move with minimax
        int eval = minimax(chess_board, target_depth - 1, MIN_EVAL, MAX_EVAL, false, opponent_turn, ai_color, start_i, start_j, dest_i, dest_j, _white_king_moved, _white_king_side_rook_moved, _white_queen_side_rook_moved, _black_king_moved, _black_king_side_rook_moved, _black_queen_side_rook_moved);

        // Undo the move
        chess_board[start_i][start_j] = piece;
        chess_board[dest_i][dest_j] = captured;

        // Restore castling rights
        _white_king_moved = white_king_moved_backup;
        _white_king_side_rook_moved = white_ks_rook_moved_backup;
        _white_queen_side_rook_moved = white_qs_rook_moved_backup;
        _black_king_moved = black_king_moved_backup;
        _black_king_side_rook_moved = black_ks_rook_moved_backup;
        _black_queen_side_rook_moved = black_qs_rook_moved_backup;

        if (eval > best_eval)
        {
            best_eval = eval;
            best_move = start + "|" + dest;
        }
    }

    // If no legal moves found, return empty string
    if (best_move.empty())
    {
        return "0,0|0,0";
    }

    return best_move;
}

int minimax(std::vector<std::vector<char>>& chess_board, int depth, int alpha, int beta, bool maximizing_player, char current_turn, char ai_color, int opp_move_start_i, int opp_move_start_j, int opp_move_dest_i, int opp_move_dest_j, bool& _white_king_moved, bool& _white_king_side_rook_moved, bool& _white_queen_side_rook_moved, bool& _black_king_moved, bool& _black_king_side_rook_moved, bool& _black_queen_side_rook_moved)
{
    // Terminal node evaluation
    if (depth == 0)
    {
        return static_evaluate(chess_board, ai_color, current_turn);
    }

    // Check for checkmate or stalemate
    if (is_checkmate(chess_board, current_turn, opp_move_start_i, opp_move_start_j, opp_move_dest_i, opp_move_dest_j))
    {
        // If AI is checkmated, return very bad score
        if (current_turn == ai_color)
            return MIN_EVAL + depth;
        // If opponent is checkmated, return very good score
        else
            return MAX_EVAL - depth;
    }

    if (is_stalemate(chess_board, current_turn, opp_move_start_i, opp_move_start_j, opp_move_dest_i, opp_move_dest_j))
    {
        return 0; // Stalemate is neutral
    }

    if (maximizing_player)
    {
        int max_eval = MIN_EVAL;

        // Generate all legal moves
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                char piece = chess_board[i][j];

                if (piece == '.' || (current_turn == 'W' && islower(piece)) || (current_turn == 'B' && isupper(piece)))
                    continue;

                std::vector<std::string> moves = generate_legal_moves_for_a_piece(
                    chess_board, current_turn, i, j,
                    opp_move_start_i, opp_move_start_j,
                    opp_move_dest_i, opp_move_dest_j,
                    _white_king_moved, _white_king_side_rook_moved,
                    _white_queen_side_rook_moved, _black_king_moved,
                    _black_king_side_rook_moved, _black_queen_side_rook_moved
                );

                for (const auto& move : moves)
                {
                    int dest_i = move[0] - '0';
                    int dest_j = move[2] - '0';

                    // Make move
                    char captured = chess_board[dest_i][dest_j];
                    chess_board[dest_i][dest_j] = piece;
                    chess_board[i][j] = '.';

                    // Backup castling rights
                    bool wkm = _white_king_moved;
                    bool wksrm = _white_king_side_rook_moved;
                    bool wqsrm = _white_queen_side_rook_moved;
                    bool bkm = _black_king_moved;
                    bool bksrm = _black_king_side_rook_moved;
                    bool bqsrm = _black_queen_side_rook_moved;

                    // Update castling rights
                    if (piece == 'K') _white_king_moved = true;
                    if (piece == 'k') _black_king_moved = true;
                    if (piece == 'R')
                    {
                        if (i == 7 && j == 0) _white_queen_side_rook_moved = true;
                        if (i == 7 && j == 7) _white_king_side_rook_moved = true;
                    }
                    if (piece == 'r')
                    {
                        if (i == 0 && j == 0) _black_queen_side_rook_moved = true;
                        if (i == 0 && j == 7) _black_king_side_rook_moved = true;
                    }

                    char next_turn = (current_turn == 'W') ? 'B' : 'W';

                    int eval = minimax(chess_board, depth - 1, alpha, beta, false, next_turn, ai_color, i, j, dest_i, dest_j, _white_king_moved, _white_king_side_rook_moved, _white_queen_side_rook_moved, _black_king_moved, _black_king_side_rook_moved, _black_queen_side_rook_moved);

                    // Undo move
                    chess_board[i][j] = piece;
                    chess_board[dest_i][dest_j] = captured;

                    // Restore castling rights
                    _white_king_moved = wkm;
                    _white_king_side_rook_moved = wksrm;
                    _white_queen_side_rook_moved = wqsrm;
                    _black_king_moved = bkm;
                    _black_king_side_rook_moved = bksrm;
                    _black_queen_side_rook_moved = bqsrm;

                    max_eval = std::max(max_eval, eval);
                    alpha = std::max(alpha, eval);

                    if (beta <= alpha)
                        break; // Beta cutoff
                }
            }
        }

        return max_eval;
    }
    else
    {
        int min_eval = MAX_EVAL;

        // Generate all legal moves for opponent
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                char piece = chess_board[i][j];

                if (piece == '.' || (current_turn == 'W' && islower(piece)) || (current_turn == 'B' && isupper(piece)))
                    continue;

                std::vector<std::string> moves = generate_legal_moves_for_a_piece(
                    chess_board, current_turn, i, j,
                    opp_move_start_i, opp_move_start_j,
                    opp_move_dest_i, opp_move_dest_j,
                    _white_king_moved, _white_king_side_rook_moved,
                    _white_queen_side_rook_moved, _black_king_moved,
                    _black_king_side_rook_moved, _black_queen_side_rook_moved
                );

                for (const auto& move : moves)
                {
                    int dest_i = move[0] - '0';
                    int dest_j = move[2] - '0';

                    // Make move
                    char captured = chess_board[dest_i][dest_j];
                    chess_board[dest_i][dest_j] = piece;
                    chess_board[i][j] = '.';

                    // Backup castling rights
                    bool wkm = _white_king_moved;
                    bool wksrm = _white_king_side_rook_moved;
                    bool wqsrm = _white_queen_side_rook_moved;
                    bool bkm = _black_king_moved;
                    bool bksrm = _black_king_side_rook_moved;
                    bool bqsrm = _black_queen_side_rook_moved;

                    // Update castling rights
                    if (piece == 'K') _white_king_moved = true;
                    if (piece == 'k') _black_king_moved = true;
                    if (piece == 'R')
                    {
                        if (i == 7 && j == 0) _white_queen_side_rook_moved = true;
                        if (i == 7 && j == 7) _white_king_side_rook_moved = true;
                    }
                    if (piece == 'r')
                    {
                        if (i == 0 && j == 0) _black_queen_side_rook_moved = true;
                        if (i == 0 && j == 7) _black_king_side_rook_moved = true;
                    }

                    char next_turn = (current_turn == 'W') ? 'B' : 'W';

                    int eval = minimax(chess_board, depth - 1, alpha, beta, true, next_turn, ai_color, i, j, dest_i, dest_j, _white_king_moved, _white_king_side_rook_moved, _white_queen_side_rook_moved, _black_king_moved, _black_king_side_rook_moved, _black_queen_side_rook_moved);

                    // Undo move
                    chess_board[i][j] = piece;
                    chess_board[dest_i][dest_j] = captured;

                    // Restore castling rights
                    _white_king_moved = wkm;
                    _white_king_side_rook_moved = wksrm;
                    _white_queen_side_rook_moved = wqsrm;
                    _black_king_moved = bkm;
                    _black_king_side_rook_moved = bksrm;
                    _black_queen_side_rook_moved = bqsrm;

                    min_eval = std::min(min_eval, eval);
                    beta = std::min(beta, eval);

                    if (beta <= alpha)
                        break; // Alpha cutoff
                }
            }
        }

        return min_eval;
    }
}