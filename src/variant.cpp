// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#include "hsmcpp/variant.hpp"

#include <cstring>

#include "hsmcpp/os/os.hpp"

// These macroses can't be converted to 'constexpr' template functions
// NOLINTBEGIN(cppcoreguidelines-macro-usage)

// cppcheck-suppress misra-c2012-20.7 ; parentheses are not needed
#define IMPL_CONSTRUCTOR(_val_type, _internal_type) \
  Variant::Variant(const _val_type v) {             \
    assign(v, _internal_type);                      \
  }

// cppcheck-suppress misra-c2012-20.7 ; parentheses are not needed
#define IMPL_OPERATOR_ASSIGN(_val_type, _internal_type) \
  Variant& Variant::operator=(const _val_type v) {      \
    assign(v, _internal_type);                          \
    return *this;                                       \
  }

// cppcheck-suppress misra-c2012-20.7 ; parentheses are not needed
#define IMPL_MAKE(_val_type)                 \
  Variant Variant::make(const _val_type v) { \
    return Variant(v);                       \
  }

// NOLINTEND(cppcoreguidelines-macro-usage)

namespace hsmcpp {

// =================================================================================================================
// make()
IMPL_MAKE(int8_t)
IMPL_MAKE(int16_t)
IMPL_MAKE(int32_t)
IMPL_MAKE(int64_t)
IMPL_MAKE(uint8_t)
IMPL_MAKE(uint16_t)
IMPL_MAKE(uint32_t)
IMPL_MAKE(uint64_t)
IMPL_MAKE(double)
IMPL_MAKE(bool)
IMPL_MAKE(std::string&)
IMPL_MAKE(ByteArray_t&)
IMPL_MAKE(VariantVector_t&)
IMPL_MAKE(VariantList_t&)
IMPL_MAKE(VariantMap_t&)
IMPL_MAKE(VariantPair_t&)
IMPL_MAKE(Variant&)

Variant Variant::make(const char* v) {
    return Variant(v);
}

Variant Variant::make(const char* binaryData, const size_t bytesCount) {
    return Variant(binaryData, bytesCount);
}

// =================================================================================================================
// Constructors
Variant::Variant(std::shared_ptr<void> d, const Type t)
    : data(std::move(d))
    , type(t) {}

Variant::~Variant() {
    freeMemory();
}

Variant::Variant(const Variant& v) {
    *this = v;
}

Variant::Variant(Variant&& v) noexcept
    : data(std::move(v.data))
    , type(v.type)
    , memoryAllocator(std::move(v.memoryAllocator))
    , compareOperator(std::move(v.compareOperator)) {
    v.type = Type::UNKNOWN;
}

IMPL_CONSTRUCTOR(int8_t, Type::BYTE_1)
IMPL_CONSTRUCTOR(int16_t, Type::BYTE_2)
IMPL_CONSTRUCTOR(int32_t, Type::BYTE_4)
IMPL_CONSTRUCTOR(int64_t, Type::BYTE_8)
IMPL_CONSTRUCTOR(uint8_t, Type::UBYTE_1)
IMPL_CONSTRUCTOR(uint16_t, Type::UBYTE_2)
IMPL_CONSTRUCTOR(uint32_t, Type::UBYTE_4)
IMPL_CONSTRUCTOR(uint64_t, Type::UBYTE_8)
IMPL_CONSTRUCTOR(double, Type::DOUBLE)
IMPL_CONSTRUCTOR(bool, Type::BOOL)
IMPL_CONSTRUCTOR(std::string&, Type::STRING)
IMPL_CONSTRUCTOR(ByteArray_t&, Type::BYTEARRAY)
IMPL_CONSTRUCTOR(VariantVector_t&, Type::VECTOR)
IMPL_CONSTRUCTOR(VariantList_t&, Type::LIST)
IMPL_CONSTRUCTOR(VariantMap_t&, Type::MAP)
IMPL_CONSTRUCTOR(VariantPair_t&, Type::PAIR)

Variant::Variant(const char* v)
    // cppcheck-suppress misra-c2012-10.4 : false-positive. thinks that ':' is arithmetic operation
    : Variant(std::string(v)) {}

Variant::Variant(const char* binaryData, const size_t bytesCount)
    // cppcheck-suppress misra-c2012-10.4 : false-positive. thinks that ':' is arithmetic operation
    : Variant(ByteArray_t(binaryData, &binaryData[bytesCount])) {} // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

// =================================================================================================================
// Assign operators
IMPL_OPERATOR_ASSIGN(int8_t, Type::BYTE_1)
IMPL_OPERATOR_ASSIGN(int16_t, Type::BYTE_2)
IMPL_OPERATOR_ASSIGN(int32_t, Type::BYTE_4)
IMPL_OPERATOR_ASSIGN(int64_t, Type::BYTE_8)
IMPL_OPERATOR_ASSIGN(uint8_t, Type::UBYTE_1)
IMPL_OPERATOR_ASSIGN(uint16_t, Type::UBYTE_2)
IMPL_OPERATOR_ASSIGN(uint32_t, Type::UBYTE_4)
IMPL_OPERATOR_ASSIGN(uint64_t, Type::UBYTE_8)
IMPL_OPERATOR_ASSIGN(double, Type::DOUBLE)
IMPL_OPERATOR_ASSIGN(bool, Type::BOOL)
IMPL_OPERATOR_ASSIGN(std::string&, Type::STRING)
IMPL_OPERATOR_ASSIGN(ByteArray_t&, Type::BYTEARRAY)
IMPL_OPERATOR_ASSIGN(VariantVector_t&, Type::VECTOR)
IMPL_OPERATOR_ASSIGN(VariantList_t&, Type::LIST)
IMPL_OPERATOR_ASSIGN(VariantMap_t&, Type::MAP)
IMPL_OPERATOR_ASSIGN(VariantPair_t&, Type::PAIR)

Variant& Variant::operator=(const char* v) {
    assign(std::string(v), Type::STRING);
    return *this;
}

Variant& Variant::operator=(const Variant& v) {
    if (false == isSameObject(v)) {
        if ((Type::UNKNOWN != v.type) && (v.memoryAllocator)) {
            data = v.memoryAllocator(v.data.get());
            type = v.type;
            memoryAllocator = v.memoryAllocator;
            compareOperator = v.compareOperator;
        } else {
            freeMemory();
        }
    }

    return *this;
}

Variant& Variant::operator=(Variant&& v) noexcept {
    if (false == isSameObject(v)) {
        data = std::move(v.data);
        type = v.type;
        memoryAllocator = std::move(v.memoryAllocator);
        compareOperator = std::move(v.compareOperator);

        v.type = Type::UNKNOWN;
    }

    return *this;
}

// =================================================================================================================
Variant::operator bool() const {
    return (data && (Type::UNKNOWN != type));
}

bool Variant::operator!=(const Variant& val) const {
    return !(*this == val);
}

bool Variant::operator>(const Variant& val) const {
    bool isGreater = false;

    if ((data != val.data) && (data) && (val.data)) {
        if (isNumeric() && val.isNumeric()) {
            if ((Type::DOUBLE == type) || (Type::DOUBLE == val.type)) {
                isGreater = toDouble() > val.toDouble();
            } else if (isUnsignedNumeric() || val.isUnsignedNumeric()) {
                isGreater = toUInt64() > val.toUInt64();
            } else {
                isGreater = toInt64() > val.toInt64();
            }
        } else if (type == val.type) {
            isGreater = (1 == compareOperator(data.get(), val.data.get()));
        } else {
            // do nothing
        }
    }

    return isGreater;
}

bool Variant::operator>=(const Variant& val) const {
    return (*this == val) || (*this > val);
}

bool Variant::operator<(const Variant& val) const {
    return (*this != val) && !(*this > val);
}

bool Variant::operator<=(const Variant& val) const {
    return (*this == val) || !(*this > val);
}

bool Variant::operator==(const Variant& val) const {
    constexpr double comparePrecision = 0.00000001;
    bool equal = false;

    if (data != val.data) {
        if ((data) && (val.data)) {
            if (isNumeric() && val.isNumeric()) {
                if ((Type::DOUBLE == type) || (Type::DOUBLE == val.type)) {
                    // compare with precision for double
                    // cppcheck-suppress misra-c2012-10.4 : false positive. both operands have type double
                    equal = (std::abs(toDouble() - val.toDouble()) < comparePrecision);
                } else if (isUnsignedNumeric() || val.isUnsignedNumeric()) {
                    equal = (toUInt64() == val.toUInt64());
                } else {
                    equal = (toInt64() == val.toInt64());
                }
            } else if (val.type == type) {
                equal = (0 == compareOperator(data.get(), val.data.get()));
            } else {
                // do nothing
            }
        }
    } else {
        equal = true;
    }

    return equal;
}

void Variant::clear() {
    freeMemory();
}

bool Variant::isEmpty() const {
    return (type == Type::UNKNOWN);
}

bool Variant::isString() const {
    return (type == Type::STRING);
}

bool Variant::isByteArray() const {
    return (type == Type::BYTEARRAY);
}

bool Variant::isVector() const {
    return (type == Type::VECTOR);
}

bool Variant::isList() const {
    return (type == Type::LIST);
}

bool Variant::isMap() const {
    return (type == Type::MAP);
}

bool Variant::isPair() const {
    return (type == Type::PAIR);
}

bool Variant::isCustomType() const {
    return (type == Type::CUSTOM);
}

bool Variant::isNumeric() const {
    bool numeric = false;

    switch (type) {
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
        case Type::MAP:
        case Type::PAIR:
        default:
            numeric = false;
            break;
    }

    return numeric;
}

bool Variant::isSignedNumeric() const {
    bool isSigned = false;

    switch (type) {
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

bool Variant::isUnsignedNumeric() const {
    bool isUnsigned = false;

    switch (type) {
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

bool Variant::isBool() const {
    return (type == Type::BOOL);
}

std::string Variant::toString() const {
    std::string result;

    switch (getType()) {
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
        case Type::BYTEARRAY: {
            std::shared_ptr<ByteArray_t> val = value<ByteArray_t>();

            result.assign(val->begin(), val->end());
            break;
        }
        case Type::VECTOR: {
            std::shared_ptr<VariantVector_t> val = value<VariantVector_t>();

            // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
            if (val) {
                for (const Variant& item: *val) {
                    if (false == result.empty()) {
                        result.append(", ");
                    }

                    result += item.toString();
                }
            }
            break;
        }
        case Type::LIST: {
            std::shared_ptr<VariantList_t> val = value<VariantList_t>();

            // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
            if (val) {
                for (const Variant& item: *val) {
                    if (false == result.empty()) {
                        result.append(", ");
                    }

                    result += item.toString();
                }
            }
            break;
        }
        case Type::MAP: {
            std::shared_ptr<VariantMap_t> val = value<VariantMap_t>();

            // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
            if (val) {
                for (const auto& item: *val) {
                    if (false == result.empty()) {
                        result.append(", ");
                    }
                    result += item.first.toString().append("=[").append(item.second.toString()).append("]");
                }
            }
            break;
        }
        case Type::PAIR:
            result = std::string("(") + value<VariantPair_t>()->first.toString() + std::string(", ") +
                     value<VariantPair_t>()->second.toString() + std::string(")");
            break;
        default:
            break;
    }

    return result;
}

int64_t Variant::toInt64() const {
    int64_t result = 0;

    switch (type) {
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
            result = static_cast<int64_t>(*value<bool>());
            break;

        case Type::STRING:
            HSM_TRY {
                result = std::stoll(toString());
            }
            HSM_CATCH(...) {
                // return empty value
            }
            break;

        case Type::BYTEARRAY:
        case Type::VECTOR:
        case Type::LIST:
        case Type::MAP:
        case Type::PAIR:
        default:
            break;
    }

    return result;
}

uint64_t Variant::toUInt64() const {
    uint64_t result = 0;

    switch (type) {
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
            result = static_cast<int64_t>(*value<bool>());
            break;

        case Type::STRING:
            HSM_TRY {
                result = std::stoull(toString());
            }
            HSM_CATCH(...) {
                // return empty value
            }
            break;

        case Type::BYTEARRAY:
        case Type::VECTOR:
        case Type::LIST:
        case Type::MAP:
        case Type::PAIR:
        default:
            break;
    }

    return result;
}

double Variant::toDouble() const {
    double result = 0.0;

    switch (type) {
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
            result = static_cast<double>(toBool());
            break;

        case Type::STRING:
            HSM_TRY {
                result = std::stod(toString());
            }
            HSM_CATCH(...) {
                // return empty value
            }
            break;

        case Type::BYTEARRAY:
        case Type::VECTOR:
        case Type::LIST:
        case Type::MAP:
        case Type::PAIR:
        default:
            break;
    }

    return result;
}

bool Variant::toBool() const {
    bool result = false;

    if (Type::BOOL == type) {
        result = *value<bool>();
    } else if (Type::STRING == type) {
        const std::string strValue = toString();

        if (strValue != "true") {
            HSM_TRY {
                result = (0 != std::stoll(strValue));
            }
            HSM_CATCH(...) {
                // return empty value
            }
        } else {
            result = true;
        }
    } else {
        result = (0 != toInt64());
    }

    return result;
}

ByteArray_t Variant::toByteArray() const {
    ByteArray_t result;

    switch (getType()) {
        case Type::BYTE_1:
        case Type::UBYTE_1:
            result.resize(sizeof(int8_t));
            static_cast<void>(std::memcpy(result.data(), data.get(), sizeof(int8_t)));
            break;
        case Type::BYTE_2:
        case Type::UBYTE_2:
            result.resize(sizeof(int16_t));
            static_cast<void>(memcpy(result.data(), data.get(), sizeof(int16_t)));
            break;
        case Type::BYTE_4:
        case Type::UBYTE_4:
            result.resize(sizeof(int32_t));
            static_cast<void>(memcpy(result.data(), data.get(), sizeof(int32_t)));
            break;
        case Type::BYTE_8:
        case Type::UBYTE_8:
            result.resize(sizeof(int64_t));
            static_cast<void>(memcpy(result.data(), data.get(), sizeof(int64_t)));
            break;
        case Type::DOUBLE:
            result.resize(sizeof(double));
            static_cast<void>(memcpy(result.data(), data.get(), sizeof(double)));
            break;
        case Type::BOOL:
            result.push_back((toBool() == true) ? 0x01 : 0x00);
            break;
        case Type::STRING: {
            std::shared_ptr<std::string> val = value<std::string>();

            // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
            if (val) {
                result.assign(val->begin(), val->end());
            }
            break;
        }
        case Type::BYTEARRAY:
            result = *(value<ByteArray_t>());
            break;
        case Type::VECTOR: {
            std::shared_ptr<VariantVector_t> data = value<VariantVector_t>();

            // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
            if (data) {
                for (const Variant& item: *data) {
                    const ByteArray_t curValue = item.toByteArray();

                    (void)result.insert(result.end(), curValue.begin(), curValue.end());
                }
            }
            break;
        }
        case Type::LIST: {
            std::shared_ptr<VariantList_t> data = value<VariantList_t>();

            // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
            if (data) {
                for (const Variant& item: *data) {
                    const ByteArray_t curValue = item.toByteArray();

                    (void)result.insert(result.end(), curValue.begin(), curValue.end());
                }
            }
            break;
        }
        case Type::PAIR: {
            // TODO: is this even needed?
            ByteArray_t secondValue = value<VariantPair_t>()->second.toByteArray();

            result = value<VariantPair_t>()->first.toByteArray();
            (void)result.insert(result.end(), secondValue.begin(), secondValue.end());
            break;
        }

        case Type::MAP:
        default:
            break;
    }

    return result;
}

std::shared_ptr<ByteArray_t> Variant::getByteArray() const {
    std::shared_ptr<ByteArray_t> result;

    if (true == isByteArray()) {
        result = value<ByteArray_t>();
    }

    return result;
}

std::shared_ptr<VariantVector_t> Variant::getVector() const {
    std::shared_ptr<VariantVector_t> result;

    if (true == isVector()) {
        result = value<VariantVector_t>();
    }

    return result;
}

std::shared_ptr<VariantList_t> Variant::getList() const {
    std::shared_ptr<VariantList_t> result;

    if (true == isList()) {
        result = value<VariantList_t>();
    }

    return result;
}

std::shared_ptr<VariantMap_t> Variant::getMap() const {
    std::shared_ptr<VariantMap_t> result;

    if (true == isMap()) {
        result = value<VariantMap_t>();
    }

    return result;
}

std::shared_ptr<VariantPair_t> Variant::getPair() const {
    std::shared_ptr<VariantPair_t> result;

    if (true == isPair()) {
        result = value<VariantPair_t>();
    }

    return result;
}

bool Variant::isSameObject(const Variant& val) const {
    return ((val.data.get() == data.get()) && data);
}

void Variant::freeMemory() {
    data.reset();
    type = Type::UNKNOWN;
    memoryAllocator = nullptr;
    compareOperator = nullptr;
}

}  // namespace hsmcpp