#pragma once

#include <any>
#include <cmath>
#include <atomic>
#include <cstdint>
#include <array>
#include <type_traits>
#include <cmath>
#include <functional>
#include <sstream>
#include <boost/algorithm/string.hpp>


// see https://stackoverflow.com/questions/1070497/c-convert-hex-string-to-signed-integer

//    std::enable_if https://en.cppreference.com/w/cpp/types/enable_if.html
//    std::is_integral<TYPE>::value, std::is_floating_point

//////////////////////////////////////////////////////////////////////////////
//  determine type of shared_ptr<CacheObject<T>> 
//    if (auto p = dynamic_cast<typename std::shared_ptr<T>::element_type*>(r.get()))
//        return std::shared_ptr<T>{r, p};


template<typename TYPE>
class CacheObject; // needed by Serial...:get
template<typename TYPE, typename CONV_TYPE>
class AnyCacheObject; // needed by Serial...:get


class SerializedCacheObject {
public:
    SerializedCacheObject(const std::string& shortDescr)
        : mShortDescr(shortDescr) {}
    virtual ~SerializedCacheObject() = default;

    virtual const std::string& getShortDescription() const final {
        return mShortDescr;
    }

    void setDescription(const std::string& description) {
        mDescription = description;
    }

    virtual const std::string& getDescription() const final {
        return mDescription;
    }

    virtual void setRaw(const std::string& rawValue) = 0;

    // template<typename TYPE, typename CONV_TYPE>
    // CONV_TYPE get() const {
    //     auto child = std::dynamic_pointer_cast<CacheObject<TYPE, CONV_TYPE>>(this);
    //     if (child == nullptr) {
    //         printf("failed to convert in setRAw \n");
    //         return;
    //     }
    //     return child->get();
    // }
protected:
    std::string mShortDescr;
    std::string mDescription;
};


enum class Toggle : bool {
    On = true,
    Off = false,
};


// template<class ENUM_T>
// class EnumCacheObject
//     : public SerializedCacheObject
// {
// private:
//     bool mIsComponentsMapped = false;
//     // FIXME: mValue as second template type
//     std::array<ENUM_T, 2> mValue = {};
//     std::atomic_bool citem; // FIXME: make reference to atomic in DeviceCache
   
// public:
//     using value_t = ENUM_T;
//     using conversion_t = std::string;

//     EnumCacheObject(const std::string& shortDescr)
//         : SerializedCacheObject(shortDescr) {}
//     virtual ~EnumCacheObject() = default;

//     virtual void setRaw(const std::string& rawValue) override {
//         if (boost::iequals(rawValue, "on")) {
//             mValue[0] = Toggle::On;
//         } else {
//             mValue[0] = Toggle::Off;
//         }
//     }

//     void set(const std::string value) {
//         mValue[0] = value;
//     }

//     std::string get() const {
//         return mValue[0];
//     }
// };



// access object via template parameter for type-checked access
template<typename TYPE>
class CacheObject
    : public SerializedCacheObject
{
public:
    using value_t = TYPE;

    CacheObject(const std::atomic_bool& toggle, std::string shortDescr);
    // nativeToUnitFactor == 0 ==> output: unformatted value without units,
    //                             e.g. use for strings
    // nativeToUnitFactor: mValue[in units] = nativeToUnitFactor * raw value
    CacheObject(const std::atomic_bool& toggle, std::string shortDescr, double nativeToUnitFactor);

    virtual ~CacheObject() = default;

    virtual std::string getUnits() const { return {}; };

    // getValue() {
    //     const div = 1.0 / this.precision;
    //     const l = String(div).length;
    //     let scaledToIntPrecision = Number(this.value * this.nativeToUnitFactor) + Number.EPSILON;
    //     scaledToIntPrecision *= div;
    //     return (Math.round(scaledToIntPrecision) * this.precision).toFixed(l-1);
    // }

    void set(const value_t value);
    value_t get() const;

protected:
    const std::atomic_bool& mReadable;

    bool mIsComponentsMapped = false; // whats this for?
    // FIXME: mValue as second template type
    std::array<value_t, 2> mValue = {};
 // TODO: implement a sigc signal to signal a change to all interested parties
    double mNativeToUnitFactor = 1.0;
};


