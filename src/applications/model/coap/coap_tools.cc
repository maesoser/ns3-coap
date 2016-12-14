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

uint64_t CoapNode::Normal(double max) {
	ns3::Ptr<ns3::UniformRandomVariable> random = ns3::CreateObject<ns3::UniformRandomVariable>();
	return random->GetValue(0.0, max);
}

void CoapNode::split( std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

std::vector<std::string> CoapNode::split( std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

std::string CoapNode::getTypeStr(uint8_t type){
    std::string result = "UNKNOWN";
    if(type==0) result = "COAP_CON";
    if(type==1) result = "COAP_NONCON";
    if(type==2) result = "COAP_ACK";
    if(type==3) result = "OAP_RESET";
    return result;
}
std::string CoapNode::getMthStr(uint8_t type){
  std::string result = "UNKNOWN";
  if(type==0) result = "COAP_NULL";
  if(type==1) result = "COAP_GET";
  if(type==2) result = "COAP_POST ";
  if(type==3) result = "COAP_PUT";
  if(type==4) result = "COAP_DELETE";
  return result;

}
