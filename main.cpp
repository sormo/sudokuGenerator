#include "sudoku.h"
#include <iostream>
//#include <benchmark/benchmark.h>
//
//// Alternatively, can add libraries using linker options.
//#ifdef _WIN32
//#pragma comment(lib, "Shlwapi.lib")
//#ifdef _DEBUG
//#pragma comment(lib, "benchmarkd.lib")
//#pragma comment(lib, "benchmark_maind.lib")
//#else
//#pragma comment(lib, "benchmark.lib")
//#pragma comment(lib, "benchmark_main.lib")
//#endif
//#endif

//static void BM_GenerateSudoku(benchmark::State& state)
//{
//    while(state.KeepRunning())
//    {
//        auto [board, solution] = Sudoku::generateSudoku(55);
//
//        // TODO single candidate method
//        while(!Sudoku::solveSudoku(board, true))
//            std::tie(board, solution) = Sudoku::generateSudoku(55);
//    }
//    //state.SetLabel("Solutions(" + std::to_string(solutions) + ") Spaces(" + std::to_string(state.range(0)) + ")");
//}
//BENCHMARK(BM_GenerateSudoku)->Unit(benchmark::kMillisecond);
//
//BENCHMARK_MAIN();

int main()
{
    auto [board, solution] = Sudoku::generateSudokuWithDifficulty(55, 90, 100);

    while(!Sudoku::solveSudoku(board, true))
        std::tie(board, solution) = Sudoku::generateSudokuWithDifficulty(55, 90, 100);

    size_t difficulty = Sudoku::computeDifficulty(solution, board);

    std::cout << "Difficulty: " << difficulty << "\n";
    Sudoku::printBoard(board);
    Sudoku::printBoard(solution);
}
