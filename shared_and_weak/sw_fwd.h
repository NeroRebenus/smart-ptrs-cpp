#pragma once

#include <exception>
#include <array>

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

class ControlBlock {
public:
    ControlBlock() : shared_count_(1), weak_count_(0) {
    }
    virtual ~ControlBlock() = default;

    void IncrementSharedCount() {
        ++shared_count_;
    }
    void DecrementSharedCount() {
        if (shared_count_ == 1) {
            Deleter();
        }
        --shared_count_;
        if (shared_count_ == 0 && weak_count_ == 0) {
            delete this;
        }
    }
    void IncrementWeakCount() {
        ++weak_count_;
    }
    void DecrementWeakCount() {
        if (--weak_count_ == 0 && shared_count_ == 0) {
            delete this;
        }
    }
    virtual void Deleter() = 0;

    size_t GetSharedCount() const {
        return shared_count_;
    }
    size_t GetWeakCount() const {
        return weak_count_;
    }

private:
    size_t shared_count_;
    size_t weak_count_;
};

template <typename T>
class ControlBlockObj : public ControlBlock {
public:
    ~ControlBlockObj() override = default;
    template <typename... Args>
    ControlBlockObj(Args&&... args) {
        new (&object_) T(std::forward<Args>(args)...);
    }
    void Deleter() override {
        Get()->~T();
    }
    T* Get() {
        return reinterpret_cast<T*>(&object_);
    }

private:
    alignas(T) std::array<char, sizeof(T)> object_;
};

template <typename T>
class ControlBlockPtr : public ControlBlock {
public:
    ~ControlBlockPtr() override = default;
    ControlBlockPtr(T* ptr) : ptr_(ptr) {
    }

    void Deleter() override {
        delete ptr_;
    }

    T* Get() {
        return ptr_;
    }

private:
    T* ptr_;
};