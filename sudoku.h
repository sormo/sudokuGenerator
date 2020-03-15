#pragma once
#include <array>
#include <vector>

namespace Sudoku
{
    static const size_t BOARD_SIZE = 9;
    static const size_t GRID_COUNT = 3;

    using Board = std::array<std::array<uint8_t, BOARD_SIZE>, BOARD_SIZE>;

    void printBoard(const Board& board);

    size_t getSolutions(Board& board, std::vector<Board>* solutions = nullptr);
    std::vector<uint8_t> getCandidates(const Board& board, size_t row, size_t column);
    Board generateSudoku(size_t solutions, size_t spaces);
}
