/****************************************************************************************
*
* File:
* 		MockNode.h
*
* Purpose:
*		A mock node for testing whether message passing is working correctly.
*
* Developer Notes:
*
*
***************************************************************************************/


#pragma once

//TODO - Jordan: Improve include paths so they aren't sooo long
#include "../../../Nodes/Node.h"
#include "../../../Messages/GPSDataMsg.h"
#include "../../../Messages/WindDataMsg.h"
#include "../../../Messages/CompassDataMsg.h"



class MockNode : Node {
public:
	MockNode(MessageBus& msgBus, bool& registered)
	:Node(NodeID::MessageLogger, msgBus), m_MessageReceived(false), m_HasFix(false),
	m_Online(false), m_WindDir(0), m_WindSpeed(0)
	{
		if(msgBus.registerNode(*this, MessageType::GPSData) && msgBus.registerNode(*this, MessageType::WindData) &&
			msgBus.registerNode(*this, MessageType::CompassData))
		{
			registered = true;
		}
	}


	virtual ~MockNode() {}

	bool init() { return true; }

	void processMessage(const Message* message)
	{
		MessageType type = message->messageType();

		switch(type)
		{
			case MessageType::GPSData:
			{
				m_MessageReceived = true;
				GPSDataMsg* gpsMsg = (GPSDataMsg*)message;
				m_HasFix = gpsMsg->hasFix();
				m_Online = gpsMsg->gpsOnline();
				m_Lat = gpsMsg->latitude();
				m_Lon = gpsMsg->longitude();
				m_Speed = gpsMsg->speed();
				m_Course = gpsMsg->heading();
			}
			break;
			case MessageType::WindData:
			{
				m_MessageReceived = true;
				WindDataMsg* windMsg = (WindDataMsg*)message;
				m_WindDir = windMsg->windDirection();
				m_WindSpeed = windMsg->windSpeed();
				m_WindTemp = windMsg->windTemp();
			}
			break;

			case MessageType::CompassData:
			{
				m_MessageReceived = true;
				CompassDataMsg* compassMsg = (CompassDataMsg*)message;
				m_Heading = compassMsg->heading();
			}
			default:
			return;
		}
	}

	bool 	m_MessageReceived;

	bool	m_HasFix;
	bool	m_Online;
	double 	m_Lat;
	double 	m_Lon;
	double m_Speed;
	double m_Course;

	float 	m_WindDir;
	float 	m_WindSpeed;
	float 	m_WindTemp;
	float 	m_Heading;

};
