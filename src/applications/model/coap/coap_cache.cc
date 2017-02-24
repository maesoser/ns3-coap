#include "coap_node.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CoapNode_cache");

bool CoapNode::existEntry(Ipv4Address ip, std::string url){
	if(ip==GetAddr()) return true;
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
	if(!m_cache.empty()){
		for (u_int32_t i=0; i<m_cache.size(); ++i){
			NS_LOG_INFO("CACHE_DUMP,"<<Simulator::Now ().GetSeconds ()<<","<< GetAddr()<<","<< std::to_string(m_cache[i].age)<<","<<Ipv4AddressToString(m_cache[i].ip)<<"/"<<m_cache[i].url);
		}
	}
  else{
      NS_LOG_INFO("CACHE_DUMP,"<<Simulator::Now ().GetSeconds ()<<","<< GetAddr()<<",EMP");
  }
	saveCache();
  m_showCache = Simulator::Schedule (Seconds(30), &CoapNode::showCache, this);
	checkCache();
	deleteOutdated();
}

void CoapNode::saveCache(){
	Ptr<OutputStreamWrapper> cacheStream = Create<OutputStreamWrapper>(Ipv4AddressToString(GetAddr())+"_cache", std::ios::app);
	std::ostream *stream = cacheStream->GetStream ();
	if(!m_cache.empty()){
		*stream << "CACHE_DUMP\t"<<Simulator::Now ().GetSeconds ()<<"\t"<< GetAddr() <<"\t"<< m_cache.size() <<std::endl;
		for (u_int32_t i=0; i<m_cache.size(); ++i){
			*stream << std::to_string(m_cache[i].age)<<"\t"<<Ipv4AddressToString(m_cache[i].ip)<<"/"<<m_cache[i].url<<std::endl;
		}
	}
  else{
		*stream << "CACHE_DUMP\t"<<Simulator::Now ().GetSeconds ()<<"\t"<< GetAddr() <<"\t0"<<std::endl;
  }
}

void CoapNode::sendMDnsCache(Query query){
	//checkCache();
	deleteOutdated();
	NS_LOG_INFO("MDNS_CACHE_SEND");
	MDns cmdns(m_dnssocket,m_txTrace);
	cmdns.Clear();

	Ipv4Address dnsmcast(MDNS_MCAST_ADDR);

	std::string result = Ipv4AddressToString(GetAddr())+ "/temp";
	const char * cresult = result.c_str();
	struct Answer ownansw;
	strncpy(ownansw.rdata_buffer,cresult, MAX_MDNS_NAME_LEN);
	strncpy(ownansw.name_buffer,  query.qname_buffer, MAX_MDNS_NAME_LEN);
	ownansw.rrtype = MDNS_TYPE_PTR;
	ownansw.rrclass = 1;    // "INternet"
	ownansw.rrttl = m_ageTime;
	ownansw.rrset = 1;
	cmdns.AddAnswer(ownansw);
	if (m_cachesize!=0){
		if(!m_cache.empty()){
			for (u_int32_t i=0; i<m_cache.size(); ++i){
				std::ostringstream oss;
				m_cache[i].ip.Print (oss);
				std::string result = oss.str()+ "/" +m_cache[i].url;
				const char * cresult = result.c_str();
				struct Answer rransw;
				//NS_LOG_INFO("DEBUG_ANSW from " << Ipv4AddressToString(GetAddr()) << " SEND " << cresult);
				strncpy(rransw.rdata_buffer,cresult, MAX_MDNS_NAME_LEN);
				strncpy(rransw.name_buffer,  query.qname_buffer, MAX_MDNS_NAME_LEN);
				rransw.rrtype = MDNS_TYPE_PTR;
				rransw.rrclass = 1;    // "INternet"
				rransw.rrttl = m_ageTime;
				rransw.rrset = 1;
				cmdns.AddAnswer(rransw);
				//NS_LOG_INFO("MDNS_CACHE_SEND,"<<Simulator::Now ().GetSeconds ()<<","<< GetAddr()<<","<< std::to_string(m_cache[i].age)<<","<<Ipv4AddressToString(m_cache[i].ip)<<"/"<<m_cache[i].url);
			}
		}
	}
	cmdns.Send(m_dnssocket,dnsmcast,m_txTrace);
}
bool CoapNode::updateEntry(Ipv4Address addr,std::string url, uint32_t maxAge){
	size_t prophash = std::hash<std::string>()(Ipv4AddressToString(addr)+""+url);
	if(!m_cache.empty()){
		for (u_int32_t i=0; i<m_cache.size(); ++i){
      size_t hash = std::hash<std::string>()(Ipv4AddressToString(m_cache[i].ip)+""+m_cache[i].url);
			if(hash==prophash) {
				m_cache[i].age = (uint32_t) Simulator::Now().GetSeconds() + maxAge; // This is the time where the entry is gonna be outdated
				return true;
			}
		}
	}
	return false;
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
		if(m_activatePing){
			NS_LOG_INFO("START_PING");
			for (u_int32_t i=0; i<m_cache.size(); ++i){
				ping(m_cache[i].ip,COAP_DEFAULT_PORT);
			}
			NS_LOG_INFO("STOP_PING");
		}
	}
}
