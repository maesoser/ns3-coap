/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef COAP_NODE_H
#define COAP_NODE_H

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ipv4.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/traced-callback.h"
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <arpa/inet.h>
#include <algorithm>
#include "ns3/output-stream-wrapper.h"
#include "mdns.h"

#define COAP_MCAST_ADDR "224.0.1.187"
#define MDNS_MCAST_ADDR "224.0.1.251" // CORRECT ADDR is 224.0.0.251

#define NEW_DTG_ID 0

#define CONF_WAIT_REPL 3

#define COAP_HEADER_SIZE 4
#define COAP_OPTION_HEADER_SIZE 1
#define COAP_PAYLOAD_MARKER 0xFF
#define MAX_OPTION_NUM 10
#define BUF_MAX_SIZE 512
#define COAP_DEFAULT_PORT 5683

#define MAX_DNSPACKET_SIZE 2048

#define RESPONSE_CODE(class, detail) ((class << 5) | (detail))
#define COAP_OPTION_DELTA(v, n) (v < 13 ? (*n = (0xFF & v)) : (v <= 0xFF + 13 ? (*n = 13) : (*n = 14)))

namespace ns3 {

	typedef enum {
	    COAP_CON = 0,
	    COAP_NONCON = 1,
	    COAP_ACK = 2,
	    COAP_RESET = 3
	} COAP_TYPE;

	typedef enum {
			COAP_NULL = 0,
	    COAP_GET = 1,
	    COAP_POST = 2,
	    COAP_PUT = 3,
	    COAP_DELETE = 4
	} COAP_METHOD;

	typedef enum {
	    COAP_CREATED = RESPONSE_CODE(2, 1),
	    COAP_DELETED = RESPONSE_CODE(2, 2),
	    COAP_VALID = RESPONSE_CODE(2, 3),
	    COAP_CHANGED = RESPONSE_CODE(2, 4),
	    COAP_CONTENT = RESPONSE_CODE(2, 5),
	    COAP_BAD_REQUEST = RESPONSE_CODE(4, 0),
	    COAP_UNAUTHORIZED = RESPONSE_CODE(4, 1),
	    COAP_BAD_OPTION = RESPONSE_CODE(4, 2),
	    COAP_FORBIDDEN = RESPONSE_CODE(4, 3),
	    COAP_NOT_FOUNT = RESPONSE_CODE(4, 4),
	    COAP_METHOD_NOT_ALLOWD = RESPONSE_CODE(4, 5),
	    COAP_NOT_ACCEPTABLE = RESPONSE_CODE(4, 6),
	    COAP_PRECONDITION_FAILED = RESPONSE_CODE(4, 12),
	    COAP_REQUEST_ENTITY_TOO_LARGE = RESPONSE_CODE(4, 13),
	    COAP_UNSUPPORTED_CONTENT_FORMAT = RESPONSE_CODE(4, 15),
	    COAP_INTERNAL_SERVER_ERROR = RESPONSE_CODE(5, 0),
	    COAP_NOT_IMPLEMENTED = RESPONSE_CODE(5, 1),
	    COAP_BAD_GATEWAY = RESPONSE_CODE(5, 2),
	    COAP_SERVICE_UNAVALIABLE = RESPONSE_CODE(5, 3),
	    COAP_GATEWAY_TIMEOUT = RESPONSE_CODE(5, 4),
	    COAP_PROXYING_NOT_SUPPORTED = RESPONSE_CODE(5, 5)
	} COAP_RESPONSE_CODE;

typedef enum {
	    COAP_IF_MATCH = 1,
	    COAP_URI_HOST = 3,
	    COAP_E_TAG = 4,
	    COAP_IF_NONE_MATCH = 5,
	    COAP_URI_PORT = 7,
	    COAP_LOCATION_PATH = 8,
	    COAP_URI_PATH = 11,
	    COAP_CONTENT_FORMAT = 12,
	    COAP_MAX_AGE = 14,
	    COAP_URI_QUERY = 15,
	    COAP_ACCEPT = 17,
	    COAP_LOCATION_QUERY = 20,
	    COAP_PROXY_URI = 35,
	    COAP_PROXY_SCHEME = 39
	} COAP_OPTION_NUMBER;

typedef enum {
	    COAP_NONE = -1,
	    COAP_TEXT_PLAIN = 0,
	    COAP_APPLICATION_LINK_FORMAT = 40,
	    COAP_APPLICATION_XML = 41,
	    COAP_APPLICATION_OCTET_STREAM = 42,
	    COAP_APPLICATION_EXI = 47,
	    COAP_APPLICATION_JSON = 50
	} COAP_CONTENT_TYPE;

	class CoapOption {
	    public:
		    uint8_t number;
		    uint8_t length;
		    uint8_t *buffer;
	};

	class CoapPacket {
	    public:
	    uint8_t type;
	    uint8_t code;
	    uint8_t *token;
	    uint8_t tokenlen;
	    uint8_t *payload;
	    uint8_t payloadlen;
	    uint16_t messageid;

	    uint8_t optionnum;
	    CoapOption options[MAX_OPTION_NUM];
	};

