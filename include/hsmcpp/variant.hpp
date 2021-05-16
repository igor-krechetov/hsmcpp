// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef __HSMCPP_VARIANT_HPP__
#define __HSMCPP_VARIANT_HPP__

#include <string>
#include <utility>
#include <map>

class Variant;
typedef std::map<std::string, Variant> VariantDict_t;
typedef std::pair<Variant, Variant> VariantPair_t;

#define DEF_OPERATOR_ASSIGN(_val_type, _internal_type)              \
    Variant& operator=(const _val_type& v) {                        \
        assign<_val_type>(v, _internal_type);                       \
        return *this;                                               \
    }

class Variant
{
public:
    enum class Type
    {
        UNKNOWN,

        BYTE_1,
        BYTE_2,
        BYTE_4,
        BYTE_8,

        UBYTE_1,
        UBYTE_2,
        UBYTE_4,
        UBYTE_8,

        DOUBLE,
        BOOL,

        STRING,         // std::string

        DICTIONARY,     // VariantDict_t
        PAIR,           // VariantPair_t
    };

    enum class SwitchState
    {
        DEFAULT,
        OFF,
        ON,
        INVALID
    };

public:
    static Variant make(const int8_t v);
    static Variant make(const int16_t v);
    static Variant make(const int32_t v);
    static Variant make(const int64_t v);
    static Variant make(const uint8_t v);
    static Variant make(const uint16_t v);
    static Variant make(const uint32_t v);
    static Variant make(const uint64_t v);
    static Variant make(const double v);
    static Variant make(const bool v);
    static Variant make(const std::string& v);
    static Variant make(const char* v);
    static Variant make(const VariantDict_t& v);
    static Variant make(const Variant& first, const Variant& second);
    static Variant make(const Variant& v);

public:
    Variant() = default;
    Variant(const Variant& v);
    Variant(Variant&& v);
    ~Variant();

    Variant& operator=(const Variant& v);
    Variant& operator=(Variant&& v);

    DEF_OPERATOR_ASSIGN(int8_t, Type::BYTE_1)
    DEF_OPERATOR_ASSIGN(int16_t, Type::BYTE_2)
    DEF_OPERATOR_ASSIGN(int32_t, Type::BYTE_4)
    DEF_OPERATOR_ASSIGN(int64_t, Type::BYTE_8)
    DEF_OPERATOR_ASSIGN(uint8_t, Type::UBYTE_1)
    DEF_OPERATOR_ASSIGN(uint16_t, Type::UBYTE_2)
    DEF_OPERATOR_ASSIGN(uint32_t, Type::UBYTE_4)
    DEF_OPERATOR_ASSIGN(uint64_t, Type::UBYTE_8)
    DEF_OPERATOR_ASSIGN(double, Type::DOUBLE)
    DEF_OPERATOR_ASSIGN(bool, Type::BOOL)
    DEF_OPERATOR_ASSIGN(std::string, Type::STRING)
    DEF_OPERATOR_ASSIGN(VariantDict_t, Type::DICTIONARY)
    DEF_OPERATOR_ASSIGN(VariantPair_t, Type::PAIR)

    inline Type getType() const
    {
        return type;
    }

    bool isString() const;
    bool isNumeric() const;
    bool isSignedNumeric() const;
    bool isUnsignedNumeric() const;
    bool isBool() const;

    template <typename T>
    inline T* value() const
    {
        return (static_cast<T*>(data));
    }

    std::string toString() const;
    int64_t toInt64() const;
    uint64_t toUInt64() const;
    double toDouble() const;
    bool toBool() const;

    operator bool() const;

    bool operator!=(const Variant& val) const;
    bool operator>(const Variant& val) const;
    bool operator>=(const Variant& val) const;
    bool operator<(const Variant& val) const;
    bool operator<=(const Variant& val) const;
    bool operator==(const Variant& val) const;

private:
    Variant(void* d, const Type t);

    bool isSameObject(const Variant& val) const;

    template <typename T>
    void assign(const T& v, const Type t)
    {
        free();
        data = new T(v);
        type = t;
    }

    void free();

private:
    void* data = nullptr;
    Type type = Type::UNKNOWN;
};

template struct std::pair<Variant, Variant>;

#endif  // __HSMCPP_VARIANT_HPP__
