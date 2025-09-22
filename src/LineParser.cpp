
#include "LineParser.h"


#include <numeric>


void LineParser::parse(std::istream& str) {
    static int dataset = 0; //!< two dataset come in each with their own checksum
    
    std::string line;
    // a line from Victron BMV is
    // <cmd>\t<value>\r\n
    // command is a char string
    // value is a character string that represents either
    //       - an integer 
    //       - a bool (given as string 'ON' or 'OFF
    //      
    // TODO: use views and ranges instead of getline... -> C++20
    std::getline(str, line, '\n'); // scans till \n and swallows trailing \n
    //for (auto c: line) std::cout << std::hex << " 0x" << static_cast<int>(c); std::cout << std::endl;
    auto isChecksum = isCorrectChecksum(line + '\n'); // get
    if (isChecksum) {
        // two correct checksums have to arrive until the dataset if complete
        if (dataset == 1) mCache.toggleCache();
        dataset = (dataset+1) % 2;
    }
    std::string tabDelimiter = "\t"; // \x09
    size_t posDelimiter = 0;
    std::string name = line.substr(0, posDelimiter = line.find(tabDelimiter)); 
    // std::cout << "name: " << name << std::endl
    //           << "isChecksum: " << (isChecksum ? "true" : "false")  << std::endl
    //           << "tabDel at: " << posDelimiter << std::endl
    //           << "length: " << line.length() << std::endl;
    if (! name.empty() && ! isChecksum && posDelimiter < line.length()) {
        // tabDelimiter is at posDelimiter - we don't want the tab in 
        // our value => posDelimiter+1
        const auto valueStart = posDelimiter+1;
        // the lines were scanned til \n by getline, \r is before \n
        // and is still at the end of line => remove it using -1
        std::string value = line.substr(valueStart, line.length()-1-valueStart);
        //std::cout << name << ":" << name.length() 
        //          << " = " << value << ":" << value.length() << std::endl;
        std::cout << "*****************************************" << std::endl;
        std::cout << name << " = " << value << std::endl;
        printf("registering\n");
        mCache.registerComponent(name);
        printf("setting %s\n", name.c_str());
        mCache.setByVictronID_(name, value);
        
        mCache.getByVictronID_<int>("AR");
        mCache.getByVictronID_<double>("I");
        mCache.getByVictronID_<double>("IL");
        mCache.getByVictronID_<std::string>("LOAD");
        mCache.getByVictronID_<double>("V"); // working
        mCache.getByVictronID_<double>("VM"); // working
        mCache.getByVictronID_<double>("P");
        mCache.getByVictronID_<double>("SOC");
        mCache.getByVictronID_<double>("VS");
        mCache.getByVictronID_<double>("CE");
        mCache.getByVictronID_<double>("DM");

        mCache.getByVictronID_<double>("VPV");
        mCache.getByVictronID_<double>("PPV");
        mCache.getByVictronID_<double>("CS");
        mCache.getByVictronID_<int>("PID");
        mCache.getByVictronID_<double>("FW");
        mCache.getByVictronID_<double>("H1");
        mCache.getByVictronID_<double>("H2");
        mCache.getByVictronID_<double>("H3");
        mCache.getByVictronID_<int>("H4");
        mCache.getByVictronID_<int>("H5");
        mCache.getByVictronID_<double>("H6");
        mCache.getByVictronID_<double>("H7");
        mCache.getByVictronID_<double>("H8");
        mCache.getByVictronID_<int>("H9");
        mCache.getByVictronID_<int>("H10");
        mCache.getByVictronID_<int>("H11");
        mCache.getByVictronID_<int>("H12");
        mCache.getByVictronID_<double>("H13");
        mCache.getByVictronID_<double>("H14");
        mCache.getByVictronID_<double>("H15");
        mCache.getByVictronID_<double>("H16");
        mCache.getByVictronID_<double>("H17");
        mCache.getByVictronID_<double>("H18");
        mCache.getByVictronID_<double>("H19");
        mCache.getByVictronID_<double>("H20");
        mCache.getByVictronID_<double>("H21");
        mCache.getByVictronID_<double>("H22");
        mCache.getByVictronID_<double>("H23");
        mCache.getByVictronID_<int>("ERR");
        mCache.getByVictronID_<int>("WARN");
        mCache.getByVictronID_<int>("SER#");
        mCache.getByVictronID_<int>("HSDS");
        mCache.getByVictronID_<int>("MODE");
        mCache.getByVictronID_<double>("AC_OUT_V");
        mCache.getByVictronID_<double>("AC_OUT_I");
        mCache.getByVictronID_<int>("TTG");
        // TODO: move over to enums ON/OFF
        mCache.getByVictronID_<Toggle>("Alarm");
        //mCache.getByVictronID_<std::string, std::string>("Relay");
        mCache.getByVictronID_<Toggle>("Relay");
        mCache.getByVictronID_<int>("BMV");

        //mCache.printByVictronID(name);
        
//	CacheObject(double nativeToUnitFactor,
//		std::string units,
//		std::string shortDescr)
//		//std::map<std:string, TYPE> options)
    }

}

bool LineParser::isCorrectChecksum(const std::string& line) {
    //logger.trace('RegularUpdateChecksum::update');

    // count all characters as uint8_t from first
    // value pair name until and including Checksum - tab- and the
    // uint8_t checksum value. Also include all the tabs between
    // name and value pair and the \n and \r after each line.
    // This checksum must be 0 (% 256)

    std::string checksumId = "Checksum";
    size_t checksumIdLength = checksumId.length();
    bool   isChecksum = line.compare(0, checksumIdLength, checksumId) == 0;
    // using namespace std;
    // cout << "line: " << line << endl;
    // cout << "line sub: " << line.substr(0, checksumIdLength+1) << endl;
    // cout << "checksum: " << std::boolalpha << isChecksum << endl;

    mChecksum = std::accumulate(line.begin(), line.end(), mChecksum);
		
    if (isChecksum) {
        if (mChecksum != 0) {
            std::cerr << std::endl << std::endl
                      << "Feck Wrong checksum " << std::hex
                      << "calc 0x" << static_cast<int>(mChecksum)
                      << std::endl << std::endl;
            mChecksum = 0;
            return false;
        }
        else std::cout << "CHECKSUM OK" << std::endl;
    }
    return isChecksum; 
}

