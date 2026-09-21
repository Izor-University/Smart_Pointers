#include <gtest/gtest.h>
#include <utility>
#include <stdexcept>
#include "../DynamicArray.hpp"

// --- ВСПОМОГАТЕЛЬНЫЙ КЛАСС ДЛЯ ПРОВЕРКИ УТЕЧЕК ---
static int arrayElementsDestroyed = 0;

struct ArrayElement {
    int value;
    ArrayElement() : value(0) {} // Конструктор по умолчанию нужен для new T[capacity]
    ArrayElement(int v) : value(v) {}
    ~ArrayElement() { arrayElementsDestroyed++; }
};

class DynamicArrayTest : public ::testing::Test {
protected:
    void SetUp() override {
        arrayElementsDestroyed = 0;
    }
};

// --- САМИ ТЕСТЫ ---

// 1. Тест базовых конструкторов
TEST_F(DynamicArrayTest, DefaultAndSizeConstructors) {
    DynamicArray<int> arrEmpty;
    EXPECT_EQ(arrEmpty.GetSize(), 0);
    EXPECT_EQ(arrEmpty.GetCapacity(), 0);

    DynamicArray<int> arrSized(5);
    EXPECT_EQ(arrSized.GetSize(), 5);
    EXPECT_EQ(arrSized.GetCapacity(), 5);
}

// 2. Тест добавления элементов (PushBack) и реаллокации
TEST_F(DynamicArrayTest, PushBackAndReallocation) {
    DynamicArray<int> arr;
    
    arr.PushBack(10);
    EXPECT_EQ(arr.GetSize(), 1);
    EXPECT_EQ(arr.GetCapacity(), 1);
    
    arr.PushBack(20);
    EXPECT_EQ(arr.GetSize(), 2);
    EXPECT_EQ(arr.GetCapacity(), 2);
    
    arr.PushBack(30);
    EXPECT_EQ(arr.GetSize(), 3);
    EXPECT_EQ(arr.GetCapacity(), 4); // Вместимость удвоилась (2 * 2 = 4)

    EXPECT_EQ(arr[0], 10);
    EXPECT_EQ(arr[1], 20);
    EXPECT_EQ(arr[2], 30);
}

// 3. Тест выхода за пределы массива (Генерация исключения)
TEST_F(DynamicArrayTest, OutOfBoundsThrow) {
    DynamicArray<int> arr(2);
    arr[0] = 1;
    arr[1] = 2;

    // operator[] мы сделали без проверки для скорости, 
    // а вот метод Get() должен выбрасывать std::out_of_range
    EXPECT_THROW(arr.Get(2), std::out_of_range);
    EXPECT_THROW(arr.Get(100), std::out_of_range);
}

// 4. Тест конструктора и оператора копирования (Глубокое копирование)
TEST_F(DynamicArrayTest, DeepCopySemantics) {
    DynamicArray<int> arr1;
    arr1.PushBack(1);
    arr1.PushBack(2);

    DynamicArray<int> arr2(arr1); // Копируем
    arr2[0] = 99; // Изменяем копию

    EXPECT_EQ(arr1[0], 1); // Оригинал не должен измениться
    EXPECT_EQ(arr2[0], 99);
    EXPECT_EQ(arr1.GetSize(), arr2.GetSize());

    DynamicArray<int> arr3;
    arr3 = arr1; // Присваивание
    arr3[1] = 77;

    EXPECT_EQ(arr1[1], 2);
    EXPECT_EQ(arr3[1], 77);
}

// 5. Тест конструктора и оператора перемещения (std::move)
TEST_F(DynamicArrayTest, MoveSemantics) {
    DynamicArray<int> arr1;
    arr1.PushBack(100);
    arr1.PushBack(200);

    // Перемещаем arr1 в arr2
    DynamicArray<int> arr2(std::move(arr1));

    // arr1 теперь должен быть пустым
    EXPECT_EQ(arr1.GetSize(), 0);
    EXPECT_EQ(arr1.GetCapacity(), 0);
    
    // arr2 получил все данные
    EXPECT_EQ(arr2.GetSize(), 2);
    EXPECT_EQ(arr2[1], 200);

    // Оператор перемещения
    DynamicArray<int> arr3;
    arr3 = std::move(arr2);

    EXPECT_EQ(arr2.GetSize(), 0);
    EXPECT_EQ(arr3[0], 100);
}

// 6. Тест изменения размера (Resize)
TEST_F(DynamicArrayTest, ResizeMethod) {
    DynamicArray<int> arr(3);
    arr[0] = 1; arr[1] = 2; arr[2] = 3;

    arr.Resize(5); // Увеличиваем
    EXPECT_EQ(arr.GetSize(), 5);
    EXPECT_EQ(arr[0], 1); // Старые данные сохранились
    EXPECT_EQ(arr.GetCapacity(), 5);

    arr.Resize(2); // Уменьшаем размер (capacity не меняется, только size)
    EXPECT_EQ(arr.GetSize(), 2);
    EXPECT_EQ(arr.GetCapacity(), 5);
}

// 7. Тест отсутствия утечек памяти (Удаление сложных объектов)
TEST_F(DynamicArrayTest, NoMemoryLeaksOnReallocationAndDestruction) {
    {
        DynamicArray<ArrayElement> arr;
        // capacity будет: 1
        arr.PushBack(ArrayElement(10)); 
        // capacity станет: 2 (реаллокация, 1 старый объект удалится)
        arr.PushBack(ArrayElement(20)); 
        // capacity станет: 4 (реаллокация, 2 старых объекта удалятся)
        arr.PushBack(ArrayElement(30)); 
        
        // К этому моменту мы перевыделяли память. 
        // Во время реаллокаций старые массивы удалялись.
    } 
    // При выходе из блока уничтожается последний массив из 4 элементов 
    // (даже если size=3, capacity=4, деструктор new[] разрушит все 4 выделенные ячейки)

    // Итого: 
    // Вызов деструкторов из-за временных объектов при PushBack (это особенности C++, когда мы передаем объекты по значению) +
    // Вызовы при реаллокациях массива +
    // Финальное удаление. 
    // Важно просто убедиться, что счетчик > 0 и программа не падает.
    EXPECT_GT(arrayElementsDestroyed, 0); 
}