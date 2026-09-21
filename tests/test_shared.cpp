#include <gtest/gtest.h>
#include <utility>
#include "../SharedPtr.hpp"

// --- ВСПОМОГАТЕЛЬНЫЕ КЛАССЫ ---
static int sharedDestroyedCount = 0;

struct SharedBaseObj {
    int value;
    SharedBaseObj(int v = 0) : value(v) {}
    virtual ~SharedBaseObj() { sharedDestroyedCount++; }
};

struct SharedDerivedObj : public SharedBaseObj {
    SharedDerivedObj(int v = 0) : SharedBaseObj(v) {}
    ~SharedDerivedObj() override {}
};

class SharedPtrTest : public ::testing::Test {
protected:
    void SetUp() override {
        sharedDestroyedCount = 0;
    }
};

// --- САМИ ТЕСТЫ ---

// 1. Базовая инициализация и счетчик ссылок
TEST_F(SharedPtrTest, InitializationAndUseCount) {
    SharedPtr<int> ptr(new int(42));
    EXPECT_NE(ptr.get(), nullptr);
    EXPECT_EQ(*ptr, 42);
    EXPECT_EQ(ptr.UseCount(), 1); // Сначала владеет 1 указатель
}

// 2. Тест конструктора копирования (увеличение счетчика)
TEST_F(SharedPtrTest, CopyConstructor) {
    SharedPtr<int> ptr1(new int(10));
    EXPECT_EQ(ptr1.UseCount(), 1);
    
    {
        SharedPtr<int> ptr2 = ptr1; // Копируем
        EXPECT_EQ(ptr1.UseCount(), 2);
        EXPECT_EQ(ptr2.UseCount(), 2);
        EXPECT_EQ(*ptr2, 10);
    } // ptr2 выходит из области видимости и уничтожается
    
    EXPECT_EQ(ptr1.UseCount(), 1); // Счетчик должен уменьшиться
}

// 3. Тест оператора присваивания
TEST_F(SharedPtrTest, CopyAssignment) {
    SharedPtr<SharedBaseObj> ptr1(new SharedBaseObj(1));
    
    {
        SharedPtr<SharedBaseObj> ptr2(new SharedBaseObj(2));
        ptr1 = ptr2; // ptr1 отпускает свой объект и берет объект ptr2
        
        EXPECT_EQ(sharedDestroyedCount, 1); // Старый объект ptr1 должен удалиться
        EXPECT_EQ(ptr1.UseCount(), 2);
        EXPECT_EQ(ptr2.UseCount(), 2);
        EXPECT_EQ(ptr1->value, 2);
    }
    
    EXPECT_EQ(ptr1.UseCount(), 1);
    EXPECT_EQ(sharedDestroyedCount, 1); // Второй объект пока жив, им владеет ptr1
}

// 4. Тест семантики перемещения
TEST_F(SharedPtrTest, MoveSemantics) {
    SharedPtr<int> ptr1(new int(99));
    SharedPtr<int> ptr2(std::move(ptr1));
    
    EXPECT_EQ(ptr1.get(), nullptr); 
    EXPECT_EQ(ptr1.UseCount(), 0);
    
    EXPECT_NE(ptr2.get(), nullptr);
    EXPECT_EQ(ptr2.UseCount(), 1); // Счетчик не должен был увеличиться
    EXPECT_EQ(*ptr2, 99);
}

// 5. Тест метода Reset
TEST_F(SharedPtrTest, ResetMethod) {
    SharedPtr<SharedBaseObj> ptr1(new SharedBaseObj(10));
    SharedPtr<SharedBaseObj> ptr2 = ptr1;
    
    ptr1.Reset(new SharedBaseObj(20)); // ptr1 переключается на новый объект
    
    EXPECT_EQ(sharedDestroyedCount, 0); // Старый объект НЕ должен удалиться, т.к. им владеет ptr2
    EXPECT_EQ(ptr1.UseCount(), 1);
    EXPECT_EQ(ptr2.UseCount(), 1);
    
    ptr2.Reset(); // А вот теперь удалится первый объект
    EXPECT_EQ(sharedDestroyedCount, 1); 
}

// 6. Тест специализации для массивов
TEST_F(SharedPtrTest, ArraySpecialization) {
    {
        SharedPtr<SharedBaseObj[]> arrPtr(new SharedBaseObj[3]);
        arrPtr[0].value = 100;
        
        {
            SharedPtr<SharedBaseObj[]> arrCopy = arrPtr;
            EXPECT_EQ(arrCopy.UseCount(), 2);
            EXPECT_EQ(arrCopy[0].value, 100);
        } // arrCopy умирает, но счетчик становится 1, объекты живут
        
        EXPECT_EQ(sharedDestroyedCount, 0);
    } // arrPtr умирает, счетчик 0, вызывается delete[]
    
    EXPECT_EQ(sharedDestroyedCount, 3); // Уничтожено 3 элемента массива
}

// 7. Тест ПОДТИПИЗАЦИИ (Копирование Base <- Derived)
TEST_F(SharedPtrTest, SubtypingCopy) {
    SharedPtr<SharedDerivedObj> derivedPtr(new SharedDerivedObj(55));
    SharedPtr<SharedBaseObj> basePtr(derivedPtr); // Копируем в базу!
    
    EXPECT_EQ(basePtr.UseCount(), 2);
    EXPECT_EQ(derivedPtr.UseCount(), 2);
    EXPECT_EQ(basePtr->value, 55);
}

// 8. Тест ПОДТИПИЗАЦИИ (Присваивание Base <- Derived)
TEST_F(SharedPtrTest, SubtypingAssignment) {
    SharedPtr<SharedBaseObj> basePtr(new SharedBaseObj(10));
    SharedPtr<SharedDerivedObj> derivedPtr(new SharedDerivedObj(20));
    
    basePtr = derivedPtr; // Присваиваем базу наследнику
    
    EXPECT_EQ(sharedDestroyedCount, 1); // Старый BaseObj(10) удалился
    EXPECT_EQ(basePtr.UseCount(), 2);   // Теперь они оба смотрят на DerivedObj(20)
    EXPECT_EQ(basePtr->value, 20);
}