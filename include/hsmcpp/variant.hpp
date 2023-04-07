// Copyright (C) 2021 Igor Krechetov
// Distributed under MIT license. See file LICENSE for details

#ifndef HSMCPP_VARIANT_HPP
#define HSMCPP_VARIANT_HPP

#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace hsmcpp {

class Variant;
typedef std::vector<Variant> VariantVector_t;
typedef std::list<Variant> VariantList_t;
typedef std::map<Variant, Variant> VariantDict_t;
///< Provides a way to store two values as a single unit.
typedef std::pair<Variant, Variant> VariantPair_t;

// cppcheck-suppress misra-c2012-20.7
#define DEF_CONSTRUCTOR(_val_type, _internal_type)               \
  /** @brief Constructs a new variant with an _val_type value */ \
  explicit Variant(const _val_type& v) {                         \
    assign<_val_type>(v, _internal_type);                        \
  }

// cppcheck-suppress misra-c2012-20.7
#define DEF_OPERATOR_ASSIGN(_val_type, _internal_type)                                    \
  /** @brief Assigns _val_type value to current variant object and changing it's type. */ \
  Variant& operator=(const _val_type& v) {                                                \
    assign<_val_type>(v, _internal_type);                                                 \
    return *this;                                                                         \
  }

/**
 * @brief Variant class represents a type-safe union.
 * @details Variant class is supposed to be a substitute for std::variant which is not available prior to C++17.
 *
 * The Variant class is a generic container that can hold various types of data (similar to union). It is designed to be
 * flexible and efficient by allowing the user to store data of different types and sizes, and retrieve them in a type-safe
 * manner. The class definition provides an Variant::Type enumeration of different data types that the Variant object can hold,
 * including numeric types, boolean values, strings, and more complex types like vectors, lists, dictionaries and pairs.
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
        BYTEARRAY,  ///< std::vector<char>

        LIST,    ///< VariantList_t
        VECTOR,  ///< VariantVector_t

        DICTIONARY,  ///< VariantDict_t
        PAIR,        ///< VariantPair_t
    };

public:
    /**
     * @brief Creates a Variant object with a value of type Type::BYTE_1.
     * @param v The value to assign to the Variant object.
     * @return Newly constructed Variant object.
     */
    static Variant make(const int8_t v);

    /**
     * @brief Creates a Variant object with a value of type Type::BYTE_2.
     * @copydetails make(const int8_t v)
     */
    static Variant make(const int16_t v);

    /**
     * @brief Creates a Variant object with a value of type Type::BYTE_4.
     * @copydetails make(const int8_t v)
     */
    static Variant make(const int32_t v);

    /**
     * @brief Creates a Variant object with a value of type Type::BYTE_8.
     * @copydetails make(const int8_t v)
     */
    static Variant make(const int64_t v);

    /**
     * @brief Creates a Variant object with a value of type Type::UBYTE_1.
     * @copydetails make(const int8_t v)
     */
    static Variant make(const uint8_t v);

    /**
     * @brief Creates a Variant object with a value of type Type::UBYTE_2.
     * @copydetails make(const int8_t v)
     */
    static Variant make(const uint16_t v);

    /**
     * @brief Creates a Variant object with a value of type Type::UBYTE_4.
     * @copydetails make(const int8_t v)
     */
    static Variant make(const uint32_t v);

    /**
     * @brief Creates a Variant object with a value of type Type::UBYTE_8.
     * @copydetails make(const int8_t v)
     */
    static Variant make(const uint64_t v);

    /**
     * @brief Creates a Variant object with a value of type Type::DOUBLE.
     * @copydetails make(const int8_t v)
     */
    static Variant make(const double v);

    /**
     * @brief Creates a Variant object with a value of type Type::BOOL.
     * @copydetails make(const int8_t v)
     */
    static Variant make(const bool v);

    /**
     * @brief Creates a Variant object with a value of type Type::STRING.
     * @copydetails make(const int8_t v)
     */
    static Variant make(const std::string& v);

    /**
     * @brief Creates a Variant object with a value of type Type::STRING.
     * @copydetails make(const int8_t v)
     */
    static Variant make(const char* v);

    /**
     * @brief Creates a Variant object with a value of type Type::VECTOR.
     * @copydetails make(const int8_t v)
     */
    static Variant make(const std::vector<char>& v);

    /**
     * @brief Creates a Variant object with a value of type Type::VECTOR.
     * @param binaryData The binary data to store.
     * @param bytesCount The size of the binaryData in bytes.
     * @return Newly constructed Variant object.
     */
    static Variant make(const char* binaryData, const size_t bytesCount);

    /**
     * @brief Creates a Variant object with a value of type Type::VECTOR.
     * @copydetails make(const int8_t v)
     */
    static Variant make(const VariantVector_t& v);

    /**
     * @brief Creates a Variant object with a value of type Type::VECTOR.
     * @copydetails make(const int8_t v)
     */
    template <typename T>
    static Variant make(const std::vector<T>& v);

    /**
     * @brief Creates a Variant object with a value of type Type::LIST.
     * @copydetails make(const int8_t v)
     */
    static Variant make(const VariantList_t& v);

    /**
     * @brief Creates a Variant object with a value of type Type::LIST.
     * @copydetails make(const int8_t v)
     */
    template <typename T>
    static Variant make(const std::list<T>& v);

    /**
     * @brief Creates a Variant object with a value of type Type::DICTIONARY.
     * @copydetails make(const int8_t v)
     */
    static Variant make(const VariantDict_t& v);

    /**
     * @brief Creates a Variant object with a value of type Type::DICTIONARY.
     * @copydetails make(const int8_t v)
     */
    template <typename K, typename V>
    static Variant make(const std::map<K, V>& v);

    /**
     * @brief Creates a Variant object with a value of type Type::DICTIONARY.
     * @param first stored as first value of a pair
     * @param second stored as second value of a pair
     * @return Newly constructed Variant object.
     */
    static Variant make(const Variant& first, const Variant& second);

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
    Variant(Variant&& v);

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
    DEF_CONSTRUCTOR(std::string, Type::STRING)
    /** @brief Constructs a new variant with an const char* value */
    explicit Variant(const char* v);

    DEF_CONSTRUCTOR(std::vector<char>, Type::BYTEARRAY)
    /** @brief Constructs a new variant of type Type::BYTEARRAY. Copies bytesCount bytes from binaryData buffer. */
    explicit Variant(const char* binaryData, const size_t bytesCount);

    DEF_CONSTRUCTOR(VariantVector_t, Type::VECTOR)
    DEF_CONSTRUCTOR(VariantList_t, Type::LIST)
    DEF_CONSTRUCTOR(VariantDict_t, Type::DICTIONARY)
    DEF_CONSTRUCTOR(VariantPair_t, Type::PAIR)

    /** @brief Assigns the value of the \c v to this variant. */
    Variant& operator=(const Variant& v);
    /** @brief Move-assigns the value of the \c v to this variant. */
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

    /**
     * @brief Get the type of values stored in the Variant object
     * @return type of the stored value
     */
    inline Type getType() const {
        return type;
    }

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
     * @brief Check if the Variant object contains a dictionary
     * @return true if the Variant object contains a dictionary, false otherwise
     */
    bool isDictionary() const;

    /**
     * @brief Check if the Variant object contains a pair
     * @return true if the Variant object contains a pair, false otherwise
     */
    bool isPair() const;

    /**
     * @brief Returns pointer to internal data.
     *
     * @warning This function is not type-safe and simply does static_cast to requested type. Always prefer using toX() methods
     * instead. This method was added only for situations when performance is critical and we are trying to avoid additional
     * memory allocation or copy. When using this API make sure to call getType() and check which type of data is contained
     * inside variant object.
     */
    template <typename T>
    inline T* value() const {
        return (static_cast<T*>(data));
    }

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
     *  \li Type::DICTIONARY
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
     *  \li Type::DICTIONARY
     *  \li Type::PAIR
     *
     * @todo add support for VECTOR and LIST
     *
     * @return byte array representation of the Variant object (calling this method on an unsupported variant type returns an
     * empty vector).
     */
    std::vector<char> toByteArray() const;

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
     *
     * @todo add support for BOOL and STRING
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
     *
     * @todo add support for BOOL and STRING
     *
     * @return boolean representation of the Variant object (calling this method on an unsupported variant type returns false).
     */
    bool toBool() const;

    /**
     * @brief Returns the variant value represented as vector.
     * @details Only supports Type::VECTOR. Method converts item values to the type T specified as a template. For supported
     * conversions see corresponsing toX() functions.
     *
     * @return vector representation of the Variant object
     */
    template <typename T>
    std::vector<T> toVector() const;

    /**
     * @brief Returns the variant value represented as vector.
     * @details Only supports Type::LIST. Method converts item values to the type T specified as a template. For supported
     * conversions see corresponsing toX() functions.
     *
     * @return list representation of the Variant object
     */
    template <typename T>
    std::list<T> toList() const;

    /**
     * @brief Returns the variant value represented as map.
     * @details Only supports Type::DICTIONARY. Method converts item values to the type T specified as a template. For supported
     * conversions see corresponsing toX() functions.
     *
     * @return map representation of the Variant object
     */
    template <typename K, typename V>
    std::map<K, V> toMap() const;

    /**
     * @brief Returns the variant value represented as a pair.
     * @details Only supports Type::PAIR. Method converts item values to the type T specified as a template. For supported
     * conversions see corresponsing toX() functions.
     *
     * @return std::pair representation of the Variant object
     */
    template <typename F, typename S>
    std::pair<F, S> toPair() const;

    /**
     * @brief Checks if variant contains value or not
     * @retval true variant was initialzied with a value and it's type is not Type::UNKNOWN
     * @retval false no value was set for the variant and it's type is Type::UNKNOWN
     */
    operator bool() const;

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
    Variant(void* d, const Type t);

    bool isSameObject(const Variant& val) const;

    template <typename T>
    void assign(const T& v, const Type t) {
        freeMemory();
        data = new T(v);
        type = t;
    }

    void freeMemory();

