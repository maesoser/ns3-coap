/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */



#include "ns3/socket-factory.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/simulator.h"

#include "ns3/names.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/ipv4-route.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket.h"
#include "ns3/smf.h"
#include "smf.h"

#include<algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("smfLog");

    namespace smf {
        NS_OBJECT_ENSURE_REGISTERED(RoutingProtocol);

        TypeId RoutingProtocol::GetTypeId(void) {
            static TypeId tid = TypeId("ns3::smf::RoutingProtocol")
                    .SetParent<Ipv4RoutingProtocol> ()
                    .SetGroupName("Smf")
                    .AddConstructor<RoutingProtocol> ();

            return tid;
        }

        RoutingProtocol::RoutingProtocol() :
        m_ipv4(0)
         {
        }

        RoutingProtocol::~RoutingProtocol() {
        };

        ns3::Ptr<ns3::Ipv4> m_ipv4;
        std::set<uint32_t> m_netdevice;
        std::vector<uint32_t> v;      // Buffer of sent packet hashes
        //std::vector<std::string> vStrings;
        Timer m_cleanTimer;
        Time m_cleanIntervall = Time("10s");
        uint32_t iidout;

        void RoutingProtocol::SetNetdevicelistener(std::set<uint32_t> listen) {
            m_netdevice = listen;
        }
        void RoutingProtocol::SetManetIid(uint32_t intout){
            iidout = intout;
        }

        Ptr<Ipv4MulticastRoute> RoutingProtocol::LookupStatic(Ipv4Address origin,Ipv4Address group,uint32_t interface, uint8_t ttl) {
            Ptr<Ipv4MulticastRoute> mrtentry = 0;
            mrtentry = Create<Ipv4MulticastRoute> ();
            mrtentry->SetGroup(group);
            mrtentry->SetOrigin(origin);
            mrtentry->SetParent(iidout);
            mrtentry->SetOutputTtl(iidout,ttl);
            return mrtentry;
        }

        void RoutingProtocol::SetIpv4(Ptr<Ipv4> ipv4) {
            NS_ASSERT(ipv4 != 0);
            NS_ASSERT(m_ipv4 == 0);
            m_cleanTimer.SetFunction(&RoutingProtocol::Clean, this);
            m_ipv4 = ipv4;
        }

        void RoutingProtocol::DoDispose() {
            m_ipv4 = 0;
             m_cleanTimer.Cancel();
        }
        Ptr<Ipv4Route> RoutingProtocol::RouteOutput(Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno & sockerr) {
            Ptr<Ipv4Route> rtentry = 0;
            /*
            Ipv4Address dst = header.GetDestination ();
            Ipv4Address origin = header.GetSource ();
            Ipv4Address m_mainAddress = getMainLocalAddr();

            rtentry = Create<Ipv4Route> ();
            rtentry->SetSource(origin);
            rtentry->SetDestination(dst);
            rtentry->SetGateway(m_mainAddress);
            rtentry->SetOutputDevice(m_ipv4->GetNetDevice (0));*/

            return rtentry;

        }

        /*
         * checkhash(Ptr<const Packet> p)
         *
         * Returns FALSE if the item has been already SENT
         * Returns TRUE if the item is NEW
         *
         */
        bool RoutingProtocol::checkhash(Ptr<const Packet> p, Ipv4Address ipaddr) {
            std::ostringstream oss;
            ipaddr.Print (oss);
            std::string stringToHash = oss.str()+(p->ToString());
            uint32_t hash = Hash32(stringToHash);
            uint32_t nodeid = m_ipv4->GetObject<Node>()-> GetId();
            if (std::find(v.begin(), v.end(), hash) == v.end()) {
                // The element is not on the list
                //NS_LOG_DEBUG (Simulator::Now ().GetSeconds () <<" node "<<getMainLocalAddr()<<" PACKET HASH: "<< hash << " (NEW)");
                //vStrings.push_back(stringToHash); -> Just in case we want to use the whole packet
                v.push_back(hash);// Add the element
                return true;
            } else {
                //NS_LOG_DEBUG (Simulator::Now ().GetSeconds () <<" node "<<getMainLocalAddr()<<" PACKET HASH : "<< hash << " (REPEATED)");
                return false;
            }
        }
        Ipv4Address RoutingProtocol::getMainLocalAddr(){
            Ipv4Address loopback ("127.0.0.1");
            for (uint32_t i = 0; i < m_ipv4->GetNInterfaces (); i++){
                // Use primary address, if multiple
                Ipv4Address addr = m_ipv4->GetAddress (i, 0).GetLocal ();
                if (addr != loopback){
                    return addr;
                }
            }
            return 0;
        }
        void RoutingProtocol::Clean() {
            NS_LOG_DEBUG ("ERASING HASH TABLE");
            v.erase(v.begin(), v.begin()+(v.size() / 3));
            m_cleanTimer.Schedule(m_cleanIntervall);
        }

        void RoutingProtocol::PrintRoutingTable(Ptr<OutputStreamWrapper> stream) const {

        }

        bool RoutingProtocol::RouteInput(Ptr<const Packet> p,
                    const Ipv4Header &header,
                    Ptr<const NetDevice> idev,
                    UnicastForwardCallback ucb,
                    MulticastForwardCallback mcb,
                    LocalDeliverCallback lcb,
                    ErrorCallback ecb) {

            NS_ASSERT(m_ipv4 != 0); // Check if input device supports IP
            NS_ASSERT(m_ipv4->GetInterfaceForDevice(idev) >= 0);

            Ipv4Address dst = header.GetDestination ();     // Destination Address
            Ipv4Address origin = header.GetSource ();       // Origin address
            Ipv4Address m_mainAddress = getMainLocalAddr(); // Address of this node
            uint32_t outif = m_ipv4->GetInterfaceForDevice(idev);
            uint8_t outttl = header.GetTtl();
            if (origin==m_mainAddress){ //Consume self-originated packets
                return true;
            }
            if(dst==origin){  // Consume loop packets
              return true;
            }
            if(outttl<1){ // Consume packets with no ttl
                return true;
            }

            if(checkhash(p,origin)){ // If it is NEW
              uint32_t nodeid = m_ipv4->GetObject<Node>()->GetId();
              if(dst.IsMulticast()){
                Ptr<Ipv4MulticastRoute> mrtentry = new Ipv4MulticastRoute();
                mrtentry->SetGroup(dst);
                mrtentry->SetOrigin(origin);
                mrtentry->SetOutputTtl(outif, outttl-1);
                mrtentry->SetParent (outif);
                //NS_LOG_INFO("["<<nodeid<<"] FORWARD from " << mrtentry->GetOrigin() << " to "<< mrtentry->GetGroup()<< " via "<< outif << " TTL:"<< unsigned(outttl)-1);
                NS_LOG_INFO(Simulator::Now ().GetSeconds () <<" node "<<getMainLocalAddr()<<" FORWARD from " << mrtentry->GetOrigin() << " to "<< mrtentry->GetGroup()<< " via "<< outif << " TTL:"<< unsigned(outttl)-1);
                mcb(mrtentry, p, header);
                return true;
              }else{      // If it is unicast, send through multicast channel
                NS_LOG_INFO("["<< nodeid <<"] FORWARD TO UPPER ROUTING DIRECTIVE");
                return false;
              }
            }else{  // If it is not new
              return true; // Process (discard)
            }
            /*if (dst.IsMulticast()) {    // If it is multicast
                Ptr<Ipv4MulticastRoute> mrtentry = LookupStatic(origin,dst, outif, outttl);
                if (mrtentry){
                    if(checkhash(p,origin)) { // SI EL PAQUETE ES NUEVO
                        uint32_t nodeid = m_ipv4->GetObject<Node>()->GetId();
                        NS_LOG_INFO("["<<nodeid<<"] FORWARD from " << mrtentry->GetOrigin() << " to "<< mrtentry->GetGroup()<< " via "<< outif << " TTL:"<< unsigned(outttl)-1);
                        //QUIZAS TENGO QUE CAMBIAR EL PUEERTO?
                        //mcb(mrtentry, p, header);
                        return false;
                    }
                    else{
                        // SI EL PAQUETE ES VIEJO, YO ME OCUPO Y ME LO CARGO
                        return true;
                    }
                }

            }*/
            return false;
        }

        void RoutingProtocol::NotifyInterfaceUp(uint32_t i) {
        }

        void RoutingProtocol::NotifyInterfaceDown(uint32_t i) {
        }

        void RoutingProtocol::NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address) {
        }

        void RoutingProtocol::NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address) {
        }

        void RoutingProtocol::DoInitialize() {
        }
    }

}
