#pragma once

class AnyObj {
public:
    constexpr AnyObj() = default;
    template<class T>
    constexpr AnyObj(T* p) : ptr_(p) {}
    constexpr AnyObj(std::nullptr_t) : ptr_(nullptr) {}
    template<class T>
    constexpr T& As() { return *reinterpret_cast<T*>(ptr_); }
private:
    void* ptr_{};
};
