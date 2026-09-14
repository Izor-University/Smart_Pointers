#pragma once
#include "UniquePtr.hpp"
#include <stdexcept>

template <typename T>
class DynamicArray {
private:
    UniquePtr<T[]> data;
    size_t size;
    size_t capacity;

    void reallocate(size_t new_capacity) {
        UniquePtr<T[]> new_data(new T[new_capacity]);
        for (size_t i = 0; i < size; ++i) {
            new_data[i] = std::move(data[i]);
        }
        data = std::move(new_data);
        capacity = new_capacity;
    }

public:
    DynamicArray() : data(nullptr), size(0), capacity(0) {}

    DynamicArray(size_t count) : size(count), capacity(count) {
        if (count > 0) {
            data.reset(new T[count]());
        }
    }

    DynamicArray(T* items, size_t count) : size(count), capacity(count) {
        if (count > 0) {
            data.reset(new T[count]);
            for (size_t i = 0; i < count; ++i) {
                data[i] = items[i];
            }
        }
    }

    DynamicArray(const DynamicArray& other) : size(other.size), capacity(other.capacity) {
        if (capacity > 0) {
            data.reset(new T[capacity]);
            for (size_t i = 0; i < size; ++i) {
                data[i] = other.data[i];
            }
        }
    }

    DynamicArray& operator=(const DynamicArray& other) {
        if (this != &other) {
            size = other.size;
            capacity = other.capacity;
            if (capacity > 0) {
                UniquePtr<T[]> new_data(new T[capacity]);
                for (size_t i = 0; i < size; ++i) {
                    new_data[i] = other.data[i];
                }
                data = std::move(new_data); // Освободит старую память автоматически
            } else {
                data.reset();
            }
        }
        return *this;
    }

    DynamicArray(DynamicArray&& other) noexcept 
        : data(std::move(other.data)), size(other.size), capacity(other.capacity) {
        other.size = 0;
        other.capacity = 0;
    }

    DynamicArray& operator=(DynamicArray&& other) noexcept {
        if (this != &other) {
            data = std::move(other.data);
            size = other.size;
            capacity = other.capacity;
            
            other.size = 0;
            other.capacity = 0;
        }
        return *this;
    }

    size_t GetSize() const { return size; }
    size_t GetCapacity() const { return capacity; }

    T& Get(size_t index) {
        if (index >= size) throw std::out_of_range("Index out of range");
        return data[index];
    }

    const T& Get(size_t index) const {
        if (index >= size) throw std::out_of_range("Index out of range");
        return data[index];
    }

    T& operator[](size_t index) { return Get(index); }
    const T& operator[](size_t index) const { return Get(index); }

    void Resize(size_t new_size) {
        if (new_size > capacity) {
            reallocate(new_size);
        }
        size = new_size;
    }

    void PushBack(const T& value) {
        if (size == capacity) {
            reallocate(capacity == 0 ? 1 : capacity * 2);
        }
        data[size] = value;
        size++;
    }
};