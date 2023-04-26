// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "TestsCommon.hpp"
#include "hsmcpp/variant.hpp"
#include <inttypes.h>

constexpr int gIndexValue = 0;
constexpr int gIndexValueLess = 1;
constexpr int gIndexValueGreater = 2;
constexpr int gIndexValueComparable = 3;
constexpr int gIndexTypeName = 4;
constexpr int gIndexType = 5;
constexpr int gIndexFuncTypeCheck = 6;
constexpr int gIndexFuncValueCheck = 7;

// custom type used for testing
struct CustomType {
    std::string a;
    int b = 0;
    bool* wasDeleted = nullptr;

    CustomType(const std::string& v1, const int v2) : a(v1), b(v2){}
    CustomType(const CustomType& src) {
        a = src.a;
        b = src.b;
        wasDeleted = src.wasDeleted;
    }

    ~CustomType() {
        if (nullptr != wasDeleted) {
            *wasDeleted = true;
        }
    }

    bool operator>(const CustomType& val) const {
        return (a > val.a);
    }

    bool operator==(const CustomType& val) const {
        return (a == val.a);
    }
};

// NOTE: copied from hsm.hpp
template <typename... Args>
void makeVariantList(VariantVector_t& vList, Args&&... args) {
    volatile int make_variant[] = {0, (vList.push_back(Variant::make(std::forward<Args>(args))), 0)...};
    (void)make_variant;
}

template<typename T>
std::string getTypeNameFromTuple(const int index, const T& container) {
    const auto& types = std::get<gIndexTypeName>(container);

    if (index < types.size()) {
        return types[index];
    }

    return "";
}

template<typename T>
Variant::Type getTypeFromTuple(const int index, const T& container) {
    const auto& types = std::get<gIndexType>(container);

    if (index < types.size()) {
        return types[index];
    }

    return Variant::Type::UNKNOWN;
}

template<typename T>
bool isComparableType(const int index, const T& container) {
    const auto& comparable = std::get<gIndexValueComparable>(container);

    if (index < comparable.size()) {
        return comparable[index];
    }

    return false;
}

template<typename T>
Variant constructVariantFromTuple(const int index, const T& container) {
    #define CONSTRUCT_VARIANT_FROM_TUPLE_CASE(i)    case i:     return Variant(std::get<i>(container));

    switch (index) {
        CONSTRUCT_VARIANT_FROM_TUPLE_CASE(0)
        CONSTRUCT_VARIANT_FROM_TUPLE_CASE(1)
        CONSTRUCT_VARIANT_FROM_TUPLE_CASE(2)
        CONSTRUCT_VARIANT_FROM_TUPLE_CASE(3)
        CONSTRUCT_VARIANT_FROM_TUPLE_CASE(4)
        CONSTRUCT_VARIANT_FROM_TUPLE_CASE(5)
        CONSTRUCT_VARIANT_FROM_TUPLE_CASE(6)
        CONSTRUCT_VARIANT_FROM_TUPLE_CASE(7)
        CONSTRUCT_VARIANT_FROM_TUPLE_CASE(8)
        CONSTRUCT_VARIANT_FROM_TUPLE_CASE(9)
        CONSTRUCT_VARIANT_FROM_TUPLE_CASE(10)
        CONSTRUCT_VARIANT_FROM_TUPLE_CASE(11)
        CONSTRUCT_VARIANT_FROM_TUPLE_CASE(12)
        CONSTRUCT_VARIANT_FROM_TUPLE_CASE(13)
        CONSTRUCT_VARIANT_FROM_TUPLE_CASE(14)
        CONSTRUCT_VARIANT_FROM_TUPLE_CASE(15)
        CONSTRUCT_VARIANT_FROM_TUPLE_CASE(16)
        CONSTRUCT_VARIANT_FROM_TUPLE_CASE(17)
        CONSTRUCT_VARIANT_FROM_TUPLE_CASE(18)
    }

    return Variant();
}

template<typename T>
Variant makeVariantFromTuple(const int index, const T& container) {
    #define MAKE_VARIANT_FROM_TUPLE_CASE(i)     case i:     return Variant::make(std::get<i>(container));

    switch (index) {
        MAKE_VARIANT_FROM_TUPLE_CASE(0)
        MAKE_VARIANT_FROM_TUPLE_CASE(1)
        MAKE_VARIANT_FROM_TUPLE_CASE(2)
        MAKE_VARIANT_FROM_TUPLE_CASE(3)
        MAKE_VARIANT_FROM_TUPLE_CASE(4)
        MAKE_VARIANT_FROM_TUPLE_CASE(5)
        MAKE_VARIANT_FROM_TUPLE_CASE(6)
        MAKE_VARIANT_FROM_TUPLE_CASE(7)
        MAKE_VARIANT_FROM_TUPLE_CASE(8)
        MAKE_VARIANT_FROM_TUPLE_CASE(9)
        MAKE_VARIANT_FROM_TUPLE_CASE(10)
        MAKE_VARIANT_FROM_TUPLE_CASE(11)
        MAKE_VARIANT_FROM_TUPLE_CASE(12)
        MAKE_VARIANT_FROM_TUPLE_CASE(13)
        MAKE_VARIANT_FROM_TUPLE_CASE(14)
        MAKE_VARIANT_FROM_TUPLE_CASE(15)
        MAKE_VARIANT_FROM_TUPLE_CASE(16)
        MAKE_VARIANT_FROM_TUPLE_CASE(17)
        MAKE_VARIANT_FROM_TUPLE_CASE(18)
    }

    return Variant();
}

template<typename T>
void assignNewVariantValue(Variant& outV, const int index, const T& container) {
    #define ASSIGN_NEW_VARIANT_VALUE_CASE(i)     case i:    outV = std::get<i>(container); break;

    switch (index) {
        ASSIGN_NEW_VARIANT_VALUE_CASE(0)
        ASSIGN_NEW_VARIANT_VALUE_CASE(1)
        ASSIGN_NEW_VARIANT_VALUE_CASE(2)
        ASSIGN_NEW_VARIANT_VALUE_CASE(3)
        ASSIGN_NEW_VARIANT_VALUE_CASE(4)
        ASSIGN_NEW_VARIANT_VALUE_CASE(5)
        ASSIGN_NEW_VARIANT_VALUE_CASE(6)
        ASSIGN_NEW_VARIANT_VALUE_CASE(7)
        ASSIGN_NEW_VARIANT_VALUE_CASE(8)
        ASSIGN_NEW_VARIANT_VALUE_CASE(9)
        ASSIGN_NEW_VARIANT_VALUE_CASE(10)
        ASSIGN_NEW_VARIANT_VALUE_CASE(11)
        ASSIGN_NEW_VARIANT_VALUE_CASE(12)
        ASSIGN_NEW_VARIANT_VALUE_CASE(13)
        ASSIGN_NEW_VARIANT_VALUE_CASE(14)
        ASSIGN_NEW_VARIANT_VALUE_CASE(15)
        ASSIGN_NEW_VARIANT_VALUE_CASE(16)
        ASSIGN_NEW_VARIANT_VALUE_CASE(17)
        ASSIGN_NEW_VARIANT_VALUE_CASE(18)
    }
}

