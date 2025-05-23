#pragma once
#include <cstdint>
#include <type_traits>
#include "SafeCallback.hpp"

namespace dsp {

inline static constexpr int32_t ClampUncheck(int32_t value, int32_t min, int32_t max) {
    return value < min ? min : value > max ? max : value;
}

inline static constexpr float ClampUncheck(float value, float min, float max) {
    return value < min ? min : value > max ? max : value;
}

static constexpr float LerpUncheck(float a, float b, float t) {
    return a * (1 - t) + b * t;
}

enum class ParamType { kInt, kFloat, kBool, kEnum };
struct IParamDesc {
    virtual ~IParamDesc() = default;
    virtual void Reset() = 0;
    virtual void Add(int32_t dvalue, bool alt) = 0;
    virtual ParamType GetParamType() const = 0;

    template<class T>
    T& As() {
        return *static_cast<T*>(this);
    }
};

struct IntParamDesc : public IParamDesc {
    const char* const name;
    const int32_t min;
    const int32_t max;
    const int32_t defaultValue;
    int32_t value{};
    const int32_t altMul;
    const ThreadSafeCallback::Proxy callback_;

    IntParamDesc(ThreadSafeCallback& callback, const char* name, int32_t min, int32_t max, int32_t defaultValue, int32_t altMul)
        : name(name)
        , min(min)
        , max(max)
        , defaultValue(defaultValue)
        , value(defaultValue)
        , altMul(altMul)
        , callback_(callback.NewProxy()) {}

    void Add(int32_t dvalue, bool alt) override {
        if (alt) {
            auto nvalue = ClampUncheck(value + dvalue * altMul, min, max);
            SetValue(nvalue);
        }
        else {
            auto nvalue = ClampUncheck(value + dvalue, min, max);
            SetValue(nvalue);
        }
    }

    void Reset() override {
        SetValue(defaultValue);
    }

    int32_t Get() const {
        return value;
    }

    void SetValue(int32_t newValue) {
        if (newValue != value) {
            callback_.MarkDirty();
        }
        value = newValue;
    }

    float GetValueAsNormalized() const {
        return static_cast<float>(value - min) / static_cast<float>(max - min);
    }

    void SetCallback(ThreadSafeCallback::CallbackFunc func) {
        callback_.SetCallback(func);
    }

    ParamType GetParamType() const { return ParamType::kInt; }
};

struct FloatParamDesc : public IParamDesc {
    static constexpr int32_t kScale = 100000;
    const char* const name;
    const int32_t min;
    const int32_t max;
    const int32_t step;
    const int32_t defaultValue;
    int32_t value{};
    const int32_t altMul;
    const ThreadSafeCallback::Proxy callback_;
    float modulationValue; // -1~1

    constexpr float GetMax() const {
        return max / static_cast<float>(kScale);
    }

    constexpr float GetMin() const {
        return min / static_cast<float>(kScale);
    }

    FloatParamDesc(ThreadSafeCallback& callback, const char* name, float min, float max, float step, float defaultValue, int32_t altMul)
        : name(name)
        , min(min * kScale)
        , max(max * kScale)
        , step(step * kScale)
        , defaultValue(defaultValue * kScale)
        , value(defaultValue * kScale)
        , altMul(altMul)
        , callback_(callback.NewProxy()) {}

    void Add(int32_t dvalue, bool alt) override {
        if (alt) {
            auto nvalue = ClampUncheck(value + dvalue * step * altMul, min, max);
            SetValue(nvalue);
        }
        else {
            auto nvalue = ClampUncheck(value + dvalue * step, min, max);
            SetValue(nvalue);
        }
    }

    float Get() const {
        return value / static_cast<float>(kScale); 
    }

    float GetWithModulation() const {
        auto map0 = (max - min) * modulationValue + value;
        auto clamp0 = map0 < min ? min : map0 > max ? max : map0;
        return clamp0 / static_cast<float>(kScale);
    }

    float GetFloatValue(float offset) const {
        auto map0 = (max - min) * offset + value;
        auto clamp0 = map0 < min ? min : map0 > max ? max : map0;
        return clamp0 / static_cast<float>(kScale);
    }

    void Reset() override {
        SetValue(defaultValue);
    }

    float GetValueAsNormalized() const {
        return static_cast<float>(value - min) / static_cast<float>(max - min);
    }

    void SetValue(int32_t newValue) {
        if (newValue != value) {
            callback_.MarkDirty();
        }
        value = newValue;
    }

    void SetCallback(ThreadSafeCallback::CallbackFunc func) {
        callback_.SetCallback(func);
    }

    void SetFloat(float newValue) {
        auto v = LerpUncheck(static_cast<float>(min), static_cast<float>(max), newValue);
        auto iv = static_cast<int32_t>(v);
        SetValue(iv);
    }

    ParamType GetParamType() const { return ParamType::kFloat; }
};

struct BoolParamDesc : public IParamDesc {
    const char* const name;
    bool value{};
    const bool defaultValue{};
    const ThreadSafeCallback::Proxy callback_;

    BoolParamDesc(ThreadSafeCallback& callback, const char* name, bool defaultValue) 
        : name(name)
        , value(defaultValue)
        , defaultValue(defaultValue)
        , callback_(callback.NewProxy()) {}

    void Add(int32_t dvalue, bool /*alt*/) override {
        if (dvalue > 0) {
            SetValue(true);
        }
        else {
            SetValue(false);
        }
    }

    bool Get() const {
        return value; 
    }

    void Reset() override {
        SetValue(defaultValue);
    }

    void SetValue(bool newValue) {
        if (newValue != value) {
            callback_.MarkDirty();
        }
        value = newValue;
    }

    void SetCallback(ThreadSafeCallback::CallbackFunc func) {
        callback_.SetCallback(func);
    }

    ParamType GetParamType() const { return ParamType::kBool; }
};

template<class E> requires std::is_enum_v<E>
struct EnumNumTrait {
    static constexpr int32_t GetEnumNumber() {
        static_assert(requires { E::kCount; });
        return static_cast<int32_t>(E::kCount) - 1;
    }

    static constexpr int32_t kValue = GetEnumNumber();
};

template<class T> requires std::is_enum_v<T>
struct EnumParamDesc : public IParamDesc {
    const char* const name;
    const int32_t min = 0;
    const int32_t max = EnumNumTrait<T>::kValue;
    int32_t value{};
    const int32_t defaultValue{};
    const ThreadSafeCallback::Proxy callback_;

    EnumParamDesc(ThreadSafeCallback& callback, const char* name, T defaultValue) 
        : name(name)
        , value(static_cast<int32_t>(defaultValue))
        , defaultValue(static_cast<int32_t>(defaultValue))
        , callback_(callback.NewProxy()) {}

    void Add(int32_t dvalue, bool alt) override {
        auto nvalue = ClampUncheck(value + dvalue, min, max);
        SetValue(nvalue);
    }

    T Get() const {
        return static_cast<T>(value);
    }

    int32_t GetInt() const {
        return value;
    }

    void Reset() override {
        SetValue(defaultValue);
    }

    template<std::size_t N>
    const char* GetName(const char* const (&names)[N]) const {
        static_assert(N == EnumNumTrait<T>::kValue + 1);
        return names[value];
    }

    void SetValue(int32_t newValue) {
        if (newValue != value) {
            callback_.MarkDirty();
        }
        value = newValue;
    }

    void SetCallback(ThreadSafeCallback::CallbackFunc func) {
        callback_.SetCallback(func);
    }

    ParamType GetParamType() const { return ParamType::kEnum; }
};

}