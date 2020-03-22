#pragma once
#include <array>
#include <vector>
#include <optional>

namespace Sudoku
{
    static const size_t BOARD_SIZE = 9;
    static const size_t GRID_COUNT = 3;

    using Board = std::array<std::array<uint8_t, BOARD_SIZE>, BOARD_SIZE>;

    void printBoard(const Board& board);

    std::vector<uint8_t> getCandidates(const Board& board, size_t row, size_t column);
    size_t getSolutions(Board& board, std::vector<Board>& solutions);
    Board generateSudoku(size_t spaces);

    // solve with simple single candidate method
    std::optional<Board> solveSudoku(const Board& board);
}
