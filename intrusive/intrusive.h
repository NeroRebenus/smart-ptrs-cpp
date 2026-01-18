#pragma once

#include <cstddef>  // for std::nullptr_t
#include <utility>  // for std::exchange / std::swap

class SimpleCounter {
public:
    SimpleCounter() = default;
    ~SimpleCounter() = default;

    size_t IncRef() {
        return ++count_;
    }
    size_t DecRef() {
        return --count_;
    }
    size_t RefCount() const {
        return count_;
    }

private:
    size_t count_;
};

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter = DefaultDelete>
class RefCounted {
public:
    RefCounted() : counter_() {
    }
    ~RefCounted() {
        DecRef();
    }

    RefCounted(const RefCounted& other) : counter_(other) {
        IncRef();
    }
    RefCounted(RefCounted&& other) : counter_(other) {
        other.~RefCounted();
    }

    RefCounted& operator=(const RefCounted& other) {
        auto tmp = counter_;
        counter_ = other.counter_;
        IncRef();
        tmp.DecRef();
        return *this;
    }
    RefCounted& operator=(RefCounted&& other) noexcept {
        auto tmp = counter_;
        counter_ = other.counter_;
        IncRef();
        tmp.DecRef();
        other.~RefCounted();
        return *this;
    }

    // Increase reference counter.
    void IncRef() {
        counter_.IncRef();
    }

    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef() {
        if (counter_.DecRef() == 0) {
            Deleter().Destroy(static_cast<Derived*>(this));
        }
    }
    // Get current counter value (the number of strong references).
    size_t RefCount() const {
        return counter_.RefCount();
    }

private:
    Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
public:
    // Constructors
    IntrusivePtr() : ptr_() {
    }

    IntrusivePtr(std::nullptr_t) : ptr_(nullptr) {
    }
    IntrusivePtr(T* ptr) : ptr_(ptr) {
        if (ptr_) {
            ptr_->IncRef();
        }
    }

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) : ptr_(other.ptr_) {
        if (ptr_) {
            ptr_->IncRef();
        }
    }
    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) noexcept : ptr_(std::move(other.ptr_)) {
        other.ptr_ = nullptr;
    }
    IntrusivePtr(const IntrusivePtr& other) : ptr_(other.ptr_) {
        if (ptr_) {
            ptr_->IncRef();
        }
    }
    IntrusivePtr(IntrusivePtr&& other) noexcept : ptr_(std::move(other.ptr_)) {
        other.ptr_ = nullptr;
    }

    // `operator=`-s
    IntrusivePtr& operator=(const IntrusivePtr& other) {
        if (ptr_ != other.ptr_) {
            if (ptr_) {
                ptr_->DecRef();
            }
            ptr_ = other.ptr_;
            if (ptr_) {
                ptr_->IncRef();
            }
        }
        return *this;
    }
    IntrusivePtr& operator=(IntrusivePtr&& other) noexcept {
        if (ptr_ != other.ptr_) {
            if (ptr_) {
                ptr_->DecRef();
            }
            ptr_ = std::move(other.ptr_);
            other.ptr_ = nullptr;
        }
        return *this;
    }

    // Destructor
    ~IntrusivePtr() {
        Reset();
    }

    // Modifiers
    void Reset() {
        if (ptr_) {
            ptr_->DecRef();
        }
        ptr_ = nullptr;
    }
    void Reset(T* ptr) {
        if (ptr_) {
            ptr_->DecRef();
        }
        ptr_ = ptr;
        if (ptr_) {
            ptr_->IncRef();
        }
    }
    void Swap(IntrusivePtr& other) {
        std::swap(ptr_, other.ptr_);
    }

    // Observers
    T* Get() const {
        return ptr_;
    }
    T& operator*() const {
        return *Get();
    }
    T* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        return ptr_ ? ptr_->RefCount() : 0;
    }
    explicit operator bool() const {
        return ptr_ != nullptr;
    }

private:
    T* ptr_;
    template <typename Y>
    friend class IntrusivePtr;
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    return IntrusivePtr<T>(new T(std::forward<Args>(args)...));
}
