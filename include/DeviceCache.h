#pragma once

#include <atomic>
#include <memory>
#include <string>
#include "Dictionary.h"
#include "CacheObject.h"


class DeviceCache {
public:
    DeviceCache() {;
        mapStaticComponents();
    }
    ~DeviceCache() = default;

    template<class T>
    void setByVictronID(const std::string& key, const T& value);

    /**!
       @brief Seaches key in dict
       @param key the label to search for in dict
       @param dict the given dictionary
       @return true if found in in dict, otherwise false
    */
    template<class T>
    bool get(const std::string& key, Dictionary& dict) {
        printf("in DeviceCache::get(.,.)\n");
        auto iter = dict.find(key);
        if (iter == dict.end()) {
            //std::cerr << key << " not found in victron map" << std::endl;
            return false;
        }
        // key, value must have been inserted using registerComponent:
        std::shared_ptr<SerializedCacheObject> d
            = dict.at<SerializedCacheObject>(key);
        std::shared_ptr<CacheObject<T>> c = std::dynamic_pointer_cast<CacheObject<T>>(d);
        if (c == nullptr) {
            printf("failed to dyn pointer cast in getByVictronID for %s\n", d->getShortDescription().c_str());
            return false;
        }

        auto descr = c->getShortDescription();
        auto value = c->get();
        
        std::cout << descr << " ("
                  << key << ")" << ": "
                  << value
                  << c->getUnits()
                  << std::endl;
        return true;
    }

    bool isHexString(const std::string&key) const {
        return key.size() > 2
            && key.compare(0, 2, "0x") == 0
            && key.find_first_not_of("0123456789abcdefABCDEF", 2) == std::string::npos;
    }

    template<class T>
    std::string getInDicts(const std::string& key) {
        printf("in DeviceCache::getInDicts\n");
        T value = {};

        // if key is an hex address then mAddressCache has to be used
        if (isHexString(key) && get<T>(key, mAddressCache)) return "found in address cache";
        if (get<T>(key, mCache)) return "found in cache";
        if (get<T>(key, mVictronMap)) return "found in Victron cache";
        return "not found";
    }

    std::string get(const std::string& key) {
        printf("in DeviceCache::get\n");
        std::string retval = {};
        try { retval = getInDicts<int>(key); } catch (...) {};
        try { retval = getInDicts<double>(key); } catch (...) {};
        try { retval = getInDicts<std::string>(key); } catch (...) {};
        try { retval = getInDicts<Toggle>(key); } catch (...) {};
        return retval.empty() ? "not found" : retval;
    }

    template<class T>
    void getByVictronID_(const std::string& key) {
        auto iter = mVictronSerialMap.find(key);
        if (iter == mVictronSerialMap.end()) {
            //std::cerr << key << " not found in victron map" << std::endl;
            return;
        }
        // key, value must have been inserted using registerComponent:
        std::shared_ptr<SerializedCacheObject> d
            = mVictronSerialMap.at<SerializedCacheObject>(key);
        std::shared_ptr<CacheObject<T>> c = std::dynamic_pointer_cast<CacheObject<T>>(d);
        if (c == nullptr) {
            printf("failed to dyn pointer cast in getByVictronID for %s\n", d->getShortDescription().c_str());
            return;
        }

        auto descr = c->getShortDescription();
        auto value = c->get();
        
        std::cout << descr << " ("
                  << key << ")" << ": "
                  << value
                  << c->getUnits()
                  << std::endl;
    }


    void setByVictronID_(const std::string& key, const std::string& value);

    void printByVictronID(const std::string& key); //const

    void toggleCache() {
        mReadable = !mReadable;
    }
    
    // For dynamical registration of objects as they come in from the Victron unit.
    // Dynamical registration saves about 50 objects when operating with BMV-702
    // (34 instead of all 84 objects).
    void registerComponent(const std::string& key);

    Dictionary mCache;
    Dictionary mVictronMap;
    Dictionary mVictronSerialMap; //!< for serial writer to set values - could be move inside registration function?
    Dictionary mAddressCache;

private:
    // creates and maps those components that are not covered by the frequent
    // updates coming from the device
    void mapStaticComponents();

    // \brief adds obj to mCache using label as key,
    //        mVictronMap and mVictronSerialMap using bmvLabel as key
    //        and mAddressCache using address as key    
    template<typename SHAREDOBJ>
    void addToDictionaries(const SHAREDOBJ obj,
                           const std::string& label,    // e.g. alarmReason
                           const std::string& bmvLabel, // e.g. "AR"
                           const std::string& address = {}  // BMV address
                           );

    // switching between 2 values in Cache
    std::atomic_bool mReadable = false;

};