// child object with two template parameters to store type for conversion
template<typename TYPE, typename CONV_TYPE>
class AnyCacheObject
    : public CacheObject<TYPE>
{
public:
    using value_t = TYPE;
    using conversion_t = CONV_TYPE;

    AnyCacheObject(const std::atomic_bool& toggle, std::string shortDescr)
        : CacheObject<TYPE>(toggle, shortDescr) {};
    // nativeToUnitFactor == 0 ==> output: unformatted value without units,
    //                             e.g. use for strings
    // nativeToUnitFactor: mValue[in units] = nativeToUnitFactor * raw value
    AnyCacheObject(const std::atomic_bool& toggle, std::string shortDescr, double nativeToUnitFactor)
        : CacheObject<TYPE>(toggle, shortDescr, nativeToUnitFactor) {};

    // units == "" for unit-less values
    // units == "s" ==> output in format weeks days h m s;
    //                  \pre nativeToUnitFactor must convert to seconds
    AnyCacheObject(const std::atomic_bool& toggle, std::string shortDescr,
                double nativeToUnitFactor,
                std::string units)
        : CacheObject<TYPE>(toggle, shortDescr, nativeToUnitFactor, units) {}

    virtual ~AnyCacheObject() = default;

private:
    virtual void setRaw(const std::string& rawValue) override;
};


// child object with two template parameters to store type for conversion
template<typename TYPE, typename CONV_TYPE>
class NumericCacheObject
    : public CacheObject<TYPE>
{
public:
    using value_t = TYPE;
    using conversion_t = CONV_TYPE;

    NumericCacheObject(const std::atomic_bool& toggle, std::string shortDescr)
        : CacheObject<TYPE>(toggle, shortDescr) {};

    // nativeToUnitFactor == 0 ==> output: unformatted value without units,
    //                             e.g. use for strings
    // nativeToUnitFactor: mValue[in units] = nativeToUnitFactor * raw value
    NumericCacheObject(const std::atomic_bool& toggle, std::string shortDescr, double nativeToUnitFactor)
        : CacheObject<TYPE>(toggle, shortDescr, nativeToUnitFactor) {};

    // units == "" for unit-less values
    // units == "s" ==> output in format weeks days h m s;
    //                  \pre nativeToUnitFactor must convert to seconds
    NumericCacheObject(const std::atomic_bool& toggle, std::string shortDescr,
                double nativeToUnitFactor,
                std::string units)
        : CacheObject<TYPE>(toggle, shortDescr, nativeToUnitFactor)
        , mUnits(units) {};

    virtual ~NumericCacheObject() = default;

    // precision: negative: -n; round to n digits right to the decimal separator
    //            zero:      0; round to integer
    //            positive: +n; round to the n-th digit left from decimal separator
    //            default:  -2; round to 2 digits right to the decimal separator
    void setPrecision(const int digits);
    double getPrecision() const;
    void setDelta(const double digits);
    [[nodiscard]] double getDelta() const;

    std::string getUnits() const override { return mUnits; };

private:
    virtual void setRaw(const std::string& rawValue) override;

    std::string mUnits = {}; //!< e.g. A, V, km/h, % ...
    //!< default precision -2 digits after dot
    value_t mPrecision = (std::is_floating_point<value_t>::value
                          ? static_cast<value_t>(0.01)
                          : static_cast<value_t>(1));
    value_t mDelta = mPrecision; //!< default delta; if delta exceeded,change propagated via sigc
};

