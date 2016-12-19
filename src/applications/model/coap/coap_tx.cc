#include "coap_node.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CoapNode_tx");

uint16_t CoapNode::sendResponse(Ipv4Address ip, int port, uint16_t messageid, char *payload, int payloadlen,COAP_RESPONSE_CODE code, COAP_CONTENT_TYPE type, uint8_t *token, int tokenlen) {
    // make packet
    CoapPacket packet;

    packet.type = COAP_ACK;
    packet.code = code;
    packet.token = token;
    packet.tokenlen = tokenlen;
    packet.payload = (uint8_t *)payload;
    packet.payloadlen = payloadlen;
    packet.optionnum = 0;
    packet.messageid = messageid;

    // if more options?
    char optionBuffer[2];
    optionBuffer[0] = ((uint16_t)type & 0xFF00) >> 8;
    optionBuffer[1] = ((uint16_t)type & 0x00FF) ;
    packet.options[packet.optionnum].buffer = (uint8_t *)optionBuffer;
    packet.options[packet.optionnum].length = 2;
    packet.options[packet.optionnum].number = COAP_CONTENT_FORMAT;
    packet.optionnum++;

    return sendDtg(packet, ip, port);
}

void CoapNode::sendCache(Ipv4Address ip, int port, uint16_t messageid){
  std::string result = "";

  deleteOutdated();
  if(!m_cache.empty()){
    u_int32_t i_actual = 0;
    for (u_int32_t i=0; i<m_cache.size(); ++i){
      if(m_cache[i].ip!=ip){
        result = result + "</"+Ipv4AddressToString(m_cache[i].ip)+ "/" +m_cache[i].url+ ">;title=\"This is a test\",";
        i_actual++;
      }
      if(i_actual==3){
        i_actual=0;
        sendCachePart(ip,port, messageid,result);
        result = "";
      }
    }
  }
  result = result +"</temp>;title=\"This is the local temperature sensor\"";
  sendCachePart(ip,port, messageid,result);

}

uint16_t CoapNode::sendCoap(Ipv4Address ip, int port, char *url, COAP_TYPE type, COAP_METHOD method, uint8_t *token, uint8_t tokenlen, uint8_t *payload, uint32_t payloadlen) {

    // make packet
    CoapPacket packet;

    packet.type = type;
    packet.code = method;
    packet.token = token;
    packet.tokenlen = tokenlen;
    packet.payload = payload;
    packet.payloadlen = payloadlen;
    packet.optionnum = 0;
    packet.messageid = rand();

    // if more options?
    if(url){
      packet.options[packet.optionnum].buffer = (uint8_t *) url;
      packet.options[packet.optionnum].length = strlen(url);
    }else{
      packet.options[packet.optionnum].length = 0;
    }
    packet.options[packet.optionnum].number = COAP_URI_PATH;
    packet.optionnum++;

    // send packet
    return sendDtg(packet, ip, port);
}

uint16_t CoapNode::sendResponse(Ipv4Address ip, int port, uint16_t messageid) {
    return sendResponse(ip, port, messageid, NULL, 0, COAP_CONTENT, COAP_TEXT_PLAIN, NULL, 0);
}

uint16_t CoapNode::sendResponse(Ipv4Address ip, int port, uint16_t messageid, char *payload) {
    return sendResponse(ip, port, messageid, payload, strlen(payload), COAP_CONTENT, COAP_TEXT_PLAIN, NULL, 0);
}

uint16_t CoapNode::sendResponse(Ipv4Address ip, int port, uint16_t messageid, char *payload, int payloadlen) {
    return sendResponse(ip, port, messageid, payload, payloadlen, COAP_CONTENT, COAP_TEXT_PLAIN, NULL, 0);
}

