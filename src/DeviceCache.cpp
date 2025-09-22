
#include <climits> //!< import INT_MAX...
#include <iostream> //!< import std::cout...
#include <memory> //!< import std::make_shared
#include <any>
#include <type_traits>
#include <typeindex>
#include <typeinfo>

#include "DeviceCache.h"
#include "CacheObject.h"


namespace detail {
    template <typename T>
    void setByVictronID(const std::string& key, const T& value, Dictionary& mVictronMap) {
        auto iter = mVictronMap.find(key);
        if (iter == mVictronMap.end()) {
            // log error
            return;
        }
        auto anyObjectPtr = iter->second;
        try {
            auto objPtr = std::any_cast<std::shared_ptr<CacheObject<T>>>(anyObjectPtr);
            objPtr->set(value);
        } catch(...) {}
    }

    // template <>
    // void setByVictronID<std::string>(const std::string& key, const std::string& value, Dictionary& mVictronMap) {
    //     auto iter = mVictronMap.find(key);
    //     if (iter == mVictronMap.end()) {
    //         // log error
    //         return;
    //     }
    //     auto anyObjectPtr = iter->second;
    //     auto objPtr = std::any_cast<std::shared_ptr<CacheObject<std::string>>>(anyObjectPtr);
    //     objPtr->set(value);
    // }
}



template<typename T>
void DeviceCache::setByVictronID(const std::string& key, const T& value) {
    detail::setByVictronID<T>(key, value, mVictronMap);
}

void DeviceCache::setByVictronID_(const std::string& key, const std::string& value) {
    // key, value must have been inserted using registerComponent:
    auto iter = mVictronSerialMap.find(key);
    if (iter == mVictronSerialMap.end()) {
        // log error
        return;
    }
    
    auto anyObjectPtr = iter->second;
    try {
        auto objPtr = std::any_cast<std::shared_ptr<SerializedCacheObject>>(anyObjectPtr);
        printf("trying to set value for key %s\n", key.c_str());
        objPtr->setRaw(value);
        printf("done - value set\n");
    } catch(...) {
        std::cerr << "Never set value for " << key << " because ..." << std::endl;
        if (key.compare("AR") == 0) exit(0);
    }

    //if (std::is_same<T, int16_t>::value) {
    // if (std::is_same<decltype(this->mVictronMap.at<int16_t>(key)), int16_t>::value) {
    //     int16_t v;
    //     str2int<int16_t>(v, value.c_str(), 10);
    //     try {
    //         setByVictronID(key, v);
    //     } catch (...) {}
    // }
    // if (std::is_same<decltype(this->mVictronMap.at<uint16_t>(key)), uint16_t>::value) {
    //     uint16_t v;
    //     str2int<uint16_t>(v, value.c_str(), 10);
    //     try {
    //         setByVictronID(key, v);
    //     } catch (...) {}
    // }
    // if (std::is_same<decltype(this->mVictronMap.at<int>(key)), int>::value) {
    //     int v;
    //     str2int<int>(v, value.c_str(), 10);
    //     try {
    //         setByVictronID(key, v);
    //     } catch (...) {}
    // } else if (std::is_same<decltype(this->mVictronMap.at<std::string>(key)), std::string>::value) {
    //     std::string v;
    //     try {
    //         setByVictronID(key, v);
    //     } catch (...) {}
    // }


    //(std::any_cast<std::shared_ptr<CacheObject<T>>(anyObjectPtr));

    //std::is_same<T1, iter->second->value_t>::value
    //using value_t = iter->second->value_t;
    //        decl_type<iter->second->value_t> value = std::any_cast<std::shared_ptr<CacheObject<iter->second->value_t>>>(iter->second)->get<iter->second->value_t>()
    //->get<>();
}

// void DeviceCache::printByVictronID(const std::string& key) {
//     // key, value must have been inserted using registerComponent:
//     auto iter = mVictronMap.find(key);
//     if (iter == mVictronMap.end()) {
//         // log error
//         return;
//     }

