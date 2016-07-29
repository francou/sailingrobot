#ifndef H_XBEE
#define H_XBEE

#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>
#include <fstream>
#include <wiringSerial.h>
#include <iostream>
#include <unistd.h>

#include <errno.h>

#define BAUD_RATE 57600

class xBee {
	private:

		std::string m_receivedBuffer;


	public:

		// searches the buffer for a message and returns it
		std::string findXmlMessage(std::string* buffer);
		
		// Initializes the xBee
		int init();

		// Reads incoming echoes for the specified device
		std::string receiveData(int fd);

		// Reads incoming echoes for the specified device,
		// requires data to be in <message></message> tags
		std::string receiveXMLData(int fd);

		// Sends a string to the given filedescriptor
		void transmitData(int fd, std::string data);
	};

#endif
