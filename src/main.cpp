/* 
   To end the application, send Ctrl-C on standard input 
*/ 

#include <systemd/sd-bus.h>
#include <sdbus-c++/sdbus-c++.h>
#include <iostream> 
#include <thread> 
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
// Enable once compiler is C++20 and use views in parsing instead of getline
//#include <ranges>
//#include <string_view>
#include "DeviceCache.h"
#include "SerialPortCommunicator.h"
#include "bmvmonitor.h"

#ifdef POSIX 
#include <termios.h> 
#endif 

using namespace std; 
namespace po = boost::program_options;


int main(int argc, char* argv[]) 
{ 
    // Declare the supported options.
    po::options_description desc("bmvmonitor <baudrate=19200> /dev/serial.... Supported options");
    desc.add_options()
        ("help", "produce help message")
        //("compression", po::value<int>(), "set compression level")
        ("cout", po::value<bool>(), "enable/disable console output")
        ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);    

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    if (vm.count("cout") && vm["cout"].as<bool>()) {
        std::cout << "console output enabled" << std::endl;
    } else {
        std::cout << "console output disabled" << std::endl;
    }

    // on Unix POSIX based systems, turn off line buffering of input, so cin.get() returns after every keypress 
    // On other systems, you'll need to look for an equivalent 
#ifdef POSIX // needed with boost asio (see serialdingens)
    termios stored_settings; 
    tcgetattr(0, &stored_settings); 
    termios new_settings = stored_settings; 
    new_settings.c_lflag &= (~ICANON); 
    new_settings.c_lflag &= (~ISIG); // don't automatically handle control-C 
    tcsetattr(0, TCSANOW, &new_settings); 
#endif 
    try { 
        if (argc != 3) { 
            std::cerr << desc << std::endl;
            return 1; 
        } 
        boost::asio::io_service io_service;
        DeviceCache cache;
        LineParser parser(cache);
        // define an instance of the main class of this program 
        SerialPortCommunicator c(io_service,
                                 boost::lexical_cast<unsigned int>(argv[1]),
                                 argv[2],
                                 parser); 
        // run the IO service as a separate thread, so the main thread can block on standard input 
        std::thread t([&]{ io_service.run(); });

        // version 2.1.x initialisation
        // Create D-Bus connection to (either the session or system) bus and requests a well-known name on it.
        sdbus::ServiceName serviceName{"org.thelinks.bmvmonitor"};
        auto connection = sdbus::createBusConnection(serviceName);

        // Create monitor D-Bus object.
        sdbus::ObjectPath objectPath{"/org/thelinks/bmvmonitor"};
        BMVMonitor monitor(*connection, std::move(objectPath), cache);

        connection->enterEventLoop();

//        while (c.isActive()) { 
//            // check the internal state of the connection to make sure it's still running 
//            char ch; 
//            cin.get(ch); // blocking wait for standard input 
//            if (ch == 3) // ctrl-C to end program 
//                break; 
//            c.write(ch); 
//        } 
//
//        c.close(); // close the minicom client connection 
//        t.join(); // wait for the IO service thread to close 
    } 
    catch (exception& e) { 
        cerr << "Exception: " << e.what() << "\n"; 
    } 
#ifdef POSIX // restore default buffering of standard input 
    tcsetattr(0, TCSANOW, &stored_settings); 
#endif 
    return 0; 
}
