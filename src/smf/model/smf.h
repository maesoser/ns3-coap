/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef SMF_H
#define SMF_H

#include "ns3/test.h"
#include "ns3/object.h"
#include "ns3/node.h"
#include "ns3/timer.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-static-routing.h"
#include "ns3/ipv4-address.h"

#include <vector>
#include <map>

namespace ns3 {

    namespace smf {

        class RoutingProtocol;

        class RoutingProtocol : public Ipv4RoutingProtocol {
        public:

            RoutingProtocol();
            void SetNetdevicelistener(std::set<uint32_t> listen);
            void SetManetIid(uint32_t intout);

            virtual ~RoutingProtocol();
            static TypeId GetTypeId(void);
            virtual Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p,
                    const Ipv4Header &header,
                    Ptr<NetDevice> oif,
                    Socket::SocketErrno & sockerr);
            virtual bool RouteInput(Ptr<const Packet> p,
                    const Ipv4Header &header,
                    Ptr<const NetDevice> idev,
                    UnicastForwardCallback ucb,
                    MulticastForwardCallback mcb,
                    LocalDeliverCallback lcb,
                    ErrorCallback ecb);
            virtual void NotifyInterfaceUp(uint32_t interface);
            virtual void NotifyInterfaceDown(uint32_t interface);
            virtual void NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address);
            virtual void NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address);
            virtual void SetIpv4(Ptr<Ipv4> ipv4);
            virtual void PrintRoutingTable(Ptr<OutputStreamWrapper> stream) const;

        protected:
            Ptr<Ipv4MulticastRoute> LookupStatic(Ipv4Address origin, Ipv4Address group, uint32_t interface, uint8_t will);
            void Clean();
            virtual void DoInitialize(void);
            virtual void DoDispose(void);
            bool checkhash(Ptr<const Packet> p, Ipv4Address ipaddr);
            Ipv4Address getMainLocalAddr();

        private:
            std::set<uint32_t> m_netdevice;
            std::vector<uint32_t> v;
            Timer m_cleanTimer;
            Time m_cleanIntervall;
            Ptr<Ipv4> m_ipv4;
            uint32_t iidout;
        };
    }
}

#endif /* SMF_H */

