#include "sudoku.h"
#include <random>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <set>
#include <map>

namespace Sudoku
{
    static const size_t NUMBERS_COUNT = BOARD_SIZE + 1; // +1 here because 0 is valid number (empty)

    std::random_device g_rd;
    std::mt19937 g_mt(g_rd());

    struct RowCol
    {
        size_t row = 0;
        size_t col = 0;

        RowCol() {}
        RowCol(size_t r, size_t c) : row(r), col(c) {}

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

        friend bool operator==(const RowCol& l, const RowCol& r) noexcept
        {
            return l.row == r.row && l.col == r.col;
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

    Candidates getCandidatesArray(const Board& board, size_t row, size_t col)
    {
        Candidates candidates;
        std::fill(std::begin(candidates), std::end(candidates), true);

        // check the row
        for (size_t i = 0; i < BOARD_SIZE; ++i)
            candidates[board[row][i]] = false;

        // check the column
        for (size_t i = 0; i < BOARD_SIZE; ++i)
            candidates[board[i][col]] = false;

        // check the grid
        size_t rowStart = (row / GRID_COUNT) * GRID_COUNT;
        size_t colStart = (col / GRID_COUNT) * GRID_COUNT;
        for (size_t r = rowStart; r < rowStart + GRID_COUNT; ++r)
        {
            for (size_t c = colStart; c < colStart + GRID_COUNT; ++c)
            {
                candidates[board[r][c]] = false;
            }
        }

        return candidates;
    }

    std::vector<uint8_t> getCandidates(const Board& board, size_t row, size_t column)
    {
        auto candidates = getCandidatesArray(board, row, column);

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

    void applyConstraints(const Board* constraints, size_t row, size_t col, Candidates& candidates)
    {
        if (!constraints || row > BOARD_SIZE || col > BOARD_SIZE)
            return;

        if (constraints->at(row)[col] == 0)
            return;

        candidates[constraints->at(row)[col]] = false;
    }

    std::tuple<RowCol, Candidates> getLeastCandidates(const Board& board)
    {
        RowCol result(BOARD_SIZE + 1, BOARD_SIZE + 1);
        size_t resultCandidatesCount = std::numeric_limits<size_t>::max();
        Candidates resultCandidates;;

        for (size_t r = 0; r < BOARD_SIZE; r++)
        {
            for (size_t c = 0; c < BOARD_SIZE; c++)
            {
                if (board[r][c] != 0)
                    continue;

                auto candidates = getCandidatesArray(board, r, c);
                size_t count = 0;
                for (size_t i = 1; i < candidates.size(); i++)
                {
                    if (candidates[i])
                        count++;
                }
                if (count < resultCandidatesCount)
                {
                    resultCandidates = candidates;
                    resultCandidatesCount = count;
                    result = { r, c };
                }
            }
        }

        return { result, resultCandidates };
    }

    bool solveRandomBoardRecursive(Board& board, RowCol rowCol, const Candidates& candidates, const Board* constraints)
    {
        // no more empty cells, solved
        auto [row, col] = rowCol;
        if (row >= BOARD_SIZE)
        {
            return true;
        }

        assert(board[row][col] == 0);

        for (uint8_t number = 1; number <= BOARD_SIZE; number++)
        {
            // here we are checking candidates, if we find cell with no candidates, there is no solution
            if (!candidates[number])
                continue;

            board[row][col] = number;

            auto [nextCell, nextCandidates] = getLeastCandidates(board);
            applyConstraints(constraints, nextCell.row, nextCell.col, nextCandidates);

            if (solveRandomBoardRecursive(board, nextCell, nextCandidates, constraints))
                return true;

            // this is important (:
            board[row][col] = 0;
        }

        return false;
    }

    std::optional<Board> solveRandomBoard(const Board& board, const Board* constraints = nullptr)
    {
        Board tmp = board;
        auto [nextCell, nextCandidates] = getLeastCandidates(board);
        applyConstraints(constraints, nextCell.row, nextCell.col, nextCandidates);

        if (solveRandomBoardRecursive(tmp, nextCell, nextCandidates, constraints))
            return tmp;

        return {};
    }

    Board prepareRandomBoard()
    {
        Board board{};

        // initialize diagonal grids which are indenpendent
        for (size_t start = 0; start < GRID_COUNT; ++start)
        {
            std::array<uint8_t, BOARD_SIZE> array;
            for (uint8_t v = 0; v < BOARD_SIZE; ++v)
                array[v] = v;
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
        return *solveRandomBoard(board);
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

    std::vector<RowCol> getEmptyCells(const Board& board)
    {
        std::vector<RowCol> result;
        for (size_t r = 0; r < BOARD_SIZE; ++r)
            for (size_t c = 0; c < BOARD_SIZE; ++c)
                if (board[r][c] == 0)
                    result.push_back({ r, c });
        return result;
    }

    void updateCandidatesAndEmptyCells(const Board& board, size_t filledIndex, std::vector<RowCol>& emptyCells, std::vector<std::vector<uint8_t>>& candidates)
    {
        for (size_t i = 0; i < emptyCells.size(); i++)
        {
            if (i == filledIndex)
                continue;

            candidates[i] = getCandidates(board, emptyCells[i].row, emptyCells[i].col);
        }

        emptyCells.erase(std::begin(emptyCells) + filledIndex);
        candidates.erase(std::begin(candidates) + filledIndex);
    }

    std::vector<size_t> getIndicesWithSingleCandidate(const std::vector<RowCol>& cells, const std::vector<std::vector<uint8_t>>& candidates)
    {
        std::vector<size_t> result;
        for (size_t i = 0; i < cells.size(); i++)
        {
            if (candidates[i].size() == 1)
                result.push_back(i);
        }
        return result;
    }

    size_t getCellIndexWithLeastCandidates(const std::vector<RowCol>& cells, const std::vector<std::vector<uint8_t>>& candidates)
    {
        size_t minCount = std::numeric_limits<size_t>::max();
        size_t result = -1;
        for (size_t i = 0; i < cells.size(); i++)
        {
            if (candidates[i].size() < minCount)
            {
                result = i;
                minCount = candidates[i].size();
            }
        }
        return result;
    }

    size_t computeDifficulty(const Board& solution, const Board& board)
    {
        std::vector<RowCol> emptyCells = getEmptyCells(board);
        std::vector<std::vector<uint8_t>> candidates;

        size_t numberOfEmptyCells = emptyCells.size();
        size_t singleCellCandidates = 0;

        for (const auto& cell : emptyCells)
            candidates.push_back(getCandidates(board, cell.row, cell.col));

        Board tmpBoard = board;
        while (emptyCells.size())
        {
            size_t cellIndex;

            auto singleCandidates = getIndicesWithSingleCandidate(emptyCells, candidates);
            singleCellCandidates += singleCandidates.size();

            if (singleCandidates.empty())
            {
                cellIndex = getCellIndexWithLeastCandidates(emptyCells, candidates);
            }
            else
            {
                cellIndex = singleCandidates[0];
            }

            auto cell = emptyCells[cellIndex];
            tmpBoard[cell.row][cell.col] = solution[cell.row][cell.col];

            updateCandidatesAndEmptyCells(tmpBoard, cellIndex, emptyCells, candidates);
        }

        return singleCellCandidates;
    }

    bool removeSpaces(Board& board, size_t spaces)
    {
        Board solution = board;

        // create vector of candidates for spaces
        auto spaceCandidates = initializeSpaceCandidates();
        Board constraints{};

        size_t currentSpaces = 0;
        while (currentSpaces < spaces)
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
            if (solveRandomBoard(board, &constraints))
            {
                // revert back
                board[row][col] = constraints[row][col];
            }
            else
            {
                currentSpaces++;
            }

            constraints[row][col] = 0;
        }

        return true;
    }

    RowCol GetRandomSpaceCell(const Board& board)
    {
        std::uniform_int_distribution<size_t> rand(0, 9 - 1);

        RowCol result(rand(g_mt), rand(g_mt));
        while (board[result.row][result.col] != 0)
            result = RowCol(rand(g_mt), rand(g_mt));

        return result;
    }

    RowCol GetRandomNumberCell(const Board& board)
    {
        std::uniform_int_distribution<size_t> rand(0, 9 - 1);

        RowCol result(rand(g_mt), rand(g_mt));
        while (board[result.row][result.col] == 0)
            result = RowCol(rand(g_mt), rand(g_mt));

        return result;
    }

    void changeSpace(Board& board, const Board& solution)
    {
        RowCol space = GetRandomSpaceCell(board);
        RowCol number = GetRandomNumberCell(board);

        board[space.row][space.col] = solution[space.row][space.col];

        Board constraints{};

        constraints[number.row][number.col] = board[number.row][number.col];
        board[number.row][number.col] = 0;

        while (solveRandomBoard(board, &constraints))
        {
            board[number.row][number.col] = constraints[number.row][number.col];
            constraints[number.row][number.col] = 0;

            number = GetRandomNumberCell(board);
            constraints[number.row][number.col] = board[number.row][number.col];
            board[number.row][number.col] = 0;
        }
    }

    std::tuple<Board, Board> generateSudoku(size_t spaces)
    {
        Board solution = prepareRandomBoard();
        Board board = solution;

        while (!removeSpaces(board, spaces))
        {
            solution = prepareRandomBoard();
            board = solution;
        }

        return { board, solution };
    }

    std::tuple<Board, Board> generateSudokuWithDifficulty(size_t spaces, size_t minDifficulty, size_t maxDifficulty)
    {
        static const uint32_t TotalNumberOfAttempts = 81;

        auto [board, solution] = generateSudoku(spaces);

        while (true)
        {
            size_t difficulty = computeDifficulty(solution, board);

            if (difficulty >= minDifficulty && difficulty < maxDifficulty)
                return { board, solution };

            uint32_t numberOfAttempts = 0;
            while (numberOfAttempts < TotalNumberOfAttempts)
            {
                changeSpace(board, solution);
                changeSpace(board, solution);

                difficulty = computeDifficulty(solution, board);

                if (difficulty >= minDifficulty && difficulty < maxDifficulty)
                    return { board, solution };

                numberOfAttempts++;
            }

            std::tie(board, solution) = generateSudoku(spaces);
        }

        return { board, solution };
    }

    size_t countSpaces(const Board& board)
    {
        size_t res = 0;

        for (size_t r = 0; r < Sudoku::BOARD_SIZE; ++r)
            for (size_t c = 0; c < Sudoku::BOARD_SIZE; ++c)
                res += board[r][c] == 0 ? 1 : 0;

        return res;
    }

    BoardCandidates getBoardCandidates(const Board& board)
    {
        BoardCandidates result;

        for (size_t r = 0; r < Sudoku::BOARD_SIZE; ++r)
        {
            for (size_t c = 0; c < Sudoku::BOARD_SIZE; ++c)
            {
                if (board[r][c] == 0)
                    result[r][c] = getCandidatesArray(board, r, c);
            }
        }

        return result;
    }

    std::optional<uint8_t> getSingleCandidate(const Candidates& candidates)
    {
        std::optional<uint8_t> result;
        for (uint8_t v = 0; v < candidates.size(); v++)
        {
            if (candidates[v])
            {
                if (result)
                    return {};
                result = v;
            }
        }
        return result;
    }

    void updateBoardCandidatesArray(BoardCandidates& boardCandidates, size_t row, size_t col, uint8_t value)
    {
        for (size_t c = 0; c < BOARD_SIZE; c++)
            boardCandidates[row][c][value] = false;

        for (size_t r = 0; r < BOARD_SIZE; r++)
            boardCandidates[r][col][value] = false;

        size_t rowStart = (row / GRID_COUNT) * GRID_COUNT;
        size_t colStart = (col / GRID_COUNT) * GRID_COUNT;
        for (size_t r = rowStart; r < rowStart + GRID_COUNT; ++r)
        {
            for (size_t c = colStart; c < colStart + GRID_COUNT; ++c)
                boardCandidates[r][c][value] = false;
        }
    }

    bool eliminateSingleCandidates(Board& board, size_t& spaces, BoardCandidates& boardCandidates)
    {
        bool somethingChanged = false;

        while (spaces != 0)
        {
            bool filled = false;

            for (size_t r = 0; r < Sudoku::BOARD_SIZE; ++r)
            {
                for (size_t c = 0; c < Sudoku::BOARD_SIZE; ++c)
                {
                    if (board[r][c] != 0)
                        continue;

                    if (auto singleCandidate = getSingleCandidate(boardCandidates[r][c]))
                    {
                        board[r][c] = *singleCandidate;
                        updateBoardCandidatesArray(boardCandidates, r, c, board[r][c]);
                        spaces--;
                        somethingChanged = true;

                        filled = true;
                        break;
                    }
                }

                if (filled)
                    break;
            }

            // if nothing was filled, there is no point to continue
            if (!filled)
                break;
        }

        return somethingChanged;
    }

    std::array<RowCol, BOARD_SIZE> Grids
    { 
        RowCol{0, 0}, RowCol{0, 3}, RowCol{0, 6},
        RowCol{3, 0}, RowCol{3, 3}, RowCol{3, 6},
        RowCol{6, 0}, RowCol{6, 3}, RowCol{6, 6}
    };

    bool eliminateCandidatesRowCol(Board& board, size_t& spaces, BoardCandidates& boardCandidates)
    {
        bool somethingChanged = false;

        auto EliminateRow = [&boardCandidates, &somethingChanged](size_t grid, size_t row, uint8_t number)
        {
            for (size_t c = Grids[grid].col; c < Grids[grid].col + GRID_COUNT; ++c)
            {
                if (boardCandidates[row][c][number])
                {
                    somethingChanged = true;
                    boardCandidates[row][c][number] = false;
                }
            }
        };

        auto EliminateCol = [&boardCandidates, &somethingChanged](size_t grid, size_t col, uint8_t number)
        {
            for (size_t r = Grids[grid].row; r < Grids[grid].row + GRID_COUNT; ++r)
            {
                if (boardCandidates[r][col][number])
                {
                    somethingChanged = true;
                    boardCandidates[r][col][number] = false;
                }
            }
        };

        // iterate over grids
        for (size_t g = 0; g < Grids.size(); g++)
        {
            // in each grid check each number
            for (uint8_t number = 1; number <= BOARD_SIZE; number++)
            {
                // rows and cols in which this number is
                std::set<size_t> rows, cols;
                for (size_t r = Grids[g].row; r < Grids[g].row + GRID_COUNT; ++r)
                {
                    for (size_t c = Grids[g].col; c < Grids[g].col + GRID_COUNT; ++c)
                    {
                        if (board[r][c] != 0)
                            continue;

                        if (boardCandidates[r][c][number])
                        {
                            rows.insert(r);
                            cols.insert(c);
                        }
                    }
                }
                // check the result
                if (rows.size() == 1)
                {
                    // iterate over grids in this row and remove candidates
                    size_t firstGrid = (g / 3) * GRID_COUNT;
                    for (size_t i = firstGrid; i < firstGrid + GRID_COUNT; i++)
                    {
                        if (i != g)
                            EliminateRow(i, *rows.begin(), number);
                    }
                }

                if (cols.size() == 1)
                {
                    // iterate over grids in this column and remove candidates
                    size_t firstGrid = g % 3;
                    for (size_t i = firstGrid; i < BOARD_SIZE; i = i + 3)
                    {
                        if (i != g)
                            EliminateCol(i, *cols.begin(), number);
                    }
                }
            }
        }

        return somethingChanged;
    }

    bool solveSudoku(const Board& board, bool allowRowColElimination)
    {
        size_t spaces = countSpaces(board);
        Board workingBoard = board;
        BoardCandidates boardCandidates = getBoardCandidates(board);

        bool somethingChanged = false;
        while (spaces != 0)
        {
            somethingChanged |= eliminateSingleCandidates(workingBoard, spaces, boardCandidates);

            if (spaces == 0)
                break;

            if (allowRowColElimination)
                somethingChanged |= eliminateCandidatesRowCol(workingBoard, spaces, boardCandidates);

            if (!somethingChanged)
                break;
            somethingChanged = false;
        }

        return spaces == 0;
    }
}
