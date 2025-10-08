#pragma once

#include <sdbus-c++/sdbus-c++.h>
#include "bmvmonitor-server.h"
#include "DeviceCache.h"

class BMVMonitor : public sdbus::AdaptorInterfaces<org::thelinks::bmvmonitor_adaptor /*, more adaptor classes if there are more interfaces*/>
{
public:
    BMVMonitor(sdbus::IConnection& connection, sdbus::ObjectPath objectPath, DeviceCache& cache)
        : AdaptorInterfaces(connection, std::move(objectPath))
        , mCache(cache)
    {
        registerAdaptor();
    }

    ~BMVMonitor()
    {
        unregisterAdaptor();
    }

protected:
    void quit() {
        // do quit eventloop: connection->quit??
    }

    /* std::string concatenate(const std::vector<int32_t>& numbers, const std::string& separator) override */
    /* { */
    /*     // Return error if there are no numbers in the collection */
    /*     if (numbers.empty()) */
    /*         throw sdbus::Error(sdbus::Error::Name{"org.sdbuscpp.BMVMonitor.Error"}, "No numbers provided"); */

    /*     // Concatenate the numbers */
    /*     std::string result; */
    /*     for (auto number : numbers) */
    /*     { */
    /*         result += (result.empty() ? std::string() : separator) + std::to_string(number); */
    /*     } */

    /*     // Emit the 'concatenated' signal with the resulting string */
    /*     emitConcatenated(result); */

    /*     // Return the resulting string */
    /*     return result; */
    /* } */

    sdbus::Variant getValue(const std::string& label) override {
        return {};
    };

    sdbus::Struct<sdbus::Variant, std::string> getMeasurement(const std::string& label) override {
        return {};
    };
    
    std::string getUnits(const std::string& label) override {
        return "Ah";
    };
    
    std::string getDescription(const std::string& label) override {
        printf("Youhu reached Dbus desciption\n");
        return mCache.get(label);
    };
    
    std::string getAbbreviation(const std::string& label) override {
        return mCache.get(label);
    };
    
    void setPrecision(const std::string& label, const uint8_t& precision) override {
    };
    
    void setDelta(const std::string& label, const uint8_t& delta) override {
    };

    uint8_t Bar() { return 0; };
    void Bar(const uint8_t& v) {};

private:
    DeviceCache& mCache;
};
