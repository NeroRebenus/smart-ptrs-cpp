#pragma once

#include "sw_fwd.h"  // Forward declaration

template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    WeakPtr() : control_block_(), ptr_() {
    }

    template <typename Y>
    WeakPtr(const WeakPtr<Y>& other) : control_block_(other.control_block_), ptr_(other.ptr_) {
        IncreaseCount();
    }
    WeakPtr(const WeakPtr& other) : control_block_(other.control_block_), ptr_(other.ptr_) {
        IncreaseCount();
    }
    template <typename Y>
    WeakPtr(WeakPtr<Y>&& other) noexcept
        : control_block_(std::move(other.control_block_)), ptr_(std::move(other.ptr_)) {
        other.control_block_ = nullptr;
        other.ptr_ = nullptr;
    }
    WeakPtr(WeakPtr&& other) noexcept
        : control_block_(std::move(other.control_block_)), ptr_(std::move(other.ptr_)) {
        other.control_block_ = nullptr;
        other.ptr_ = nullptr;
    }
    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) : control_block_(other.control_block_), ptr_(other.ptr_) {
        IncreaseCount();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    WeakPtr& operator=(const WeakPtr& other) {
        DecreaseCount();
        control_block_ = other.control_block_;
        ptr_ = other.ptr_;
        IncreaseCount();
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) noexcept {
        DecreaseCount();
        control_block_ = std::move(other.control_block_);
        ptr_ = std::move(other.ptr_);
        other.control_block_ = nullptr;
        other.ptr_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers
    void IncreaseCount() {
        if (control_block_) {
            control_block_->IncrementWeakCount();
        }
    }
    void DecreaseCount() {
        if (control_block_) {
            control_block_->DecrementWeakCount();
        }
    }
    void Reset() {
        DecreaseCount();
        control_block_ = nullptr;
    }
    void Swap(WeakPtr& other) {
        std::swap(control_block_, other.control_block_);
        std::swap(ptr_, other.ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        return control_block_ ? control_block_->GetSharedCount() : 0;
    }
    bool Expired() const {
        return !UseCount();
    }
    SharedPtr<T> Lock() const {
        if (control_block_ && UseCount() != 0) {
            return SharedPtr<T>(*this);
        }
        return SharedPtr<T>();
    }

private:
    ControlBlock* control_block_;
    T* ptr_;

    template <typename Y>
    friend class WeakPtr;

    template <typename Y>
    friend class SharedPtr;
};
