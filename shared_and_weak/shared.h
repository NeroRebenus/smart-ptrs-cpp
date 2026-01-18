#pragma once

#include "sw_fwd.h"  // Forward declaration
#include <memory>
#include <cstddef>  // std::nullptr_t

class EnableSharedFromThisBase {};

template <typename T>
class EnableSharedFromThis : public EnableSharedFromThisBase {
public:
    EnableSharedFromThis() = default;
    SharedPtr<T> SharedFromThis() {
        return weak_this_.Lock();
    }

    SharedPtr<const T> SharedFromThis() const {
        return weak_this_.Lock();
    }

    WeakPtr<T> WeakFromThis() noexcept {
        return weak_this_;
    }

    WeakPtr<const T> WeakFromThis() const noexcept {
        return weak_this_;
    }

private:
    WeakPtr<T> weak_this_;

    template <typename Y>
    friend class SharedPtr;
};

template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    SharedPtr() : control_block_(), ptr_() {
    }
    SharedPtr(std::nullptr_t) : control_block_(nullptr), ptr_(nullptr) {
    }

    template <typename Y>
    SharedPtr(Y* ptr) : control_block_(new ControlBlockPtr<Y>(ptr)), ptr_(ptr) {
        if constexpr (std::is_convertible_v<Y*, EnableSharedFromThisBase*>) {
            if (control_block_) {
                InitWeakThis(ptr);
            }
        }
    }
    SharedPtr(ControlBlockObj<T>* control_block)
        : control_block_(control_block), ptr_(control_block->Get()) {
        if constexpr (std::is_convertible_v<T*, EnableSharedFromThisBase*>) {
            if (control_block_) {
                InitWeakThis(ptr_);
            }
        }
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other) : control_block_(other.control_block_), ptr_(other.ptr_) {
        IncreaseCount();
    }
    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other) noexcept
        : control_block_(std::move(other.control_block_)), ptr_(std::move(other.ptr_)) {
        other.control_block_ = nullptr;
        other.ptr_ = nullptr;
    }
    SharedPtr(const SharedPtr& other) : control_block_(other.control_block_), ptr_(other.ptr_) {
        IncreaseCount();
    }

    SharedPtr(SharedPtr&& other) noexcept
        : control_block_(std::move(other.control_block_)), ptr_(std::move(other.ptr_)) {
        other.control_block_ = nullptr;
        other.ptr_ = nullptr;
    }
    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) : control_block_(other.control_block_), ptr_(ptr) {
        IncreaseCount();
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.Expired()) {
            throw BadWeakPtr();
        }
        control_block_ = other.control_block_;
        ptr_ = other.ptr_;
        IncreaseCount();
    }
    template <typename Y>
    void InitWeakThis(EnableSharedFromThis<Y>* e) {
        e->weak_this_ = WeakPtr<Y>(*this);
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        DecreaseCount();
        control_block_ = other.control_block_;
        ptr_ = other.ptr_;
        IncreaseCount();
        return *this;
    }
    SharedPtr& operator=(SharedPtr&& other) noexcept {
        DecreaseCount();
        control_block_ = std::move(other.control_block_);
        ptr_ = std::move(other.ptr_);
        other.control_block_ = nullptr;
        other.ptr_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers
    void IncreaseCount() {
        if (control_block_) {
            control_block_->IncrementSharedCount();
        }
    }
    void DecreaseCount() {
        if (control_block_) {
            control_block_->DecrementSharedCount();
        }
    }
    void Reset() {
        DecreaseCount();
        control_block_ = nullptr;
        ptr_ = nullptr;
    }
    template <typename Y>
    void Reset(Y* ptr) {
        DecreaseCount();
        control_block_ = new ControlBlockPtr<Y>(ptr);
        ptr_ = ptr;
    }
    void Swap(SharedPtr& other) {
        std::swap(control_block_, other.control_block_);
        std::swap(ptr_, other.ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers
    T* Get() const {
        return ptr_;
    }
    T& operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        return control_block_ ? control_block_->GetSharedCount() : 0;
    }
    explicit operator bool() const {
        return ptr_ != nullptr;
    }

private:
    ControlBlock* control_block_;
    T* ptr_;

    template <typename Y>
    friend class SharedPtr;

    template <typename Y>
    friend class WeakPtr;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    return SharedPtr(new ControlBlockObj<T>(std::forward<Args>(args)...));
}
