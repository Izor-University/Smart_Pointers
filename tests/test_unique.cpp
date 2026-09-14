#include <gtest/gtest.h>
#include <utility>
#include "../UniquePtr.hpp"

// --- ВСПОМОГАТЕЛЬНЫЕ КЛАССЫ ДЛЯ ТЕСТИРОВАНИЯ ---

// Глобальная переменная для подсчета вызовов деструкторов
static int destroyedCount = 0;

// Базовый класс для проверки утечек и подтипизации
struct BaseObj {
    int value;
    BaseObj(int v = 0) : value(v) {}
    virtual ~BaseObj() { destroyedCount++; }
};

// Класс-наследник для проверки подтипизации
struct DerivedObj : public BaseObj {
    DerivedObj(int v = 0) : BaseObj(v) {}
    ~DerivedObj() override { /* Вызовется деструктор базы, который увеличит счетчик */ }
};


// Фикстура (Fixture) для сброса счетчика перед каждым тестом
class UniquePtrTest : public ::testing::Test {
protected:
    void SetUp() override {
        destroyedCount = 0;
    }
};

// --- САМИ ТЕСТЫ ---

// 1. Тест создания пустого указателя
TEST_F(UniquePtrTest, DefaultConstructor) {
    UniquePtr<int> ptr;
    EXPECT_EQ(ptr.Get(), nullptr);
}

// 2. Тест создания из сырого указателя и разыменования
TEST_F(UniquePtrTest, ValueConstructorAndDereference) {
    UniquePtr<int> ptr(new int(42));
    EXPECT_NE(ptr.Get(), nullptr);
    EXPECT_EQ(*ptr, 42);
    
    *ptr = 100;
    EXPECT_EQ(*ptr, 100);
}

// 3. Тест оператора ->
TEST_F(UniquePtrTest, ArrowOperator) {
    UniquePtr<BaseObj> ptr(new BaseObj(15));
    EXPECT_EQ(ptr->value, 15);
}

// 4. Тест конструктора перемещения
TEST_F(UniquePtrTest, MoveConstructor) {
    UniquePtr<int> ptr1(new int(10));
    UniquePtr<int> ptr2(std::move(ptr1));
    
    EXPECT_EQ(ptr1.Get(), nullptr); // ptr1 должен обнулиться
    EXPECT_NE(ptr2.Get(), nullptr);
    EXPECT_EQ(*ptr2, 10);
}

// 5. Тест оператора перемещения и защиты от самоприсваивания
TEST_F(UniquePtrTest, MoveAssignment) {
    UniquePtr<int> ptr1(new int(10));
    UniquePtr<int> ptr2(new int(20));
    
    ptr2 = std::move(ptr1); // Перемещаем ptr1 в ptr2
    
    EXPECT_EQ(ptr1.Get(), nullptr);
    EXPECT_NE(ptr2.Get(), nullptr);
    EXPECT_EQ(*ptr2, 10); // ptr2 теперь владеет "10"
    
    // Самоприсваивание
    int* rawAddr = ptr2.Get();
    ptr2 = std::move(ptr2); 
    EXPECT_EQ(ptr2.Get(), rawAddr); // Ничего не должно сломаться
}

// 6. Тест метода release (отдача владения)
TEST_F(UniquePtrTest, ReleaseMethod) {
    UniquePtr<int> ptr(new int(99));
    int* rawPtr = ptr.release();
    
    EXPECT_EQ(ptr.Get(), nullptr);
    EXPECT_EQ(*rawPtr, 99);
    
    delete rawPtr; // Удаляем руками, так как мы забрали владение
}

// 7. Тест метода reset
TEST_F(UniquePtrTest, ResetMethod) {
    {
        UniquePtr<BaseObj> ptr(new BaseObj(1));
        ptr.reset(new BaseObj(2)); // Старый объект должен удалиться
        EXPECT_EQ(destroyedCount, 1);
        EXPECT_EQ(ptr->value, 2);
        ptr.reset(); // Удаляем второй объект (передаем nullptr)
        EXPECT_EQ(destroyedCount, 2);
        EXPECT_EQ(ptr.Get(), nullptr);
    }
}

// 8. Тест специализации для массивов T[]
TEST_F(UniquePtrTest, ArraySpecialization) {
    {
        // Создаем массив из 3 элементов
        UniquePtr<BaseObj[]> arrPtr(new BaseObj[3]);
        
        arrPtr[0].value = 10;
        arrPtr[1].value = 20;
        arrPtr[2].value = 30;
        
        EXPECT_EQ(arrPtr[1].value, 20);
    } 
    // При выходе из блока должен вызваться delete[] для 3 элементов
    EXPECT_EQ(destroyedCount, 3);
}

// 9. Тест ПОДТИПИЗАЦИИ (Конструктор перемещения Base <- Derived)
TEST_F(UniquePtrTest, SubtypingMoveConstructor) {
    UniquePtr<DerivedObj> derivedPtr(new DerivedObj(55));
    UniquePtr<BaseObj> basePtr(std::move(derivedPtr));
    
    EXPECT_EQ(derivedPtr.Get(), nullptr);
    EXPECT_NE(basePtr.Get(), nullptr);
    EXPECT_EQ(basePtr->value, 55);
}

// 10. Тест ПОДТИПИЗАЦИИ (Оператор присваивания Base <- Derived)
TEST_F(UniquePtrTest, SubtypingMoveAssignment) {
    UniquePtr<BaseObj> basePtr(new BaseObj(10));
    UniquePtr<DerivedObj> derivedPtr(new DerivedObj(20));
    
    basePtr = std::move(derivedPtr);
    
    EXPECT_EQ(derivedPtr.Get(), nullptr); // Отдал владение
    EXPECT_EQ(destroyedCount, 1);         // Старый объект BaseObj(10) был удален
    EXPECT_EQ(basePtr->value, 20);        // Теперь хранит DerivedObj
}