//     auto anyObjectPtr = iter->second;
//     if (std::is_same<decltype(this->mVictronMap.at<int16_t>(key)), int16_t>::value) {
//         try {
//             auto objPtr = std::any_cast<std::shared_ptr<CacheObject<int16_t>>>(anyObjectPtr);
//             std::cout << std::endl <<
//                 objPtr->getShortDescription() << ": \t" << objPtr->mValue << objPtr->mUnits << std::endl;
//         } catch(...) {}            
//     }
//     if (std::is_same<decltype(this->mVictronMap.at<uint16_t>(key)), uint16_t>::value) {
//         try {
//             auto objPtr = std::any_cast<std::shared_ptr<CacheObject<uint16_t>>>(anyObjectPtr);
//             std::cout << std::endl <<
//                 objPtr->getShortDescription() << ": \t" << objPtr->mValue << objPtr->mUnits << std::endl;
//         } catch(...) {}
//     }
//     if (std::is_same<decltype(this->mVictronMap.at<int>(key)), int>::value) {
//         try {
//             auto objPtr = std::any_cast<std::shared_ptr<CacheObject<int>>>(anyObjectPtr);
//             std::cout << std::endl <<
//                 objPtr->getShortDescription() << ": \t" << objPtr->mValue << objPtr->mUnits << std::endl;
//         } catch(...) {}
//     }
//     if (std::is_same<decltype(this->mVictronMap.at<std::string>(key)), std::string>::value) {
//         try {
//             auto objPtr = std::any_cast<std::shared_ptr<CacheObject<std::string>>>(anyObjectPtr);
//             std::cout << std::endl <<
//                 objPtr->getShortDescription() << ": \t" << objPtr->mValue << std::endl;
//         } catch(...) {}
//     }

// }

template<typename SHAREDOBJ>
void DeviceCache::addToDictionaries(const SHAREDOBJ obj,
                              const std::string& label,    // e.g. alarmReason
                              const std::string& bmvLabel, // e.g. "AR"
                              const std::string& address   // BMV address
                              ) {
    if (!label.empty()) mCache.insert(label, obj);
    if (!bmvLabel.empty()) {
        mVictronMap.insert(bmvLabel, obj);
        auto serialObj = std::dynamic_pointer_cast<SerializedCacheObject>(obj);
        if (serialObj == nullptr) {
            printf("failed to convert %s \n", label.c_str());
            return;
        }
        mVictronSerialMap.insert(bmvLabel, serialObj);
    }
    if (!address.empty()) mAddressCache.insert(address, obj);
}


