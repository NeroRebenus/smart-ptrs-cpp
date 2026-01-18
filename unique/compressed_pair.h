#pragma once

// Paste here your implementation of compressed_pair from seminar 2 to use in UniquePtr

#include <type_traits>
#include <utility>

template <typename T, bool index, bool = std::is_empty_v<T> && !std::is_final_v<T>>
class Compress {
public:
    Compress() : value_(T{}) {
    }
    ~Compress() = default;

    Compress(T& value) : value_(value) {
    }
    Compress(const T& value) : value_(value) {
    }
    Compress(T&& value) : value_(std::forward<T>(value)) {
    }
    Compress& operator=(const Compress& other) {
        if (this != &other) {
            Compress temp(other);
            std::swap(value_, temp.value_);
        }
        return *this;
    }
    Compress& operator=(const Compress&& other) noexcept {
        if (this != &other) {
            Compress temp(std::forward<T>(other));
            std::swap(value_, temp.value_);
        }
        return *this;
    }
    T& Get() {
        return value_;
    }
    const T& Get() const {
        return value_;
    }

private:
    T value_;
};

template <typename T, bool index>
class Compress<T, index, true> : T {
public:
    Compress() = default;
    ~Compress() = default;

    Compress(T&) {
    }
    Compress(T&&) {
    }

    T& Get() {
        return *this;
    }
    const T& Get() const {
        return *this;
    }
};

// Me think, why waste time write lot code, when few code do trick.
template <typename F, typename S>
class CompressedPair : Compress<F, 0>, Compress<S, 1> {
public:
    CompressedPair() = default;
    ~CompressedPair() = default;

    CompressedPair(F& first, S& second) : Compress<F, 0>(first), Compress<S, 1>(second) {
    }

    template <typename A, typename B>
    CompressedPair(A&& first, B&& second)
        : Compress<F, 0>(std::forward<A>(first)), Compress<S, 1>(std::forward<B>(second)) {
    }

    F& GetFirst() {
        return this->Compress<F, 0>::Get();
    }
    const F& GetFirst() const {
        return this->Compress<F, 0>::Get();
    }

    S& GetSecond() {
        return this->Compress<S, 1>::Get();
    }
    const S& GetSecond() const {
        return this->Compress<S, 1>::Get();
    }
};