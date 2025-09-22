#include <any>
#include <map>
#include <memory> //!< import std::shared_ptr
#include <string>

//#include "CacheObject.h"


// hex to int conversion
// #include <sstream>
// #include <iostream>
//
// int main() {
//    unsigned int x;   
//    std::stringstream ss;
//    ss << std::hex << "fffefffe";
//    ss >> x;
//    // output it as a signed type
//    std::cout << static_cast<int>(x) << std::endl;
//}

// or std::stoul
// or boost lexical_cast
//
// see https://stackoverflow.com/questions/1070497/c-convert-hex-string-to-signed-integer

class Dictionary {
    using dictionary_t = std::map<std::string, std::any>;
    dictionary_t mDictionary;
public:
    
    // operator[] not implemented since it is not recommended to use it in many cases

    //template <typename T>
    //T at(const std::string &key) const
    template<class OBJECT>
    std::shared_ptr<OBJECT> at(const std::string &key) const
    {
        auto mapItem = mDictionary.at(key);
        //std::shared_ptr<CacheObject<T>> item;
        try {
            ///*auto*/ item = std::any_cast<std::shared_ptr<CacheObject<T>>>(mapItem);
            return std::any_cast<std::shared_ptr<OBJECT>>(mapItem);
        } catch(...) {};
        return {};
        //return item->get();
    };

    //template <typename T>
    //T at(const std::string &key, T defaultValue) const
    // template <typename T>
    // std::shared_ptr<OBJECT> at(const std::string &key, T defaultValue) const
    // {
    //     auto it = mDictionary.find(key);
    //     if (it == mDictionary.end())
    //         return defaultValue;
    //     return this->at<T>(key);
    // };

    // template <typename T>
    // T at_(const std::string &key) const
    // {
    //     auto mapItem = mDictionary.at(key);
    //     std::shared_ptr<CacheObject<T>> item;
    //     try {
    //         /*auto*/ item = std::any_cast<std::shared_ptr<CacheObject<T>>>(mapItem);
    //     } catch(...) {
    //         std::cerr << "Conversion for shared_ptr went wrong for key " << key << std::endl;
    //         exit(0);
    //     }
    //     return item;
    // };

    //template <typename T>
    //void insert(const std::string &key, std::shared_ptr<CacheObject<T>> value)
    template<class OBJECT>
    void insert(const std::string &key, std::shared_ptr<OBJECT> value)
    {
        try {
            mDictionary.insert({std::pair(key, static_cast<std::any>(value))});
        } catch(...) {}
    };

    // void insert(const std::string &key, std::shared_ptr<SerializedCacheObject> value)
    // {
    //     try {
    //         mDictionary.insert({std::pair(key, static_cast<std::any>(value))});
    //     } catch(...) {}
    // };

    dictionary_t::iterator find(const std::string& key) {
        return mDictionary.find(key);
    };
    dictionary_t::const_iterator find(const std::string& key) const {
        return mDictionary.find(key);
    };

    dictionary_t::iterator begin() {
        return mDictionary.begin();
    };
    dictionary_t::iterator end() {
        return mDictionary.end();
    };

};