enum class Alarmcode : uint16_t {
    // BMV alarms + Phoenix Inverter alarms
    Low_voltage = 0x0001,
    High_voltage = 0x0002,
    Low_temperature = 0x0020,
    High_temperature = 0x0040,
    // BMV (only) alarms
    Low_state_of_charge = 0x0004,
    Low_starter_voltage = 0x0008,
    High_starter_voltage = 0x0010,
    Mid_voltage = 0x0080,
    // Phoenix Inverter alarms
    Overload = 0x0100,
    DC_ripple = 0x0200,
    Low_V_AC_out = 0x0400,
    High_V_AC_out = 0x0800,
    Max = 0x0FFF,
    //friend std::ostream& operator<<(std::ostream& os, Toggle t);
};

inline std::string to_string(Toggle t) {
    switch (t) {
    case Toggle::On:    return "On";
    case Toggle::Off:   return "Off";
    default:            return "Unknown";
    }
};

// Definition of the << operator for enum Toggle
inline std::ostream& operator<<(std::ostream& os, Toggle t) {
    os << to_string(t);
    return os;
};
    

inline std::string getAlarmText(uint16_t alarmcode) {
    // BMV alarms + Phoenix Inverter alarms
    if (alarmcode & 0x0001) return "Low voltage";
    if (alarmcode & 0x0002) return "High voltage";
    if (alarmcode & 0x0020) return "Low temperature";
    if (alarmcode & 0x0040) return "High temperature";
    // BMV (only) alarms
    if (alarmcode & 0x0004) return "Low state of charge (SOC)";
    if (alarmcode & 0x0008) return "Low starter voltage";
    if (alarmcode & 0x0010) return "High starter voltage";
    if (alarmcode & 0x0080) return "Mid voltage";
    // Phoenix Inverter alarms
    if (alarmcode & 0x0100) return "Overload";
    if (alarmcode & 0x0200) return "DC-ripple";
    if (alarmcode & 0x0400) return "Low V AC out";
    if (alarmcode & 0x0800) return "High V AC out";
    if (alarmcode > 0x0FFF) std::cerr << "getAlarmText: Unknown alarm code: " + alarmcode << std::endl;
    return "no alarm";
};

inline std::string getStateOfOperationText(int state) {
    // State of operation
    switch(state) {
    case    '0': // applies to MPPT and Inverter
        return "OFF";
    case    '1': // applies to Inverter
        return "Low power"; // load search
    case    '2': // applies to MPPT and Inverter
        return "Fault"; // off until user reset
    case    '3': // applies to MPPT
        return "Bulk";
    case    '4': // applies to MPPT
        return "Absorption";
    case    '5': // applies to MPPT
        return "Float";
    case    '9': // applies to Inverter
        return "Inverting"; // on
    }
    std::cerr << "getStateOfOperationText: Unknown charge state: " << state << std::endl;
    return "unknown";
};

