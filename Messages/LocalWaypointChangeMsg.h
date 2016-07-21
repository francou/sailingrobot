
#pragma once

#include "Message.h"

class LocalWaypointChangeMsg : public Message {
public:
	LocalWaypointChangeMsg(NodeID destinationID, NodeID sourceID)
		:Message(MessageType::LocalWaypointChange, sourceID, destinationID)
	{ }

	LocalWaypointChangeMsg()
		:Message(MessageType::LocalWaypointChange, NodeID::None, NodeID::None)
	{ }

	virtual ~LocalWaypointChangeMsg() { }

};