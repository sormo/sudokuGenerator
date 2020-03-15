#include "sudoku.h"
#include <iostream>
#include <benchmark/benchmark.h>

#pragma comment(lib, "benchmark.lib")
#pragma comment(lib, "benchmark_main.lib")

// Alternatively, can add libraries using linker options.
#ifdef _WIN32
#pragma comment ( lib, "Shlwapi.lib" )
#ifdef _DEBUG
#pragma comment ( lib, "benchmarkd.lib" )
#else
#pragma comment ( lib, "benchmark.lib" )
#endif
#endif

static void CustomArguments(benchmark::internal::Benchmark* b)
{
    for (int i = 1; i <= 5; ++i)
        for (int j = 30; j <= 50; ++j)
            b->Args({ i, j });
}

static void BM_GenerateSudoku(benchmark::State& state)
{
    while (state.KeepRunning())
    {
        Sudoku::generateSudoku(state.range(0), state.range(1));
    }
    //state.SetLabel("Solutions(" + std::to_string(solutions) + ") Spaces(" + std::to_string(state.range(0)) + ")");
}
BENCHMARK(BM_GenerateSudoku)->Apply(CustomArguments)->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();