template<typename T>
bool isCorrectType(const int index, const Variant& v, const T& container) {
    #define IS_CORRECT_TYPE_CASE(i)       case i:     return std::get<i>(container)(v);

    switch (index) {
        IS_CORRECT_TYPE_CASE(0)
        IS_CORRECT_TYPE_CASE(1)
        IS_CORRECT_TYPE_CASE(2)
        IS_CORRECT_TYPE_CASE(3)
        IS_CORRECT_TYPE_CASE(4)
        IS_CORRECT_TYPE_CASE(5)
        IS_CORRECT_TYPE_CASE(6)
        IS_CORRECT_TYPE_CASE(7)
        IS_CORRECT_TYPE_CASE(8)
        IS_CORRECT_TYPE_CASE(9)
        IS_CORRECT_TYPE_CASE(10)
        IS_CORRECT_TYPE_CASE(11)
        IS_CORRECT_TYPE_CASE(12)
        IS_CORRECT_TYPE_CASE(13)
        IS_CORRECT_TYPE_CASE(14)
        IS_CORRECT_TYPE_CASE(15)
        IS_CORRECT_TYPE_CASE(16)
        IS_CORRECT_TYPE_CASE(17)
        IS_CORRECT_TYPE_CASE(18)
    }

    return false;
}

template<typename TFuncs, typename TValues>
void checkVariantValue(const int index, const Variant& v, const TFuncs& functions, const TValues& values) {
    #define CHECK_VARIANT_VALUE_CASE(i)         case i:     std::get<i>(functions)(v, std::get<i>(values)); break;

    switch (index) {
        CHECK_VARIANT_VALUE_CASE(0)
        CHECK_VARIANT_VALUE_CASE(1)
        CHECK_VARIANT_VALUE_CASE(2)
        CHECK_VARIANT_VALUE_CASE(3)
        CHECK_VARIANT_VALUE_CASE(4)
        CHECK_VARIANT_VALUE_CASE(5)
        CHECK_VARIANT_VALUE_CASE(6)
        CHECK_VARIANT_VALUE_CASE(7)
        CHECK_VARIANT_VALUE_CASE(8)
        CHECK_VARIANT_VALUE_CASE(9)
        CHECK_VARIANT_VALUE_CASE(10)
        CHECK_VARIANT_VALUE_CASE(11)
        CHECK_VARIANT_VALUE_CASE(12)
        CHECK_VARIANT_VALUE_CASE(13)
        CHECK_VARIANT_VALUE_CASE(14)
        CHECK_VARIANT_VALUE_CASE(15)
        CHECK_VARIANT_VALUE_CASE(16)
        CHECK_VARIANT_VALUE_CASE(17)
        CHECK_VARIANT_VALUE_CASE(18)
    }
}

std::string variantTypeToStr(const Variant::Type t) {
    std::string res;

    switch(t) {
        case Variant::Type::BYTE_1:         res = "BYTE_1"; break;
        case Variant::Type::BYTE_2:         res = "BYTE_2"; break;
        case Variant::Type::BYTE_4:         res = "BYTE_4"; break;
        case Variant::Type::BYTE_8:         res = "BYTE_8"; break;
        case Variant::Type::UBYTE_1:        res = "UBYTE_1"; break;
        case Variant::Type::UBYTE_2:        res = "UBYTE_2"; break;
        case Variant::Type::UBYTE_4:        res = "UBYTE_4"; break;
        case Variant::Type::UBYTE_8:        res = "UBYTE_8"; break;
        case Variant::Type::DOUBLE:         res = "DOUBLE"; break;
        case Variant::Type::BOOL:           res = "BOOL"; break;
        case Variant::Type::STRING:         res = "STRING"; break;
        case Variant::Type::BYTEARRAY:      res = "BYTEARRAY"; break;
        case Variant::Type::LIST:           res = "LIST"; break;
        case Variant::Type::VECTOR:         res = "VECTOR"; break;
        case Variant::Type::MAP:     res = "MAP"; break;
        case Variant::Type::PAIR:           res = "PAIR"; break;
        case Variant::Type::CUSTOM:         res = "CUSTOM"; break;
        case Variant::Type::UNKNOWN:
        default:
            res ="UNKNOWN";
            break;
    }

    return res;
}

// global consts used by multiple tests
const int8_t i8 = -8;
const int16_t i16 = -16;
const int32_t i32 = -32;
const int64_t i64 = -64;
const uint8_t ui8 = 8;
const uint16_t ui16 = 16;
const uint32_t ui32 = 32;
const uint64_t ui64 = 64;
const double d = 123.45;
const bool b = false;
const std::string s1 = "abc";
const char* s2 = "dfg";
const char binary1[] = {1, 2, 0, 3};
const char binary1Less[] = {1, 0};
const char binary1Larger[] = {1, 2, 0, 3, 7, 8};
const ByteArray_t binary2 = {4, 5, 0, 6};
const VariantList_t listInt = {Variant(1), Variant(2), Variant(3)};
const VariantList_t listStr = {Variant("aa"), Variant("bb"), Variant("cc")};
const VariantVector_t vectorBool = {Variant(false), Variant(true), Variant(false)};
const VariantMap_t mapIntStr = {{Variant(1), Variant("aa")}, {Variant(2), Variant("bb")}, {Variant(3), Variant("cc")}};
const VariantPair_t pairIntStr = {Variant(static_cast<int32_t>(7)), Variant("ab")};
CustomType customTypeValue = {"abc", 17};