inline std::string getProductLongname(int pid) {
    // logger.trace('getProductLongname');
    if (pid == 0x203) return "BMV-700";
    if (pid == 0x204) return "BMV-702";
    if (pid == 0x205) return "BMV-700H";
    if (pid == 0xA381) return "BMV-712";
    if (pid == 0x300) return "BlueSolar MPPT 70/15";        // model phased out
    if (pid == 0xA04C) return "BlueSolar MPPT 75/10";
    if (pid == 0xA042) return "BlueSolar MPPT 75/15";
    if (pid == 0xA040) return "BlueSolar MPPT 75/50";        // model phased out
    if (pid == 0xA043) return "BlueSolar MPPT 100/15";
    if (pid == 0xA044) return "BlueSolar MPPT 100/30";       // model phased out
    if (pid == 0xA04A) return "BlueSolar MPPT 100/30 rev 2";
    if (pid == 0xA045) return "BlueSolar MPPT 100/50 rev 1"; // model phased out
    if (pid == 0xA049) return "BlueSolar MPPT 100/50 rev 2";
    if (pid == 0xA041) return "BlueSolar MPPT 150/35 rev 1"; // model phased out
    if (pid == 0xA04B) return "BlueSolar MPPT 150/35 rev 2";
    if (pid == 0xA04D) return "BlueSolar MPPT 150/45";
    if (pid == 0xA04E) return "BlueSolar MPPT 150/60";
    if (pid == 0xA046) return "BlueSolar MPPT 150/70";
    if (pid == 0xA04F) return "BlueSolar MPPT 150/85";
    if (pid == 0xA047) return "BlueSolar MPPT 150/100";
    if (pid == 0xA051) return "SmartSolar MPPT 150/100";
    if (pid == 0xA050) return "SmartSolar MPPT 250/100";
    if (pid == 0xA201) return "Phoenix Inverter 12V 250VA 230V";
    if (pid == 0xA202) return "Phoenix Inverter 24V 250VA 230V";
    if (pid == 0xA204) return "Phoenix Inverter 48V 250VA 230V";
    if (pid == 0xA211) return "Phoenix Inverter 12V 375VA 230V";
    if (pid == 0xA212) return "Phoenix Inverter 24V 375VA 230V";
    if (pid == 0xA214) return "Phoenix Inverter 48V 375VA 230V";
    if (pid == 0xA221) return "Phoenix Inverter 12V 500VA 230V";
    if (pid == 0xA222) return "Phoenix Inverter 24V 500VA 230V";
    if (pid == 0xA224) return "Phoenix Inverter 48V 500VA 230V";
    if (pid) std::cerr << "getProductLongname: Unknown product: " << pid << std::endl;
    return "Unknown";
};


namespace detail {
    // works for std::string and std::wstring

// data sizees - uint8_t, uint16_t, uint32_t 1, 2, 4, 
//data sizees - int8_t, int16_t, int32_t 1, 2, 4, 
// data sizees - int, long, long long 4, 4, 8
// missing short and ushort
// float       stof ( const std::string& str, std::size_t* pos = nullptr );
// (1)	(since C++11)
// float       stof ( const std::wstring& str, std::size_t* pos = nullptr );
// (2)	(since C++11)
// double      stod ( const std::string& str, std::size_t* pos = nullptr );
// (3)	(since C++11)
// double      stod ( const std::wstring& str, std::size_t* pos = nullptr );
// (4)	(since C++11)
// long double stold( const std::string& str, std::size_t* pos = nullptr );
// (5)	(since C++11)
// long double stold( const std::wstring& str, std::size_t* pos = nullptr );
// (6)	(since C++11)



    template <typename T, typename CT>
    static void set(T& value, const std::string& strValue, double convFactor, T precision) {
        try {
            // converts string to int for int, uint, int8_t, uint8_t, int16_t, uint16_t
            CT cValue = std::stoi(strValue);
            value = static_cast<T>(cValue);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Conversion of " << strValue << " not possible" << e.what() << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Conversion out of range" << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Conversion not possible" << std::endl;
        }
        value = std::round(value * convFactor / precision) * precision;
    }

    // template <>
    // void set<Alarmcode, uint16_t>(Alarmcode& value, const std::string& strValue, double convFactor, double precision) {
    //     try {
    //         // converts string to int for int, uint, int8_t, uint8_t, int16_t, uint16_t
    //         uint16_t cValue = std::stoi(strValue);
    //         value = static_cast<Alarmcode>(cValue);
    //     } catch (const std::invalid_argument& e) {
    //         std::cerr << "Conversion of " << strValue << " not possible" << e.what() << std::endl;
    //     } catch (const std::out_of_range& e) {
    //         std::cerr << "Conversion out of range" << e.what() << std::endl;
    //     } catch (...) {
    //         std::cerr << "Conversion not possible" << std::endl;
    //     }
    // }

