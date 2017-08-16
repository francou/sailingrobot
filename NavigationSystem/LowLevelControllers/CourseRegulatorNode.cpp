/****************************************************************************************
 *
 * File:
 * 		CourseRegulatorNode.cpp
 *
 * Purpose:
 *      This file realize the regulation of the rudder by the desired course.
 *      It sends the value to the rudder actuator Node
 *
 * License:
 *      This file is subject to the terms and conditions defined in the file
 *      'LICENSE.txt', which is part of this source code package.
 *
 ***************************************************************************************/
#include "CourseRegulatorNode.h"
#include <chrono>
#include <thread>
#include <math.h>
//#include <cstdlib>
#include "Math/Utility.h"
#include "Messages/ActuatorPositionMsg.h"
#include "Messages/StateMessage.h"
#include "SystemServices/Logger.h"
#include "SystemServices/Timer.h"

#define HEADING_ERROR_VALUE 370

CourseRegulatorNode::CourseRegulatorNode( MessageBus& msgBus,  DBHandler& dbhandler, double loopTime, double maxRudderAngle,
    double configPGain, double configIGain):ActiveNode(NodeID::CourseRegulatorNode,msgBus), m_VesselHeading(HEADING_ERROR_VALUE), m_VesselSpeed(0),
    m_MaxRudderAngle(maxRudderAngle),m_DesiredHeading(HEADING_ERROR_VALUE),m_db(dbhandler), m_LoopTime(loopTime),pGain(configPGain),iGain(configIGain)
{
    msgBus.registerNode( *this, MessageType::StateMessage);
    msgBus.registerNode( *this, MessageType::DesiredCourse);
    msgBus.registerNode( *this, MessageType::LocalNavigation);
    msgBus.registerNode( *this, MessageType::ServerConfigsReceived);
}

///----------------------------------------------------------------------------------
CourseRegulatorNode::~CourseRegulatorNode(){}

///----------------------------------------------------------------------------------
bool CourseRegulatorNode::init(){ return true;}

///----------------------------------------------------------------------------------
void CourseRegulatorNode::start()
{
    runThread(CourseRegulatorNodeThreadFunc);
}

///----------------------------------------------------------------------------------
void CourseRegulatorNode::updateConfigsFromDB()
{
    m_LoopTime = m_db.retrieveCellAsDouble("sailing_robot_config","1","loop_time");
    m_MaxRudderAngle = m_db.retrieveCellAsDouble("sailing_robot_config","1","???");
    pGain = m_db.retrieveCellAsDouble("sailing_robot_config","1","???");
    iGain = m_db.retrieveCellAsDouble("sailing_robot_config","1","???");
}

///----------------------------------------------------------------------------------
void CourseRegulatorNode::processMessage( const Message* msg )
{
    MessageType type = msg->messageType();
    switch(type)
    {
        case MessageType::StateMessage:
        processStateMessage(static_cast< const StateMessage*>(msg));
        break;
        case MessageType::DesiredCourse:
        processDesiredCourseMessage(static_cast< const DesiredCourseMsg*>(msg)); //verify
        break;
        case MessageType::LocalNavigation:
        processLocalNavigationMessage(static_cast< const LocalNavigationMsg*>(msg));
        break;
        case MessageType::ServerConfigsReceived:
        updateConfigsFromDB();
        break;
        default:
        return;
        //Logger::info("Desired Course: %d Heading: %d", desiredHeading, heading);
    }
}

///----------------------------------------------------------------------------------
void CourseRegulatorNode::processStateMessage(const StateMessage* msg)
{
    m_VesselHeading = msg->heading();
    m_VesselSpeed = msg->speed();
}

///----------------------------------------------------------------------------------
void CourseRegulatorNode::processDesiredCourseMessage(const DesiredCourseMsg* msg)
{
    m_DesiredHeading = static_cast<double>(msg->desiredCourse());
}

///----------------------------------------------------------------------------------
void CourseRegulatorNode::processLocalNavigationMessage(const LocalNavigationMsg* msg)
{
    //std::lock_guard<std::mutex> lock_guard(m_lock);
    //m_NavigationState = msg->navigationState();
    // Not use
    //m_CourseToSteer = msg->courseToSteer();
    //m_TargetSpeed = msg->targetSpeed();
    //Windvane?
    //m_Tack = msg->tack(); // useful ?
    // No definition for the starboard
}

///----------------------------------------------------------------------------------
double CourseRegulatorNode::calculateRudderAngle()
{
    if((m_DesiredHeading != HEADING_ERROR_VALUE) and (m_VesselHeading != HEADING_ERROR_VALUE)){

        double difference_Heading = Utility::limitAngleRange(m_VesselHeading) - Utility::limitAngleRange(m_DesiredHeading);
        // Equation from book "Robotic Sailing 2015 ", page 141
        // The MAX_RUDDER_ANGLE is a parameter configuring the variation around the desired heading.
        // Also the reaction could be configure by the frequence of the thread.
        if(cos(Utility::degreeToRadian(difference_Heading)) < 0) // Wrong sense because over 90°
        {
            // Max Rudder angle in the opposite way
            return Utility::sgn(m_VesselSpeed)*(Utility::sgn(sin(Utility::degreeToRadian(difference_Heading))))*m_MaxRudderAngle;
        }
        // Regulation of the rudder
        return Utility::sgn(m_VesselSpeed)*(sin(Utility::degreeToRadian(difference_Heading))*m_MaxRudderAngle);

    }
    return 0;
}

///----------------------------------------------------------------------------------
void CourseRegulatorNode::CourseRegulatorNodeThreadFunc(ActiveNode* nodePtr)
{
    CourseRegulatorNode* node = dynamic_cast<CourseRegulatorNode*> (nodePtr);

    // An initial sleep, its purpose is to ensure that most if not all the sensor data arrives
    // at the start before we send out the state message.
    std::this_thread::sleep_for(std::chrono::milliseconds(node->STATE_INITIAL_SLEEP));

    Timer timer;
    timer.start();

    while(true)
    {

        node->m_lock.lock();
        //std::lock_guard<std::mutex> lock_guard(node->m_lock);
        // TODO : Modify Actuator Message for adapt to this Node
        MessagePtr actuatorMessage = std::make_unique<ActuatorPositionMsg>(node->calculateRudderAngle(),0);
        //std::cout << std::endl << "COURSE REG NODE ######### Send : CalcR " << node->calculateRudderAngle();
        node->m_MsgBus.sendMessage(std::move(actuatorMessage));
        node->m_lock.unlock();

        // Broadcast() or selected sent???
        timer.sleepUntil(node->m_LoopTime); //insert updateFrequencyThread in the function ?
        timer.reset();
    }
}