private:
    void* data = nullptr;
    Type type = Type::UNKNOWN;
};

template <typename T>
Variant Variant::make(const std::vector<T>& v) {
    VariantVector_t* dest = new VariantVector_t();

    for (auto it = v.begin(); it != v.end(); ++it) {
        dest->push_back(make(*it));
    }

    return Variant(dest, Type::VECTOR);
}

template <typename T>
Variant Variant::make(const std::list<T>& v) {
    VariantList_t* dest = new VariantList_t();

    for (auto it = v.begin(); it != v.end(); ++it) {
        dest->push_back(make(*it));
    }

    return Variant(dest, Type::LIST);
}

template <typename K, typename V>
Variant Variant::make(const std::map<K, V>& v) {
    VariantDict_t* dict = new VariantDict_t();

    for (auto it = v.begin(); it != v.end(); ++it) {
        dict->emplace(make(it->first), make(it->second));
    }

    return Variant(dict, Type::DICTIONARY);
}

template <typename T>
std::vector<T> Variant::toVector() const {
    std::vector<T> result;

    if (true == isVector()) {
        VariantVector_t* data = value<VariantVector_t>();

        if (nullptr != data) {
            for (auto it = data->begin(); it != data->end(); ++it) {
                result.push_back(*(it->value<T>()));
            }
        }
    }

    return result;
}

template <typename T>
std::list<T> Variant::toList() const {
    std::list<T> result;

    if (true == isList()) {
        VariantList_t* data = value<VariantList_t>();

        if (nullptr != data) {
            for (auto it = data->begin(); it != data->end(); ++it) {
                result.push_back(*(it->value<T>()));
            }
        }
    }

    return result;
}

template <typename K, typename V>
std::map<K, V> Variant::toMap() const {
    std::map<K, V> result;

    if (true == isDictionary()) {
        VariantDict_t* dict = value<VariantDict_t>();

        if (nullptr != dict) {
            for (auto it = dict->begin(); it != dict->end(); ++it) {
                result.emplace(*(it->first.value<K>()), *(it->second.value<V>()));
            }
        }
    }

    return result;
}

template <typename F, typename S>
std::pair<F, S> Variant::toPair() const {
    std::pair<F, S> result;

    if (true == isPair()) {
        VariantPair_t* p = value<VariantPair_t>();

        if (nullptr != p) {
            result.first = *(p->first.value<F>());
            result.second = *(p->second.value<S>());
        }
    }

    return result;
}

}  // namespace hsmcpp

template struct std::pair<hsmcpp::Variant, hsmcpp::Variant>;

#endif  // HSMCPP_VARIANT_HPP
