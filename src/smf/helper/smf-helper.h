/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef SMF_HELPER_H
#define SMF_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/ipv4-routing-helper.h"
#include <map>
#include <set>

namespace ns3 {
    
class smfHelper : public Ipv4RoutingHelper {
    public:
        /**
         * Create an smfHelper that makes life easier for people who want to install
         * smf routing to nodes.
         */
        smfHelper();

        /**
         * \brief Construct an smfHelper from another previously initialized instance
         * (Copy Constructor).
         */
        smfHelper(const smfHelper &);

        /**
         * \returns pointer to clone of this smfHelper 
         * 
         * This method is mainly for internal use by the other helpers;
         * clients are expected to free the dynamic memory allocated by this method
         */
        smfHelper* Copy(void) const;

        /**
         * \param node the node for which an exception is to be defined
         * \param interface an interface of node on which smf is not to be installed
         *
         * This method allows the user to specify an interface on which smf is not to be installed on
         */
        

        void SetnonMANETNetDeviceID(Ptr<Node> node, uint32_t interface);
        void SetMANETNetDeviceID(Ptr<Node> node, uint32_t interface);

        /**
         * \param node the node on which the routing protocol will run
         * \returns a newly-created routing protocol
         *
         * This method will be called by ns3::InternetStackHelper::Install
         */
        virtual Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const;

        /**
         * \param name the name of the attribute to set
         * \param value the value of the attribute to set.
         *
         * This method controls the attributes of ns3::smf::RoutingProtocol
         */
        void Set(std::string name, const AttributeValue &value);

        /**
         * Assign a fixed random variable stream number to the random variables
         * used by this model.  Return the number of streams (possibly zero) that
         * have been assigned.  The Install() method of the InternetStackHelper
         * should have previously been called by the user.
         *
         * \param stream first stream index to use
         * \param c NodeContainer of the set of nodes for which the smfRoutingProtocol
         *          should be modified to use a fixed stream
         * \return the number of stream indices assigned by this helper
         */
      

    private:
        /**
         * \brief Assignment operator declared private and not implemented to disallow
         * assignment and prevent the compiler from happily inserting its own.
         */
        smfHelper &operator=(const smfHelper &);
        ObjectFactory m_agentFactory; //!< Object factory

        
        std::map< Ptr<Node>, std::set<uint32_t> > m_netdevices;
        std::map< Ptr<Node>, uint32_t > m_iidout;
    };

}


#endif /* SMF_HELPER_H */

