#include "TestsCommon.hpp"
#include "hsmcpp/variant.hpp"

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

// TODO: add more tests