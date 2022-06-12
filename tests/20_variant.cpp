// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details
#include "TestsCommon.hpp"
#include "hsmcpp/variant.hpp"

// NOTE: copied from hsm.hpp
template <typename... Args>
void makeVariantList(VariantVector_t& vList, Args&&... args)
{
    volatile int make_variant[] = {0, (vList.push_back(Variant::make(std::forward<Args>(args))), 0)...};
    (void)make_variant;
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
const std::vector<char> binary2 = {4, 5, 0, 6};
const VariantList_t listInt = {Variant(1), Variant(2), Variant(3)};
const VariantList_t listStr = {Variant("aa"), Variant("bb"), Variant("cc")};
const VariantVector_t vectorBool = {Variant(false), Variant(true), Variant(false)};
const VariantDict_t dictIntStr = {{Variant(1), Variant("aa")},
                            {Variant(2), Variant("bb")},
                            {Variant(3), Variant("cc")}};

TEST(variant, vector)
{
    TEST_DESCRIPTION("");

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

    EXPECT_TRUE(v0.toVector<int>().empty());
    EXPECT_EQ(v1.toVector<int>(), intVector);
    EXPECT_EQ(v2.toVector<std::string>(), strVector);
}

TEST(variant, list)
{
    TEST_DESCRIPTION("");

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

    EXPECT_TRUE(v0.toList<int>().empty());
    EXPECT_EQ(v1.toList<int>(), intList);
    EXPECT_EQ(v2.toList<std::string>(), strList);
}

TEST(variant, map)
{
    TEST_DESCRIPTION("");

    //-------------------------------------------
    // PRECONDITIONS
    std::map<int, std::string> mapIntStr = {{1, "aa"}, {2, "bb"}, {3, "cc"}};

    //-------------------------------------------
    // ACTIONS
    Variant v1 = Variant::make(mapIntStr);

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(v1.isDictionary());

    std::map<int, std::string> mapV1 = v1.toMap<int, std::string>();
    EXPECT_EQ(mapV1, mapIntStr);
}

TEST(variant, constructors)
{
    TEST_DESCRIPTION("");

    //-------------------------------------------
    // PRECONDITIONS

    //-------------------------------------------
    // ACTIONS
    Variant vNumericI8(i8);
    Variant vNumericI16(i16);
    Variant vNumericI32(i32);
    Variant vNumericI64(i64);
    Variant vNumericUI8(ui8);
    Variant vNumericUI16(ui16);
    Variant vNumericUI32(ui32);
    Variant vNumericUI64(ui64);
    Variant vNumericDouble(d);
    Variant vBool(b);
    Variant vStr1(s1);
    Variant vStr2(s2);
    Variant vBinaryArray1(binary1, sizeof(binary1));
    Variant vBinaryArray2(binary2);
    Variant vListInt(listInt);
    Variant vListStr(listStr);
    Variant vVector(vectorBool);
    Variant vDict(dictIntStr);

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(vNumericI8.isSignedNumeric());
    EXPECT_TRUE(vNumericI16.isSignedNumeric());
    EXPECT_TRUE(vNumericI32.isSignedNumeric());
    EXPECT_TRUE(vNumericI64.isSignedNumeric());
    EXPECT_TRUE(vNumericUI8.isUnsignedNumeric());
    EXPECT_TRUE(vNumericUI16.isUnsignedNumeric());
    EXPECT_TRUE(vNumericUI32.isUnsignedNumeric());
    EXPECT_TRUE(vNumericUI64.isUnsignedNumeric());
    EXPECT_TRUE(vNumericDouble.isNumeric());
    EXPECT_TRUE(vBool.isBool());
    EXPECT_TRUE(vStr1.isString());
    EXPECT_TRUE(vStr2.isString());
    EXPECT_TRUE(vBinaryArray1.isByteArray());
    EXPECT_TRUE(vBinaryArray2.isByteArray());
    EXPECT_TRUE(vListInt.isList());
    EXPECT_TRUE(vListStr.isList());
    EXPECT_TRUE(vVector.isVector());
    EXPECT_TRUE(vDict.isDictionary());

    EXPECT_EQ(vNumericI8.toInt64(), i8);
    EXPECT_EQ(vNumericI16.toInt64(), i16);
    EXPECT_EQ(vNumericI32.toInt64(), i32);
    EXPECT_EQ(vNumericI64.toInt64(), i64);
    EXPECT_EQ(vNumericUI8.toUInt64(), ui8);
    EXPECT_EQ(vNumericUI16.toUInt64(), ui16);
    EXPECT_EQ(vNumericUI32.toUInt64(), ui32);
    EXPECT_EQ(vNumericUI64.toUInt64(), ui64);
    EXPECT_EQ(vNumericDouble.toDouble(), d);
    EXPECT_EQ(vBool.toBool(), b);
    EXPECT_EQ(vStr1.toString(), s1);
    EXPECT_STREQ(vStr2.toString().c_str(), s2);
    EXPECT_EQ(vBinaryArray2.toByteArray(), binary2);

    std::vector<char> validateBA1 = vBinaryArray1.toByteArray();
    ASSERT_EQ(validateBA1.size(), sizeof(binary1));

    for (int i = 0 ; i < sizeof(binary1); ++i)
    {
        EXPECT_EQ(validateBA1[i], binary1[i]);
    }
}

TEST(variant, make)
{
    TEST_DESCRIPTION("");

    //-------------------------------------------
    // PRECONDITIONS

    //-------------------------------------------
    // ACTIONS
    Variant vNumericI8 = Variant::make(i8);
    Variant vNumericI16 = Variant::make(i16);
    Variant vNumericI32 = Variant::make(i32);
    Variant vNumericI64 = Variant::make(i64);
    Variant vNumericUI8 = Variant::make(ui8);
    Variant vNumericUI16 = Variant::make(ui16);
    Variant vNumericUI32 = Variant::make(ui32);
    Variant vNumericUI64 = Variant::make(ui64);
    Variant vNumericDouble = Variant::make(d);
    Variant vBool = Variant::make(b);
    Variant vStr1 = Variant::make(s1);
    Variant vStr2 = Variant::make(s2);
    Variant vBinaryArray1 = Variant::make(binary1, sizeof(binary1));
    Variant vBinaryArray2 = Variant::make(binary2);
    Variant vListInt = Variant::make(listInt);
    Variant vListStr = Variant::make(listStr);
    Variant vVector = Variant::make(vectorBool);
    Variant vDict = Variant::make(dictIntStr);

    //-------------------------------------------
    // VALIDATION
    EXPECT_TRUE(vNumericI8.isSignedNumeric());
    EXPECT_TRUE(vNumericI16.isSignedNumeric());
    EXPECT_TRUE(vNumericI32.isSignedNumeric());
    EXPECT_TRUE(vNumericI64.isSignedNumeric());
    EXPECT_TRUE(vNumericUI8.isUnsignedNumeric());
    EXPECT_TRUE(vNumericUI16.isUnsignedNumeric());
    EXPECT_TRUE(vNumericUI32.isUnsignedNumeric());
    EXPECT_TRUE(vNumericUI64.isUnsignedNumeric());
    EXPECT_TRUE(vNumericDouble.isNumeric());
    EXPECT_TRUE(vBool.isBool());
    EXPECT_TRUE(vStr1.isString());
    EXPECT_TRUE(vStr2.isString());
    EXPECT_TRUE(vBinaryArray1.isByteArray());
    EXPECT_TRUE(vBinaryArray2.isByteArray());
    EXPECT_TRUE(vListInt.isList());
    EXPECT_TRUE(vListStr.isList());
    EXPECT_TRUE(vVector.isVector());
    EXPECT_TRUE(vDict.isDictionary());

    EXPECT_EQ(vNumericI8.toInt64(), i8);
    EXPECT_EQ(vNumericI16.toInt64(), i16);
    EXPECT_EQ(vNumericI32.toInt64(), i32);
    EXPECT_EQ(vNumericI64.toInt64(), i64);
    EXPECT_EQ(vNumericUI8.toUInt64(), ui8);
    EXPECT_EQ(vNumericUI16.toUInt64(), ui16);
    EXPECT_EQ(vNumericUI32.toUInt64(), ui32);
    EXPECT_EQ(vNumericUI64.toUInt64(), ui64);
    EXPECT_EQ(vNumericDouble.toDouble(), d);
    EXPECT_EQ(vBool.toBool(), b);
    EXPECT_EQ(vStr1.toString(), s1);
    EXPECT_STREQ(vStr2.toString().c_str(), s2);
    EXPECT_EQ(vBinaryArray2.toByteArray(), binary2);

    std::vector<char> validateBA1 = vBinaryArray1.toByteArray();
    ASSERT_EQ(validateBA1.size(), sizeof(binary1));

    for (int i = 0 ; i < sizeof(binary1); ++i)
    {
        EXPECT_EQ(validateBA1[i], binary1[i]);
    }
}

TEST(variant, type_boolean)
{
    TEST_DESCRIPTION("test boolean type storage and implicit conversion");

    //-------------------------------------------
    // PRECONDITIONS

    //-------------------------------------------
    // ACTIONS
    VariantVector_t resultByVariable;
    VariantVector_t resultByValue;

    makeVariantList(resultByVariable, i8, i16, i32, i64, ui8, ui16, ui32, ui64, d, b, s1, s2, listInt, vectorBool, dictIntStr);
    makeVariantList(resultByValue, 1, 0, 2.3, "aaa", false, true);

    //-------------------------------------------
    // VALIDATION
    int i = 0;
    
    EXPECT_TRUE(resultByVariable[i++].isBool());// SignedNumeric
    EXPECT_TRUE(resultByVariable[i++].isBool());// SignedNumeric
    EXPECT_TRUE(resultByVariable[i++].isBool());// SignedNumeric
    EXPECT_TRUE(resultByVariable[i++].isBool());// SignedNumeric
    EXPECT_TRUE(resultByVariable[i++].isBool());// UnsignedNumeric
    EXPECT_TRUE(resultByVariable[i++].isBool());// UnsignedNumeric
    EXPECT_TRUE(resultByVariable[i++].isBool());// UnsignedNumeric
    EXPECT_TRUE(resultByVariable[i++].isBool());// UnsignedNumeric
    EXPECT_TRUE(resultByVariable[i++].isBool());// Numeric
    EXPECT_TRUE(resultByVariable[i++].isBool());// Bool
    EXPECT_FALSE(resultByVariable[i++].isBool());// String
    EXPECT_FALSE(resultByVariable[i++].isBool());// String
    EXPECT_FALSE(resultByVariable[i++].isBool());// List
    EXPECT_FALSE(resultByVariable[i++].isBool());// Vector
    EXPECT_FALSE(resultByVariable[i++].isBool());// Dictionary
    
    i = 0;
    EXPECT_TRUE(resultByValue[i++].isBool());// Numeric
    EXPECT_TRUE(resultByValue[i++].isBool());// Numeric
    EXPECT_TRUE(resultByValue[i++].isBool());// Numeric
    EXPECT_FALSE(resultByValue[i++].isBool());// String
    EXPECT_TRUE(resultByValue[i++].isBool());// Bool
    EXPECT_TRUE(resultByValue[i++].isBool());// Bool

    i = 0;
    EXPECT_EQ(resultByValue[i++].toBool(), true);// Numeric
    EXPECT_EQ(resultByValue[i++].toBool(), false);// Numeric
    EXPECT_EQ(resultByValue[i++].toBool(), true);// Numeric
    EXPECT_EQ(resultByValue[i++].toBool(), false);// String
    EXPECT_EQ(resultByValue[i++].toBool(), false);// Bool
    EXPECT_EQ(resultByValue[i++].toBool(), true);// Bool
}

TEST(variant, variadic_arguments)
{
    TEST_DESCRIPTION("validate that initializing variant arguments through variadic arguments works fine");

    //-------------------------------------------
    // PRECONDITIONS

    //-------------------------------------------
    // ACTIONS
    VariantVector_t result;
    VariantVector_t resultByValue;

    makeVariantList(result, i8, i16, i32, i64, ui8, ui16, ui32, ui64, d, b, s1, s2, listInt, vectorBool, dictIntStr);
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
    EXPECT_TRUE(result[i++].isDictionary());
    
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

    i = 0;
    EXPECT_TRUE(resultByValue[i++].isNumeric());
    EXPECT_TRUE(resultByValue[i++].isNumeric());
    EXPECT_TRUE(resultByValue[i++].isString());
    EXPECT_TRUE(resultByValue[i++].isBool());
    EXPECT_TRUE(resultByValue[i++].isBool());
}

// TODO: add more tests