// =================================================================================================================
const auto allTypeValues = std::make_tuple(
                std::make_tuple(i8, i16, i32, i64, ui8, ui16, ui32, ui64, d, b, s1, s2, binary2, listInt, listStr, vectorBool, mapIntStr, pairIntStr, customTypeValue),
                // lesser value
                std::make_tuple(i8 - 1, i16 - 1, i32 - 1, i64 - 1, ui8 - 1, ui16 - 1, ui32 - 1, ui64 - 1, d - 1.0,
                                false, std::string("a"), "d",
                                ByteArray_t{4, 5, 0},
                                VariantList_t{Variant(1), Variant(2)},
                                VariantList_t{Variant("aa"), Variant("bb")},
                                VariantVector_t{Variant(false), Variant(true)},
                                VariantMap_t{{Variant(1), Variant("aa")}, {Variant(2), Variant("bb")}},
                                VariantPair_t{Variant(-3), Variant("a")},
                                CustomType{"a", 3}),
                // greater value
                std::make_tuple(i8 + 1, i16 + 1, i32 + 1, i64 + 1, ui8 + 1, ui16 + 1, ui32 + 1, ui64 + 1, d + 1.0,
                                true, s1 + "aaa", "dfgaaa",
                                ByteArray_t{4, 5, 0, 6, 2, 7},
                                VariantList_t{Variant(1), Variant(2), Variant(3), Variant(4)},
                                VariantList_t{Variant("aa"), Variant("bb"), Variant("cc"), Variant("dd")},
                                VariantVector_t{Variant(false), Variant(true), Variant(false), Variant(true)},
                                VariantMap_t{{Variant(1), Variant("aa")}, {Variant(2), Variant("bb")}, {Variant(3), Variant("cc")}, {Variant(4), Variant("dd")}},
                                VariantPair_t{Variant(102), Variant("abc")},
                                CustomType{"abcde", 71}),
                // is comparable
                std::vector<bool>{true,true,true,true,true,true,true,true,true,false,true,true,true,true,true,true,true,true,true},
                std::vector<std::string>{
                    "int8_t",
                    "int16_t",
                    "int32_t",
                    "int64_t",
                    "uint8_t",
                    "uint16_t",
                    "uint32_t",
                    "uint64_t",
                    "double",
                    "bool",
                    "std_string",
                    "char_ptr",
                    "std_vector__char",
                    "VariantList_t__int",
                    "VariantList_t__str",
                    "VariantVector_t__bool",
                    "VariantMap_t__int_str",
                    "VariantPair_t__int_str",
                    "CustomType"
                },
                std::vector<Variant::Type>{
                    Variant::Type::BYTE_1,
                    Variant::Type::BYTE_2,
                    Variant::Type::BYTE_4,
                    Variant::Type::BYTE_8,
                    Variant::Type::UBYTE_1,
                    Variant::Type::UBYTE_2,
                    Variant::Type::UBYTE_4,
                    Variant::Type::UBYTE_8,
                    Variant::Type::DOUBLE,
                    Variant::Type::BOOL,
                    Variant::Type::STRING,
                    Variant::Type::STRING,
                    Variant::Type::BYTEARRAY,
                    Variant::Type::LIST,
                    Variant::Type::LIST,
                    Variant::Type::VECTOR,
                    Variant::Type::MAP,
                    Variant::Type::PAIR,
                    Variant::Type::CUSTOM
                },
                std::make_tuple(
                    [](const Variant& v) { return v.isSignedNumeric(); },
                    [](const Variant& v) { return v.isSignedNumeric(); },
                    [](const Variant& v) { return v.isSignedNumeric(); },
                    [](const Variant& v) { return v.isSignedNumeric(); },
                    [](const Variant& v) { return v.isUnsignedNumeric(); },
                    [](const Variant& v) { return v.isUnsignedNumeric(); },
                    [](const Variant& v) { return v.isUnsignedNumeric(); },
                    [](const Variant& v) { return v.isUnsignedNumeric(); },
                    [](const Variant& v) { return v.isNumeric(); },
                    [](const Variant& v) { return v.isBool(); },
                    [](const Variant& v) { return v.isString(); },
                    [](const Variant& v) { return v.isString(); },
                    [](const Variant& v) { return v.isByteArray(); },
                    [](const Variant& v) { return v.isList(); },
                    [](const Variant& v) { return v.isList(); },
                    [](const Variant& v) { return v.isVector(); },
                    [](const Variant& v) { return v.isMap(); },
                    [](const Variant& v) { return v.isPair(); },
                    [](const Variant& v) { return v.isCustomType(); }
                ),
                std::make_tuple(
                    [](const Variant& v, const int8_t& expected) { EXPECT_EQ(v.toInt64(), expected); },
                    [](const Variant& v, const int16_t& expected) { EXPECT_EQ(v.toInt64(), expected); },
                    [](const Variant& v, const int32_t& expected) { EXPECT_EQ(v.toInt64(), expected); },
                    [](const Variant& v, const int64_t& expected) { EXPECT_EQ(v.toInt64(), expected); },
                    [](const Variant& v, const uint8_t& expected) { EXPECT_EQ(v.toUInt64(), expected); },
                    [](const Variant& v, const uint16_t& expected) { EXPECT_EQ(v.toUInt64(), expected); },
                    [](const Variant& v, const uint32_t& expected) { EXPECT_EQ(v.toUInt64(), expected); },
                    [](const Variant& v, const uint64_t& expected) { EXPECT_EQ(v.toUInt64(), expected); },
                    [](const Variant& v, const double& expected) { EXPECT_DOUBLE_EQ(v.toDouble(), expected); },
                    [](const Variant& v, const bool& expected) { EXPECT_EQ(v.toBool(), expected); },
                    [](const Variant& v, const std::string& expected) { EXPECT_EQ(v.toString(), expected); },
                    [](const Variant& v, const char* expected) { EXPECT_EQ(v.toString(), expected); },
                    [](const Variant& v, const ByteArray_t& expected) {
                        ByteArray_t binaryData = v.toByteArray();
                        ASSERT_EQ(binaryData.size(), expected.size());

                        for (int i = 0; i < expected.size(); ++i) {
                            EXPECT_EQ(binaryData[i], expected[i]);
                        }
                    },
                    [](const Variant& v, const VariantList_t& expected) { /*EXPECT_EQ(v.getList<int64_t>(), expected);*/ },
                    [](const Variant& v, const VariantList_t& expected) { /*EXPECT_EQ(v.getList<std::string>(), expected);*/ },
                    [](const Variant& v, const VariantVector_t& expected) { /*EXPECT_EQ(v.getVector<bool>(), expected);*/ },
                    [](const Variant& v, const VariantMap_t& expected) { /*EXPECT_EQ(v.getMap<int64_t, std::string>(), expected);*/ },
                    [](const Variant& v, const VariantPair_t& expected) { /*EXPECT_EQ(v.getMap<int64_t, std::string>(), expected);*/ },
                    [](const Variant& v, const CustomType& expected) {
                        auto vData = v.getCustomType<CustomType>();

                        if (vData) {
                            EXPECT_EQ(vData->a, expected.a);
                            EXPECT_EQ(vData->b, expected.b);
                        }
                    }
                )
            );
constexpr int allTypeValuesSize = std::tuple_size<std::tuple_element<0, decltype(allTypeValues)>::type>::value;

// =================================================================================================================
constexpr int typeValidationArgsSize = 11;
const auto typeValidationArgs = std::make_tuple(
    std::vector<std::function<bool(const Variant&)>> {
        [](const Variant& v){ return v.isNumeric(); },
        [](const Variant& v){ return v.isSignedNumeric(); },
        [](const Variant& v){ return v.isUnsignedNumeric(); },
        [](const Variant& v){ return v.isBool(); },
        [](const Variant& v){ return v.isString(); },
        [](const Variant& v){ return v.isByteArray(); },
        [](const Variant& v){ return v.isVector(); },
        [](const Variant& v){ return v.isList(); },
        [](const Variant& v){ return v.isMap(); },
        [](const Variant& v){ return v.isPair(); },
        [](const Variant& v){ return v.isCustomType(); }
    },
    std::vector<std::string> {
        "isNumeric",
        "isSignedNumeric",
        "isUnsignedNumeric",
        "isBool",
        "isString",
        "isByteArray",
        "isVector",
        "isList",
        "isMap",
        "isPair",
        "isCustomType"
    },
    std::vector<std::vector<Variant::Type>>{
        {// isNumeric
            Variant::Type::BYTE_1,
            Variant::Type::BYTE_2,
            Variant::Type::BYTE_4,
            Variant::Type::BYTE_8,
            Variant::Type::UBYTE_1,
            Variant::Type::UBYTE_2,
            Variant::Type::UBYTE_4,
            Variant::Type::UBYTE_8,
            Variant::Type::DOUBLE
        },
        {// isSignedNumeric
            Variant::Type::BYTE_1,
            Variant::Type::BYTE_2,
            Variant::Type::BYTE_4,
            Variant::Type::BYTE_8,
            Variant::Type::DOUBLE
        },
        {// isUnsignedNumeric
            Variant::Type::UBYTE_1,
            Variant::Type::UBYTE_2,
            Variant::Type::UBYTE_4,
            Variant::Type::UBYTE_8
        },
        { Variant::Type::BOOL },
        { Variant::Type::STRING },
        { Variant::Type::BYTEARRAY },
        { Variant::Type::VECTOR },
        { Variant::Type::LIST },
        { Variant::Type::MAP },
        { Variant::Type::PAIR },
        { Variant::Type::CUSTOM }
    });

// =================================================================================================================
constexpr int gIndexConversionFunc = 0;
constexpr int gIndexConversionName = 1;
constexpr int gIndexConversionValues = 2;
constexpr int gIndexConversionExpected = 3;

