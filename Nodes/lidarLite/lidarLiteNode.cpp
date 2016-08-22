#include "lidarLiteNode.h"
using namespace std;


lidarLiteNode::lidarLiteNode(MessageBus& msgBus,int delay,bool debug)
	: ActiveNode(NodeID::ColorDetection, msgBus),m_delay(delay),m_fd(0),m_debug(debug),m_Initialised(false)
{

}

lidarLiteNode::lidarLiteNode(MessageBus& msgBus,int delay)
	: ActiveNode(NodeID::ColorDetection, msgBus),m_delay(delay),m_fd(0),m_debug(false),m_Initialised(false)
{

}

bool lidarLiteNode::init(){

	m_fd = lidar_init(m_debug);
	if(m_fd != -1){
		m_Initialised=true;
	}
	else{
		m_Initialised=false;
	}
	return m_Initialised;

}

void lidarLiteNode::start()
{
	if(m_Initialised)
	{
		runThread(lidarThreadFunc);
	}
	else{
		Logger::error("%sCannot open the Lidar connexion", __PRETTY_FUNCTION__);
	}
}

void lidarLiteNode::lidarThreadFunc(void* nodePtr)
{
	lidarLiteNode* node = (lidarLiteNode*)nodePtr;
	Logger::info("lidarLiteNode thread started");
	int res, st;
	for (;;) {
		res = lidar_read(node->m_fd);
		st = lidar_status(node->m_fd);
		Logger::info("%3.0d cm \n", res);
		lidar_status_print(st);

		MessagePtr msg = std::make_unique<LidarMsg>(res);
		node->m_MsgBus.sendMessage(std::move(msg));

		std::this_thread::sleep_for(std::chrono::milliseconds(node->m_delay));
	}
}

void lidarLiteNode::processMessage(const Message* msgPtr)
{

}