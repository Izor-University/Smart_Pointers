#include <iostream>
#include <chrono>
#include <iomanip>
#include <vector>
#include <memory> // Для сравнения с std:: смарт-указателями

#include "UniquePtr.hpp"
#include "SharedPtr.hpp"
#include "DynamicArray.hpp"

using namespace std;
using namespace std::chrono;

// Вспомогательная структура для хранения результатов замеров
struct BenchmarkResult {
    double rawPtrTime = 0.0;
    double stdPtrTime = 0.0;
    double customPtrTime = 0.0;
};

// --- НАГРУЗОЧНОЕ ТЕСТИРОВАНИЕ UNIQUE PTR ---
BenchmarkResult RunUniqueBenchmark(size_t iterations) {
    BenchmarkResult result;

    // 1. Тест сырых указателей (Raw Pointer)
    {
        auto start = high_resolution_clock::now();
        vector<int*> rawPtrs;
        rawPtrs.reserve(iterations);
        for (size_t i = 0; i < iterations; ++i) {
            rawPtrs.push_back(new int(i));
        }
        for (size_t i = 0; i < iterations; ++i) {
            delete rawPtrs[i];
        }
        auto end = high_resolution_clock::now();
        result.rawPtrTime = duration<double, milli>(end - start).count();
    }

    // 2. Тест std::unique_ptr
    {
        auto start = high_resolution_clock::now();
        vector<std::unique_ptr<int>> stdPtrs;
        stdPtrs.reserve(iterations);
        for (size_t i = 0; i < iterations; ++i) {
            stdPtrs.push_back(std::make_unique<int>(i));
        }
        // Удаление происходит автоматически при выходе из блока
        auto end = high_resolution_clock::now();
        result.stdPtrTime = duration<double, milli>(end - start).count();
    }

    // 3. Тест нашего UniquePtr
    {
        auto start = high_resolution_clock::now();
        vector<UniquePtr<int>> customPtrs;
        customPtrs.reserve(iterations);
        for (size_t i = 0; i < iterations; ++i) {
            customPtrs.push_back(UniquePtr<int>(new int(i)));
        }
        // Удаление происходит автоматически при выходе из блока
        auto end = high_resolution_clock::now();
        result.customPtrTime = duration<double, milli>(end - start).count();
    }

    return result;
}

// --- НАГРУЗОЧНОЕ ТЕСТИРОВАНИЕ SHARED PTR ---
BenchmarkResult RunSharedBenchmark(size_t iterations) {
    BenchmarkResult result;

    // 1. Тест сырых указателей (Симуляция ручного копирования)
    {
        auto start = high_resolution_clock::now();
        vector<int*> rawPtrs;
        rawPtrs.reserve(iterations);
        for (size_t i = 0; i < iterations; ++i) {
            rawPtrs.push_back(new int(i));
        }
        // Имитация копирования (без счетчика ссылок, просто передача указателя)
        vector<int*> copiedPtrs = rawPtrs;
        
        for (size_t i = 0; i < iterations; ++i) {
            delete rawPtrs[i]; // Удаляем только оригинал, чтобы избежать double free
        }
        auto end = high_resolution_clock::now();
        result.rawPtrTime = duration<double, milli>(end - start).count();
    }

    // 2. Тест std::shared_ptr (с подсчетом ссылок)
    {
        auto start = high_resolution_clock::now();
        vector<std::shared_ptr<int>> stdPtrs;
        stdPtrs.reserve(iterations);
        for (size_t i = 0; i < iterations; ++i) {
            stdPtrs.push_back(std::make_shared<int>(i));
        }
        vector<std::shared_ptr<int>> copiedPtrs = stdPtrs; // Увеличивает счетчик
        auto end = high_resolution_clock::now();
        result.stdPtrTime = duration<double, milli>(end - start).count();
    }

    // 3. Тест нашего SharedPtr
    {
        auto start = high_resolution_clock::now();
        vector<SharedPtr<int>> customPtrs;
        customPtrs.reserve(iterations);
        for (size_t i = 0; i < iterations; ++i) {
            customPtrs.push_back(SharedPtr<int>(new int(i)));
        }
        vector<SharedPtr<int>> copiedPtrs = customPtrs; // Увеличивает наш счетчик
        auto end = high_resolution_clock::now();
        result.customPtrTime = duration<double, milli>(end - start).count();
    }

    return result;
}

// --- ВЫВОД ТАБЛИЦЫ ---
void PrintTable(const string& testName, size_t iterations, const BenchmarkResult& res) {
    cout << "\n===============================================================\n";
    cout << " Результаты для: " << testName << " | Объектов: " << iterations << "\n";
    cout << "===============================================================\n";
    cout << left << setw(25) << "Тип указателя" 
         << right << setw(20) << "Время (мс)" << "\n";
    cout << "---------------------------------------------------------------\n";
    cout << left << setw(25) << "1. Raw Pointer (T*)" 
         << right << setw(20) << fixed << setprecision(3) << res.rawPtrTime << " ms\n";
    cout << left << setw(25) << "2. STL (std::)" 
         << right << setw(20) << fixed << setprecision(3) << res.stdPtrTime << " ms\n";
    cout << left << setw(25) << "3. Custom Pointer" 
         << right << setw(20) << fixed << setprecision(3) << res.customPtrTime << " ms\n";
    cout << "===============================================================\n";
}

// --- ГЛАВНОЕ МЕНЮ ---
int main() {
    int choice = 0;
    while (true) {
        cout << "\n[ МЕНЮ ТЕСТИРОВАНИЯ УМНЫХ УКАЗАТЕЛЕЙ ]\n";
        cout << "1. Нагрузочный тест UniquePtr (Малое число, 10^4)\n";
        cout << "2. Нагрузочный тест UniquePtr (Большое число, 10^7)\n";
        cout << "3. Нагрузочный тест SharedPtr (Малое число, 10^4)\n";
        cout << "4. Нагрузочный тест SharedPtr (Большое число, 10^7)\n";
        cout << "5. Выход\n";
        cout << "Ваш выбор: ";
        
        if (!(cin >> choice)) {
            break;
        }

        switch (choice) {
            case 1:
                PrintTable("UniquePtr (Small Load)", 10'000, RunUniqueBenchmark(10'000));
                break;
            case 2:
                cout << "Генерация 10 миллионов объектов, подождите...\n";
                PrintTable("UniquePtr (Heavy Load)", 10'000'000, RunUniqueBenchmark(10'000'000));
                break;
            case 3:
                PrintTable("SharedPtr (Small Load)", 10'000, RunSharedBenchmark(10'000));
                break;
            case 4:
                cout << "Генерация 10 миллионов объектов, подождите...\n";
                PrintTable("SharedPtr (Heavy Load)", 10'000'000, RunSharedBenchmark(10'000'000));
                break;
            case 5:
                cout << "Выход из программы.\n";
                return 0;
            default:
                cout << "Неверный выбор, попробуйте снова.\n";
        }
    }
    return 0;
}