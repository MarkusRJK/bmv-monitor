#pragma once

#include <atomic>
#include <memory>
#include <string>
#include "Dictionary.h"
#include "CacheObject.h"


class DeviceCache {
public:
    DeviceCache() = default;
    ~DeviceCache() = default;

    template<class T>
    void setByVictronID(const std::string& key, const T& value);

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
    // switching between 2 values in Cache
    std::atomic_bool mReadable = false;

    // \brief adds obj to mCache using label as key,
    //        mVictronMap and mVictronSerialMap using bmvLabel as key
    //        and mAddressCache using address as key    
    template<typename SHAREDOBJ>
    void addToDictionaries(const SHAREDOBJ obj,
                           const std::string& label,    // e.g. alarmReason
                           const std::string& bmvLabel, // e.g. "AR"
                           const std::string& address = {}  // BMV address
                           );

};

