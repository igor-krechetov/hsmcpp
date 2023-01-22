// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/variant.hpp"
#include <cstring>

namespace hsmcpp
{

Variant Variant::make(const int8_t v) { return Variant(new int8_t(v), Type::BYTE_1); }
Variant Variant::make(const int16_t v) { return Variant(new int16_t(v), Type::BYTE_2); }
Variant Variant::make(const int32_t v) { return Variant(new int32_t(v), Type::BYTE_4); }
Variant Variant::make(const int64_t v) { return Variant(new int64_t(v), Type::BYTE_8); }
Variant Variant::make(const uint8_t v) { return Variant(new uint8_t(v), Type::UBYTE_1); }
Variant Variant::make(const uint16_t v) { return Variant(new uint16_t(v), Type::UBYTE_2); }
Variant Variant::make(const uint32_t v) { return Variant(new uint32_t(v), Type::UBYTE_4); }
Variant Variant::make(const uint64_t v) { return Variant(new uint64_t(v), Type::UBYTE_8); }
Variant Variant::make(const double v) { return Variant(new double(v), Type::DOUBLE); }
Variant Variant::make(const bool v) { return Variant(new bool(v), Type::BOOL); }
Variant Variant::make(const std::string& v) { return Variant(new std::string(v), Type::STRING); }
Variant Variant::make(const char* v) { return make(std::string(v)); }
Variant Variant::make(const std::vector<char>& v) { return Variant(new std::vector<char>(v), Type::BYTEARRAY); }
Variant Variant::make(const char* binaryData, const size_t bytesCount){ return Variant(new std::vector<char>(binaryData, binaryData + bytesCount), Type::BYTEARRAY); }
Variant Variant::make(const VariantVector_t& v) { return Variant(new std::vector<Variant>(v), Type::VECTOR); }
Variant Variant::make(const VariantList_t& v)  { return Variant(new std::list<Variant>(v), Type::LIST); }
Variant Variant::make(const VariantDict_t& v) { return Variant(new VariantDict_t(v), Type::DICTIONARY); }
Variant Variant::make(const Variant& first, const Variant& second) { return Variant(new VariantPair_t(first, second), Type::PAIR); }
Variant Variant::make(const Variant& v) { return v; }

Variant::Variant(void* d, const Type t)
    : data(d)
    , type(t)
{
}

Variant::~Variant()
{
    free();
}

Variant::Variant(const Variant& v)
{
    *this = v;
}

Variant::Variant(Variant&& v)
{
    data = v.data;
    type = v.type;

    v.data = nullptr;
    v.type = Type::UNKNOWN;
}

Variant::Variant(const char* v) : Variant(std::string(v))
{
}

Variant::Variant(const char* binaryData, const size_t bytesCount)
    : Variant(std::vector<char>(binaryData, binaryData + bytesCount))
{
}

Variant& Variant::operator=(const Variant& v)
{
    if (false == isSameObject(v))
    {
        switch (v.type)
        {
            case Type::BYTE_1:
                *this = *v.value<int8_t>();
                break;
            case Type::BYTE_2:
                *this = *v.value<int16_t>();
                break;
            case Type::BYTE_4:
                *this = *v.value<int32_t>();
                break;
            case Type::BYTE_8:
                *this = *v.value<int64_t>();
                break;

            case Type::UBYTE_1:
                *this = *v.value<uint8_t>();
                break;
            case Type::UBYTE_2:
                *this = *v.value<uint16_t>();
                break;
            case Type::UBYTE_4:
                *this = *v.value<uint32_t>();
                break;
            case Type::UBYTE_8:
                *this = *v.value<uint64_t>();
                break;

            case Type::DOUBLE:
                *this = *v.value<double>();
                break;
            case Type::BOOL:
                *this = *v.value<bool>();
                break;

            case Type::STRING:
                *this = *v.value<std::string>();
                break;

            case Type::BYTEARRAY:
                *this = *v.value<std::vector<char>>();
                break;

            case Type::VECTOR:
                *this = *v.value<std::vector<Variant>>();
                break;

            case Type::LIST:
                *this = *v.value<std::list<Variant>>();
                break;

            case Type::DICTIONARY:
                *this = *v.value<VariantDict_t>();
                break;

            case Type::PAIR:
                *this = *v.value<VariantPair_t>();
                break;

            default:
                free();
                break;
        }
    }

    return *this;
}

Variant& Variant::operator=(Variant&& v)
{
    data = v.data;
    type = v.type;

    v.data = nullptr;
    v.type = Type::UNKNOWN;

    return *this;
}

Variant::operator bool() const
{
    return ((nullptr != data) && (Type::UNKNOWN != type));
}

bool Variant::operator!=(const Variant& val) const
{
    return !(*this == val);
}

bool Variant::operator>(const Variant& val) const
{
    bool isGreater = false;

    if (isNumeric() && val.isNumeric())
    {
        if ((Type::DOUBLE == type) || (Type::DOUBLE == val.type))
        {
            isGreater = toDouble() > val.toDouble();
        }
        else if (isUnsignedNumeric() || val.isUnsignedNumeric())
        {
            isGreater = toUInt64() > val.toUInt64();
        }
        else
        {
            isGreater = toInt64() > val.toInt64();
        }
    }
    else if ((Type::STRING == type) || (Type::STRING == val.type))
    {
        isGreater = (*value<std::string>() > *(val.value<std::string>()));
    }
    else if ((Type::BYTEARRAY == type) || (Type::BYTEARRAY == val.type))
    {
        std::vector<char>* left = value<std::vector<char>>();
        std::vector<char>* right = val.value<std::vector<char>>();

        isGreater = (left->size() > right->size());
    }
    else if (Type::VECTOR == val.type)
    {
        std::vector<Variant>* left = value<std::vector<Variant>>();
        std::vector<Variant>* right = val.value<std::vector<Variant>>();

        isGreater = (left->size() > right->size());
    }
    else if (Type::LIST == val.type)
    {
        std::list<Variant>* left = value<std::list<Variant>>();
        std::list<Variant>* right = val.value<std::list<Variant>>();

        isGreater = (left->size() > right->size());
    }
    else if ((Type::BOOL == type) || (Type::BOOL == val.type))
    {
        isGreater = (*value<bool>() > *(val.value<bool>()));
    }

    return isGreater;
}

bool Variant::operator>=(const Variant& val) const
{
    return (*this == val) || (*this > val);
}

bool Variant::operator<(const Variant& val) const
{
    return (*this != val) && !(*this > val);
}

bool Variant::operator<=(const Variant& val) const
{
    return (*this == val) || !(*this > val);
}

bool Variant::operator==(const Variant& val) const
{
    bool equal = false;

    if (val.type == type)
    {
        switch (type)
        {
            case Type::BYTE_1:
                equal = (*value<int8_t>() == *val.value<int8_t>());
                break;
            case Type::BYTE_2:
                equal = (*value<int16_t>() == *val.value<int16_t>());
                break;
            case Type::BYTE_4:
                equal = (*value<int32_t>() == *val.value<int32_t>());
                break;
            case Type::BYTE_8:
                equal = (*value<int64_t>() == *val.value<int64_t>());
                break;

            case Type::UBYTE_1:
                equal = (*value<uint8_t>() == *val.value<uint8_t>());
                break;
            case Type::UBYTE_2:
                equal = (*value<uint16_t>() == *val.value<uint16_t>());
                break;
            case Type::UBYTE_4:
                equal = (*value<uint32_t>() == *val.value<uint32_t>());
                break;
            case Type::UBYTE_8:
                equal = (*value<uint64_t>() == *val.value<uint64_t>());
                break;

            case Type::DOUBLE:
                equal = (*value<double>() == *val.value<double>());
                break;
            case Type::BOOL:
                equal = (*value<bool>() == *val.value<bool>());
                break;

            case Type::STRING:
                equal = (*value<std::string>() == *val.value<std::string>());
                break;

            case Type::BYTEARRAY:
                equal = (*value<std::vector<char>>() == *val.value<std::vector<char>>());
                break;

            case Type::VECTOR:
                equal = (*value<std::vector<Variant>>() == *val.value<std::vector<Variant>>());
                break;

            case Type::LIST:
                equal = (*value<std::list<Variant>>() == *val.value<std::list<Variant>>());
                break;

            case Type::DICTIONARY:
            {
                VariantDict_t* left = value<VariantDict_t>();
                VariantDict_t* right = val.value<VariantDict_t>();

                if (left->size() == right->size())
                {
                    equal = true;

                    for (auto itLeft = left->begin(); (itLeft != left->end()) && (true == equal) ; ++itLeft)
                    {
                        auto itRight = right->find(itLeft->first);

                        if (right->end() != itRight)
                        {
                            equal = (itLeft->second == itRight->second);
                        } else {
                            equal = false;
                        }
                    }
                }
                break;
            }

            case Type::PAIR:
            {
                VariantPair_t* left = value<VariantPair_t>();
                VariantPair_t* right = val.value<VariantPair_t>();

                equal = ((left->first == right->first) && (left->second == right->second));
                break;
            }

            case Type::UNKNOWN:
                equal = true;
                break;

            default:
                break;
        }
    }

    return equal;
}

bool Variant::isString() const
{
    return (type == Type::STRING);
}

bool Variant::isByteArray() const
{
    return (type == Type::BYTEARRAY);
}

bool Variant::isVector() const
{
    return (type == Type::VECTOR);
}

bool Variant::isList() const
{
    return (type == Type::LIST);
}

bool Variant::isDictionary() const
{
    return (type == Type::DICTIONARY);
}

bool Variant::isNumeric() const
{
    bool numeric = false;

    switch (type)
    {
        case Type::BYTE_1:
        case Type::BYTE_2:
        case Type::BYTE_4:
        case Type::BYTE_8:
        case Type::UBYTE_1:
        case Type::UBYTE_2:
        case Type::UBYTE_4:
        case Type::UBYTE_8:
        case Type::DOUBLE:
            numeric = true;
            break;

        case Type::BOOL:
        case Type::STRING:
        case Type::BYTEARRAY:
        case Type::VECTOR:
        case Type::LIST:
        case Type::DICTIONARY:
        case Type::PAIR:
        default:
            numeric = false;
            break;
    }

    return numeric;
}

bool Variant::isSignedNumeric() const
{
    bool isSigned = false;

    switch (type)
    {
        case Type::BYTE_1:
        case Type::BYTE_2:
        case Type::BYTE_4:
        case Type::BYTE_8:
        case Type::DOUBLE:
            isSigned = true;
            break;

        default:
            isSigned = false;
            break;
    }

    return isSigned;
}

bool Variant::isUnsignedNumeric() const
{
    bool isUnsigned = false;

    switch (type)
    {
        case Type::UBYTE_1:
        case Type::UBYTE_2:
        case Type::UBYTE_4:
        case Type::UBYTE_8:
            isUnsigned = true;
            break;

        default:
            isUnsigned = false;
            break;
    }

    return isUnsigned;
}

bool Variant::isBool() const
{
    return (type == Type::BOOL) || (true == isNumeric());
}

std::string Variant::toString() const
{
    std::string result;

    switch (getType())
    {
        case Type::BYTE_1:
            result = std::to_string(*(value<int8_t>()));
            break;
        case Type::BYTE_2:
            result = std::to_string(*(value<int16_t>()));
            break;
        case Type::BYTE_4:
            result = std::to_string(*(value<int32_t>()));
            break;
        case Type::BYTE_8:
            result = std::to_string(*(value<int64_t>()));
            break;
        case Type::UBYTE_1:
            result = std::to_string(*(value<uint8_t>()));
            break;
        case Type::UBYTE_2:
            result = std::to_string(*(value<uint16_t>()));
            break;
        case Type::UBYTE_4:
            result = std::to_string(*(value<uint32_t>()));
            break;
        case Type::UBYTE_8:
            result = std::to_string(*(value<uint64_t>()));
            break;
        case Type::DOUBLE:
            result = std::to_string(*(value<double>()));
            break;
        case Type::BOOL:
            result = (*(value<bool>()) ? "true" : "false");
            break;
        case Type::STRING:
            result = *(value<std::string>());
            break;
        case Type::BYTEARRAY:
        {
            std::vector<char>* val = value<std::vector<char>>();

            result.assign(val->data(), val->size());
            break;
        }
        case Type::VECTOR:
        {
            std::vector<Variant>* val = value<std::vector<Variant>>();

            for (auto it = val->begin(); it != val->end(); ++it)
            {
                if (false == result.empty())
                {
                    result += ", ";
                }

                result += it->toString();
            }
            break;
        }
        case Type::LIST:
        {
            std::list<Variant>* val = value<std::list<Variant>>();

            for (auto it = val->begin(); it != val->end(); ++it)
            {
                if (false == result.empty())
                {
                    result += ", ";
                }

                result += it->toString();
            }
            break;
        }
        case Type::DICTIONARY:
        {
            VariantDict_t* dict = value<VariantDict_t>();

            for (auto it = dict->begin() ; it != dict->end(); ++it)
            {
                result += it->first.toString() + "=[" + it->second.toString() + "], ";
            }
            break;
        }
        case Type::PAIR:
            result = "(" + value<VariantPair_t>()->first.toString() + ", " + value<VariantPair_t>()->second.toString() + ")";
            break;
        default:
            break;
    }

    return result;
}

std::vector<char> Variant::toByteArray() const
{
    std::vector<char> result;

    switch (getType())
    {
        case Type::BYTE_1:
        case Type::UBYTE_1:
            result.resize(sizeof(int8_t));
            std::memcpy(result.data(), data, sizeof(int8_t));
            break;
        case Type::BYTE_2:
        case Type::UBYTE_2:
            result.resize(sizeof(int16_t));
            memcpy(result.data(), data, sizeof(int16_t));
            break;
        case Type::BYTE_4:
        case Type::UBYTE_4:
            result.resize(sizeof(int32_t));
            memcpy(result.data(), data, sizeof(int32_t));
            break;
        case Type::BYTE_8:
        case Type::UBYTE_8:
            result.resize(sizeof(int64_t));
            memcpy(result.data(), data, sizeof(int64_t));
            break;
        case Type::DOUBLE:
            result.resize(sizeof(double));
            memcpy(result.data(), data, sizeof(double));
            break;
        case Type::BOOL:
            result.push_back(*(value<bool>()) == true ? 1 : 0);
            break;
        case Type::STRING:
        {
            std::string* val = value<std::string>();

            result.assign(val->begin(), val->end());
            break;
        }
        case Type::BYTEARRAY:
            result = *(value<std::vector<char>>());
            break;
        case Type::VECTOR:
            // TODO
            break;
        case Type::LIST:
            // TODO
            break;
        case Type::DICTIONARY:
        {
            VariantDict_t* dict = value<VariantDict_t>();

            for (auto it = dict->begin() ; it != dict->end(); ++it)
            {
                std::vector<char> curValue = it->second.toByteArray();

                result.insert(result.end(), curValue.begin(), curValue.end());
            }
            break;
        }
        case Type::PAIR:
        {
            std::vector<char> secondValue = value<VariantPair_t>()->second.toByteArray();

            result = value<VariantPair_t>()->first.toByteArray();
            result.insert(result.end(), secondValue.begin(), secondValue.end());
            break;
        }
        default:
            break;
    }

    return result;
}

int64_t Variant::toInt64() const
{
    int64_t result = 0;

    switch (type)
    {
        case Type::BYTE_1:
            result = static_cast<int64_t>(*value<int8_t>());
            break;
        case Type::BYTE_2:
            result = static_cast<int64_t>(*value<int16_t>());
            break;
        case Type::BYTE_4:
            result = static_cast<int64_t>(*value<int32_t>());
            break;
        case Type::BYTE_8:
            result = *value<int64_t>();
            break;

        case Type::UBYTE_1:
            result = static_cast<int64_t>(*value<uint8_t>());
            break;
        case Type::UBYTE_2:
            result = static_cast<int64_t>(*value<uint16_t>());
            break;
        case Type::UBYTE_4:
            result = static_cast<int64_t>(*value<uint32_t>());
            break;
        case Type::UBYTE_8:
            result = static_cast<int64_t>(*value<uint64_t>());
            break;

        case Type::DOUBLE:
            result = static_cast<int64_t>(*value<double>());
            break;

        case Type::BOOL:
        case Type::STRING:
        case Type::BYTEARRAY:
        case Type::VECTOR:
        case Type::LIST:
        case Type::DICTIONARY:
        case Type::PAIR:
        default:
            break;
    }

    return result;
}

uint64_t Variant::toUInt64() const
{
    uint64_t result = 0;

    switch (type)
    {
        case Type::BYTE_1:
            result = static_cast<uint64_t>(*value<int8_t>());
            break;
        case Type::BYTE_2:
            result = static_cast<uint64_t>(*value<int16_t>());
            break;
        case Type::BYTE_4:
            result = static_cast<uint64_t>(*value<int32_t>());
            break;
        case Type::BYTE_8:
            result = static_cast<uint64_t>(*value<int64_t>());
            break;

        case Type::UBYTE_1:
            result = static_cast<uint64_t>(*value<uint8_t>());
            break;
        case Type::UBYTE_2:
            result = static_cast<uint64_t>(*value<uint16_t>());
            break;
        case Type::UBYTE_4:
            result = static_cast<uint64_t>(*value<uint32_t>());
            break;
        case Type::UBYTE_8:
            result = *value<uint64_t>();
            break;

        case Type::DOUBLE:
            result = static_cast<uint64_t>(*value<double>());
            break;

        case Type::BOOL:
        case Type::STRING:
        case Type::BYTEARRAY:
        case Type::VECTOR:
        case Type::LIST:
        case Type::DICTIONARY:
        case Type::PAIR:
        default:
            break;
    }

    return result;
}

double Variant::toDouble() const
{
    double result = 0.0;

    switch (type)
    {
        case Type::BYTE_1:
            result = static_cast<double>(*value<int8_t>());
            break;
        case Type::BYTE_2:
            result = static_cast<double>(*value<int16_t>());
            break;
        case Type::BYTE_4:
            result = static_cast<double>(*value<int32_t>());
            break;
        case Type::BYTE_8:
            result = static_cast<double>(*value<int64_t>());
            break;

        case Type::UBYTE_1:
            result = static_cast<double>(*value<uint8_t>());
            break;
        case Type::UBYTE_2:
            result = static_cast<double>(*value<uint16_t>());
            break;
        case Type::UBYTE_4:
            result = static_cast<double>(*value<uint32_t>());
            break;
        case Type::UBYTE_8:
            result = static_cast<double>(*value<uint64_t>());
            break;

        case Type::DOUBLE:
            result = *value<double>();
            break;

        case Type::BOOL:
        case Type::STRING:
        case Type::BYTEARRAY:
        case Type::VECTOR:
        case Type::LIST:
        case Type::DICTIONARY:
        case Type::PAIR:
        default:
            break;
    }

    return result;
}

bool Variant::toBool() const
{
    bool result = false;

    if (Type::BOOL == type)
    {
        result = *value<bool>();
    } else {
        result = (0 != toInt64());
    }

    return result;
}

bool Variant::isSameObject(const Variant& val) const
{
    return (val.data == data) && (nullptr != data);
}

void Variant::free()
{
    switch (type)
    {
        case Type::BYTE_1:
            delete static_cast<int8_t*>(data);
            break;
        case Type::BYTE_2:
            delete static_cast<int16_t*>(data);
            break;
        case Type::BYTE_4:
            delete static_cast<int32_t*>(data);
            break;
        case Type::BYTE_8:
            delete static_cast<int64_t*>(data);
            break;

        case Type::UBYTE_1:
            delete static_cast<uint8_t*>(data);
            break;
        case Type::UBYTE_2:
            delete static_cast<uint16_t*>(data);
            break;
        case Type::UBYTE_4:
            delete static_cast<uint32_t*>(data);
            break;
        case Type::UBYTE_8:
            delete static_cast<uint64_t*>(data);
            break;

        case Type::DOUBLE:
            delete static_cast<double*>(data);
            break;
        case Type::BOOL:
            delete static_cast<bool*>(data);
            break;

        case Type::STRING:
            delete static_cast<std::string*>(data);
            break;

        case Type::BYTEARRAY:
            delete static_cast<std::vector<char>*>(data);
            break;

        case Type::VECTOR:
            delete static_cast<std::vector<Variant>*>(data);
            break;

        case Type::LIST:
            delete static_cast<std::list<Variant>*>(data);
            break;

        case Type::DICTIONARY:
            delete static_cast<VariantDict_t*>(data);
            break;

        case Type::PAIR:
            delete static_cast<VariantPair_t*>(data);
            break;

        default:  // Type::UNKNOWN
            break;
    }

    data = nullptr;
    type = Type::UNKNOWN;
}

} // namespace hsmcpp