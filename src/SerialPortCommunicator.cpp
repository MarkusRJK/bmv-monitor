
#include "SerialPortCommunicator.h"

#include <iostream> 

using namespace std;

SerialPortCommunicator::SerialPortCommunicator(boost::asio::io_service& io_service, unsigned int baud, const string& device, LineParser& parser) 
    : mIsActive(true)
    , mIoService(io_service)
    , mSerialPort(io_service, device)
    , mParser(parser)
{
    if (!mSerialPort.is_open()) { 
        cerr << "Failed to open serial port\n"; 
        return; 
    } 
    boost::asio::serial_port_base::baud_rate baud_option(baud); 
    boost::asio::serial_port_base::character_size charSize(8); 
    boost::asio::serial_port_base::parity p(boost::asio::serial_port_base::parity::none); 
    boost::asio::serial_port_base::stop_bits s(boost::asio::serial_port_base::stop_bits::one); 
    mSerialPort.set_option(baud_option); // set options after the port has been opened
    mSerialPort.set_option(charSize);
    mSerialPort.set_option(p);
    mSerialPort.set_option(s);

    //startRead(); 
    readline();
} 

void SerialPortCommunicator::write(const char msg) // pass the write data to the doWrite function via the io service in the other thread 
{ 
    mIoService.post(std::bind(&SerialPortCommunicator::doWrite, this, msg)); 
} 

void SerialPortCommunicator::close() // call the doClose function via the io service in the other thread 
{ 
    mIoService.post(std::bind(&SerialPortCommunicator::doClose, this, boost::system::error_code())); 
} 

bool SerialPortCommunicator::isActive() const noexcept // return true if the socket is still active 
{ 
    return mIsActive; 
} 

void SerialPortCommunicator::readline(void) { 
    // mReadMessage.prepare(mMaxReadLength);
    boost::asio::async_read_until(mSerialPort,
                                  mReadLine,
                                  "\n", // boost::regex("\x0d\x0a"), // CR(\r) + LF(\n)
                                  [this](const boost::system::error_code& error, std::size_t bytesTransferred) {
                                      lineReceived(error, bytesTransferred);
                                  });
} 

void SerialPortCommunicator::lineReceived(const boost::system::error_code& ec, std::size_t size) {
    if (ec) {
        std::cerr << "Line Error" << std::endl;
        return;
    }
    std::istream str(&mReadLine);
    if (str.good()) {
        mParser.parse(str);
    }
    readline();
}

void SerialPortCommunicator::startRead(void) { 
    // Start an asynchronous read and call read_complete when it completes or fails 
    mSerialPort.async_read_some(boost::asio::buffer(mReadMessage, mMaxReadLength), 
                                [this](const boost::system::error_code& error, size_t bytesTransferred) {
                                    completeRead(error, bytesTransferred);
                                });
} 

void SerialPortCommunicator::completeRead(const boost::system::error_code& error, size_t bytes_transferred) { 
    // the asynchronous read operation has now completed or failed and returned an error 
    if (!error) { // read completed, so process the data 
        cout.write(mReadMessage, bytes_transferred); // echo to standard output
        cout << "----------------------------------" << endl;
        startRead(); // start waiting for another asynchronous read again 
    } 
    else {
        doClose(error); 
    }
} 

void SerialPortCommunicator::doWrite(const char msg) {
    // callback to handle write call from outside this class 
    bool write_in_progress = !mWriteMessages.empty(); // is there anything currently being written? 
    mWriteMessages.push_back(msg); // store in write buffer 
    if (!write_in_progress) { // if nothing is currently being written, then start 
        startWrite(); 
    }
} 

void SerialPortCommunicator::startWrite(void) {
    // Start an asynchronous write and call completeWrite when it completes or fails 
    boost::asio::async_write(mSerialPort, 
                             boost::asio::buffer(&mWriteMessages.front(), 1), 
                             [&](const boost::system::error_code& error, unsigned int) {
                                 SerialPortCommunicator::completeWrite(error);
                             });
} 

void SerialPortCommunicator::completeWrite(const boost::system::error_code& error) { 
    // the asynchronous read operation has now completed or failed and returned an error 
    if (!error) { // write completed, so send next write data 
        mWriteMessages.pop_front(); // remove the completed data 
        if (!mWriteMessages.empty()) { // if there is anthing left to be written 
            startWrite(); // then start sending the next item in the buffer 
        }
    } else {
        doClose(error); 
    }
} 

void SerialPortCommunicator::doClose(const boost::system::error_code& error) {
    // something has gone wrong, so close the socket & make this object inactive 
    if (error == boost::asio::error::operation_aborted) // if this call is the result of a timer cancel() 
        return; // ignore it because the connection cancelled the timer 
    if (error) 
        cerr << "Error: " << error.message() << endl; // show the error message 
    else 
        cout << "Error: Connection did not succeed.\n"; 
    cout << "Press Enter to exit\n"; 
    mSerialPort.close(); 
    mIsActive = false; 
} 
