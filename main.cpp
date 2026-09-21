#include <iostream>
#include <chrono>
#include <iomanip>
#include <memory>
#include <cstdlib>
#include <new>
#include <string> // Добавлено для std::string

#include "UniquePtr.hpp"
#include "SharedPtr.hpp"
#include "DynamicArray.hpp"

// ============================================================================
// GLOBAL MEMORY TRACKER
// ============================================================================
static std::size_t g_memory_allocated = 0;

void* operator new(std::size_t size) {
    g_memory_allocated += size;
    void* ptr = std::malloc(size);
    if (!ptr) throw std::bad_alloc();
    return ptr;
}

void operator delete(void* ptr) noexcept {
    std::free(ptr);
}

void operator delete(void* ptr, std::size_t /*size*/) noexcept {
    std::free(ptr);
}

// ============================================================================
// BENCHMARK STRUCTURES
// ============================================================================
struct BenchmarkResult {
    double timeMs = 0.0;
    std::size_t memoryBytes = 0;
};

struct TestSuiteResult {
    BenchmarkResult rawPtr;
    BenchmarkResult stdPtr;
    BenchmarkResult customPtr;
};

// ============================================================================
// UNIQUE POINTER BENCHMARKS
// ============================================================================
TestSuiteResult RunUniqueBenchmark(std::size_t iterations) {
    TestSuiteResult results;

    // 1. Raw Pointer Test
    {
        // Allocate storage array FIRST, so it doesn't affect our memory tracking
        int** arr = new int*[iterations];

        std::size_t memBefore = g_memory_allocated;
        auto start = std::chrono::high_resolution_clock::now();

        for (std::size_t i = 0; i < iterations; ++i) {
            arr[i] = new int(i);
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::size_t memAfter = g_memory_allocated;

        results.rawPtr.timeMs = std::chrono::duration<double, std::milli>(end - start).count();
        results.rawPtr.memoryBytes = memAfter - memBefore;

        // Cleanup
        for (std::size_t i = 0; i < iterations; ++i) {
            delete arr[i];
        }
        delete[] arr;
    }

    // 2. std::unique_ptr Test
    {
        std::unique_ptr<int>* arr = new std::unique_ptr<int>[iterations];

        std::size_t memBefore = g_memory_allocated;
        auto start = std::chrono::high_resolution_clock::now();

        for (std::size_t i = 0; i < iterations; ++i) {
            arr[i] = std::make_unique<int>(i);
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::size_t memAfter = g_memory_allocated;

        results.stdPtr.timeMs = std::chrono::duration<double, std::milli>(end - start).count();
        results.stdPtr.memoryBytes = memAfter - memBefore;

        delete[] arr;
    }

    // 3. Custom UniquePtr Test
    {
        UniquePtr<int>* arr = new UniquePtr<int>[iterations];

        std::size_t memBefore = g_memory_allocated;
        auto start = std::chrono::high_resolution_clock::now();

        for (std::size_t i = 0; i < iterations; ++i) {
            arr[i] = UniquePtr<int>(new int(i));
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::size_t memAfter = g_memory_allocated;

        results.customPtr.timeMs = std::chrono::duration<double, std::milli>(end - start).count();
        results.customPtr.memoryBytes = memAfter - memBefore;

        delete[] arr;
    }

    return results;
}

// ============================================================================
// SHARED POINTER BENCHMARKS
// ============================================================================
TestSuiteResult RunSharedBenchmark(std::size_t iterations) {
    TestSuiteResult results;

    // 1. Raw Pointer Test (Simulating shared behavior without ref counting)
    {
        int** arr = new int*[iterations];
        int** copies = new int*[iterations];

        std::size_t memBefore = g_memory_allocated;
        auto start = std::chrono::high_resolution_clock::now();

        for (std::size_t i = 0; i < iterations; ++i) {
            arr[i] = new int(i);
        }
        for (std::size_t i = 0; i < iterations; ++i) {
            copies[i] = arr[i]; // Manual copy, zero overhead
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::size_t memAfter = g_memory_allocated;

        results.rawPtr.timeMs = std::chrono::duration<double, std::milli>(end - start).count();
        results.rawPtr.memoryBytes = memAfter - memBefore;

        for (std::size_t i = 0; i < iterations; ++i) {
            delete arr[i];
        }
        delete[] arr;
        delete[] copies;
    }

    // 2. std::shared_ptr Test
    {
        std::shared_ptr<int>* arr = new std::shared_ptr<int>[iterations];
        std::shared_ptr<int>* copies = new std::shared_ptr<int>[iterations];

        std::size_t memBefore = g_memory_allocated;
        auto start = std::chrono::high_resolution_clock::now();

        for (std::size_t i = 0; i < iterations; ++i) {
            arr[i] = std::make_shared<int>(i);
        }
        for (std::size_t i = 0; i < iterations; ++i) {
            copies[i] = arr[i]; // Triggers atomic ref count increment
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::size_t memAfter = g_memory_allocated;

        results.stdPtr.timeMs = std::chrono::duration<double, std::milli>(end - start).count();
        results.stdPtr.memoryBytes = memAfter - memBefore;

        delete[] arr;
        delete[] copies;
    }

    // 3. Custom SharedPtr Test
    {
        SharedPtr<int>* arr = new SharedPtr<int>[iterations];
        SharedPtr<int>* copies = new SharedPtr<int>[iterations];

        std::size_t memBefore = g_memory_allocated;
        auto start = std::chrono::high_resolution_clock::now();

        for (std::size_t i = 0; i < iterations; ++i) {
            arr[i] = SharedPtr<int>(new int(i));
        }
        for (std::size_t i = 0; i < iterations; ++i) {
            copies[i] = arr[i]; // Triggers our custom ref count increment
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::size_t memAfter = g_memory_allocated;

        results.customPtr.timeMs = std::chrono::duration<double, std::milli>(end - start).count();
        results.customPtr.memoryBytes = memAfter - memBefore;

        delete[] arr;
        delete[] copies;
    }

    return results;
}

// ============================================================================
// CLI INTERFACE & TABLE RENDERING
// ============================================================================
void PrintResults(const std::string& testName, std::size_t iterations, const TestSuiteResult& res) {
    auto toMB = [](std::size_t bytes) { return static_cast<double>(bytes) / (1024.0 * 1024.0); };

    std::cout << "\n===============================================================================\n";
    std::cout << " TEST: " << testName << " | ALLOCATIONS: " << iterations << "\n";
    std::cout << "===============================================================================\n";
    std::cout << std::left << std::setw(25) << "Pointer Type"
              << std::right << std::setw(20) << "Time Executed"
              << std::right << std::setw(30) << "Heap Memory Allocated" << "\n";
    std::cout << "-------------------------------------------------------------------------------\n";

    std::cout << std::left << std::setw(25) << "1. Raw Pointer (T*)"
              << std::right << std::setw(17) << std::fixed << std::setprecision(2) << res.rawPtr.timeMs << " ms"
              << std::right << std::setw(27) << std::fixed << std::setprecision(2) << toMB(res.rawPtr.memoryBytes) << " MB\n";

    std::cout << std::left << std::setw(25) << "2. std:: STL Pointer"
              << std::right << std::setw(17) << std::fixed << std::setprecision(2) << res.stdPtr.timeMs << " ms"
              << std::right << std::setw(27) << std::fixed << std::setprecision(2) << toMB(res.stdPtr.memoryBytes) << " MB\n";

    std::cout << std::left << std::setw(25) << "3. Custom Ptr"
              << std::right << std::setw(17) << std::fixed << std::setprecision(2) << res.customPtr.timeMs << " ms"
              << std::right << std::setw(27) << std::fixed << std::setprecision(2) << toMB(res.customPtr.memoryBytes) << " MB\n";

    std::cout << "===============================================================================\n";
}

int main() {
    int choice = 0;
    while (true) {
        std::cout << "\n[ SMART POINTERS PERFORMANCE BENCHMARK ]\n";
        std::cout << "1. UniquePtr - Medium Load  (100,000 objects)\n";
        std::cout << "2. UniquePtr - Heavy Load   (1,000,000 objects)\n";
        std::cout << "3. UniquePtr - Extreme Load (10,000,000 objects)\n";
        std::cout << "4. SharedPtr - Medium Load  (100,000 objects)\n";
        std::cout << "5. SharedPtr - Heavy Load   (1,000,000 objects)\n";
        std::cout << "6. SharedPtr - Extreme Load (10,000,000 objects)\n";
        std::cout << "7. Exit\n";
        std::cout << "> Select an option: ";

        if (!(std::cin >> choice)) break;

        switch (choice) {
            case 1:
                PrintResults("UniquePtr (Medium)", 100'000, RunUniqueBenchmark(100'000));
                break;
            case 2:
                std::cout << "Processing 1 million allocations...\n";
                PrintResults("UniquePtr (Heavy)", 1'000'000, RunUniqueBenchmark(1'000'000));
                break;
            case 3:
                std::cout << "Processing 10 million allocations, please wait...\n";
                PrintResults("UniquePtr (Extreme)", 10'000'000, RunUniqueBenchmark(10'000'000));
                break;
            case 4:
                PrintResults("SharedPtr (Medium)", 100'000, RunSharedBenchmark(100'000));
                break;
            case 5:
                std::cout << "Processing 1 million allocations...\n";
                PrintResults("SharedPtr (Heavy)", 1'000'000, RunSharedBenchmark(1'000'000));
                break;
            case 6:
                std::cout << "Processing 10 million allocations, please wait...\n";
                PrintResults("SharedPtr (Extreme)", 10'000'000, RunSharedBenchmark(10'000'000));
                break;
            case 7:
                std::cout << "Exiting benchmark tool.\n";
                return 0;
            default:
                std::cout << "Invalid selection. Try again.\n";
        }
    }
    return 0;
}