void CoapNode::sendCachePart(Ipv4Address ip, int port, uint16_t messageid,std::string payloadstr){
  CoapPacket packet;

  packet.type = COAP_ACK;
  packet.code = COAP_CONTENT;
  packet.token = NULL;
  packet.tokenlen = 0;

  char *resultpayload = &payloadstr[0];
  packet.payload = (uint8_t *)resultpayload;
  packet.payloadlen = strlen(resultpayload);
  packet.optionnum = 0;
  packet.messageid = messageid;

  // if more options?

  uint8_t format = COAP_APPLICATION_LINK_FORMAT;
  packet.options[packet.optionnum].buffer = &format ;
  packet.options[packet.optionnum].length = 1;
  packet.options[packet.optionnum].number = 0b00001100;  // 12
  packet.optionnum = packet.optionnum + 1;

  uint8_t ageBuffer[4];
  ageBuffer[3] = (m_ageTime & 0x000000ff);
  ageBuffer[2] = (m_ageTime & 0x0000ff00) >> 8;
  ageBuffer[1] = (m_ageTime & 0x00ff0000) >> 16;
  ageBuffer[0] = (m_ageTime & 0xff000000) >> 24;
  packet.options[packet.optionnum].buffer = (uint8_t *) ageBuffer;
  packet.options[packet.optionnum].length = sizeof(ageBuffer);
  packet.options[packet.optionnum].number = COAP_MAX_AGE;
  packet.optionnum = packet.optionnum + 1;

  sendDtg(packet, ip, port);
}

void CoapNode::ping(Ipv4Address ip, int port){
  sendCoap(ip, port, NULL, COAP_CON, COAP_NULL, NULL, 0, NULL, 0);
}

void CoapNode::SendDiscovery(){
  char* discostr = "well-known/core";
  get(m_mcastAddr,m_port,discostr);
  if(m_interval!=0) ScheduleTransmit(m_interval);
}

uint16_t CoapNode::get(Ipv4Address ip, int port, char *url) {
    return sendCoap(ip, port, url, COAP_CON, COAP_GET, NULL, 0, NULL, 0);
}

uint16_t CoapNode::put(Ipv4Address ip, int port, char *url, char *payload) {
    return sendCoap(ip, port, url, COAP_CON, COAP_PUT, NULL, 0, (uint8_t *)payload, strlen(payload));
}

uint16_t CoapNode::put(Ipv4Address ip, int port, char *url, char *payload, int payloadlen) {
    return sendCoap(ip, port, url, COAP_CON, COAP_PUT, NULL, 0, (uint8_t *)payload, payloadlen);
}

void CoapNode::sendmDnsRequest(){
   MDns my_mdns;
   my_mdns.Clear();
   struct Query query_mqtt;
   strncpy(query_mqtt.qname_buffer,  "_mqtt._tcp.local", MAX_MDNS_NAME_LEN);
   query_mqtt.qtype = MDNS_TYPE_PTR;
   query_mqtt.qclass = 1;    // "INternet"
   query_mqtt.unicast_response = 0;
   my_mdns.AddQuery(query_mqtt);
   Ipv4Address dnsmcast(MDNS_MCAST_ADDR);
   //my_mdns.Send(m_dnssocket,dnsmcast,m_txTrace);

   Ptr<Packet> udp_p;
   udp_p = Create<Packet> ((uint8_t *)my_mdns.data_buffer, my_mdns.data_size);
   udp_p->RemoveAllPacketTags ();
   udp_p->RemoveAllByteTags ();

   m_txTrace(udp_p);

   m_dnssocket->SendTo(udp_p,0,InetSocketAddress(Ipv4Address::ConvertFrom(dnsmcast), MDNS_TARGET_PORT));

   if (Ipv4Address::IsMatchingType (dnsmcast))
     {
       NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s "<< GetAddr() <<" send " << my_mdns.data_size << " bytes to " << Ipv4Address::ConvertFrom (dnsmcast) << ":" << MDNS_MCAST_ADDR);
     }
   else if (Ipv6Address::IsMatchingType (dnsmcast))
     {
       NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s send " << my_mdns.data_size << " bytes to " << Ipv6Address::ConvertFrom (dnsmcast) << ":" << MDNS_MCAST_ADDR);
     }
   else if (InetSocketAddress::IsMatchingType (dnsmcast))
     {
       NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s send " << my_mdns.data_size << " bytes to " << InetSocketAddress::ConvertFrom (dnsmcast).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (dnsmcast).GetPort ());
     }
   else if (Inet6SocketAddress::IsMatchingType (dnsmcast))
     {
       NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s send " << my_mdns.data_size << " bytes to " << Inet6SocketAddress::ConvertFrom (dnsmcast).GetIpv6 () << ":" << Inet6SocketAddress::ConvertFrom (dnsmcast).GetPort ());
     }

     if(m_interval!=0) ScheduleTransmit(m_interval);

}

