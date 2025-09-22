#pragma once

#include <deque> 
#include <boost/asio.hpp> 
#include <boost/asio/serial_port.hpp> 
// FIXME: breaks linkage
//#include <boost/regex.hpp>
#include <string>
// Enable once compiler is C++20 and use views in parsing instead of getline
//#include <ranges>
//#include <string_view>
#include "LineParser.h"

class SerialPortCommunicator 
{ 
public: 
    SerialPortCommunicator(boost::asio::io_service& io_service, unsigned int baud, const std::string& device, LineParser& parser);
    ~SerialPortCommunicator() = default;

    void write(const char msg);
    void close();
    bool isActive() const noexcept;

private: 

    void readline(void);
    void lineReceived(const boost::system::error_code& ec, std::size_t size);
    void startRead(void);
    void completeRead(const boost::system::error_code& error, size_t bytes_transferred);
    void doWrite(const char msg);
    void startWrite(void);
    void completeWrite(const boost::system::error_code& error);
    void doClose(const boost::system::error_code& error);

    static const int mMaxReadLength = 512; // maximum amount of data to read in one operation 
    bool mIsActive; // remains true while this object is still operating 
    boost::asio::io_service& mIoService; // the main IO service that runs this connection 
    boost::asio::serial_port mSerialPort; // the serial port this instance is connected to 
    char mReadMessage[mMaxReadLength]; // data read from the socket
    boost::asio::streambuf mReadLine;
    std::deque<char> mWriteMessages; // buffered write data
    LineParser& mParser;
}; 