	struct coapCacheItem{
		Ipv4Address ip;
		uint32_t age;
		std::string url;
		std::string title;
	};


class Socket;
class Packet;

class CoapNode : public Application{
		public:
			CoapNode ();
			virtual ~CoapNode ();
			static TypeId GetTypeId (void);
			Ipv4Address GetAddr();
			Ptr<Socket> m_dnssocket;
			TracedCallback<Ptr<const Packet> > m_txTrace;     /// Callbacks for tracing the packet Tx events
			std::vector<struct ns3::coapCacheItem> m_cache;

		protected:
			virtual void DoDispose (void);
		private:
			virtual void StartApplication (void);
			virtual void StopApplication (void);
			void splitlist( std::string &s, char delim, std::vector<std::string> &elems);
			std::vector<std::string> split( std::string &s, char delim);
			void ScheduleTransmit (Time dt);
			void SchedulePurgeCache (uint32_t deltime);

			// RECEIVE THINGS
			void HandleDns(Ptr<Socket> socket);
			void HandleRead (Ptr<Socket> socket);
			int parseOption(CoapOption *option, uint16_t *running_delta, uint8_t **buf, size_t buflen);
			bool recvDtg(Ptr<Socket> socket);
			std::string Ipv4AddressToString (Ipv4Address ad);

			// SEND THINGS
			// Prepares the packet to be sent
			uint16_t sendCoap(Ipv4Address ip, int port, char *url, COAP_TYPE type, COAP_METHOD method, uint8_t *token, uint8_t tokenlen, uint8_t *payload, uint32_t payloadlen);
			// Make last things and send the datagram
			uint16_t sendDtg(CoapPacket &packet, Ipv4Address ip, int port);
			// Bindings for common send operations
			uint16_t get(Ipv4Address ip, int port, char *url);
			uint16_t put(Ipv4Address ip, int port, char *url, char *payload);
			uint16_t put(Ipv4Address ip, int port, char *url, char *payload, int payloadlen);
			//Deal wioth the scheduling thing and all that stuff
			void SendDiscovery();
			void sendCache(Ipv4Address ip, int port, uint16_t messageid);
			void sendMDnsCache(Query query, Address from);
			void ping(Ipv4Address ip, int port);
			void sendCachePart(Ipv4Address ip, int port, uint16_t messageid,std::string payloadstr);
			// Send responses (Taking care of messageid and changing some other things)
			uint16_t sendResponse(Ipv4Address ip, int port, uint16_t messageid);
			uint16_t sendResponse(Ipv4Address ip, int port, uint16_t messageid, char *payload);
			uint16_t sendResponse(Ipv4Address ip, int port, uint16_t messageid, char *payload, int payloadlen);
			uint16_t sendResponse(Ipv4Address ip, int port, uint16_t messageid, char *payload, int payloadlen,COAP_RESPONSE_CODE code, COAP_CONTENT_TYPE type, uint8_t *token, int tokenlen);
			void sendmDnsRequest();

			// CACHE STUFF
			int GetEntryIndex(size_t nodeid);
			size_t getOldestEntry();
			bool addEntry(Ipv4Address ip,std::string url,uint32_t maxAge);
			bool updateEntry(Ipv4Address ip,std::string url,uint32_t maxAge);

			bool deleteEntry(size_t nodeid);
			uint32_t deleteOutdated();
			void showCache();
			void saveCache();
			bool existEntry(Ipv4Address ip, std::string url);

			// DISCOVERY REQUESTS CACHE
			bool addID(uint16_t id, EventId eid);
			bool delID(uint16_t id);
			bool checkID(uint16_t id);

			// Tools
			uint64_t Normal(double max);

			void readServicesFile();

			std::string getTypeStr(uint8_t type);
			std::string getMthStr(uint8_t type);

			void checkCache();

			uint32_t m_etag; //! Is it mcast or ucast answer?
			uint32_t m_stime; //! Is it mcast or ucast answer?
			uint32_t m_cacheinterval;
			
			uint32_t m_mcast; //! Is it mcast or ucast answer?
			uint16_t m_dataType; //! Is it get disco or get temp
			uint16_t m_port; //!< Port on which we listen for incoming packets.
			Time m_interval; //!< Packet inter-send time
			uint32_t m_startDelay;	//! Retraso inicial en el envío de peticiones
			uint32_t m_count; //! Número máximos de peticiones
			bool m_petitionLimit;
			uint32_t m_ageTime;	//! TTL del caché
			uint16_t m_cachesize;

			struct eventItem{
				EventId eid;
				uint16_t id;
			};

			std::vector<eventItem> m_idlist;

			uint8_t *m_data; //!< packet payload data
			Ptr<Socket> m_socket; //!< IPv4 Socket
			Ptr<Socket> m_socket6; //!< IPv6 Socket

			Address m_local; //!< local multicast address

			Ipv4Address m_mcastAddr;

			EventId m_sendEvent; //!< Event to send the next packet
			EventId m_showCache;


			uint32_t m_activatemDns = 0;
            uint32_t m_activatePing = 0;

	}; // Class coapNode

} // namespace ns3

#endif /* COAP_NODE_H */