// For dynamical registration of objects as they come in from the Victron unit.
// Dynamical registration saves about 50 objects when operating with BMV-702
// (34 instead of all 84 objects).
void DeviceCache::registerComponent(const std::string& key) {
    //logger.trace("registerComponent(" + key + ")");
    // options:    list of key values, known keys: precision, description, formatter
    //             precision: negative: -n; round to n digits right to the decimal separator
    //                        zero:      0; round to integer
    //                        positive: +n; round to the n-th digit left from decimal separator
    //                        default:  -2; round to 2 digits right to the decimal separator

    if (mVictronMap.find(key) != mVictronMap.end()) return;

    if (key == "AR") {
        // Monitored values:
        // BMV600, BMV700, Phoenix Inverter
        //auto obj = std::make_shared<CacheObject<Alarmcode, uint16_t>>("Alarm reason");
        auto obj = std::make_shared<NumericCacheObject<int, uint16_t>>(mReadable, "Alarm reason");
        addToDictionaries(obj, "alarmReason", "AR", "0xEEFC");

        // // mCache["alarmReason"]         = new CacheObject(1,      "",    "Alarm reason",
        // //                                               {'precision") { 0, 'formatter' : function() 
        // //                                                {
        // //                                                    return this.getAlarmText(this.value);
        // //                                                }});
        // // // the key of the map is a string identifier that comes with the value sent by BMV
    } else if (key == "I") {
        // BMV600, BMV700, MPPT - Type Sn16; Unit: 0.1A!!!
        // On BMV-712 >v4.01 and BMV-70x >v3.09: Type: Sn32; Unit: 0.001A
        // FIXME: different type depending on BMV...
        auto obj = std::make_shared<NumericCacheObject<double, int16_t>>(mReadable, "Battery Current", 0.001,  "A");
        addToDictionaries(obj, "batteryCurrent", "I", "0xED8F");

        // //{'fromHexStr") { (hex) => { return 100 * conv.hexToSint(hex); } });
        // only on BMV-712 > v4.01 and BMV-70x > v3.09: is might be address '0xED8C'
    } else if (key == "IL") {
        // MPPT
        auto obj = std::make_shared<NumericCacheObject<double, int>>(mReadable, "Load Current", 0.001,  "A");
        addToDictionaries(obj, "loadCurrent", "IL");
        // FIXME: what about mAddressCache?
    } else if (key == "LOAD") {
        // MPPT - returns string 'ON' or 'OFF'
        auto obj = std::make_shared<AnyCacheObject<Toggle, std::string>>(mReadable, "Load Output State");
        //auto obj = std::make_shared<TextCacheObject>("Load Output State");
        //obj->setHexConversion("hexToOnOff");
        addToDictionaries(obj, "load", "LOAD");
        // FIXME: what about mAddressCache?
    } else if (key == "V") {
        // BMV600, BMV700, MPPT, Phoenix Inverter - Display: MAIN; Type: Sn16; Unit: 0.01V!!!
        auto obj = std::make_shared<NumericCacheObject<double, int16_t>>(mReadable, "Main Voltage", 0.001,  "V");
        //auto serialObj = std::dynamic_pointer_cast<SerializedCacheObject>(obj);
        obj->setDescription("Main (Battery) Voltage");
        obj->setPrecision(-1);
        addToDictionaries(obj, "upperVoltage", "V");
        // //        'fromHexStr") { (hex) => { return 10 * conv.hexToSint(hex); }});
        // not to be used: mAddressCache.insert("0xED8D", obj);
    } else if (key == "VM") {
        // BMV700 - Display: MID; Type: Un16; Units: 0.01V!!! (only BMV-702 and BMV-712)
        auto obj = std::make_shared<NumericCacheObject<double, uint16_t>>(mReadable, "Mid Voltage", 0.001,  "V");
        obj->setDescription("Mid-point Voltage of the Battery Bank");
        obj->setPrecision(-1);
        addToDictionaries(obj, "midVoltage", "VM", "0x0382");
    } else if (key == "P") {
        // BMV700 - Type: Sn16; Unit: W
        auto obj = std::make_shared<NumericCacheObject<double, int16_t>>(mReadable, "Instantaneous Power", 1.0, "W");
        addToDictionaries(obj, "instantPower", "P", "0xED8E");
    } else if (key == "SOC") {
        // BMV600, BMV700 - Type: Un16; Unit: 0.01%!!! for 0x0FFF
        //                  Type: Un8 for 0xEEB6 ??? (Synchronisation State)
        auto obj = std::make_shared<NumericCacheObject<double, uint16_t>>(mReadable, "State of charge", 0.1,    "%");
        obj->setPrecision(-1);
        addToDictionaries(obj, "stateOfCharge", "SOC", "0x0FFF");
    } else if (key == "VS") {
        // BMV600, BMV700 - Display: AUX; Type: Sn16; Unit: 0.01V!!! (not available on BMV-702 and BMV-712)
        auto obj = std::make_shared<NumericCacheObject<double, int16_t>>(mReadable, "Aux. Voltage", 0.001,  "V");
        obj->setPrecision(-1);
        obj->setDescription("Auxiliary (starter) Voltage");
        addToDictionaries(obj, "auxVolt", "VS", "0xED7D");
    } else if (key == "CE") {
        // BMV600, BMV700 - Type: Sn32; Unit: 0.1 Ah!!!
        auto obj = std::make_shared<NumericCacheObject<double, int32_t>>(mReadable, "Consumed", 0.001,  "Ah");
        obj->setDescription("Consumed Ampere Hours");
        addToDictionaries(obj, "consumedAh", "CE", "0xEEFF");
    } else if (key == "DM") {
        // BMV700 - Display: MID; Type: Sn16; Units: 0.1 %
        auto obj = std::make_shared<NumericCacheObject<double, int16_t>>(mReadable, "Mid Deviation", 1.0,  "%");
        obj->setDescription("Mid-point Deviation of the Battery Bank");
        addToDictionaries(obj, "midDeviation", "DM", "0x0383");
    } else if (key == "VPV") {
        // MPPT
        auto obj = std::make_shared<NumericCacheObject<double, int>>(mReadable, "Panel Voltage", 0.001,  "V");
        addToDictionaries(obj, "panelVoltage", "VPV");
    } else if (key == "PPV") {
        // MPPT
        auto obj = std::make_shared<NumericCacheObject<double, int>>(mReadable, "Panel Power", 1.0,  "W");
        addToDictionaries(obj, "panelPower", "PPV");
    } else if (key == "CS") {
        // MPPT, Phoenix Inverter
        auto obj = std::make_shared<NumericCacheObject<int, int>>(mReadable, "State of Operation");
        addToDictionaries(obj, "stateOfOperation", "CS", "");

        // mCache.insert("stateOfOperation", obj);
        // // mCache["stateOfOperation"]    = new CacheObject(0,      "",    "State of Operation",
        // //                                               {'formatter' : function() 
        // //                                                {
        // //                                                    return this.getStateOfOperationText(this.value);
        // //                                                }});
        // mVictronMap.insert("CS", obj);
    } else if (key == "PID") {
        // BMV700, MPPT, Phoenix Inverter
        auto obj = std::make_shared<NumericCacheObject<int, int>>(mReadable, "Product ID");
        addToDictionaries(obj, "productId", "PID");

        // // {'formatter' : function() 
        // //         {
        // //             this.mapComponents(mCache["productId.value);
        // //                                       return this.getProductLongname(this.value);
        // //                                       }});
    } else if (key == "FW") {
        // BMV600, BMV700, MPPT, Phoenix Inverter
        auto obj = std::make_shared<NumericCacheObject<double, int>>(mReadable, "Firmware version", 0.01);
        obj->setPrecision(-2);
        addToDictionaries(obj, "version", "FW");
    } else 
    // History values
    if (key == "H1") {
        // BMV600, BMV700
        auto obj = std::make_shared<NumericCacheObject<double, int>>(mReadable, "Deepest Discharge", 0.001,  "Ah");
        obj->setPrecision(-2);
        obj->setDescription("Depth of deepest discharge");
        addToDictionaries(obj, "deepestDischarge", "H1", "0x0300");

        // //                          'fromHexStr' : (hex) => { return 100 * conv.hexToSint(hex); } });
    } else if (key == "H2") {
        // BMV600, BMV700
        auto obj = std::make_shared<NumericCacheObject<double, int>>(mReadable, "Last Discharge", 0.001,  "Ah");
        obj->setPrecision(0);
        obj->setDescription("Depth of last discharge");
        addToDictionaries(obj, "maxAHsinceLastSync", "H2", "0x0301");

        // //'fromHexStr") { (hex) => { return 100 * conv.hexToSint(hex); } });
    } else if (key == "H3") {
        // BMV600, BMV700
        auto obj = std::make_shared<NumericCacheObject<double, int>>(mReadable, "Avg. Discharge", 0.001,  "Ah");
        obj->setPrecision(0);
        obj->setDescription("Depth of average discharge");
        addToDictionaries(obj, "avgDischarge", "H3", "0x0302");

        // //'fromHexStr' : (hex) => { return 100 * conv.hexToSint(hex); }});
    } else if (key == "H4") {
        // BMV600, BMV700
        auto obj = std::make_shared<NumericCacheObject<int, int>>(mReadable, "Charge Cycles");
        obj->setDescription("Number of charge cycles");
        addToDictionaries(obj, "chargeCycles", "H4", "0x0303");
    } else if (key == "H5") {
        // BMV600, BMV700
        auto obj = std::make_shared<NumericCacheObject<int, int>>(mReadable, "Full Discharges");
        obj->setDescription("Number of full discharges");
        addToDictionaries(obj, "fullDischarges", "H5", "0x0304");
    } else if (key == "H6") {
        // BMV600, BMV700
        auto obj = std::make_shared<NumericCacheObject<double, int>>(mReadable, "Cum. Ah drawn", 0.001,  "Ah");
        addToDictionaries(obj, "drawnAh", "H6", "0x0305");
        // // { 'fromHexStr") { (hex) => { return 100 * conv.hexToSint(hex); }});
    } else if (key == "H7") {
        // BMV600, BMV700
        auto obj = std::make_shared<NumericCacheObject<double, int>>(mReadable, "Min. Voltage", 0.001,  "V");
        obj->setDescription("Minimum Main (Battery) Voltage");
        addToDictionaries(obj, "minVoltage", "H7", "0x0306");
        // // 'fromHexStr") { (hex) => { return 10 * conv.hexToSint(hex); }});
    } else if (key == "H8") {
        // BMV600, BMV700
        auto obj = std::make_shared<NumericCacheObject<double, int>>(mReadable, "Max. Voltage", 0.001,  "V");
        obj->setDescription("Maximum Main (Battery) Voltage");
        addToDictionaries(obj, "maxVoltage", "H8", "0x0307");
        // // 'fromHexStr") { (hex) => { return 10 * conv.hexToSint(hex); }});
    } else if (key == "H9") {
        // BMV600, BMV700
        auto obj = std::make_shared<NumericCacheObject<int, int>>(mReadable, "Time since Full Charge", 1.0,  "s");
        obj->setDescription("Number of seconds since full charge");
        addToDictionaries(obj, "timeSinceFullCharge", "H9", "0x0308");
    } else if (key == "H10") {
        // BMV600, BMV700
        auto obj = std::make_shared<NumericCacheObject<int, int>>(mReadable, "Auto. Syncs", 1);
        obj->setDescription("Number of automatic synchronisations");
        addToDictionaries(obj, "noAutoSyncs", "H10", "0x0309");
    } else if (key == "H11") {
        // BMV600, BMV700
        // FIXME: precision: 0 not working in low/high volt alarms (particularly 0 is the issue)
        auto obj = std::make_shared<NumericCacheObject<int, int>>(mReadable, "Low Volt. Alarms", 1);
        obj->setPrecision(-1);
        obj->setDescription("Number of Low Main Voltage Alarms");
        addToDictionaries(obj, "lowVoltageAlarms", "H11", "0x030A");
    } else if (key == "H12") {
        // BMV600, BMV700
        auto obj = std::make_shared<NumericCacheObject<int, int>>(mReadable, "High Volt. Alarms", 1);
        obj->setPrecision(-1);
        obj->setDescription("Number of High Main Voltage Alarms");
        addToDictionaries(obj, "highVoltageAlarms", "H12", "0x030B");
    } else if (key == "H13") {
        // BMV600
        auto obj = std::make_shared<NumericCacheObject<int, int>>(mReadable, "Low Aux. Volt. Alarms", 1);
        obj->setDescription("Number of Low Auxiliary Voltage Alarms");
        addToDictionaries(obj, "lowAuxVoltageAlarms", "H13", "0x030C");
        // FIXME: address?
    } else if (key == "H14") {
        // BMV600
        auto obj = std::make_shared<NumericCacheObject<int, int>>(mReadable, "High Aux. Volt. Alarms", 1,     "");
        obj->setDescription("Number of High Aux. Voltage Alarms");
        addToDictionaries(obj, "highAuxVoltageAlarms", "H14", "0x030D");
    } else if (key == "H15") {
        // BMV600, BMV700
        auto obj = std::make_shared<NumericCacheObject<double, int>>(mReadable, "Min. Aux. Volt.", 0.001, "V");
        obj->setDescription("Minimal Auxiliary (Battery) Voltage");
        addToDictionaries(obj, "minAuxVoltage", "H15", "0x030E");
        // // 'fromHexStr") { (hex) => { return 10 * conv.hexToSint(hex); }});
    } else if (key == "H16") {
        // BMV600, BMV700
        auto obj = std::make_shared<NumericCacheObject<double, int>>(mReadable, "Min. Aux. Volt.", 0.001, "V");
        obj->setDescription("Maximal Auxiliary (Battery) Voltage");
        addToDictionaries(obj, "maxAuxVoltage", "H16", "0x030F");
        // // 'fromHexStr") { (hex) => { return 10 * conv.hexToSint(hex); }});
    } else if (key == "H17") {
        // BMV700
        auto obj = std::make_shared<NumericCacheObject<double, int>>(mReadable, "Drawn Energy", 0.01,  "kWh");
        obj->setDescription("Amount of Discharged Energy");
        addToDictionaries(obj, "dischargeEnergy", "H17", "0x0310");
    } else if (key == "H18") {
        // BMV700
        auto obj = std::make_shared<NumericCacheObject<double, int>>(mReadable, "Absorbed Energy", 0.01,  "kWh");
        obj->setDescription("Amount of Charged Energy");
        addToDictionaries(obj, "absorbedEnergy", "H18", "0x0311");
    } else if (key == "H19") {
        // MPPT
        auto obj = std::make_shared<NumericCacheObject<double, int>>(mReadable, "Yield Total", 0.01,  "kWh");
        obj->setDescription("User resettable counter");
        addToDictionaries(obj, "yieldTotal", "H19", "0x0312");
    } else if (key == "H20") {
        // MPPT
        auto obj = std::make_shared<NumericCacheObject<double, int>>(mReadable, "Yield Today", 0.01,  "kWh");
        addToDictionaries(obj, "yieldToday", "H20", "0x0313");
    } else if (key == "H21") {
        // MPPT
        auto obj = std::make_shared<NumericCacheObject<double, int>>(mReadable, "Max. Power Today", 1.0,   "W");
        addToDictionaries(obj, "maxPowerToday", "H21", "0x0314");
    } else if (key == "H22") {
        // MPPT
        auto obj = std::make_shared<NumericCacheObject<double, int>>(mReadable, "Yield Yesterday", 0.01,  "kWh");
        addToDictionaries(obj, "yieldYesterday", "H22", "0x0315");
    } else if (key == "H23") {
        // MPPT
        auto obj = std::make_shared<NumericCacheObject<double, int>>(mReadable, "Max. Power Yesterday", 1.0,   "W");
        addToDictionaries(obj, "maxPowerYesterday", "H23", "0x0316");
    } else if (key == "ERR") {
        // MPPT
        auto obj = std::make_shared<NumericCacheObject<int, int>>(mReadable, "MPPT Error Code", 1,     "");
        addToDictionaries(obj, "errorCode", "ERR", "");

        // mCache.insert("errorCode", obj);
        // // {'formatter' : function() 
        // //         {
        // //             return this.getErrorText(this.value);
        // //         }});
        // mVictronMap.insert("ERR", obj);
    } else if (key == "WARN") {
        // Phoenix Inverter
        // FIXME: no type was specified here
        auto obj = std::make_shared<NumericCacheObject<int, int>>(mReadable, "Warning Reason", 0);
        addToDictionaries(obj, "warnReason", "WARN");
    } else if (key == "SER#") {
        // MPPT, Phoenix Inverter
        // FIXME: no type was specified here
        auto obj = std::make_shared<NumericCacheObject<int, int>>(mReadable, "Serial Number", 0);
        addToDictionaries(obj, "serialNumber", "SER#");
    } else if (key == "HSDS") {
        // BlueSolar MPPT - returns 0WARNWARNWARN..364
        auto obj = std::make_shared<NumericCacheObject<int, int>>(mReadable, "Day Sequence Number", 1);
        addToDictionaries(obj, "daySequenceNumber", "HSDS");
    } else if (key == "MODE") {
        // Phoenix Inverter
        auto obj = std::make_shared<NumericCacheObject<int, int>>(mReadable, "Device Mode", 1);
        addToDictionaries(obj, "deviceMode", "MODE", "");

        // // {'formatter' : function() 
        // //         {
        // //             return this.getDeviceModeText(this.value);
        // //         }});
    } else if (key == "AC_OUT_V") {
        // Phoenix Inverter
        auto obj = std::make_shared<NumericCacheObject<double, int>>(mReadable, "AC Output Voltage", 0.01,"V");
        addToDictionaries(obj, "ACoutVoltage", "AC_OUT_V");
    } else if (key == "AC_OUT_I") {
        // Phoenix Inverter
        auto obj = std::make_shared<NumericCacheObject<double, int>>(mReadable, "AC Output Current", 0.1, "A");
        addToDictionaries(obj, "ACoutCurrent", "AC_OUT_I", "");
    } else if (key == "TTG") {
        // BMV600, BMV700 - Type: Un16; Units: minutes
        auto obj = std::make_shared<NumericCacheObject<int, int>>(mReadable, "Time to go", 60.0,  "s");
        obj->setDescription("Time until discharged");
        addToDictionaries(obj, "timeToGo", "TTG", "0x0FFE");
    } else if (key == "Alarm") {
        // BMV600, BMV700 - returns string 'ON' or 'OFF'
        //FIXME: no type was given
        auto obj = std::make_shared<AnyCacheObject<Toggle, std::string>>(mReadable, "Alarm state");
        obj->setDescription("Alarm condition active");
        addToDictionaries(obj, "alarmState", "Alarm");
        // // 'fromHexStr") { conv.hexToOnOff
    } else if (key == "Relay") {
        // BMV600, BMV700, SmartSolar MPPT - returns string 'ON' or 'OFF'
        auto obj = std::make_shared<AnyCacheObject<Toggle, std::string>>(mReadable, "Relay state");
        addToDictionaries(obj, "relayState", "Relay", "0x034E");
        // FIXME: how does this value behave with the inversion of the relay?
    } else if (key == "BMV") {
        // BMV600, BMV700
        //FIXME: no type was given
        auto obj = std::make_shared<NumericCacheObject<int, int>>(mReadable, "Model Description", 0,   "");
        addToDictionaries(obj, "modelDescription", "BMV", "");
    } else {
        // FIXME: the following keys 'Cap', 'CV', 'TC' etc do not exist in the
        //        frequent updates...
        //logger.warn("registerComponent: " + key
        //            + " is not a known key of a Victron device");
        std::cout << "registerComponent: " << key 
                  << " is not a known key of a Victron device" << std::endl;
    }
}