uint16_t CoapNode::sendDtg(CoapPacket &packet, Ipv4Address ip, int port) {
    uint8_t buffer[BUF_MAX_SIZE];
    uint8_t *p = buffer;
    uint16_t running_delta = 0;
    uint16_t packetSize = 0;

    // make coap packet base header
    *p = 0x01 << 6;
    *p |= (packet.type & 0x03) << 4;
    *p++ |= (packet.tokenlen & 0x0F);
    *p++ = packet.code;
    *p++ = (packet.messageid >> 8);
    *p++ = (packet.messageid & 0xFF);
    p = buffer + COAP_HEADER_SIZE;
    packetSize += 4;

    // make token
    if (packet.token != NULL && packet.tokenlen <= 0x0F) {
        memcpy(p, packet.token, packet.tokenlen);
        p += packet.tokenlen;
        packetSize += packet.tokenlen;
    }

    // make option header
    for (int i = 0; i < packet.optionnum; i++)  {
        uint32_t optdelta;
        uint8_t len, delta;

        if (packetSize + 5 + packet.options[i].length >= BUF_MAX_SIZE) {
            NS_LOG_INFO("DATAGRAM TOO BIG");
            return 0;
        }
        optdelta = packet.options[i].number - running_delta;

        COAP_OPTION_DELTA(optdelta, &delta);
        COAP_OPTION_DELTA((uint32_t)packet.options[i].length, &len);

        *p++ = (0xFF & (delta << 4 | len));
        if (delta == 13) {
            *p++ = (optdelta - 13);
            packetSize++;
        } else if (delta == 14) {
            *p++ = ((optdelta - 269) >> 8);
            *p++ = (0xFF & (optdelta - 269));
            packetSize+=2;
        } if (len == 13) {
            *p++ = (packet.options[i].length - 13);
            packetSize++;
        } else if (len == 14) {
            *p++ = (packet.options[i].length >> 8);
            *p++ = (0xFF & (packet.options[i].length - 269));
            packetSize+=2;
        }

        memcpy(p, packet.options[i].buffer, packet.options[i].length);
        p += packet.options[i].length;
        packetSize += packet.options[i].length + 1;
        running_delta = packet.options[i].number;
    }

    // make payload
    if (packet.payloadlen > 0) {
        if ((packetSize + 1 + packet.payloadlen) >= BUF_MAX_SIZE) {
            NS_LOG_INFO("DATAGRAM TOO BIG");
            return 0;
        }
        *p++ = 0xFF;
        memcpy(p, packet.payload, packet.payloadlen);
        packetSize += 1 + packet.payloadlen;
    }

     Ptr<Packet> udp_p;
     udp_p = Create<Packet> ((uint8_t *)buffer, packetSize);
     udp_p->RemoveAllPacketTags ();
     udp_p->RemoveAllByteTags ();
     // call to the trace sinks before the packet is actually sent,
     // so that tags added to the packet can be sent as well
     m_txTrace (udp_p);

     m_socket->SendTo(udp_p,0,InetSocketAddress(Ipv4Address::ConvertFrom(ip), port));

     if (Ipv4Address::IsMatchingType (ip))
       {
         NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s "<< GetAddr() <<" send " << packetSize << " bytes to " << Ipv4Address::ConvertFrom (ip) << ":" << port<< " id:"<<packet.messageid);
       }
     else if (Ipv6Address::IsMatchingType (ip))
       {
         NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s send " << packetSize << " bytes to " << Ipv6Address::ConvertFrom (ip) << ":" << port<< " id:"<<packet.messageid);
       }
     else if (InetSocketAddress::IsMatchingType (ip))
       {
         NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s send " << packetSize << " bytes to " << InetSocketAddress::ConvertFrom (ip).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (ip).GetPort ());
       }
     else if (Inet6SocketAddress::IsMatchingType (ip))
       {
         NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s send " << packetSize << " bytes to " << Inet6SocketAddress::ConvertFrom (ip).GetIpv6 () << ":" << Inet6SocketAddress::ConvertFrom (ip).GetPort ());
       }

    return packet.messageid;
}
