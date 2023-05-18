// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_VARIANT_HPP
#define HSMCPP_VARIANT_HPP

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace hsmcpp {

class Variant;

using ByteArray_t = std::vector<unsigned char>;
using VariantVector_t = std::vector<Variant>;
using VariantList_t = std::list<Variant>;
using VariantMap_t = std::map<Variant, Variant>;
using VariantPair_t = std::pair<Variant, Variant>;  ///< Provides a way to store two values as a single unit.

// cppcheck-suppress misra-c2012-20.7 ; parentheses are not needed
#define DEF_CONSTRUCTOR(_val_type, _internal_type)               \
  /** @brief Constructs a new variant with an _val_type value */ \
  explicit Variant(const _val_type v);

// cppcheck-suppress misra-c2012-20.7 ; parentheses are not needed
#define DEF_OPERATOR_ASSIGN(_val_type, _internal_type)                                                      \
  /** @brief Assigns _val_type value to current variant object and changing it's type to _internal_type. */ \
  Variant& operator=(const _val_type v);

#define DEF_MAKE_DOC(_internal_type)                                      \
  /**                                                                     \
   * @brief Creates a Variant object with a value of type _internal_type. \
   * @param v The value to assign to the Variant object.                  \
   * @return Newly constructed Variant object.                            \
   */

// cppcheck-suppress misra-c2012-20.7 ; parentheses are not needed
#define DEF_MAKE(_val_type, _internal_type) \
  DEF_MAKE_DOC(_internal_type)              \
  static Variant make(const _val_type v);

/**
 * @brief Variant class represents a type-safe union.
 * @details Variant class is supposed to be a substitute for std::variant which is not available prior to C++17.
 *
 * The Variant class is a generic container that can hold various types of data (similar to union). It is designed to be
 * flexible and efficient by allowing the user to store data of different types and sizes, and retrieve them in a type-safe
 * manner. The class definition provides an Variant::Type enumeration of different data types that the Variant object can hold,
 * including numeric types, boolean values, strings, and more complex types like vectors, lists, maps and pairs.
 *
 * A Variant object holds a single value of a single type at a time or none if it's empty. Value can be retrieved as a copy
 * using toX() methods (for example toString(), toInt64(), etc.). When asked for a type that can be generated from the stored
 * type, toX() copies and converts and leaves the object itself unchanged. When asked for a type that cannot be generated from
 * the stored type, the result depends on the type (see each the function's documentation for details).
 *
 * Here is some example code to demonstrate the use of Variant:
 *
 * \code{.cpp}
 * Variant v; // v is empty and has Type::UNKNOWN
 *
 * if (!v) {
 *      v = 42; // v now contains numeric value (usually Type::BYTE_4, but depends on compiler)
 *      std::cout << "v is an int: " << v.isNumeric() << std::endl; // output: v is an int: 1
 *      std::cout << "v == 42: " << (v == Variant(42)) << std::endl; // output: v == 42: 1
 *      std::cout << "v == \"42\": " << (v.toString() == "42") << std::endl; // output: v == "42": 1
 *
 *      v = "Hello, world!"; // v now contains a Type::STRING value
 *      std::cout << "v is a string: " << v.isString() << std::endl; // output: v is a string: 1
 *      std::cout << "v == \"Hello, world!\": " << (v == "Hello, world!") << std::endl; // output: v == "Hello, world!": 1
 *      std::cout << "v.toInt64(): " << v.toInt64() << std::endl; // output: v.toInt64(): 0
 *
 *      v = 3.14; // v now contains a Type::DOUBLE value
 *      std::cout << "v is a double: " << v.isDouble() << std::endl; // output: v is a double: 1
 *      std::cout << "v > 2.0: " << (v > 2.0) << std::endl; // output: v > 2.0: 1
 * }
 * \endcode
 */
class Variant {
private:
    /**
     * @brief Handles memory allocation/deallocation for custom types.
     *
     * @param void* memory pointer
     * @param bool true - release memory; false - copy memory
     * @return new memory pointer if copy was requested or nullptr
     */
    // using MemoryHandlerFunc_t = std::function<void*(void*, const bool)>;
    using MemoryAllocatorFunc_t = std::function<std::shared_ptr<void>(const void*)>;
    using CompareFunc_t = std::function<int(const void*, const void*)>;

public:
    /**
     * @brief Describes types that can be stored inside Variant container.
     */
    enum class Type {
        UNKNOWN,  ///< empty object

        BYTE_1,  ///< A 1-byte signed integer
        BYTE_2,  ///< A 2-byte signed integer
        BYTE_4,  ///< A 4-byte signed integer
        BYTE_8,  ///< An 8-byte signed integer

        UBYTE_1,  ///< A 1-byte unsigned integer
        UBYTE_2,  ///< A 2-byte unsigned integer
        UBYTE_4,  ///< A 4-byte unsigned integer
        UBYTE_8,  ///< An 8-byte unsigned integer

        DOUBLE,  ///< A double-precision floating-point number
        BOOL,    ///< A boolean value

        STRING,     ///< std::string
        BYTEARRAY,  ///< ByteArray_t

        LIST,    ///< VariantList_t
        VECTOR,  ///< VariantVector_t

        MAP,   ///< VariantMap_t
        PAIR,  ///< VariantPair_t

        CUSTOM  ///< any type
    };

public:
    DEF_MAKE(int8_t, Type::BYTE_1);
    DEF_MAKE(int16_t, Type::BYTE_2);
    DEF_MAKE(int32_t, Type::BYTE_4);
    DEF_MAKE(int64_t, Type::BYTE_8);
    DEF_MAKE(uint8_t, Type::UBYTE_1);
    DEF_MAKE(uint16_t, Type::UBYTE_2);
    DEF_MAKE(uint32_t, Type::UBYTE_4);
    DEF_MAKE(uint64_t, Type::UBYTE_8);
    DEF_MAKE(double, Type::DOUBLE);
    DEF_MAKE(bool, Type::BOOL);
    DEF_MAKE(std::string&, Type::STRING);
    DEF_MAKE(char*, Type::STRING);
    DEF_MAKE(ByteArray_t&, Type::VECTOR);

    /**
     * @brief Creates a Variant object with a value of type Type::VECTOR.
     * @param binaryData The binary data to store.
     * @param bytesCount The size of the binaryData in bytes.
     * @return Newly constructed Variant object.
     */
    static Variant make(const char* binaryData, const size_t bytesCount);

    DEF_MAKE(VariantVector_t&, Type::VECTOR);
    template <typename T>
    DEF_MAKE(std::vector<T>&, Type::VECTOR);
    DEF_MAKE(VariantList_t&, Type::LIST);
    template <typename T>
    DEF_MAKE(std::list<T>&, Type::LIST);
    DEF_MAKE(VariantMap_t&, Type::MAP);

    DEF_MAKE_DOC(Type::MAP)
    template <typename K, typename V>
    static Variant make(const std::map<K, V>& v);

    DEF_MAKE(VariantPair_t&, Type::PAIR);

    /**
     * @brief Creates a Variant object with a value of type Type::PAIR.
     * @param first stored as first value of a pair
     * @param second stored as second value of a pair
     * @return Newly constructed Variant object.
     */
    template <typename TFirst, typename TSecond>
    static Variant make(const TFirst& first, const TSecond& second);

    DEF_MAKE_DOC(Type::CUSTOM)
    template <typename T>
    static Variant make(const T& v);

    // DEF_MAKE_DOC(Type::CUSTOM)
    // TODO: doc
    template <typename T>
    static Variant makeCustom(const T& v);

    /**
     * @brief Create a new Variant object from an existing Variant object.
     * @param v The Variant object to copy.
     * @return Newly constructed Variant object.
     */
    static Variant make(const Variant& v);

public:
    /**
     * @brief Default constructor.
     * @details Constructs a new empty Variant object with Type::UNKNOWN and null data.
     */
    Variant() = default;

    /**
     * @brief Copy constructor for Variant object.
     * @param v Another Variant object to copy from.
     */
    Variant(const Variant& v);

    /**
     * @brief Move constructor for Variant object.
     * @param v Another Variant object to move from.
     */
    Variant(Variant&& v) noexcept;

    /**
     * @brief Destructor for Variant object.
     */
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
    DEF_CONSTRUCTOR(std::string&, Type::STRING)
    /** @brief Constructs a new variant object of type Type::STRING with an const char* value. */
    explicit Variant(const char* v);

    DEF_CONSTRUCTOR(ByteArray_t&, Type::BYTEARRAY)
    /** @brief Constructs a new variant of type Type::BYTEARRAY. Copies bytesCount bytes from binaryData buffer. */
    explicit Variant(const char* binaryData, const size_t bytesCount);

    DEF_CONSTRUCTOR(VariantVector_t&, Type::VECTOR)
    DEF_CONSTRUCTOR(VariantList_t&, Type::LIST)
    DEF_CONSTRUCTOR(VariantMap_t&, Type::MAP)
    DEF_CONSTRUCTOR(VariantPair_t&, Type::PAIR)

    template <typename T>
    DEF_CONSTRUCTOR(std::vector<T>&, Type::VECTOR);
    template <typename T>
    DEF_CONSTRUCTOR(std::list<T>&, Type::LIST);
    template <typename K, typename V>
    explicit Variant(const std::map<K, V>& v);

    /**
     * @brief Constructs a new variant object of type Type::PAIR.
     * @param first stored as first value of a pair
     * @param second stored as second value of a pair
     */
    template <typename TFirst, typename TSecond>
    explicit Variant(const TFirst& first, const TSecond& second);

    /** @brief Constructs a new variant object of type Type::CUSTOM using custom types. */
    template <typename T>
    explicit Variant(const T& v);

    /** @brief Assigns the value of the \c v to this variant. */
    Variant& operator=(const Variant& v);
    /** @brief Move-assigns the value of the \c v to this variant. */
    Variant& operator=(Variant&& v) noexcept;

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
    DEF_OPERATOR_ASSIGN(std::string&, Type::STRING)
    DEF_OPERATOR_ASSIGN(char*, Type::STRING)
    DEF_OPERATOR_ASSIGN(ByteArray_t&, Type::BYTEARRAY)
    DEF_OPERATOR_ASSIGN(VariantVector_t&, Type::VECTOR)
    DEF_OPERATOR_ASSIGN(VariantList_t&, Type::LIST)
    DEF_OPERATOR_ASSIGN(VariantMap_t&, Type::MAP)
    DEF_OPERATOR_ASSIGN(VariantPair_t&, Type::PAIR)

    template <typename T>
    DEF_OPERATOR_ASSIGN(T&, Type::CUSTOM)

    /**
     * @brief Resets value of the Varaint object and frees all allocated memory.
     * After calling this method internal type will become Type::UNKNOWN.
     */
    void clear();

    /**
     * @brief Gets the type of data stored in the Variant object
     * @return type of the stored data
     */
    inline Type getType() const;

    /**
     * @brief Checks if variant contains value or not
     * @retval true variant was initialzied with a value and it's type is not Type::UNKNOWN
     * @retval false no value was set for the variant and it's type is Type::UNKNOWN
     */
    operator bool() const;

    /**
     * @brief Check if the Variant object doesn't contain any value
     * @return true if the Variant object doesn't contain any value, false otherwise
     */
    bool isEmpty() const;

    /**
     * @brief Check if the Variant object contains a numeric value
     * @return true if the Variant object contains a numeric value, false otherwise
     */
    bool isNumeric() const;

    /**
     * @brief Check if the Variant object contains a signed numeric value
     * @return true if the Variant object contains a signed numeric value, false otherwise
     */
    bool isSignedNumeric() const;

    /**
     * @brief Check if the Variant object contains an unsigned numeric value
     * @return true if the Variant object contains an unsigned numeric value, false otherwise
     */
    bool isUnsignedNumeric() const;

    /**
     * @brief Check if the Variant object contains a boolean value
     * @return true if the Variant object contains a boolean value, false otherwise
     */
    bool isBool() const;

    /**
     * @brief Check if the Variant object contains a string
     * @return true if the Variant object contains a string, false otherwise
     */
    bool isString() const;

    /**
     * @brief Check if the Variant object contains a byte array
     * @return true if the Variant object contains a byte array, false otherwise
     */
    bool isByteArray() const;

    /**
     * @brief Check if the Variant object contains a vector
     * @return true if the Variant object contains a vector, false otherwise
     */
    bool isVector() const;
    /**
     * @brief Check if the Variant object contains a list
     * @return true if the Variant object contains a list, false otherwise
     */
    bool isList() const;

    /**
     * @brief Check if the Variant object contains a map
     * @return true if the Variant object contains a map, false otherwise
     */
    bool isMap() const;

    /**
     * @brief Check if the Variant object contains a pair
     * @return true if the Variant object contains a pair, false otherwise
     */
    bool isPair() const;

    /**
     * @brief Check if the Variant object contains a custom type
     * @return true if the Variant object contains a custom type, false otherwise
     */
    bool isCustomType() const;

    /**
     * @brief Returns the variant value represented as int64
     * @details Supported data types are:
     *  \li Type::BYTE_1
     *  \li Type::BYTE_2
     *  \li Type::BYTE_4
     *  \li Type::BYTE_8
     *  \li Type::UBYTE_1
     *  \li Type::UBYTE_2
     *  \li Type::UBYTE_4
     *  \li Type::UBYTE_8
     *  \li Type::DOUBLE
     *  \li Type::BOOL - returns 0 if the value is false and 1 if it's true
     *  \li Type::STRING - tries to convert string to number or returns 0
     *
     * @return numeric representation of the Variant object (calling this method on an unsupported variant type returns 0).
     */
    int64_t toInt64() const;

    /**
     * @brief Returns the variant value represented as uint64
     * @copydetails toInt64()
     */
    uint64_t toUInt64() const;

    /**
     * @brief Returns the variant value represented as double
     * @copydetails toInt64()
     */
    double toDouble() const;

    /**
     * @brief Returns the variant value represented as bool
     * @details Supported data types are:
     *  \li Type::BYTE_1
     *  \li Type::BYTE_2
     *  \li Type::BYTE_4
     *  \li Type::BYTE_8
     *  \li Type::UBYTE_1
     *  \li Type::UBYTE_2
     *  \li Type::UBYTE_4
     *  \li Type::UBYTE_8
     *  \li Type::DOUBLE
     *  \li Type::BOOL
     *  \li Type::STRING - returns true if string value is "true" or it represents a number different from 0
     *
     * @return boolean representation of the Variant object (calling this method on an unsupported variant type returns false).
     */
    bool toBool() const;

    /**
     * @brief Returns the variant value represented as string
     * @details If stored value is not a string, method will try to convert it. Supported data types are:
     *  \li Type::BYTE_1
     *  \li Type::BYTE_2
     *  \li Type::BYTE_4
     *  \li Type::BYTE_8
     *  \li Type::UBYTE_1
     *  \li Type::UBYTE_2
     *  \li Type::UBYTE_4
     *  \li Type::UBYTE_8
     *  \li Type::DOUBLE
     *  \li Type::BOOL
     *  \li Type::STRING
     *  \li Type::BYTEARRAY
     *  \li Type::LIST
     *  \li Type::VECTOR
     *  \li Type::MAP
     *  \li Type::PAIR
     *
     * @return string representation of the Variant object (calling this method on an unsupported variant type returns an empty
     * string).
     */
    std::string toString() const;

    /**
     * @brief Returns the variant value represented as byte array
     * @details If stored value is not a byte array, method will try to convert it. Supported data types are:
     *  \li Type::BYTE_1
     *  \li Type::BYTE_2
     *  \li Type::BYTE_4
     *  \li Type::BYTE_8
     *  \li Type::UBYTE_1
     *  \li Type::UBYTE_2
     *  \li Type::UBYTE_4
     *  \li Type::UBYTE_8
     *  \li Type::DOUBLE
     *  \li Type::BOOL
     *  \li Type::STRING
     *  \li Type::BYTEARRAY
     *  \li Type::MAP
     *  \li Type::PAIR
     *
     * @remark This function always returns a copy of data. If you know that internal type is Type::BYTEARRAY and just want to
     * access data it's better to use toByteArrayPtr() method.
     *
     * @return byte array representation of the Variant object (calling this method on an unsupported variant type returns an
     * empty vector).
     */
    ByteArray_t toByteArray() const;

    /**
     * @brief Returns pointer to internal byte array data.
     * @return pointer to internal data or nullptr if data type is not Type::BYTEARRAY
     */
    std::shared_ptr<ByteArray_t> getByteArray() const;

    /**
     * @brief Returns pointer to internal vector data.
     * @return pointer to internal data or nullptr if data type is not Type::VECTOR
     */
    std::shared_ptr<VariantVector_t> getVector() const;

    /**
     * @brief Copies internal data to a requested std::vector type.
     * Usage example:
     *
     * \code{.cpp}
     * std::vector<int> intData = {1, 2, 3};
     * Variant v(intData);
     *
     * std::vector<int> intDataConverted = v.toVector<int>([](const Variant& v){ return v.toInt64(); }));
     * // intDataConverted = {1, 2, 3}
     *
     * std::vector<std::string> strDataConverted = v.toVector<std::string>([](const Variant& v){ return v.toString(); }));
     * // strDataConverted = {"1", "2", "3"}
     * \endcode
     *
     * @param converter functor which will be called for each vector item to convert it from Variant to requested type
     * @return std::vector filled with data (empty container if data type is not Type::VECTOR)
     */
    template <typename T>
    std::vector<T> toVector(const std::function<T(const Variant&)>& converter) const;

    /**
     * @brief Returns pointer to internal list data.
     * @return pointer to internal data or nullptr if data type is not Type::LIST
     */
    std::shared_ptr<VariantList_t> getList() const;

    /**
     * @brief Copies internal data to a requested std::list type.
     * Usage example:
     *
     * \code{.cpp}
     * std::list<int> intData = {1, 2, 3};
     * Variant v(intData);
     *
     * std::list<int> intDataConverted = v.toList<int>([](const Variant& v){ return v.toInt64(); }));
     * // intDataConverted = {1, 2, 3}
     *
     * std::list<std::string> strDataConverted = v.toList<std::string>([](const Variant& v){ return v.toString(); }));
     * // strDataConverted = {"1", "2", "3"}
     * \endcode
     *
     * @param converter functor which will be called for each list item to convert it from Variant to requested type
     * @return std::list filled with data (empty container if data type is not Type::LIST)
     */
    template <typename T>
    std::list<T> toList(const std::function<T(const Variant&)>& converter) const;

    /**
     * @brief Returns pointer to internal map data.
     * @return pointer to internal data or nullptr if data type is not Type::MAP
     */
    std::shared_ptr<VariantMap_t> getMap() const;

    /**
     * @brief Copies internal data to a requested std::map type.
     * Usage example:
     *
     * \code{.cpp}
     * std::map<int, std::string> intStrData = {{1, "aa"}, {2, "bb"}, {3, "cc"}};
     * Variant v = Variant::make(intStrData);
     *
     * std::map<int, std::string> intStrDataConverted = v.toMap<int, std::string>([](const Variant& key){ return key.toInt64(); },
     *                                                                            [](const Variant& value){ return value.toString(); }));
     * \endcode
     *
     * @param converterKey functor which will be called for each map key to convert it from Variant to requested type
     * @param converterValue functor which will be called for each map value to convert it from Variant to requested type
     * @return std::map filled with data (empty container if data type is not Type::MAP)
     */
    template <typename K, typename V>
    std::map<K, V> toMap(const std::function<K(const Variant&)>& converterKey,
                         const std::function<V(const Variant&)>& converterValue) const;

    /**
     * @brief Returns pointer to internal pair data.
     * @return pointer to internal data or nullptr if data type is not Type::PAIR
     */
    std::shared_ptr<VariantPair_t> getPair() const;

    /**
     * @brief Copies internal data to a requested std::pair type.
     * Usage example:
     *
     * \code{.cpp}
     * std::pair<int, std::string> intStrData = {1, "aa"};
     * Variant v = Variant::make(intStrData);
     *
     * std::pair<int, std::string> intStrDataConverted = v.toPair<int, std::string>([](const Variant& first){ return first.toInt64(); },
     *                                                                              [](const Variant& second){ return second.toString(); }));
     * \endcode
     *
     * @param converterFirst functor which will be called for first pair element to convert it from Variant to requested type
     * @param converterSecond functor which will be called for second pair element to convert it from Variant to requested type
     * @return std::pair filled with data (empty container if data type is not Type::PAIR)
     */
    template <typename TFirst, typename TSecond>
    std::pair<TFirst, TSecond> toPair(const std::function<TFirst(const Variant&)>& converterFirst,
                                      const std::function<TSecond(const Variant&)>& converterSecond) const;

    /**
     * @brief Returns pointer to custom type data.
     *
     * @warning This function is not type-safe and simply does static_cast to requested type. User is responsible for specifying
     *          same type that was used for initialization of a variant object.
     *
     * @return pointer to internal data or nullptr if data type is not Type::CUSTOM
     */
    template <typename T>
    std::shared_ptr<T> getCustomType() const;

    /**
     * @brief Compares this Variant object to another Variant object for equality.
     * @details Variant uses the == operator of the stored type to check for equality. Variants of different types will always
     * compare as not equal.
     * @param val The Variant object to compare to.
     * @return true if this object is equal to val, false otherwise.
     */
    bool operator==(const Variant& val) const;

    /**
     * @brief Compares this Variant object to another Variant object for inequality.
     * @details Impelemented using operator==().
     * @param val The Variant object to compare to.
     * @return true if this object is not equal to val, false otherwise.
     */
    bool operator!=(const Variant& val) const;

    /**
     * @brief Checks if this Variant object is greater than another Variant object.
     * @details Operator only compares variants of the same type. Comparing logic depends on the underlying type:
     *  \li numeric, bool or string values are compared using standard operator >
     *  \li vector or list values are compared by size (A.size() > B.size())
     *  \li other types are not supported and false is always returned
     *
     * @param val The Variant object to compare to.
     * @return true if this object is greater than val, false otherwise.
     */
    bool operator>(const Variant& val) const;

    /**
     * @brief Checks if this Variant object is greater than or equal to another Variant object.
     * @details Impelemented using operator==() and operator>().
     *
     * @param val The Variant object to compare to.
     * @return true if this object is greater than or equal to val, false otherwise.
     */
    bool operator>=(const Variant& val) const;

    /**
     * @brief Checks if this Variant object is less than another Variant object.
     * @details Impelemented using operator!=() and operator>().
     *
     * @param val The Variant object to compare to.
     * @return true if this object is less than val, false otherwise.
     */
    bool operator<(const Variant& val) const;

    /**
     * @brief Checks if this Variant object is less than or equal to another Variant object.
     * @details Impelemented using operator==() and operator>().
     *
     * @param val The Variant object to compare to.
     * @return true if this object is less than or equal to val, false otherwise.
     */
    bool operator<=(const Variant& val) const;

private:
    Variant(std::shared_ptr<void> d, const Type t);

    template <typename T>
    Variant(const T& v, const Type t);

    bool isSameObject(const Variant& val) const;

    template <typename T>
    void assign(const T& v, const Type t);

    template <typename T>
    inline std::shared_ptr<T> value() const;

    template <typename T>
    void createMemoryAllocator();

    void freeMemory();

private:
    Type type = Type::UNKNOWN;
    std::shared_ptr<void> data;
    MemoryAllocatorFunc_t memoryAllocator;
    CompareFunc_t compareOperator;
};

template <typename T>
Variant Variant::make(const std::vector<T>& v) {
    return Variant(v);
}

template <typename T>
Variant Variant::make(const std::list<T>& v) {
    return Variant(v);
}

template <typename K, typename V>
Variant Variant::make(const std::map<K, V>& v) {
    return Variant(v);
}

template <typename TFirst, typename TSecond>
Variant Variant::make(const TFirst& first, const TSecond& second) {
    return Variant(first, second);
}

template <typename T>
Variant Variant::make(const T& v) {
    return makeCustom(v);
}

template <typename T>
Variant Variant::makeCustom(const T& v) {
    return Variant(v, Type::CUSTOM);
}

template <typename T>
Variant::Variant(const std::vector<T>& v) {
    std::shared_ptr<VariantVector_t> dest = std::shared_ptr<VariantVector_t>(new VariantVector_t(), [](void* ptr) {
        delete reinterpret_cast<VariantVector_t*>(ptr);
    });

    dest->reserve(v.size());

    for (auto it = v.begin(); it != v.end(); ++it) {
        dest->emplace_back(*it);
    }

    data = std::static_pointer_cast<void>(dest);
    type = Type::VECTOR;
    createMemoryAllocator<VariantVector_t>();
}

template <typename T>
Variant::Variant(const std::list<T>& v) {
    std::shared_ptr<VariantList_t> dest =
        std::shared_ptr<VariantList_t>(new VariantList_t(), [](void* ptr) { delete reinterpret_cast<VariantList_t*>(ptr); });

    for (auto it = v.begin(); it != v.end(); ++it) {
        dest->emplace_back(*it);
    }

    data = std::static_pointer_cast<void>(dest);
    type = Type::LIST;
    createMemoryAllocator<VariantList_t>();
}

template <typename K, typename V>
Variant::Variant(const std::map<K, V>& v) {
    std::shared_ptr<VariantMap_t> dest =
        std::shared_ptr<VariantMap_t>(new VariantMap_t(), [](void* ptr) { delete reinterpret_cast<VariantMap_t*>(ptr); });

    for (auto it = v.begin(); it != v.end(); ++it) {
        dest->emplace(it->first, it->second);
    }

    data = std::static_pointer_cast<void>(dest);
    type = Type::MAP;
    createMemoryAllocator<VariantMap_t>();
}

template <typename TFirst, typename TSecond>
Variant::Variant(const TFirst& first, const TSecond& second)
    // cppcheck-suppress misra-c2012-10.4 : false-positive. thinks that ':' is arithmetic operation
    : Variant(std::make_pair(Variant(first), Variant(second))) {}

template <typename T>
Variant::Variant(const T& v) {
    assign(v, Type::CUSTOM);
}

template <typename T>
Variant& Variant::operator=(const T& v) {
    assign(v, Type::CUSTOM);
    return *this;
}

Variant::Type Variant::getType() const {
    return type;
}

template <typename T>
std::shared_ptr<T> Variant::getCustomType() const {
    std::shared_ptr<T> result;

    if (true == isCustomType()) {
        result = value<T>();
    }

    return result;
}

template <typename T>
std::vector<T> Variant::toVector(const std::function<T(const Variant&)>& converter) const {
    std::vector<T> res;

    if (isVector()) {
        std::shared_ptr<VariantVector_t> vectorData = getVector();

        // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
        if (vectorData) {
            res.reserve(vectorData->size());

            for (const Variant& curItem : *vectorData) {
                res.emplace_back(converter(curItem));
            }
        }
    }

    return res;
}

template <typename T>
std::list<T> Variant::toList(const std::function<T(const Variant&)>& converter) const {
    std::list<T> res;

    if (isList()) {
        std::shared_ptr<VariantList_t> listData = getList();

        // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
        if (listData) {
            for (const Variant& curItem : *listData) {
                res.emplace_back(converter(curItem));
            }
        }
    }

    return res;
}

template <typename K, typename V>
std::map<K, V> Variant::toMap(const std::function<K(const Variant&)>& converterKey,
                              const std::function<V(const Variant&)>& converterValue) const {
    std::map<K, V> res;

    if (isMap()) {
        std::shared_ptr<VariantMap_t> mapData = getMap();

        // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
        if (mapData) {
            for (const auto& curItem : *mapData) {
                res.insert(std::make_pair(converterKey(curItem.first), converterValue(curItem.second)));
            }
        }
    }

    return res;
}

template <typename TFirst, typename TSecond>
std::pair<TFirst, TSecond> Variant::toPair(const std::function<TFirst(const Variant&)>& converterFirst,
                                           const std::function<TSecond(const Variant&)>& converterSecond) const {
    std::pair<TFirst, TSecond> res;

    if (isPair()) {
        std::shared_ptr<VariantPair_t> pairData = getPair();

        // cppcheck-suppress misra-c2012-14.4 ; false-positive. std::shared_ptr has a bool() operator
        if (pairData) {
            res = std::make_pair(converterFirst(pairData->first), converterSecond(pairData->second));
        }
    }

    return res;
}

template <typename T>
Variant::Variant(const T& v, const Type t) {
    assign(v, t);
}

template <typename T>
void Variant::assign(const T& v, const Type t) {
    freeMemory();
    createMemoryAllocator<T>();
    type = t;
    data = memoryAllocator(&v);
}

template <typename T>
inline std::shared_ptr<T> Variant::value() const {
    return std::static_pointer_cast<T>(data);
}

template <typename T>
void Variant::createMemoryAllocator() {
    memoryAllocator = [](const void* ptr) {
        // NOTE: false-positive. "return" statement belongs to lambda function, not parent function
        // cppcheck-suppress misra-c2012-15.5
        return ((nullptr != ptr) ? (std::shared_ptr<void>(new T(*reinterpret_cast<const T*>(ptr)),
                                                          [](void* ptr) { delete reinterpret_cast<const T*>(ptr); }))
                                 : nullptr);
    };

    // cppcheck-suppress misra-c2012-13.1 ; false-positive. this is a functor, not initializer list
    compareOperator = [](const void* left, const void* right) {
        int res = -1;
        if (*reinterpret_cast<const T*>(left) == *reinterpret_cast<const T*>(right)) {
            res = 0;
        } else if (*reinterpret_cast<const T*>(left) > *reinterpret_cast<const T*>(right)) {
            res = 1;
        } else {
            // do nothing
        }

        // NOTE: false-positive. "return" statement belongs to lambda function, not parent function
        // cppcheck-suppress misra-c2012-15.5
        return res;
    };
}

}  // namespace hsmcpp

#endif  // HSMCPP_VARIANT_HPP
