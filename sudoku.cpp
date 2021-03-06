#include "sudoku.h"
#include <random>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <set>

namespace Sudoku
{
    static const size_t NUMBERS_COUNT = BOARD_SIZE + 1; // +1 here because 0 is valid number (empty)

    std::random_device g_rd;
    std::mt19937 g_mt(g_rd());

    struct RowCol
    {
        size_t row = 0;
        size_t col = 0;

        RowCol next()
        {
            if (col + 1 >= BOARD_SIZE)
                return { row + 1, 0 };
            return { row, col + 1 };
        }

        friend bool operator<(const RowCol& l, const RowCol& r) noexcept
        {
            return l.row == r.row ? l.col < r.col : l.row < r.row;
        }
    };

    void printBoard(const Board& board)
    {
        for (size_t r = 0; r < board.size(); ++r)
        {
            for (size_t c = 0; c < board[r].size(); ++c)
            {
                std::cout << std::setw(4) << (int32_t)board[r][c];
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }

    std::vector<uint8_t> getCandidates(const Board& board, size_t row, size_t column)
    {
        std::array<bool, NUMBERS_COUNT> candidates;
        std::fill(std::begin(candidates), std::end(candidates), true);

        // check the row
        for (size_t i = 0; i < BOARD_SIZE; ++i)
            candidates[board[row][i]] = false;

        // check the column
        for (size_t i = 0; i < BOARD_SIZE; ++i)
            candidates[board[i][column]] = false;

        // check the grid
        size_t rowStart = (row / GRID_COUNT) * GRID_COUNT;
        size_t colStart = (column / GRID_COUNT) * GRID_COUNT;
        for (size_t r = rowStart; r < rowStart + GRID_COUNT; ++r)
        {
            for (size_t c = colStart; c < colStart + GRID_COUNT; ++c)
            {
                candidates[board[r][c]] = false;
            }
        }

        // omit the 0 as candidate
        std::vector<uint8_t> result;
        result.reserve(BOARD_SIZE);

        for (size_t i = 1; i < candidates.size(); ++i)
        {
            if (candidates[i])
                result.push_back(static_cast<uint8_t>(i));
        }

        return result;
    }

    void applyConstraints(const Board* constraints, size_t row, size_t col, std::vector<uint8_t>& candidates)
    {
        if (!constraints)
            return;

        if (constraints->at(row)[col] == 0)
            return;

        auto it = std::find(std::begin(candidates), std::end(candidates), constraints->at(row)[col]);
        if (it != std::end(candidates))
            candidates.erase(it);
    }

    bool solveRandomBoard(Board& board, const Board* constraints = nullptr, RowCol rowCol = {})
    {
        auto [row, col] = rowCol;
        if (row >= BOARD_SIZE)
            return true;

        if (board[row][col] != 0)
        {
            return solveRandomBoard(board, constraints, rowCol.next());
        }

        auto candidates = getCandidates(board, row, col);
        applyConstraints(constraints, row, col, candidates);
        std::shuffle(std::begin(candidates), std::end(candidates), g_mt);

        for (auto candidate : candidates)
        {
            board[row][col] = candidate;

            if (solveRandomBoard(board, constraints, rowCol.next()))
                return true;

            // this is important (:
            board[row][col] = 0;
        }

        return false;
    }

    Board prepareRandomBoard()
    {
        Board board{};

        // initialize diagonal grids which are indenpendent
        for (size_t start = 0; start < GRID_COUNT; ++start)
        {
            std::array<uint8_t, BOARD_SIZE> array;
            for (uint8_t v = 1; v <= BOARD_SIZE; ++v)
                array[v - 1] = v;
            std::shuffle(std::begin(array), std::end(array), g_mt);

            size_t rowStart = GRID_COUNT * start, colStart = GRID_COUNT * start, counter = 0;
            for (size_t r = rowStart; r < rowStart + GRID_COUNT; ++r)
            {
                for (size_t c = colStart; c < colStart + GRID_COUNT; ++c)
                {
                    board[r][c] = array[counter++];
                }
            }
        }

        // find first random solution
        assert(solveRandomBoard(board));

        return board;
    }

    size_t getSolutionsInternal(Board& board, std::vector<Board>* solutions = nullptr, RowCol rowCol = {})
    {
        auto [row, col] = rowCol;
        if (row >= BOARD_SIZE)
        {
            if (solutions)
                solutions->push_back(board);
            return 1;
        }

        if (board[row][col] != 0)
        {
            return getSolutionsInternal(board, solutions, rowCol.next());
        }

        auto candidates = getCandidates(board, row, col);
        size_t count = 0;

        for (auto candidate : candidates)
        {
            board[row][col] = candidate;

            count += getSolutionsInternal(board, solutions, rowCol.next());

            // this is important (:
            board[row][col] = 0;
        }

        return count;
    }

    size_t getSolutions(Board& board, std::vector<Board>& solutions)
    {
        return getSolutionsInternal(board, &solutions);
    }

    std::vector<RowCol> initializeSpaceCandidates()
    {
        std::vector<RowCol> result;
        result.reserve(BOARD_SIZE * BOARD_SIZE);

        for (size_t r = 0; r < BOARD_SIZE; ++r)
            for (size_t c = 0; c < BOARD_SIZE; ++c)
                result.push_back({ r, c });

        return result;
    }

    RowCol acquireRandomSpaceCandidate(std::vector<RowCol>& candidates)
    {
        std::uniform_int_distribution<size_t> rand(0, candidates.size() - 1);
        
        auto it = std::begin(candidates);
        std::advance(it, rand(g_mt));
        
        auto res = *it;
        candidates.erase(it);
        
        return res;
    }

    bool removeSpaces(Board& board, size_t spaces)
    {
        // create vector of candidates for spaces
        auto spaceCandidates = initializeSpaceCandidates();
        Board constraints{};

        for (size_t i = 0; i < spaces;)
        {
            if (spaceCandidates.empty())
            {
                // we are not able to create board with spaces
                return false;
            }

            auto [row, col] = acquireRandomSpaceCandidate(spaceCandidates);

            constraints[row][col] = board[row][col];
            board[row][col] = 0;

            // if we solve the board with the constraint, we have introduced
            // another solution
            Board tmp = board;
            if (solveRandomBoard(tmp, &constraints))
            {
                // revert back
                board[row][col] = constraints[row][col];
            }
            else
            {
                i++;
            }

            constraints[row][col] = 0;
        }

        return true;
    }

    Board generateSudoku(size_t spaces)
    {
        Board board = prepareRandomBoard();;

        while (!removeSpaces(board, spaces))
        {
            board = prepareRandomBoard();
        }

        return board;
    }

    size_t countSpaces(const Board& board)
    {
        size_t res = 0;

        for (size_t r = 0; r < Sudoku::BOARD_SIZE; ++r)
            for (size_t c = 0; c < Sudoku::BOARD_SIZE; ++c)
                res += board[r][c] == 0 ? 1 : 0;

        return res;
    }

    std::optional<Board> solveSudoku(const Board& board)
    {
        size_t spaces = countSpaces(board);
        Board solution = board;

        while (spaces != 0)
        {
            bool filled = false;

            for (size_t r = 0; r < Sudoku::BOARD_SIZE; ++r)
            {
                for (size_t c = 0; c < Sudoku::BOARD_SIZE; ++c)
                {
                    if (solution[r][c] != 0)
                        continue;

                    auto candidates = getCandidates(solution, r, c);
                    
                    if (candidates.size() != 1)
                        continue;

                    solution[r][c] = candidates[0];
                    spaces--;

                    filled = true;
                    break;
                }

                if (filled)
                    break;
            }

            if (!filled)
                break;
        }

        if (spaces == 0)
            return solution;

        return std::nullopt;
    }
}
