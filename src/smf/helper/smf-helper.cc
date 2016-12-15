/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "smf-helper.h"
#include "ns3/smf.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/ipv4-routing-helper.h"
//#include "ns3/smf-helper.h"
#include "ns3/object-factory.h"

#include <map>
#include <set>

namespace ns3 {
    
 smfHelper::smfHelper() {
        m_agentFactory.SetTypeId("ns3::smf::RoutingProtocol");
    }

    smfHelper::smfHelper(const smfHelper &o)
    : m_agentFactory(o.m_agentFactory) {
        m_netdevices = o.m_netdevices, m_iidout = o.m_iidout;
    }

    smfHelper*
    smfHelper::Copy(void) const {
        return new smfHelper(*this);
    }
    

    void smfHelper::SetnonMANETNetDeviceID(Ptr<Node> node, uint32_t interface) {
        std::map<Ptr<Node>, std::set<uint32_t>  >::iterator it = m_netdevices.find(node);

        if (it == m_netdevices.end()) {
            std::set<uint32_t> interfaces;
            interfaces.insert(interface);
            
            m_netdevices.insert(std::make_pair(node, std::set<uint32_t> (interfaces)));
        } else{
            it->second.insert(interface);
        }
    }
    
    void smfHelper::SetMANETNetDeviceID(Ptr<Node> node, uint32_t interface){
        std::map<Ptr<Node>, uint32_t  >::iterator it = m_iidout.find(node);
        
        if (it == m_iidout.end()) {
            
            
            m_iidout.insert(std::make_pair(node, uint32_t (interface)));
        } else{
            it->second = interface;
        }
    }

    Ptr<Ipv4RoutingProtocol>
    smfHelper::Create(Ptr<Node> node) const {
        Ptr<smf::RoutingProtocol> agent = m_agentFactory.Create<smf::RoutingProtocol> ();

       
        std::map<Ptr<Node>, std::set<uint32_t> >::const_iterator it2 = m_netdevices.find(node);
         std::map<Ptr<Node>, uint32_t  >::const_iterator it = m_iidout.find(node);

         if (it != m_iidout.end()) {

            agent->SetManetIid(it->second);
        }

        if (it2 != m_netdevices.end()) {

            agent->SetNetdevicelistener(it2->second);
        }
        
       

        node->AggregateObject(agent);
        return agent;
    }

    void
    smfHelper::Set(std::string name, const AttributeValue &value) {
        m_agentFactory.Set(name, value);
    }

}

