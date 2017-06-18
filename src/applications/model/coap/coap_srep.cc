#include "coap_node.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CoapNode_srep");

uint8_t CoapNode::checkID(uint16_t id){
	if(!m_idlist.empty()){
		for (u_int32_t i=0; i<m_idlist.size(); ++i){
			if( m_idlist[i].id==id) {
				return m_idlist[i].status;
			}
		}
	}
	return PKT_NOTFOUND;
}

bool CoapNode::setIDStatus(uint16_t id, uint8_t status){
	if(!m_idlist.empty()){
		for (u_int32_t i=0; i<m_idlist.size(); ++i){
			if( m_idlist[i].id==id) {
				m_idlist[i].status = status;
        return true;
			}
		}
	}
	return false;
}

void CoapNode::dumpIDList(){
	if(!m_idlist.empty()){
		for (u_int32_t i=0; i<m_idlist.size(); ++i){
			NS_LOG_INFO ("IDLST "<<Simulator::Now().GetSeconds () <<"\t"<< m_idlist[i].id <<"\t"<< m_idlist[i].status);
		}
	}
}

bool CoapNode::addID(uint16_t id, EventId eid){
	if(checkID(id)==false){
		eventItem itm;
		itm.id = id;
		itm.eid = eid;
		itm.status = PKT_DELAYED;
		m_idlist.push_back(itm);
		return true;
	}
	return false;
}

bool CoapNode::existAnswEntry(std::vector<size_t> cacheansw, size_t hash){
	if(!cacheansw.empty()){
		for (u_int32_t i=0; i<cacheansw.size(); ++i){
			if( cacheansw[i]==hash) {
				
				return true;
			}
		}
	}
	return false;
}

// Check if an associated response already has a the service just received. If not, it add it.
bool CoapNode::updateID(uint16_t id, Ipv4Address ip, std::string url){
  size_t hash = std::hash<std::string>()(Ipv4AddressToString(ip)+""+url);
  if(!m_idlist.empty()){
    for (u_int32_t i=0; i<m_idlist.size(); ++i){
      if( m_idlist[i].id==id) {
        if (existAnswEntry(m_idlist[i].cacheansw, hash)==false){
          m_idlist[i].cacheansw.push_back(hash);
          return true;
        }else{
          return false;
        }
      }
    }
  }
  return false;
}

// Check if an associated response already has a the service just received. If not, it add it.
bool CoapNode::checkServiceInDelayedResponse(uint16_t id, Ipv4Address ip, std::string url){
  size_t hash = std::hash<std::string>()(Ipv4AddressToString(ip)+""+url);
  if(!m_idlist.empty()){
    for (u_int32_t i=0; i<m_idlist.size(); ++i){
      if( m_idlist[i].id==id) {
        return existAnswEntry(m_idlist[i].cacheansw, hash);
      }
    }
  }
  return false;
}
bool CoapNode::delID(uint16_t id){
	if(!m_idlist.empty()){
		for (u_int32_t i=0; i<m_idlist.size(); ++i){
			if( m_idlist[i].id==id) {
				m_idlist[i].status = PKT_CANCELED;
				Simulator::Cancel(m_idlist[i].eid);
				m_idlist[i].eid.Cancel();
				//m_idlist.erase (m_idlist.begin()+i);
				return true;
			}
		}
	}
	return false;
}
