#include "coap_node.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CoapNode_cache");


bool CoapNode::existEntry(Ipv4Address ip, std::string url){
  size_t prophash = std::hash<std::string>()(Ipv4AddressToString(ip)+""+url);
	if(!m_cache.empty()){
		for (u_int32_t i=0; i<m_cache.size(); ++i){
      size_t hash = std::hash<std::string>()(Ipv4AddressToString(m_cache[i].ip)+""+m_cache[i].url);
			if(hash==prophash) {
				return true;
			}
		}
	}
	return false;
}

int CoapNode::GetEntryIndex(size_t nodeid){
	if(!m_cache.empty()){
		for (u_int32_t i=0; i<m_cache.size(); ++i){
      size_t hash = std::hash<std::string>()(Ipv4AddressToString(m_cache[i].ip)+""+m_cache[i].url);
			if(hash==nodeid) {
				return i;
			}
		}
	}
	return -1;
}
/*
bool CoapNode::RefreshEntry(uint32_t index){
	if(!m_cache.empty()){
		//m_cache[index].age = Simulator::Now();
	}
	return false;
}

*/
uint32_t CoapNode::deleteOutdated(){
  if (m_ageTime==0) return 0; // No está conectado el tiempo de expiración
	uint32_t ndel = 0;
	if(!m_cache.empty()){
		for (u_int32_t i=0; i < m_cache.size(); ++i){
      //NS_LOG_INFO ("NOW "<<Simulator::Now().GetSeconds()<<" CACHE "<<m_cache[i].age.GetSeconds());
      if(m_cache[i].age > 4000000000){
        m_cache.erase(m_cache.begin() + i);
				ndel = ndel+1;
      }else{
        if(Simulator::Now().GetSeconds() > m_cache[i].age){
          m_cache.erase(m_cache.begin() + i);
          ndel = ndel+1;
        }
      }
		}
	}
	return ndel;
}

bool CoapNode::deleteEntry(size_t nodeid){
	if(!m_cache.empty()){
		for (u_int32_t i=0; i<m_cache.size(); ++i){
      size_t hash = std::hash<std::string>()(Ipv4AddressToString(m_cache[i].ip)+""+m_cache[i].url);
			if(hash==nodeid) {
				m_cache.erase(m_cache.begin() + i);
				return true;
			}
		}
	}
	return false;
}


size_t CoapNode::getOldestEntry(){
	size_t rindex = 0;
	uint32_t firstTime = 0xFFFFFFFE;
	if(!m_cache.empty()){
		for (u_int32_t i=0; i<m_cache.size(); ++i){
			if(m_cache[i].age < firstTime) {
				firstTime = m_cache[i].age;
				rindex = std::hash<std::string>()(Ipv4AddressToString(m_cache[i].ip)+""+m_cache[i].url);
			}
		}
	}
	return rindex;
}

void CoapNode::showCache(){
  //checkCache();
  deleteOutdated();
	if(!m_cache.empty()){
		for (u_int32_t i=0; i<m_cache.size(); ++i){
		NS_LOG_INFO("CACHE_DUMP,"<<Simulator::Now ().GetSeconds ()<<","<< GetAddr()<<","<< std::to_string(m_cache[i].age)<<","<<Ipv4AddressToString(m_cache[i].ip)<<"/"<<m_cache[i].url);
		}
	}
  else{
      NS_LOG_INFO("CACHE_DUMP,"<<Simulator::Now ().GetSeconds ()<<","<< GetAddr()<<",EMP");
  }
      m_showCache = Simulator::Schedule (Seconds(30), &CoapNode::showCache, this);
}

bool CoapNode::addEntry(Ipv4Address addr,std::string url, uint32_t maxAge){
  if (existEntry(addr,url)) return false;
	struct coapCacheItem item;
	item.age = (uint32_t) Simulator::Now().GetSeconds() + maxAge; // This is the time where the entry is gonna be outdated
	item.ip = addr;
	item.url = url;
	m_cache.push_back(item);
	return true;
}


void CoapNode::checkCache(){
  if(!m_cache.empty()){
		for (u_int32_t i=0; i<m_cache.size(); ++i){
      SendPing(m_cache[i].ip,COAP_DEFAULT_PORT);
		}
	}
}
