/****************************************************************************************
*
* File:
* 		StationKeepingNode.cpp
*
* Purpose:
*		This class computes the actuator positions of the boat in order to stay near a waypoint.
*
*
***************************************************************************************/

#include "StationKeepingNode.h"

const int INITIAL_SLEEP = 2000;  //in milliseconds
#define DATA_OUT_OF_RANGE -2000

StationKeepingNode::StationKeepingNode(MessageBus& msgBus, DBHandler& db): ActiveNode(NodeID::SailingLogic, msgBus), m_db(db),
m_waypointLon(DATA_OUT_OF_RANGE), m_waypointLat(DATA_OUT_OF_RANGE), m_waypointRadius(DATA_OUT_OF_RANGE)
{
    msgBus.registerNode(*this, MessageType::StateMessage);
    msgBus.registerNode(*this, MessageType::WindState);
    msgBus.registerNode(*this, MessageType::WaypointData);
    msgBus.registerNode(*this, MessageType::WaypointStationKeeping);

    m_LoopTime = 0.1;

    m_CloseHauledAngle = Utility::degreeToRadian(45);
    m_BroadReachAngle = Utility::degreeToRadian(30);
    m_TackingDistance = 3;

    m_stationKeeping_On = 0;
}

StationKeepingNode::~StationKeepingNode() {}

bool StationKeepingNode::init()
{
    return true;
}

void StationKeepingNode::start()
{
    runThread(StationKeepingNodeThreadFunc);
}

void StationKeepingNode::processMessage(const Message* msg)
{
  MessageType type = msg->messageType();
    switch(type)
    {
    case MessageType::StateMessage:
        processStateMessage(static_cast<const StateMessage*>(msg));
        break;
    case MessageType::WindState:
        processWindStateMessage(static_cast<const WindStateMsg*>(msg));
        break;
    case MessageType::WaypointData:
        m_stationKeeping_On = 0;
        break;
    case MessageType::WaypointStationKeeping:
        m_stationKeeping_On = 1;
        processWaypointMessage(static_cast<const WaypointStationKeepingMsg*>(msg));
        break;
    default:
        return;
    }
}

void StationKeepingNode::processStateMessage(const StateMessage* vesselStateMsg )
{
    std::lock_guard<std::mutex> lock_guard(m_lock);

    m_VesselLat = vesselStateMsg->latitude();
    m_VesselLon = vesselStateMsg->longitude();
}

void StationKeepingNode::processWindStateMessage(const WindStateMsg* windStateMsg )
{
    std::lock_guard<std::mutex> lock_guard(m_lock);

    m_trueWindSpeed = windStateMsg->trueWindSpeed();
    m_trueWindDir = windStateMsg->trueWindDirection();
    // std::cout << "m_trueWindDir : " << m_trueWindDir <<std::endl;

    unsigned int twdBufferMaxSize = 200;
    Utility::addValueToBuffer(m_trueWindDir, m_TwdBuffer, twdBufferMaxSize);
}

void StationKeepingNode::processWaypointMessage(const WaypointStationKeepingMsg* waypMsg )
{
    std::lock_guard<std::mutex> lock_guard(m_lock);

    m_waypointLon = waypMsg->longitude();
    m_waypointLat = waypMsg->latitude();
    m_waypointRadius = waypMsg->radius();
}

double StationKeepingNode::computeTargetCourse()
{
     if ((m_waypointLon == DATA_OUT_OF_RANGE) || (m_VesselLat == DATA_OUT_OF_RANGE) || (m_trueWindSpeed == DATA_OUT_OF_RANGE))
    {
        return DATA_OUT_OF_RANGE;
    }
    else
    {
        double meanTrueWindDir = Utility::meanOfAngles(m_TwdBuffer);
        double trueWindAngle = Utility::limitRadianAngleRange(Utility::degreeToRadian(meanTrueWindDir)+M_PI);

        // calculate the bearing of the waypoint, which is the target course
        double targetCourse = CourseMath::calculateBTW(m_VesselLon, m_VesselLat, m_waypointLon, m_waypointLat);

        // Calculate signed distance to the line defined by the waypoint and the wind mean direction
        double signedDistance = Utility::calculateSignedDistanceToLine(m_waypointLon, m_waypointLat, m_waypointLon + cos(trueWindAngle),
            m_waypointLat + sin(trueWindAngle), m_VesselLon, m_VesselLat);

        // Change tack direction when reaching tacking distance
        if(abs(signedDistance) > m_TackingDistance)
        {
            m_TackDirection = Utility::sgn(signedDistance);
        }

        // Check if the targetcourse is inconsistent with the wind.
        if( (cos(trueWindAngle - targetCourse) + cos(m_CloseHauledAngle) < 0) || 
            ((cos(trueWindAngle - phi) + cos(m_CloseHauledAngle) < 0)) ) // need to be checked
        {   
            // Close hauled mode (Upwind beating mode).
            m_BeatingMode = true;
            targetCourse = M_PI + trueWindAngle + m_TackDirection*m_CloseHauledAngle;
            targetCourse = Utility::limitRadianAngleRange(targetCourse);
        }
        else if( (cos(trueWindAngle - targetCourse) - cos(m_BroadReachAngle) < 0) || 
                 ((cos(trueWindAngle - phi) - cos(m_BroadReachAngle) < 0) ) )
        {   
            // Broad reach mode (Downwind beating mode).
            m_BeatingMode = true;
            targetCourse = M_PI + trueWindAngle + m_TackDirection*m_BroadReachAngle;
            targetCourse = Utility::limitRadianAngleRange(targetCourse);
        }
        else
        {
            m_BeatingMode = false;
        }

        // std::cout << "trueWindAngle : " << trueWindAngle <<std::endl;

        return Utility::radianToDegree(targetCourse); // in north east down reference frame.
    }
}



double StationKeepingNode::computeRudder()
{

}

void StationKeepingNode::StationKeepingNodeThreadFunc(ActiveNode* nodePtr)
{
    StationKeepingNode* node = dynamic_cast<StationKeepingNode*> (nodePtr);

    std::this_thread::sleep_for(std::chrono::milliseconds( INITIAL_SLEEP ));

    Timer timer;
    timer.start();

    while(true)
    {

        if (node->m_stationKeeping_On == 1){
            if (CourseMath::calculateDTW(node->m_VesselLon, node->m_VesselLat, node->m_waypointLon, node->m_waypointLat) > node->m_waypointRadius/2){
                double targetCourse =  node->calculateTargetCourse();
                if (targetCourse != DATA_OUT_OF_RANGE){
                    MessagePtr LocalNavMsg = std::make_unique<LocalNavigationMsg>((float) targetCourse, NO_COMMAND, node->m_BeatingMode, node->m_TargetTackStarboard);
                    node->m_MsgBus.sendMessage( std::move( LocalNavMsg ) );
                }
            }
        }
        else{
            
        }


        timer.sleepUntil(node->m_LoopTime);
        timer.reset();
    }
}