    template <typename T, typename CT>
    static void set(T& value, const std::string& strValue, double convFactor) {
        try {
            // converts string to int for int, uint, int8_t, uint8_t, int16_t, uint16_t
            CT cValue = std::stoi(strValue);
            value = static_cast<T>(cValue);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Conversion of " << strValue << " not possible" << e.what() << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Conversion out of range" << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Conversion not possible" << std::endl;
        }
        value = std::round(value * convFactor);
    }

    template <>
    void set<Toggle, std::string>(Toggle& value, const std::string& strValue, double convFactor) {
        if (boost::iequals(strValue, "on")) {
            value = Toggle::On;
        } else {
            value = Toggle::Off;
        }
    }

    template <>
    void set<double, int32_t>(double& value, const std::string& strValue, double convFactor, double precision) {
        try {
            // converts string to long
            int32_t cValue = std::stol(strValue);
            value = static_cast<double>(cValue);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Conversion of " << strValue << " to " << " int32_t not possible" << e.what() << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Conversion out of range" << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Conversion not possible" << std::endl;
        }
        value = std::round(value * convFactor / precision) * precision;
    }

    template <>
    void set<double, uint32_t>(double& value, const std::string& strValue, double convFactor, double precision) {
        try {
            // converts string to unsigned long
            uint32_t cValue = std::stoul(strValue);
            value = static_cast<double>(cValue);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Conversion of " << strValue << " to " << " uint32_t not possible" << e.what() << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Conversion out of range" << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Conversion not possible" << std::endl;
        }
        value = std::round(value * convFactor / precision) * precision;
    }

    template <>
    void set<double, long>(double& value, const std::string& strValue, double convFactor, double precision) {
        try {
            // converts string to long
            long cValue = std::stol(strValue);
            value = static_cast<double>(cValue);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Conversion of " << strValue << " to " << " long not possible" << e.what() << std::endl;
            std::cerr << "Conversion not possible" << e.what() << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Conversion out of range" << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Conversion not possible" << std::endl;
        }
        value = std::round(value * convFactor / precision) * precision;
    }

    // long long is the same type as int64_t - no specialization needed
    template <>
    void set<double, long long>(double& value, const std::string& strValue, double convFactor, double precision) {
        try {
            // converts string to long long
            long long cValue = std::stoll(strValue);
            value = static_cast<double>(cValue);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Conversion of " << strValue << " to " << " long long not possible" << e.what() << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Conversion out of range" << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Conversion not possible" << std::endl;
        }
        value = std::round(value * convFactor / precision) * precision;
    }

    template<>
    void set<double, unsigned long>(double& value, const std::string& strValue, double convFactor, double precision) {
        try {
            // converts string to unsigned long
            unsigned long cValue = std::stoul(strValue);
            value = static_cast<double>(cValue);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Conversion of " << strValue << " to " << " unsigned long not possible" << e.what() << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Conversion out of range" << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Conversion not possible" << std::endl;
        }
        value = std::round(value * convFactor / precision) * precision;
    }

    // unsigned long long is the same type as uint64_t - no specialization needed
    template <>
    void set<double, unsigned long long>(double& value, const std::string& strValue, double convFactor, double precision) {
        try {
            // converts string to unsigned long long
            unsigned long long cValue = std::stoull(strValue);
            value = static_cast<double>(cValue);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Conversion of " << strValue << " to " << " unsigned long long not possible" << e.what() << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Conversion out of range" << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Conversion not possible" << std::endl;
        }
        value = std::round(value * convFactor / precision) * precision;
    }

    template <>
    void set<std::string, std::string>(std::string& value, const std::string& strValue,
                                       double convFactor) {
        value = strValue;
    }

    // template <>
    // void set<std::wstring>(std::wstring& value, const std::string& strValue,
    //                       CacheObject<std::wstring>* co) {
    //     value = strValue;
    // }
}



