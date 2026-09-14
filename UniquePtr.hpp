#pragma once
#include <type_traits> // Для std::is_convertible_v и std::enable_if_t

template <typename T>
class UniquePtr {
private:
    T* ptr;

    // Делаем все специализации UniquePtr друзьями, чтобы иметь доступ к ptr при подтипизации
    template <typename U> friend class UniquePtr;

public:
    UniquePtr(T* ptr = nullptr) : ptr(ptr) {}
    ~UniquePtr() { delete ptr; }

    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    UniquePtr(UniquePtr&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    // --- ПОДТИПИЗАЦИЯ ---
    // Разрешаем перемещение из UniquePtr<U>, только если U* приводится к T* (т.е. U - наследник T)
    template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    UniquePtr(UniquePtr<U>&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }

    template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    UniquePtr& operator=(UniquePtr<U>&& other) noexcept {
        if (ptr != other.ptr) {
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
    T* Get() const { return ptr; }

    T* release() {
        T* tmp = ptr;
        ptr = nullptr;
        return tmp;
    }

    void reset(T* p = nullptr) {
        if (ptr != p) {
            delete ptr;
            ptr = p;
        }
    }
};

// --- СПЕЦИАЛИЗАЦИЯ ДЛЯ МАССИВОВ ---
template <typename T>
class UniquePtr<T[]> {
private:
    T* ptr;
public:
    UniquePtr(T* ptr = nullptr) : ptr(ptr) {}
    ~UniquePtr() { delete[] ptr; }

    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    UniquePtr(UniquePtr&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            delete[] ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    T& operator[](int index) const {
        return ptr[index];
    }

    T* Get() const { return ptr; }

    T* release() {
        T* tmp = ptr;
        ptr = nullptr;
        return tmp;
    }

    void reset(T* p = nullptr) {
        if (ptr != p) {
            delete[] ptr;
            ptr = p;
        }
    }
};