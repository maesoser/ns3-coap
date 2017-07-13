#include "coap_node.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CoapNode_tools");

std::string CoapNode::Ipv4AddressToString (Ipv4Address ad){
  std::ostringstream oss;
  ad.Print (oss);
  return oss.str ();
}

Ipv4Address CoapNode::GetAddr(){
  Ptr<Node> node = GetNode(); // Get pointer to ith node in container
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>(); // Get Ipv4 instance of the node
  Ipv4Address addr = ipv4->GetAddress (1,0).GetLocal(); // Get Ipv4InterfaceAddress of xth interface.
  return addr;
}

uint64_t CoapNode::Uniform(double max) {
	ns3::Ptr<ns3::UniformRandomVariable> random = ns3::CreateObject<ns3::UniformRandomVariable>();
	return random->GetValue(0.0, max);
}

uint64_t CoapNode::getResponseTime(){
  uint16_t distime = 600;
  uint64_t maxtime = 1000*CONF_WAIT_REPL*(3600.0/(3600.0+distime*m_cache.size()));
  uint64_t dtime = Uniform(maxtime);
  return dtime;
}

void CoapNode::splitlist( std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

std::vector<std::string> CoapNode::split( std::string &s, char delim) {
    std::vector<std::string> elems;
    splitlist(s, delim, elems);
    return elems;
}

std::string CoapNode::getTypeStr(uint8_t type){
    std::string result = std::to_string(type);
    if(type==0) result = "COAP_CON";
    if(type==1) result = "COAP_NONCON";
    if(type==2) result = "COAP_ACK";
    if(type==3) result = "COAP_RESET";
    return result;
}

/*
void savelog(std::string logline){
	Ptr<OutputStreamWrapper> cacheStream = Create<OutputStreamWrapper>(Ipv4AddressToString(GetAddr())+"_nodelog", std::ios::app);
	std::ostream *stream = cacheStream->GetStream ();
	*stream << logline <<std::endl;
}
*/

std::string CoapNode::getMthStr(uint8_t type){
  std::string result = std::to_string(type);
  if(type==0) result = "COAP_NULL";
  if(type==1) result = "COAP_GET";
  if(type==2) result = "COAP_POST ";
  if(type==3) result = "COAP_PUT";
  if(type==4) result = "COAP_DELETE";
  return result;

}
