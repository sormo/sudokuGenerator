#pragma once
#include <array>
#include <vector>
#include <optional>

namespace Sudoku
{
    static const size_t BOARD_SIZE = 9;
    static const size_t GRID_COUNT = 3;

    using Board = std::array<std::array<uint8_t, BOARD_SIZE>, BOARD_SIZE>;
    using Candidates = std::array<bool, BOARD_SIZE + 1>;
    using BoardCandidates = std::array<std::array<Candidates, BOARD_SIZE>, BOARD_SIZE>;

    void printBoard(const Board& board);

    std::vector<uint8_t> getCandidates(const Board& board, size_t row, size_t column);
    size_t getSolutions(Board& board, std::vector<Board>& solutions);
    std::tuple<Board, Board> generateSudoku(size_t spaces);
    std::tuple<Board, Board> generateSudokuWithDifficulty(size_t spaces, size_t minDifficulty, size_t maxDifficulty);
    // up to 90 is hard
    // more than 300 is easy
    size_t computeDifficulty(const Board& solution, const Board& board);

    // solve with simple single candidate method
    // if allowRowColElimination is true, candidates are eliminitated if in the grid candidates for one number are in single row / column
    bool solveSudoku(const Board& board, bool allowRowColElimination);
}