const auto typeConversionArgs = std::make_tuple(
    // methods to validate
    std::make_tuple(
        [](const Variant& v){ return v.toInt64(); },
        [](const Variant& v){ return v.toUInt64(); },
        [](const Variant& v){ return v.toDouble(); },
        [](const Variant& v){ return v.toBool(); },
        [](const Variant& v){ return v.toString(); },
        [](const Variant& v){ return v.toByteArray(); },
        [](const Variant& v){ return v.getVector(); },
        [](const Variant& v){ return v.getList(); },
        [](const Variant& v){ return v.getMap(); },
        [](const Variant& v){ return v.getPair(); }
    ),
    std::vector<std::string> {
        "toInt64",
        "toUInt64",
        "toDouble",
        "toBool",
        "toString",
        "toByteArray",
        "getVector",
        "getList",
        "getMap",
        "getPair"
    },
    // test data
    std::vector<Variant> {
        Variant(static_cast<int8_t>(0)),
        Variant(i8),
        Variant(i16),
        Variant(i32),
        Variant(i64),
        Variant(ui8),
        Variant(ui16),
        Variant(ui32),
        Variant(ui64),
        Variant(d),
        Variant(b),
        Variant("-17"),
        Variant("true"),
        Variant("True"),
        Variant(ByteArray_t{0x61, 0x00, 0x62, 0x00}),
        Variant(listStr),
        Variant(Variant::make(std::vector<int16_t>{1, 2, 3})),
        Variant(mapIntStr),
        Variant(pairIntStr),
        Variant(customTypeValue)},
    // expected values after conversion
    std::vector<std::function<bool(const int, const void*)>> {
        [](const int valueIndex, const void* convertedValuePtr) {
            const int64_t convertedValue = *reinterpret_cast<const int64_t*>(convertedValuePtr);
            const std::vector<int64_t> expected{ 0, i8, i16, i32, i64, ui8, ui16, ui32, ui64, static_cast<uint64_t>(d), b, -17, 0, 0, 0, 0, 0, 0, 0, 0 };
            const bool res = ((valueIndex < expected.size()) ? (expected[valueIndex] == convertedValue) : false);

            if (false == res) {
                printf("ERROR: [%d] expected (%" PRId64 ") but got (%" PRId64 ")\n", valueIndex, expected[valueIndex], convertedValue);
            }

            return res;
        },
        [](const int valueIndex, const void* convertedValuePtr) {
            const uint64_t convertedValue = *reinterpret_cast<const int64_t*>(convertedValuePtr);
            const std::vector<uint64_t> expected{0,
                                                 static_cast<uint64_t>(i8),
                                                 static_cast<uint64_t>(i16),
                                                 static_cast<uint64_t>(i32),
                                                 static_cast<uint64_t>(i64),
                                                 ui8, ui16, ui32, ui64,
                                                 static_cast<uint64_t>(d),
                                                 static_cast<uint64_t>(b),
                                                 static_cast<uint64_t>(-17), 0, 0,
                                                 0, 0, 0, 0, 0, 0 };
            const bool res = ((valueIndex < expected.size()) ? (expected[valueIndex] == convertedValue) : false);

            if (false == res) {
                printf("ERROR: [%d] expected (%" PRIu64 ") but got (%" PRIu64 ")\n", valueIndex, expected[valueIndex], convertedValue);
            }

            return res;
        },
        [](const int valueIndex, const void* convertedValuePtr) {
            const double convertedValue = *reinterpret_cast<const double*>(convertedValuePtr);
            const std::vector<double> expected{ 0.0, i8, i16, i32, i64, ui8, ui16, ui32, ui64, d, b, -17.0, 0, 0, 0, 0, 0, 0, 0, 0 };
            const bool res = ((valueIndex < expected.size()) ? (std::abs(expected[valueIndex] - convertedValue) < 0.00001) : false);

            if (false == res) {
                printf("ERROR: [%d] expected (%f) but got (%f)\n", valueIndex, expected[valueIndex], convertedValue);
            }

            return res;
        },
        [](const int valueIndex, const void* convertedValuePtr) {
            const bool convertedValue = *reinterpret_cast<const bool*>(convertedValuePtr);
            const std::vector<bool> expected{ false, true, true, true, true, true, true, true, true, true, b, true, true, false, false, false, false, false, false, false };
            const bool res = ((valueIndex < expected.size()) ? (expected[valueIndex] == convertedValue) : false);

            if (false == res) {
                printf("ERROR: [%d] expected (%s) but got (%s)\n", valueIndex, BOOL2STR(expected[valueIndex]), BOOL2STR(convertedValue));
            }

            return res;
        },
        [](const int valueIndex, const void* convertedValuePtr) {
            const std::string convertedValue = *reinterpret_cast<const std::string*>(convertedValuePtr);
            const std::vector<std::string> expected{
                "0",
                std::to_string(i8),
                std::to_string(i16),
                std::to_string(i32),
                std::to_string(i64),
                std::to_string(ui8),
                std::to_string(ui16),
                std::to_string(ui32),
                std::to_string(ui64),
                std::to_string(d),
                BOOL2STR(b),
                "-17",
                "true",
                "True",
                std::string("a\0b", 4),
                "aa, bb, cc",
                "1, 2, 3",
                "1=[aa], 2=[bb], 3=[cc]",
                "(7, ab)",
                ""};
            const bool res = ((valueIndex < expected.size()) ? (expected[valueIndex] == convertedValue) : false);

            if (false == res) {
                printf("ERROR: [%d] expected \"%s\" (%zu) but got \"%s\" (%zu)\n", valueIndex, expected[valueIndex].c_str(), expected[valueIndex].size(), convertedValue.c_str(), convertedValue.size());
            }

            return res;
        },
        [](const int valueIndex, const void* convertedValuePtr) {
            const ByteArray_t convertedValue = *reinterpret_cast<const ByteArray_t*>(convertedValuePtr);
            std::vector<ByteArray_t> expected{
                {0x00},
                {0xF8},
                {0xF0, 0xFF},
                {0xE0, 0xFF, 0xFF, 0xFF},
                {0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                {0x08},
                {0x10, 0x00},
                {0x20, 0x00, 0x00, 0x00},
                {0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                {0xCD, 0xCC, 0xCC, 0xCC, 0xCC, 0xDC, 0x5E, 0x40},// double
                {0x00},// bool
                {0x2D, 0x31, 0x37},// str
                {0x74, 0x72, 0x75, 0x65},// str
                {0x54, 0x72, 0x75, 0x65},// str
                {0x61, 0x00, 0x62, 0x00},// bytes
                {0x61,0x61, 0x62,0x62, 0x63,0x63},// list<str>
                {0x01,0x00, 0x02,0x00, 0x03,0x00},// vector<int16>
                {},// map
                {0x07,0x00,0x00,0x00, 0x61,0x62},// pair<int32, str>
                {},
            };
            bool res = false;

            if (valueIndex < expected.size()) {
                res = (expected[valueIndex] == convertedValue);

                if (false == res) {
                    printf("ERROR: [%d] Expected (", valueIndex);
                    std::for_each(expected[valueIndex].begin(), expected[valueIndex].end(), [](char c){ printf("%02X ", 0x000000FF & static_cast<int>(c)); });
                    printf(") but got (");
                    std::for_each(convertedValue.begin(), convertedValue.end(), [](char c){ printf("%02X ", 0x000000FF & static_cast<int>(c)); });
                    printf(")\n");
                }
            }

            return res;
        },
        [](const int valueIndex, const void* convertedValuePtr) {
            const std::shared_ptr<VariantVector_t> convertedValue = *reinterpret_cast<const std::shared_ptr<VariantVector_t>*>(convertedValuePtr);
            std::vector<bool> expected{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false };
            bool res = false;

            if (valueIndex < expected.size()) {
                res = ((expected[valueIndex] == true) ? (nullptr != convertedValue) : (nullptr == convertedValue));
                if (false == res) {
                    printf("ERROR: [%d] expected empty (%s)\n", valueIndex, BOOL2STR(expected[valueIndex]));
                }
            }

            return res;
        },
        [](const int valueIndex, const void* convertedValuePtr) {
            const std::shared_ptr<VariantList_t> convertedValue = *reinterpret_cast<const std::shared_ptr<VariantList_t>*>(convertedValuePtr);
            std::vector<bool> expected{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false };
            bool res = false;

            if (valueIndex < expected.size()) {
                res = ((expected[valueIndex] == true) ? (nullptr != convertedValue) : (nullptr == convertedValue));
                if (false == res) {
                    printf("ERROR: [%d] expected empty (%s)\n", valueIndex, BOOL2STR(expected[valueIndex]));
                }
            }

            return res;
        },
        [](const int valueIndex, const void* convertedValuePtr) {
            const std::shared_ptr<VariantMap_t> convertedValue = *reinterpret_cast<const std::shared_ptr<VariantMap_t>*>(convertedValuePtr);
            std::vector<bool> expected{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false };
            bool res = false;

            if (valueIndex < expected.size()) {
                res = ((expected[valueIndex] == true) ? (nullptr != convertedValue) : (nullptr == convertedValue));
                if (false == res) {
                    printf("ERROR: [%d] expected empty (%s)\n", valueIndex, BOOL2STR(expected[valueIndex]));
                }
            }

            return res;
        },
        [](const int valueIndex, const void* convertedValuePtr) {
            const std::shared_ptr<VariantPair_t> convertedValue = *reinterpret_cast<const std::shared_ptr<VariantPair_t>*>(convertedValuePtr);
            std::vector<bool> expected{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false };
            bool res = false;

            if (valueIndex < expected.size()) {
                res = ((expected[valueIndex] == true) ? (nullptr != convertedValue) : (nullptr == convertedValue));
                if (false == res) {
                    printf("ERROR: [%d] expected empty (%s)\n", valueIndex, BOOL2STR(expected[valueIndex]));
                }
            }

            return res;
        },
    }
);

constexpr int typeConversionArgsSize = std::tuple_size<std::tuple_element<gIndexConversionFunc, decltype(typeConversionArgs)>::type>::value;

bool convertAndValidateValue(const int funcIndex, const Variant& v, const int valueIndex) {
    #define CONVERTANDVALIDATEVALUE_CASE(i)        case i:                                                          \
                                                   {                                                                \
                                                       const auto convertFunc = std::get<i>(convertFuncsList);      \
                                                       const auto convertedValue = convertFunc(v);                  \
                                                       return validateFunc(valueIndex, &convertedValue);            \
                                                   }

    const auto& convertFuncsList = std::get<gIndexConversionFunc>(typeConversionArgs);
    const auto validateFunc = std::get<gIndexConversionExpected>(typeConversionArgs)[funcIndex];

    switch(funcIndex) {
        CONVERTANDVALIDATEVALUE_CASE(0)
        CONVERTANDVALIDATEVALUE_CASE(1)
        CONVERTANDVALIDATEVALUE_CASE(2)
        CONVERTANDVALIDATEVALUE_CASE(3)
        CONVERTANDVALIDATEVALUE_CASE(4)
        CONVERTANDVALIDATEVALUE_CASE(5)
        CONVERTANDVALIDATEVALUE_CASE(6)
        CONVERTANDVALIDATEVALUE_CASE(7)
        CONVERTANDVALIDATEVALUE_CASE(8)
        CONVERTANDVALIDATEVALUE_CASE(9)
    }

    return false;
}

// =================================================================================================================
class FixtureVariantAllTypes : public testing::Test, public testing::WithParamInterface<int> {};

INSTANTIATE_TEST_CASE_P(variant,
                        FixtureVariantAllTypes,
                        ::testing::Range<int>(0, allTypeValuesSize),
                        [](const ::testing::TestParamInfo<FixtureVariantAllTypes::ParamType>& info) {
                            return getTypeNameFromTuple(info.param, allTypeValues);
                        });

class FixtureVariantTypeCheck : public testing::Test, public testing::WithParamInterface<int> {};

INSTANTIATE_TEST_CASE_P(variant,
                        FixtureVariantTypeCheck,
                        ::testing::Range<int>(0, typeValidationArgsSize),
                        [](const ::testing::TestParamInfo<FixtureVariantAllTypes::ParamType>& info) {
                            return std::get<1>(typeValidationArgs)[info.param];
                        });

class FixtureVariantTypeConversion : public testing::Test, public testing::WithParamInterface<int> {};

INSTANTIATE_TEST_CASE_P(variant,
                        FixtureVariantTypeConversion,
                        ::testing::Range<int>(0, typeConversionArgsSize),
                        [](const ::testing::TestParamInfo<FixtureVariantAllTypes::ParamType>& info) {
                            return std::get<gIndexConversionName>(typeConversionArgs)[info.param];
                        });


// =================================================================================================================
TEST_P(FixtureVariantAllTypes, constructors) {
    TEST_DESCRIPTION("variant constructors should support all basic types");

    //-------------------------------------------
    // PRECONDITIONS
    const int valueId = GetParam();
    const auto values = std::get<gIndexValue>(allTypeValues);

    //-------------------------------------------
    // ACTIONS
    Variant v(constructVariantFromTuple(valueId, values));

    EXPECT_TRUE(isCorrectType(valueId, v, std::get<gIndexFuncTypeCheck>(allTypeValues)));
    EXPECT_EQ(v.getType(), getTypeFromTuple(valueId, allTypeValues));
    checkVariantValue(valueId, v, std::get<gIndexFuncValueCheck>(allTypeValues), values);
}

TEST_P(FixtureVariantAllTypes, make) {
    TEST_DESCRIPTION("it should be possible to create variant object using make() function");

    //-------------------------------------------
    // PRECONDITIONS
    const int valueId = GetParam();
    const auto values = std::get<gIndexValue>(allTypeValues);

    //-------------------------------------------
    // ACTIONS
    Variant v(makeVariantFromTuple(valueId, values));

    EXPECT_TRUE(isCorrectType(valueId, v, std::get<gIndexFuncTypeCheck>(allTypeValues)));
    checkVariantValue(valueId, v, std::get<gIndexFuncValueCheck>(allTypeValues), values);
}

TEST_P(FixtureVariantAllTypes, copy) {
    TEST_DESCRIPTION("variant objects should be copiable");

    //-------------------------------------------
    // PRECONDITIONS
    const int valueId = GetParam();
    const auto values = std::get<gIndexValue>(allTypeValues);
    Variant v(constructVariantFromTuple(valueId, values));
    Variant vCopy;

    //-------------------------------------------
    // ACTIONS
    vCopy = v;

    EXPECT_TRUE(isCorrectType(valueId, vCopy, std::get<gIndexFuncTypeCheck>(allTypeValues)));
    checkVariantValue(valueId, vCopy, std::get<gIndexFuncValueCheck>(allTypeValues), values);
}

TEST(variant, copy_constructor) {
    TEST_DESCRIPTION("create variant object from another object");

    //-------------------------------------------
    // PRECONDITIONS
    Variant v1(7);

    //-------------------------------------------
    // ACTIONS
    Variant v2(v1);
    Variant v3 = Variant::make(v1);

    //-------------------------------------------
    // VALIDATION
    ASSERT_EQ(v1, v2);
    ASSERT_EQ(v1, v3);
}

TEST_P(FixtureVariantAllTypes, move) {
    TEST_DESCRIPTION("variant objects should be movable");

    //-------------------------------------------
    // PRECONDITIONS
    const int valueId = GetParam();
    const auto values = std::get<gIndexValue>(allTypeValues);
    Variant v(constructVariantFromTuple(valueId, values));

    //-------------------------------------------
    // ACTIONS
    Variant vNew;

    vNew = std::move(v);

    //-------------------------------------------
    // VALIDATION
    // check that new object took over ownership of the memory
    EXPECT_TRUE(v.isEmpty());
    EXPECT_FALSE(vNew.isEmpty());

    EXPECT_TRUE(isCorrectType(valueId, vNew, std::get<gIndexFuncTypeCheck>(allTypeValues)));
    checkVariantValue(valueId, vNew, std::get<gIndexFuncValueCheck>(allTypeValues), values);
}

TEST(variant, move_ownership) {
    TEST_DESCRIPTION("validate memory ownership after move operation");

    //-------------------------------------------
    // PRECONDITIONS
    Variant v(mapIntStr);
    const void* vDataPtr = v.getMap().get();

    //-------------------------------------------
    // ACTIONS
    Variant vNew;

    vNew = std::move(v);

    //-------------------------------------------
    // VALIDATION
    // check that new object took over ownership of the memory
    EXPECT_TRUE(v.isEmpty());
    EXPECT_FALSE(vNew.isEmpty());
    EXPECT_EQ(vNew.getMap().get(), vDataPtr);
    EXPECT_EQ(v.getMap().get(), nullptr);
}

TEST_P(FixtureVariantAllTypes, assign) {
    TEST_DESCRIPTION("it should be possible to assign new value to Variant object");

    //-------------------------------------------
    // PRECONDITIONS
    const int valueId = GetParam();
    const auto values = std::get<gIndexValue>(allTypeValues);

    //-------------------------------------------
    // ACTIONS
    Variant v(1);

    assignNewVariantValue(v, valueId, values);

    EXPECT_TRUE(isCorrectType(valueId, v, std::get<gIndexFuncTypeCheck>(allTypeValues)));
    checkVariantValue(valueId, v, std::get<gIndexFuncValueCheck>(allTypeValues), values);
}

TEST_P(FixtureVariantAllTypes, compare) {
    TEST_DESCRIPTION("validate ==, <, >, <=, >= operators");

    //-------------------------------------------
    // PRECONDITIONS
    const int valueId = GetParam();
    Variant v(constructVariantFromTuple(valueId, std::get<gIndexValue>(allTypeValues)));

    //-------------------------------------------
    // ACTIONS
    Variant vEqual(v);
    Variant vLess(constructVariantFromTuple(valueId, std::get<gIndexValueLess>(allTypeValues)));
    Variant vGreater(constructVariantFromTuple(valueId, std::get<gIndexValueGreater>(allTypeValues)));

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(v, vEqual);

    if (isComparableType(valueId, allTypeValues)) {
        EXPECT_LT(v, vGreater);
        EXPECT_GT(v, vLess);

        EXPECT_LE(v, vGreater);
        EXPECT_GE(v, vLess);

        EXPECT_LE(v, vEqual);
        EXPECT_GE(v, vEqual);
    }
}

TEST(variant, compare_different_types) {
    TEST_DESCRIPTION("validate that different numeric types are comparable");

    //-------------------------------------------
    // PRECONDITIONS

    //-------------------------------------------
    // ACTIONS

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(Variant(int8_t(7)), Variant(uint32_t(7)));
    EXPECT_GT(Variant(int8_t(17)), Variant(uint32_t(3)));
    EXPECT_LT(Variant(int8_t(3)), Variant(uint32_t(17)));
    EXPECT_EQ(Variant(double(7.0)), Variant(uint32_t(7)));

    EXPECT_NE(Variant("7"), Variant(uint32_t(7)));
}

TEST(variant, variadic_arguments) {
    TEST_DESCRIPTION("validate that initializing variant arguments through variadic arguments works fine");

    //-------------------------------------------
    // PRECONDITIONS

    //-------------------------------------------
    // ACTIONS
    VariantVector_t result;
    VariantVector_t resultByValue;

    makeVariantList(result, i8, i16, i32, i64, ui8, ui16, ui32, ui64, d, b, s1, s2, listInt, vectorBool, mapIntStr, customTypeValue);
    makeVariantList(resultByValue, 1, 2.3, "aaa", false, true);

    //-------------------------------------------
    // VALIDATION
    int i = 0;

    EXPECT_TRUE(result[i++].isSignedNumeric());
    EXPECT_TRUE(result[i++].isSignedNumeric());
    EXPECT_TRUE(result[i++].isSignedNumeric());
    EXPECT_TRUE(result[i++].isSignedNumeric());
    EXPECT_TRUE(result[i++].isUnsignedNumeric());
    EXPECT_TRUE(result[i++].isUnsignedNumeric());
    EXPECT_TRUE(result[i++].isUnsignedNumeric());
    EXPECT_TRUE(result[i++].isUnsignedNumeric());
    EXPECT_TRUE(result[i++].isNumeric());
    EXPECT_TRUE(result[i++].isBool());
    EXPECT_TRUE(result[i++].isString());
    EXPECT_TRUE(result[i++].isString());
    EXPECT_TRUE(result[i++].isList());
    EXPECT_TRUE(result[i++].isVector());
    EXPECT_TRUE(result[i++].isMap());
    EXPECT_TRUE(result[i++].isCustomType());

    i = 0;
    EXPECT_EQ(result[i++].toInt64(), i8);
    EXPECT_EQ(result[i++].toInt64(), i16);
    EXPECT_EQ(result[i++].toInt64(), i32);
    EXPECT_EQ(result[i++].toInt64(), i64);
    EXPECT_EQ(result[i++].toUInt64(), ui8);
    EXPECT_EQ(result[i++].toUInt64(), ui16);
    EXPECT_EQ(result[i++].toUInt64(), ui32);
    EXPECT_EQ(result[i++].toUInt64(), ui64);
    EXPECT_EQ(result[i++].toDouble(), d);
    EXPECT_EQ(result[i++].toBool(), b);
    EXPECT_EQ(result[i++].toString(), s1);
    EXPECT_STREQ(result[i++].toString().c_str(), s2);
    ++i; // listInt
    ++i; // vectorBool
    ++i; // mapIntStr
    EXPECT_EQ(result[i++].getCustomType<CustomType>()->a, customTypeValue.a);

    i = 0;
    EXPECT_TRUE(resultByValue[i++].isNumeric());
    EXPECT_TRUE(resultByValue[i++].isNumeric());
    EXPECT_TRUE(resultByValue[i++].isString());
    EXPECT_TRUE(resultByValue[i++].isBool());
    EXPECT_TRUE(resultByValue[i++].isBool());
}

TEST(variant, empty) {
    TEST_DESCRIPTION("variant object can be empty and not contain any data");

    //-------------------------------------------
    // PRECONDITIONS
    Variant v;

    //-------------------------------------------
    // ACTIONS
    ASSERT_TRUE(v.isEmpty());
    ASSERT_EQ(v.getMap(), nullptr);

    v = mapIntStr;
    ASSERT_TRUE(v);
    ASSERT_FALSE(v.isEmpty());
    ASSERT_NE(v.getMap().get(), nullptr);
    v.clear();

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(v.isEmpty());
    ASSERT_EQ(v.getMap(), nullptr);
}

TEST(variant, copy_unknown) {
    TEST_DESCRIPTION("assigning not initialized variant object should reset current object");

    //-------------------------------------------
    // PRECONDITIONS
    Variant vUnknown;
    Variant v(mapIntStr);

    ASSERT_TRUE(vUnknown.isEmpty());
    ASSERT_TRUE(v.isMap());

    //-------------------------------------------
    // ACTIONS
    v = vUnknown;

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(v.isEmpty());
    ASSERT_EQ(v.getMap().get(), nullptr);
}

TEST(variant, compare_self) {
    TEST_DESCRIPTION("comparing variant to self should return true");

    //-------------------------------------------
    // PRECONDITIONS
    Variant v(i8);

    //-------------------------------------------
    // ACTIONS

    //-------------------------------------------
    // VALIDATION
    ASSERT_EQ(v, v);
}

TEST_P(FixtureVariantTypeConversion, convert) {
    TEST_DESCRIPTION("Variant should support type conversions");

    //-------------------------------------------
    // PRECONDITIONS
    const int indexType = GetParam();
    const auto& values = std::get<gIndexConversionValues>(typeConversionArgs);

    //-------------------------------------------
    // ACTIONS
    for (int i = 0 ; i < values.size(); ++i) {
        EXPECT_TRUE(convertAndValidateValue(indexType, Variant(values[i]), i));
    }

    //-------------------------------------------
    // VALIDATION
}

TEST_P(FixtureVariantTypeConversion, convert_invalid_string) {
    TEST_DESCRIPTION("trying to convert from invalid string to a number should return 0");

    //-------------------------------------------
    // PRECONDITIONS
    Variant v1("abc");
    Variant v2("z17.a8");
    Variant v3("");

    //-------------------------------------------
    // ACTIONS

    //-------------------------------------------
    // VALIDATION
    EXPECT_EQ(v1.toInt64(), 0);
    EXPECT_EQ(v2.toInt64(), 0);
    EXPECT_EQ(v3.toInt64(), 0);

    EXPECT_EQ(v1.toUInt64(), 0);
    EXPECT_EQ(v2.toUInt64(), 0);
    EXPECT_EQ(v3.toUInt64(), 0);

    EXPECT_DOUBLE_EQ(v1.toDouble(), 0.0);
    EXPECT_DOUBLE_EQ(v2.toDouble(), 0.0);
    EXPECT_DOUBLE_EQ(v3.toDouble(), 0.0);
}

TEST_P(FixtureVariantTypeCheck, isXXX) {
    TEST_DESCRIPTION("validate isXXX() methods for all types");

    //-------------------------------------------
    // PRECONDITIONS
    const int typeIndex = GetParam();
    Variant v;
    auto isXxxType = std::get<0>(typeValidationArgs)[typeIndex];
    const auto& supportedTypes = std::get<2>(typeValidationArgs)[typeIndex];

    //-------------------------------------------
    // ACTIONS
    for (int i = 0 ; i < allTypeValuesSize; ++i) {
        v = makeVariantFromTuple(i, std::get<gIndexValue>(allTypeValues));

        if (supportedTypes.end() != std::find(supportedTypes.begin(), supportedTypes.end(), v.getType())) {
            EXPECT_TRUE(isXxxType(v));
        } else {
            EXPECT_FALSE(isXxxType(v));
        }
    }

    //-------------------------------------------
    // VALIDATION
}

TEST(variant, bytearray) {
    TEST_DESCRIPTION("validate custom bytearray methods");

    //-------------------------------------------
    // PRECONDITIONS

    //-------------------------------------------
    // ACTIONS
    Variant v0 = Variant::make(binary1, sizeof(binary1));
    Variant v1 = Variant(binary1, sizeof(binary1));

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(v0.isByteArray());
    ASSERT_TRUE(v1.isByteArray());

    EXPECT_EQ(v0, v1);

    // check that getByteArray() does not reallocate memory
    ASSERT_NE(v1.getByteArray().get(), nullptr);
    EXPECT_EQ(v1.getByteArray().get(), v1.getByteArray().get());

    // check that nullptr is returned if type is different from ByteArray
    v1 = 1;
    EXPECT_EQ(v1.getByteArray().get(), nullptr);
}

TEST(variant, vector) {
    TEST_DESCRIPTION("validate support for Vector type");

    //-------------------------------------------
    // PRECONDITIONS
    std::vector<int> emptyVector;
    std::vector<int> intVector = {1, 2, 3};
    std::vector<std::string> strVector = {"aa", "bb", "cc"};

    //-------------------------------------------
    // ACTIONS
    Variant v0 = Variant::make(emptyVector);
    Variant v1 = Variant::make(intVector);
    Variant v2 = Variant::make(strVector);

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(v0.isVector());
    EXPECT_TRUE(v1.isVector());
    EXPECT_TRUE(v2.isVector());

    ASSERT_NE(v0.getVector().get(), nullptr);
    EXPECT_TRUE(v0.getVector()->empty());

    auto v1Ptr = v1.getVector();
    auto v2Ptr = v2.getVector();

    // check that and order values matches
    ASSERT_NE(v1Ptr.get(), nullptr);
    ASSERT_NE(v2Ptr.get(), nullptr);
    ASSERT_EQ(v1Ptr->size(), intVector.size());
    ASSERT_EQ(v2Ptr->size(), strVector.size());

    EXPECT_TRUE(std::equal(v1Ptr->begin(), v1Ptr->end(), intVector.begin(), [](const Variant& left, const int right){
        return left.toInt64() == right;
    }));

    EXPECT_TRUE(std::equal(v2Ptr->begin(), v2Ptr->end(), strVector.begin(), [](const Variant& left, const std::string& right){
        return left.toString() == right;
    }));

    // validate converting to std::vector
    EXPECT_EQ(intVector, v1.toVector<int>([](const Variant& v){ return v.toInt64(); }));
    EXPECT_EQ(strVector, v2.toVector<std::string>([](const Variant& v){ return v.toString(); }));
}

TEST(variant, list) {
    TEST_DESCRIPTION("validate support for List type");

    //-------------------------------------------
    // PRECONDITIONS
    std::list<int> emptyList;
    std::list<int> intList = {1, 2, 3};
    std::list<std::string> strList = {"aa", "bb", "cc"};

    //-------------------------------------------
    // ACTIONS
    Variant v0 = Variant::make(emptyList);
    Variant v1 = Variant::make(intList);
    Variant v2 = Variant::make(strList);

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(v0.isList());
    EXPECT_TRUE(v1.isList());
    EXPECT_TRUE(v2.isList());

    ASSERT_NE(v0.getList().get(), nullptr);
    EXPECT_TRUE(v0.getList()->empty());

    auto v1Ptr = v1.getList();
    auto v2Ptr = v2.getList();

    // check that and order values matches
    ASSERT_NE(v1Ptr.get(), nullptr);
    ASSERT_NE(v2Ptr.get(), nullptr);
    ASSERT_EQ(v1Ptr->size(), intList.size());
    ASSERT_EQ(v2Ptr->size(), strList.size());

    EXPECT_TRUE(std::equal(v1Ptr->begin(), v1Ptr->end(), intList.begin(), [](const Variant& left, const int right){
        return left.toInt64() == right;
    }));

    EXPECT_TRUE(std::equal(v2Ptr->begin(), v2Ptr->end(), strList.begin(), [](const Variant& left, const std::string& right){
        return left.toString() == right;
    }));

    // validate converting to std::list
    EXPECT_EQ(intList, v1.toList<int>([](const Variant& v){ return v.toInt64(); }));
    EXPECT_EQ(strList, v2.toList<std::string>([](const Variant& v){ return v.toString(); }));
}

TEST(variant, map) {
    TEST_DESCRIPTION("validate support for Map type");

    //-------------------------------------------
    // PRECONDITIONS
    std::map<int, std::string> mapIntStr = {{1, "aa"}, {2, "bb"}, {3, "cc"}};

    //-------------------------------------------
    // ACTIONS
    Variant v1 = Variant::make(mapIntStr);

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(v1.isMap());

    auto v1Ptr = v1.getMap();

    ASSERT_NE(v1Ptr.get(), nullptr);
    EXPECT_TRUE(std::equal(v1Ptr->begin(), v1Ptr->end(), mapIntStr.begin(), [](const std::pair<Variant,Variant>& left, const std::pair<int, std::string>& right){
        return (left.first.toInt64() == right.first) && (left.second.toString() == right.second);
    }));

    // validate converting to std::map
    EXPECT_EQ(mapIntStr, (v1.toMap<int, std::string>([](const Variant& k){ return k.toInt64(); },
                                                     [](const Variant& v){ return v.toString(); })));
}

TEST(variant, pair_conversion) {
    TEST_DESCRIPTION("validate converting pair value to custom std::pair object");

    //-------------------------------------------
    // PRECONDITIONS
    const std::pair<int, std::string> intStrPair = std::make_pair(7, "abc");

    //-------------------------------------------
    // ACTIONS
    Variant v0 = Variant(intStrPair.first, intStrPair.second);

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(v0.isPair());

    // validate converting to std::pair
    EXPECT_EQ(intStrPair, (v0.toPair<int, std::string>([](const Variant& first){ return first.toInt64(); },
                                                       [](const Variant& second){ return second.toString(); })));
}

TEST(variant, pair_constructors) {
    TEST_DESCRIPTION("validate custom pair constructors");

    //-------------------------------------------
    // PRECONDITIONS
    const std::pair<int, std::string> intStrPair = std::make_pair(7, "abc");
    VariantPair_t intStrPairVariant = std::make_pair(Variant(intStrPair.first), Variant(intStrPair.second));

    //-------------------------------------------
    // ACTIONS
    Variant v0 = Variant(intStrPairVariant);// from VariantPair_t
    Variant v1 = Variant(intStrPair.first, intStrPair.second);// from int, str
    Variant v2 = Variant(intStrPairVariant.first, intStrPairVariant.second);// from Variant, Variant
    Variant v3 = Variant::make(intStrPairVariant);// from VariantPair_t
    Variant v4 = Variant::make(intStrPair.first, intStrPair.second);// from int, str
    Variant v5 = Variant::make(intStrPairVariant.first, intStrPairVariant.second);// from Variant, Variant

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(v0.isPair());
    ASSERT_TRUE(v1.isPair());
    ASSERT_TRUE(v2.isPair());
    ASSERT_TRUE(v3.isPair());
    ASSERT_TRUE(v4.isPair());
    ASSERT_TRUE(v5.isPair());

    EXPECT_EQ(v0, v1);
    EXPECT_EQ(v1, v2);
    EXPECT_EQ(v2, v3);
    EXPECT_EQ(v3, v4);
    EXPECT_EQ(v4, v5);
}

TEST(variant, customtype_copy) {
    TEST_DESCRIPTION("it should be possible to copy variant instance which contains a value of a custom type");

    //-------------------------------------------
    // PRECONDITIONS
    Variant v1 = Variant::make(customTypeValue);

    //-------------------------------------------
    // ACTIONS
    Variant v2(v1);
    Variant v3;

    v3 = v1;

    //-------------------------------------------
    // VALIDATION

    // should be not null
    ASSERT_NE(v1.getCustomType<CustomType>(), nullptr);
    ASSERT_NE(v2.getCustomType<CustomType>(), nullptr);
    ASSERT_NE(v3.getCustomType<CustomType>(), nullptr);

    // should have their own memory allocated
    ASSERT_NE(v1.getCustomType<CustomType>(), v2.getCustomType<CustomType>());
    ASSERT_NE(v1.getCustomType<CustomType>(), v3.getCustomType<CustomType>());
    ASSERT_NE(v2.getCustomType<CustomType>(), v3.getCustomType<CustomType>());

    // should have same values
    ASSERT_EQ(v1.getCustomType<CustomType>()->a, v2.getCustomType<CustomType>()->a);
    ASSERT_EQ(v1.getCustomType<CustomType>()->a, v3.getCustomType<CustomType>()->a);
}

TEST(variant, customtype_memory_management) {
    TEST_DESCRIPTION("validate memory management when dealing with custom types");

    //-------------------------------------------
    // PRECONDITIONS
    bool wasObject1Deleted = false;
    bool wasObject2Deleted = false;
    bool wasObject3Deleted = false;

    //-------------------------------------------
    // ACTIONS
    customTypeValue.wasDeleted = &wasObject1Deleted;
    Variant v1 = Variant::make(customTypeValue);

    ASSERT_NE(v1.getCustomType<CustomType>(), nullptr);

    customTypeValue.wasDeleted = &wasObject2Deleted;
    Variant v2(customTypeValue);

    customTypeValue.wasDeleted = nullptr;
    ASSERT_NE(v2.getCustomType<CustomType>(), nullptr);

    Variant v3;

    // assign operator test
    v3 = v2;
    ASSERT_NE(v3.getCustomType<CustomType>(), nullptr);
    v3.getCustomType<CustomType>()->wasDeleted = &wasObject3Deleted;

    // check that all containers have their own unique memory allocated
    ASSERT_NE(v1.getCustomType<CustomType>(), v2.getCustomType<CustomType>());
    ASSERT_NE(v1.getCustomType<CustomType>(), v3.getCustomType<CustomType>());
    ASSERT_NE(v2.getCustomType<CustomType>(), v3.getCustomType<CustomType>());

    // at this point copy of the memory stored inside v should be released
    v1.clear();
    v2.clear();
    v3.clear();

    //-------------------------------------------
    // VALIDATION
    ASSERT_TRUE(wasObject1Deleted);
    ASSERT_TRUE(wasObject2Deleted);
    ASSERT_TRUE(wasObject3Deleted);
}

TEST(variant, customtype_move) {
    TEST_DESCRIPTION("custom type should be correctly moved without memory reallocation");

    //-------------------------------------------
    // PRECONDITIONS
    CustomType* ptr = nullptr;
    bool wasObjectDeleted = false;

    //-------------------------------------------
    // ACTIONS
    customTypeValue.wasDeleted = &wasObjectDeleted;
    Variant v1 = Variant::make(customTypeValue);

    ptr = v1.getCustomType<CustomType>().get();
    ASSERT_NE(ptr, nullptr);

    Variant v2 = std::move(v1);
    ASSERT_NE(v2.getCustomType<CustomType>(), nullptr);

    //-------------------------------------------
    // VALIDATION
    ASSERT_EQ(v2.getCustomType<CustomType>().get(), ptr);

    ASSERT_EQ(v1.getCustomType<CustomType>(), nullptr);
    ASSERT_EQ(v1.getType(), Variant::Type::UNKNOWN);

    v2.clear();
    ASSERT_TRUE(wasObjectDeleted);
}
