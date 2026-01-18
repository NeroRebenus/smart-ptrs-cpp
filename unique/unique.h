#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t

template <typename T>
struct DefaultDeleter {
    DefaultDeleter() = default;
    ~DefaultDeleter() = default;

    template <typename S>
    DefaultDeleter(DefaultDeleter<S>&&){};

    void operator()(T* ptr) {
        delete ptr;
    }
};

// Primary template
template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    explicit UniquePtr(T* ptr = nullptr) : ptr_(ptr, Deleter()) {
    }
    UniquePtr(T* ptr, const Deleter& deleter) : ptr_(ptr, deleter) {
    }
    UniquePtr(T* ptr, Deleter&& deleter) noexcept : ptr_(ptr, std::move(deleter)) {
    }
    template <typename F, typename S>
    UniquePtr(UniquePtr<F, S>&& other) noexcept {
        ptr_.GetSecond() = std::move(other.GetDeleter());
        ptr_.GetFirst() = std::move(other.Release());
    }
    UniquePtr(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    template <typename F, typename S>
    UniquePtr& operator=(UniquePtr<F, S>&& other) noexcept {
        ptr_.GetSecond() = (std::move(other.GetDeleter()));
        Reset(other.Release());
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        return *this;
    }
    UniquePtr& operator=(const UniquePtr&) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor
    ~UniquePtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers
    T* Release() {
        auto tmp = ptr_.GetFirst();
        ptr_.GetFirst() = nullptr;
        return tmp;
    }
    void Reset(T* ptr = nullptr) {
        if (Get() != ptr) {
            GetDeleter()(Release());
        }
        if (ptr != nullptr) {
            ptr_.GetFirst() = ptr;
        }
    }
    void Swap(UniquePtr& other) {
        std::swap(ptr_.GetFirst(), other.ptr_.GetFirst());
        std::swap(GetDeleter(), other.GetDeleter());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers
    T* Get() const {
        return ptr_.GetFirst();
    }
    Deleter& GetDeleter() {
        return ptr_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return ptr_.GetSecond();
    }
    explicit operator bool() const {
        return Get() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators
    T operator*() const {
        return *ptr_.GetFirst();
    }
    T* operator->() const {
        return ptr_.GetFirst();
    }

private:
    CompressedPair<T*, Deleter> ptr_;
};

// Specialization for arrays
template <typename T>
struct DefaultDeleter<T[]> {
    DefaultDeleter() = default;
    ~DefaultDeleter() = default;

    template <typename S>
    DefaultDeleter(DefaultDeleter<S>&&){};

    void operator()(T* ptr) {
        delete[] ptr;
    }
};

template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    explicit UniquePtr(T* ptr = nullptr) : ptr_(ptr, Deleter()) {
    }
    UniquePtr(T* ptr, const Deleter& deleter) : ptr_(ptr, deleter) {
    }
    UniquePtr(T* ptr, Deleter&& deleter) : ptr_(ptr, std::move(deleter)) {
    }

    template <typename F>
    UniquePtr(UniquePtr<F>&& other) noexcept {
        if (ptr_.GetFirst() != other.Get()) {
            ptr_.GetSecond() = std::move(other.GetDeleter());
            ptr_.GetFirst() = std::move(other.Release());
        }
    }
    UniquePtr(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    template <typename F>
    UniquePtr& operator=(UniquePtr<F>&& other) noexcept {
        ptr_.GetSecond() = (std::move(other.GetDeleter()));
        Reset(other.Release());
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        return *this;
    }
    UniquePtr& operator=(const UniquePtr&) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor
    ~UniquePtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers
    T* Release() {
        auto tmp = Get();
        ptr_.GetFirst() = nullptr;
        return tmp;
    }
    void Reset(T* ptr = nullptr) {
        if (Get() != ptr) {
            GetDeleter()(Release());
        }
        if (ptr != nullptr) {
            ptr_.GetFirst() = ptr;
        }
    }
    void Swap(UniquePtr& other) {
        std::swap(ptr_.GetFirst(), other.ptr_.GetFirst());
        std::swap(ptr_.GetSecond(), other.ptr_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers
    T* Get() const {
        return ptr_.GetFirst();
    }
    Deleter& GetDeleter() {
        return ptr_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return ptr_.GetSecond();
    }
    explicit operator bool() const {
        return Get() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators
    T& operator[](size_t pos) const {
        return ptr_.GetFirst()[pos];
    }
    T* operator->() const {
        return ptr_.GetFirst();
    }

private:
    CompressedPair<T*, Deleter> ptr_;
};
