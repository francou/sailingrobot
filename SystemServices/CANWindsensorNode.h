/****************************************************************************************
 *
 * File:
 * 		CANWindsensorNode.h
 *
 * Purpose:
 *		Process messages from the CAN-Service
 *
 * Developer Notes:
 *
 *
 ***************************************************************************************/
#include "SystemService/CANPNGReceiver.h"


#pragma once

 struct N2kMsg
{
	uint32_t PGN;
	uint8_t Priority;
	uint8_t Source;
	uint8_t Destination;
	int DataLen;
	std::vector<uint8_t> Data;
};


class CANWindsensorNode : public Node, public CANPNGReceiver
{
public:
	CANWindsensorNode(MessageBus& msgBus, NodeID id, float windDir, float windSpeed, float windTemperature);

	~CANWindsensorNode();

	/* data */
	virtual void processPGN(N2kMsg &NMsg, uint32_t PGN) = 0;

    
    void parsePGN130306(N2kMsg &NMsg, uuint8_t &SID, float &WindSpeed,				//WindData
					float &WindAngle, uint8_t &Reference);

    void parsePGN130311(N2kMsg &Msg, uint8_t &SID, uint8_t &TemperatureInstance,	//Environmental Parameters
					uint8_t &HumidityInstance, float &Temperature,
					float &Humidity, float &AtmosphericPressure);

    void ParsePGN130312(N2kMsg &NMsg, uint8_t &SID, uint8_t &TemperatureInstance,	//Temperature
					uint8_t &TemperatureSource, float &ActualTemperature,
					float &SetTemperature);

	///----------------------------------------------------------------------------------
 	/// Setups the actuator.
 	///
 	///----------------------------------------------------------------------------------
	virtual bool init();


	///----------------------------------------------------------------------------------
 	/// Attempts to connect to the CV7 wind sensor.
 	///
 	///----------------------------------------------------------------------------------
	bool init();

	///----------------------------------------------------------------------------------
 	/// Starts the wind sensors thread so that it actively pumps data into the message
 	/// bus.
 	///
 	///----------------------------------------------------------------------------------
	void start();

private:
	bool m_Initialised;		// Indicates that the node was correctly initialised
	float m_WindDir;
	float m_WindSpeed;
	float m_WindTemperature;

};