// template<typename TYPE, CONV_TYPE>
// unsigned int CacheObject<TYPE>::hexToUint(const std::string& hexStr) {
//     unsigned int x = 0;   
//     std::stringstream ss;
//     ss << std::hex << hexStr; // without 0x
//     ss >> x;
//     return x;
// }

// template<typename TYPE, CONV_TYPE>
// static TYPE CacheObject<TYPE>::hexToSint(const std::string& hexStr) {
//     return static_cast<TYPE>(hexToUint(hexStr));
// }

    // nativeToUnitFactor == 0 ==> output: unformatted value without units,
    //                             e.g. use for strings
    // nativeToUnitFactor: mValue[in units] = nativeToUnitFactor * raw value
    // units == "" for unit-less values
    // units == "s" ==> output in format weeks days h m s;
    //                  \pre nativeToUnitFactor must convert to seconds

// template<typename TYPE>
// CacheObject<TYPE>::CacheObject(std::string shortDescr,
//                                           double nativeToUnitFactor,
//                                           std::string units)
//     : SerializedCacheObject(shortDescr)
//     , mNativeToUnitFactor(nativeToUnitFactor)
//     , mUnits(units)
// {};

template<typename TYPE>
CacheObject<TYPE>::CacheObject(const std::atomic_bool& toggle, std::string shortDescr)
    : SerializedCacheObject(shortDescr)
    , mReadable(toggle)
{}

template<typename TYPE>
CacheObject<TYPE>::CacheObject(const std::atomic_bool& toggle, std::string shortDescr, double nativeToUnitFactor)
    : CacheObject(toggle, shortDescr)
{
    mNativeToUnitFactor = nativeToUnitFactor; 
}

template<typename TYPE, typename CONV_TYPE>
void NumericCacheObject<TYPE, CONV_TYPE>::setPrecision(const int digits) {
    static_assert(std::is_integral<TYPE>::value || std::is_floating_point<TYPE>::value,
                  "ERROR: cannot set precision for non integral type");
    //!< default precision -2 digits after dot i.e. 0.01
    mPrecision = pow(10, digits);
}

template<typename TYPE, typename CONV_TYPE>
double NumericCacheObject<TYPE, CONV_TYPE>::getPrecision() const {
    return mPrecision;
}

template<typename TYPE, typename CONV_TYPE>
void NumericCacheObject<TYPE, CONV_TYPE>::setDelta(const double digits) {
    mDelta = pow(10, digits);
}

template<typename TYPE, typename CONV_TYPE>
[[nodiscard]] double NumericCacheObject<TYPE, CONV_TYPE>::getDelta() const {
    return mDelta;
}

template<typename TYPE, typename CONV_TYPE>
void NumericCacheObject<TYPE, CONV_TYPE>::setRaw(const std::string& rawValue) {
    assert(static_cast<int>(false) == 0);
    assert(static_cast<int>(true)  == 1);
    detail::set<TYPE, CONV_TYPE>(this->mValue[static_cast<int>(!this->mReadable)], rawValue, this->mNativeToUnitFactor, this->mPrecision);
}

template<typename TYPE, typename CONV_TYPE>
void AnyCacheObject<TYPE, CONV_TYPE>::setRaw(const std::string& rawValue) {
    assert(static_cast<int>(false) == 0);
    assert(static_cast<int>(true)  == 1);
    detail::set<TYPE, CONV_TYPE>(this->mValue[static_cast<int>(!this->mReadable)], rawValue, this->mNativeToUnitFactor);
}

// for setting via addresses!!!
    // const TYPE&  getRaw() const {
    //     return mValue; 
    //     // if integral: return mValue / mNativeToUnitFactor; 
    // }

template<typename TYPE>
void CacheObject<TYPE>::set(const TYPE value) {
    mValue[static_cast<int>(!mReadable)] = value;
}

template<typename TYPE>
TYPE CacheObject<TYPE>::get() const {
    return mValue[static_cast<int>(mReadable)];
}

