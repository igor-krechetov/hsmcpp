// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef __HSMCPP_VARIANT_HPP__
#define __HSMCPP_VARIANT_HPP__

#include <map>
#include <list>
#include <string>
#include <vector>
#include <utility>

namespace hsmcpp
{

class Variant;
typedef std::vector<Variant> VariantVector_t;
typedef std::list<Variant> VariantList_t;
typedef std::map<Variant, Variant> VariantDict_t;
typedef std::pair<Variant, Variant> VariantPair_t;

#define DEF_CONSTRUCTOR(_val_type, _internal_type)                  \
    explicit Variant(const _val_type& v) {                          \
        assign<_val_type>(v, _internal_type);                       \
    }

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
        BYTEARRAY,      // std::vector<char>

        LIST,
        VECTOR,

        DICTIONARY,     // VariantDict_t
        PAIR,           // VariantPair_t
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

    static Variant make(const std::vector<char>& v);
    static Variant make(const char* binaryData, const size_t bytesCount);

    static Variant make(const VariantVector_t& v);
    template <typename T>
    static Variant make(const std::vector<T>& v);

    static Variant make(const VariantList_t& v);
    template <typename T>
    static Variant make(const std::list<T>& v);

    static Variant make(const VariantDict_t& v);
    template <typename K, typename V>
    static Variant make(const std::map<K, V>& v);

    static Variant make(const Variant& first, const Variant& second);
    static Variant make(const Variant& v);

public:
    Variant() = default;
    Variant(const Variant& v);// copy constructor
    Variant(Variant&& v);// move constructor
    ~Variant();

    DEF_CONSTRUCTOR(int8_t, Type::BYTE_1)
    DEF_CONSTRUCTOR(int16_t, Type::BYTE_2)
    DEF_CONSTRUCTOR(int32_t, Type::BYTE_4)
    DEF_CONSTRUCTOR(int64_t, Type::BYTE_8)
    DEF_CONSTRUCTOR(uint8_t, Type::UBYTE_1)
    DEF_CONSTRUCTOR(uint16_t, Type::UBYTE_2)
    DEF_CONSTRUCTOR(uint32_t, Type::UBYTE_4)
    DEF_CONSTRUCTOR(uint64_t, Type::UBYTE_8)
    DEF_CONSTRUCTOR(double, Type::DOUBLE)
    DEF_CONSTRUCTOR(bool, Type::BOOL)
    DEF_CONSTRUCTOR(std::string, Type::STRING)
    explicit Variant(const char* v);

    DEF_CONSTRUCTOR(std::vector<char>, Type::BYTEARRAY)
    explicit Variant(const char* binaryData, const size_t bytesCount);

    DEF_CONSTRUCTOR(VariantVector_t, Type::VECTOR)
    DEF_CONSTRUCTOR(VariantList_t, Type::LIST)
    DEF_CONSTRUCTOR(VariantDict_t, Type::DICTIONARY)
    DEF_CONSTRUCTOR(VariantPair_t, Type::PAIR)

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
    DEF_OPERATOR_ASSIGN(std::vector<char>, Type::BYTEARRAY)
    DEF_OPERATOR_ASSIGN(std::vector<Variant>, Type::VECTOR)
    DEF_OPERATOR_ASSIGN(std::list<Variant>, Type::LIST)
    DEF_OPERATOR_ASSIGN(VariantDict_t, Type::DICTIONARY)
    DEF_OPERATOR_ASSIGN(VariantPair_t, Type::PAIR)

    inline Type getType() const
    {
        return type;
    }

    bool isString() const;
    bool isByteArray() const;
    bool isVector() const;
    bool isList() const;
    bool isDictionary() const;
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
    std::vector<char> toByteArray() const;
    int64_t toInt64() const;
    uint64_t toUInt64() const;
    double toDouble() const;
    bool toBool() const;

    template <typename T>
    std::vector<T> toVector() const;

    template <typename T>
    std::list<T> toList() const;

    template <typename K, typename V>
    std::map<K, V> toMap() const;

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

template <typename T>
Variant Variant::make(const std::vector<T>& v)
{
    VariantVector_t* dest = new VariantVector_t();

    for (auto it = v.begin() ; it != v.end(); ++it)
    {
        dest->push_back(make(*it));
    }

    return Variant(dest, Type::VECTOR);
}

template <typename T>
Variant Variant::make(const std::list<T>& v)
{
    VariantList_t* dest = new VariantList_t();

    for (auto it = v.begin() ; it != v.end(); ++it)
    {
        dest->push_back(make(*it));
    }

    return Variant(dest, Type::LIST);
}

template <typename K, typename V>
Variant Variant::make(const std::map<K, V>& v)
{
    VariantDict_t* dict = new VariantDict_t();

    for (auto it = v.begin() ; it != v.end(); ++it)
    {
        dict->emplace(make(it->first), make(it->second));
    }

    return Variant(dict, Type::DICTIONARY);
}

template <typename T>
std::vector<T> Variant::toVector() const
{
    std::vector<T> result;

    if (true == isVector())
    {
        VariantVector_t* data = value<VariantVector_t>();

        if (nullptr != data)
        {
            for (auto it = data->begin(); it != data->end(); ++it)
            {
                result.push_back(*(it->value<T>()));
            }
        }
    }

    return result;
}

template <typename T>
std::list<T> Variant::toList() const
{
    std::list<T> result;

    if (true == isList())
    {
        VariantList_t* data = value<VariantList_t>();

        if (nullptr != data)
        {
            for (auto it = data->begin(); it != data->end(); ++it)
            {
                result.push_back(*(it->value<T>()));
            }
        }
    }

    return result;
}

template <typename K, typename V>
std::map<K, V> Variant::toMap() const
{
    std::map<K, V> result;

    if (true == isDictionary())
    {
        VariantDict_t* dict = value<VariantDict_t>();

        if (nullptr != dict)
        {
            for (auto it = dict->begin(); it != dict->end(); ++it)
            {
                result.emplace(*(it->first.value<K>()), *(it->second.value<V>()));
            }
        }
    }

    return result;
}

} // namespace hsmcpp

template struct std::pair<hsmcpp::Variant, hsmcpp::Variant>;

#endif  // __HSMCPP_VARIANT_HPP__
