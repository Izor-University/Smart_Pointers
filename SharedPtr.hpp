#pragma once
#include <type_traits>

template <typename T>
class SharedPtr {
private:
    T* ptr;
    int* refCount;

    template <typename U> friend class SharedPtr; // Для подтипизации

public:
    SharedPtr(T* p = nullptr) : ptr(p), refCount(new int(1)) {}

    ~SharedPtr() {
        if (refCount && --(*refCount) == 0) {
            delete refCount;
            delete ptr;
        }
    }

    SharedPtr(const SharedPtr& other) : ptr(other.ptr), refCount(other.refCount) {
        if (refCount) ++(*refCount);
    }

    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other) {
            if (refCount && --(*refCount) == 0) {
                delete refCount;
                delete ptr;
            }
            ptr = other.ptr;
            refCount = other.refCount;
            if (refCount) ++(*refCount);
        }
        return *this;
    }

    // Конструктор и оператор перемещения
    SharedPtr(SharedPtr&& other) noexcept
        : ptr(other.ptr), refCount(other.refCount) {
        other.ptr = nullptr;
        other.refCount = nullptr;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (this != &other) {
            if (refCount && --(*refCount) == 0) {
                delete refCount;
                delete ptr;
            }
            ptr = other.ptr;
            refCount = other.refCount;
            other.ptr = nullptr;
            other.refCount = nullptr;
        }
        return *this;
    }

    // --- ПОДТИПИЗАЦИЯ ---
    template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    SharedPtr(const SharedPtr<U>& other) : ptr(other.ptr), refCount(other.refCount) {
        if (refCount) ++(*refCount);
    }

    template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    SharedPtr& operator=(const SharedPtr<U>& other) {
        if (ptr != other.ptr) {
            if (refCount && --(*refCount) == 0) {
                delete refCount;
                delete ptr;
            }
            ptr = other.ptr;
            refCount = other.refCount;
            if (refCount) ++(*refCount);
        }
        return *this;
    }

    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
    T* get() const { return ptr; }

    void Reset(T* p = nullptr) {
        if (ptr == p) return;

        if (refCount && --(*refCount) == 0) {
            delete ptr;
            delete refCount;
        }

        ptr = p;
        refCount = new int(1);
    }

    int UseCount() const {
        return refCount ? *refCount : 0;
    }
};

// --- СПЕЦИАЛИЗАЦИЯ ДЛЯ МАССИВОВ ---
template <typename T>
class SharedPtr<T[]> {
private:
    T* ptr;
    int* refCount;
public:
    SharedPtr(T* p = nullptr) : ptr(p), refCount(new int(1)) {}

    ~SharedPtr() {
        if (refCount && --(*refCount) == 0) {
            delete refCount;
            delete[] ptr;
        }
    }

    SharedPtr(const SharedPtr& other) : ptr(other.ptr), refCount(other.refCount) {
        if (refCount) ++(*refCount);
    }

    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other) {
            if (refCount && --(*refCount) == 0) {
                delete refCount;
                delete[] ptr;
            }
            ptr = other.ptr;
            refCount = other.refCount;
            if (refCount) ++(*refCount);
        }
        return *this;
    }

    SharedPtr(SharedPtr&& other) noexcept
        : ptr(other.ptr), refCount(other.refCount) {
        other.ptr = nullptr;
        other.refCount = nullptr;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (this != &other) {
            if (refCount && --(*refCount) == 0) {
                delete refCount;
                delete[] ptr;
            }
            ptr = other.ptr;
            refCount = other.refCount;
            other.ptr = nullptr;
            other.refCount = nullptr;
        }
        return *this;
    }

    T& operator[](int i) const {
        return ptr[i];
    }

    T* Get() const { return ptr; }

    void Reset(T* p = nullptr) {
        if (ptr == p) return;

        if (refCount && --(*refCount) == 0) {
            delete[] ptr;
            delete refCount;
        }

        ptr = p;
        refCount = new int(1);
    }

    int UseCount() const {
        return refCount ? *refCount : 0;
    }
};