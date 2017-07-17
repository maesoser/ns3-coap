#include "coap_node.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CoapNode_rx");

bool CoapNode::recvDtg(Ptr<Socket>socket) {
  uint8_t buffer[BUF_MAX_SIZE];

  Ptr<Packet> dtgpacket;
  Address     from;
  int packetlen = 0;

  while ((dtgpacket = socket->RecvFrom(from))) {
    packetlen = dtgpacket->GetSize();
    dtgpacket->CopyData(buffer, packetlen >= BUF_MAX_SIZE ? BUF_MAX_SIZE : packetlen);
    CoapPacket packet;

    // parse coap packet header
    if ((packetlen < COAP_HEADER_SIZE) || (((buffer[0] & 0xC0) >> 6) != 1)) {
      packetlen = dtgpacket->GetSize();
      continue;
    }

    packet.type       = (buffer[0] & 0x30) >> 4;
    packet.tokenlen   = buffer[0] & 0x0F;
    packet.code       = buffer[1];
    packet.messageid  = 0xFF00 & (buffer[2] << 8);
    packet.messageid |= 0x00FF & buffer[3];

    if (InetSocketAddress::IsMatchingType(from)) {
      NS_LOG_INFO(Simulator::Now().GetSeconds() << " " << GetAddr() <<
                  " RECV " << packetlen << " bytes from " << InetSocketAddress::ConvertFrom(from).GetIpv4() <<
                  " COAP TYPE:" << getTypeStr(packet.type) << " CODE:" << getMthStr(packet.code) << " ID:" << packet.messageid);
    }

    if (packet.tokenlen == 0) packet.token = NULL;
    else if (packet.tokenlen <= 8) packet.token = buffer + 4;
    else {
      packetlen = dtgpacket->GetSize();
      continue;
    }

    // parse packet options/payload
    if (COAP_HEADER_SIZE + packet.tokenlen < packetlen) {
      int optionIndex = 0;
      uint16_t delta  = 0;
      uint8_t *end    = buffer + packetlen;
      uint8_t *p      = buffer + COAP_HEADER_SIZE + packet.tokenlen;

      while (optionIndex < MAX_OPTION_NUM && *p != 0xFF && p < end) {
        packet.options[optionIndex];

        if (0 != parseOption(&packet.options[optionIndex], &delta, &p, end - p)) return false;

        optionIndex++;
      }
      packet.optionnum = optionIndex;

      if ((p + 1 < end) && (*p == 0xFF)) {
        packet.payload    = p + 1;
        packet.payloadlen = end - (p + 1);
      } else {
        packet.payload    = NULL;
        packet.payloadlen = 0;
      }
    }

    if (packet.type == COAP_ACK) { // It is a RESPONSE!!
      if (m_stime == ALL_OR_NOTHING) delID(packet.messageid);

      uint32_t recvAge = 0x00;

      for (int i = 0; i < packet.optionnum; i++) {
        if ((packet.options[i].number == COAP_MAX_AGE) && (packet.options[i].length > 0)) {
          memcpy(&recvAge, packet.options[i].buffer, packet.options[i].length);
          recvAge = ntohl(recvAge);
        }
      }

      if (packet.payloadlen == 0) return false;

      std::string payloadStr(packet.payload, packet.payload + packet.payloadlen);
      std::vector<std::string> vect = split(payloadStr, ',');

      for (std::string vitem:vect) { // Go through the item on the packet
        std::string vname = split(vitem, ';')[0];

        if (vname.length() > 3) {
          vname = vname.substr(2, vname.length() - 3);
        } else {
          NS_LOG_INFO("!!! -ERROR- !!! LENGTH: " << packet.payloadlen << "\t PAYLOAD: " << payloadStr);
        }

        if (vname.find("/") != std::string::npos) {
          Ipv4Address sip(split(vname, '/')[0].c_str());
          std::string vser = split(vname, '/')[1];
          NS_LOG_INFO(GetAddr() << "UPDT " << packet.messageid << "\t" << sip << "" << vser);
          updateID(packet.messageid, sip, vser);       // Register this service as one of the discovered services for the answer.

          if (addEntry(sip, vser, recvAge) == false) { // Add entry to our service cache
            // If the sender is the same as the one depicted on the cache message, update it TTL
            if (sip == InetSocketAddress::ConvertFrom(from).GetIpv4()) {
              updateEntry(sip, vser, recvAge);
            }
          }

          // if(m_activatePing) ping(sip,COAP_DEFAULT_PORT);
        } else {                                                                       // It is the proper service from the node
          addEntry(InetSocketAddress::ConvertFrom(from).GetIpv4(), vname, recvAge);
          updateEntry(InetSocketAddress::ConvertFrom(from).GetIpv4(), vname, recvAge); // ¿?
          updateID(packet.messageid, InetSocketAddress::ConvertFrom(from).GetIpv4(), vname);

          // if(m_activatePing) ping(InetSocketAddress::ConvertFrom (from).GetIpv4(),COAP_DEFAULT_PORT);
        }
      }
    }
    else if ((packet.type == COAP_CON) && (packet.code != 0x00)) { // It is a request
      // call endpoint url function
      std::string url     = "";
      uint32_t    recvAge = 0;

      for (int i = 0; i < packet.optionnum; i++) {
        if ((packet.options[i].number == COAP_URI_PATH) && (packet.options[i].length > 0)) {
          char urlname[packet.options[i].length + 1];
          memcpy(urlname, packet.options[i].buffer, packet.options[i].length);
          urlname[packet.options[i].length] = NULL;

          if (url.length() > 0) url += "/";
          url += urlname;
        }

        if ((packet.options[i].number == COAP_MAX_AGE) && (packet.options[i].length > 0)) {
          memcpy(&recvAge, packet.options[i].buffer, packet.options[i].length);
        }
      }

      if (url.find("well-known/core") != std::string::npos) {
        if (m_stime != 0) {
          if (checkID(packet.messageid) == PKT_NOTFOUND) {
            uint64_t dtime               = getResponseTime();
            EventId  m_sendDiscoResponse = Simulator::Schedule(
              MilliSeconds(dtime),
              &CoapNode::sendCache,
              this,
              InetSocketAddress::ConvertFrom(from).GetIpv4(),
              InetSocketAddress::ConvertFrom(from).GetPort(),
              packet.messageid);
            addID(packet.messageid, m_sendDiscoResponse);
            NS_LOG_INFO(Simulator::Now().GetSeconds() << " " << GetAddr() << " DELAY ANSWER ID " << packet.messageid << " TO " << InetSocketAddress::ConvertFrom(from).GetIpv4() << " " << dtime << " ms");
          }
        } else {
          sendCache(
            InetSocketAddress::ConvertFrom(from).GetIpv4(),
            InetSocketAddress::ConvertFrom(from).GetPort(),
            packet.messageid);
        }
      }
    }
    else if ((packet.type == COAP_CON) && (packet.code == 0x00)) { // Answer to an ack
      CoapPacket rstack;
      rstack.type     = COAP_RESET;
      rstack.code     = 0;
      rstack.token    = 0;
      rstack.tokenlen = 0;

      rstack.payload    = NULL;
      rstack.payloadlen = 0;

      rstack.optionnum = 0;
      rstack.messageid = packet.messageid;
      return sendDtg(rstack, InetSocketAddress::ConvertFrom(from).GetIpv4(), InetSocketAddress::ConvertFrom(from).GetPort());
    }
    else if (packet.type == COAP_RESET) {
      Ptr<OutputStreamWrapper> cacheStream = Create<OutputStreamWrapper>(Ipv4AddressToString(GetAddr()) + "_rping", std::ios::app);
      std::ostream *stream                 = cacheStream->GetStream();
      *stream << Simulator::Now().GetSeconds() << ",PONG," << Ipv4AddressToString(InetSocketAddress::ConvertFrom(from).GetIpv4()) << std::endl;
    }
  }
  return true;
}
