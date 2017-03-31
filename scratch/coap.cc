/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 Carlos III Universtiy
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
 *
 */

//
// This program configures a grid (default 25x25) of nodes on an
// 802.11b physical layer, with 802.11b NICs in adhoc mode.
//
// The default layout is like this, on a 2-D grid.
//
// n00  n01  n02  n03  n04
// n05  n06  n07  n08  n09
// n10  n11  n12  n13  n14
// n15  n16  n17  n18  n19
// n20  n21  n22  n23  n24
//
// the layout is affected by the parameters given to GridPositionAllocator;
// by default, GridWidth is 100m and numNodes is 25.
//
// There are a number of command-line options available to control
// the default behavior.  The list of available command-line options
// can be listed with the following command:
// ./waf --run "coap --help"
//
// Note that all ns-3 attributes (not just the ones exposed in the below
// script) can be changed at command line; see the ns-3 documentation.
//
// For instance, for this configuration, the physical layer will
// stop successfully receiving packets when distance increases beyond
// the default of 500m.
// To see this effect, try running:
//
// ./waf --run "coap --distance=500"
// ./waf --run "coap --distance=1000"
// ./waf --run "coap" --distance=1500"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/energy-module.h"

#include "ns3/olsr-helper.h"
#include "ns3/dsr-helper.h"
#include "ns3/dsr-main-helper.h"
#include "ns3/dsdv-helper.h"
#include "ns3/aodv-helper.h"
#include "ns3/smf-helper.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

using namespace ns3;
using namespace dsr;

NS_LOG_COMPONENT_DEFINE ("simple_log");

/* Ipv4Address to String
static std::string Ipv4AddressToString (Ipv4Address ad){
  std::ostringstream oss;
  ad.Print (oss);
  return oss.str ();
}
*/

/// Trace function for remaining energy at node.
void RemainingEnergy (double oldValue, double remainingEnergy){
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "s Remaining energy = " << remainingEnergy << "J");
}

void showSummary(){

}

/// Trace function for total energy consumption at node.
void TotalEnergy (double oldValue, double totalEnergy){
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "s Total energy consumed by radio = " << totalEnergy << "J");
}

void printNodesIp (NodeContainer nodes){
	uint32_t n_nodes = nodes.GetN();
	Ptr<Node> node = nodes.Get(0); // Get pointer to ith node in container
	Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>(); // Get Ipv4 instance of the node
	Ipv4Address addr = ipv4->GetAddress (1,0).GetLocal(); // Get Ipv4InterfaceAddress of xth interface.
	for( uint32_t a = 0; a < n_nodes; a = a + 1 )
	{
		node = nodes.Get(a); // Get pointer to ith node in container
		ipv4 = node->GetObject<Ipv4>(); // Get Ipv4 instance of the node
		addr = ipv4->GetAddress (1,0).GetLocal(); // Get Ipv4InterfaceAddress of xth interface.
		//std::cout << "Prueba";
		NS_LOG_INFO ("\tNODE " << a << ": " << addr);
	}
}

/// NEW ENERGY MODEL
template <int node>
void RemainingEnergyTrace (double oldValue, double newValue)
{
  std::stringstream ss;
  ss << "energy_" << node << ".log";

  static std::fstream f (ss.str().c_str(), std::ios::out);

  f << Simulator::Now().GetSeconds() << "    remaining energy=" << newValue << std::endl;
  //NS_LOG_DEBUG (Simulator::Now().GetSeconds() << " NODE " << node << " remaining energy=" << newValue);

}

template <int node>
void PhyStateTrace (std::string context, Time start, Time duration, enum WifiPhy::State state)
{
  std::stringstream ss;
  ss << "state_" << node << ".log";

  static std::fstream f (ss.str().c_str(), std::ios::out);

  f << Simulator::Now().GetSeconds() << "    state=" << state << " start=" << start << " duration=" << duration << std::endl;
  //NS_LOG_DEBUG (Simulator::Now().GetSeconds() << " NODE " << node << "    state=" << state << " start=" << start << " duration=" << duration);

}

int main (int argc, char *argv[])
{

  double distance = 500;  // meters
  double RxGain   = -10; // dB   -> Antiguo valor era -10
  uint32_t speed = 0;
  double initialEnergy = 5000; // joule
  double finalEnergy = 0;
  double voltage = 5.0; // volts
  // Power based on ESP8266 POwer consumption
  double idleCurrent = 0.015; // Ampere
  double txCurrent = 0.120; // Ampere
  double txPowerStart = 0.0; // dbm
  double txPowerEnd = 13.0; //

  uint32_t numNodes = 25;  // by default, 5x5

  uint32_t interval = 60;
  uint32_t maxAge = 90;

  // Direct sequence spread spectrum
	std::string phyMode ("DsssRate1Mbps");
  // Assigned coAP multicat group
	Ipv4Address multicastGroup ("224.0.1.187");
  Ipv4Address mdnsGroup("224.0.0.251");
  Ipv4Address unicastAddr ("10.1.1.23");
  Ipv4Address localBroadcast ("10.1.1.255");
  uint32_t protocol = 0; // switch routing protocol
  std::string m_protocolName; // Store the name of the protocol
  uint32_t verbose = 0;  // switch verbose level
  uint32_t runtime = 3000;
  uint32_t rectangleSize = distance*2*(sqrt(numNodes)-1); // for RandomWalk2dMobilityModel
  uint32_t mDnsOn = 0;
  uint32_t pingopt = 0;
  uint32_t cacheopt = 0;
  uint32_t mcastopt = 0;
  uint32_t cacheinterval = 30;
  uint32_t etagopt = 0;
  uint32_t stimeopt = 0;
  // Routing Helpers
  smfHelper smf;
  AodvHelper aodv;
  OlsrHelper olsr;
  DsdvHelper dsdv;
  DsrHelper dsr;
  DsrMainHelper dsrMain;

  // Command parsing
  CommandLine cmd;
  cmd.AddValue("distance", "Distance between nodes (m)", distance);
  cmd.AddValue("speed", "Speed of the nodes (m/s)", speed);
  
  // Energy related parameters
  cmd.AddValue("gain", "Rx Gain (dB)", RxGain);
  cmd.AddValue("txpower","Transmission Power (dB)",txPowerEnd);
  cmd.AddValue("energy","Initial Energy (J))",initialEnergy);
  cmd.AddValue("voltage","Battery voltage (v)",voltage);
  cmd.AddValue("idlecurrent","Current consumed when iddle (A)",idleCurrent);
  cmd.AddValue("txcurrent","Current consumed when transmitting (A)",txCurrent);
  
  cmd.AddValue("routing","Current routing protocol 0=NONE,1=SMF,2=OLSR_SMF,3=OLSR,4=AODV_SMF 5=DSDV_SMF",protocol);
  cmd.AddValue("mdns","1=mDNS 0=coAP discovery",mDnsOn);
  cmd.AddValue("ping","1=ping on 0=ping off",pingopt);
  cmd.AddValue("cache","1=cache on 0=cache off",cacheopt);
  cmd.AddValue("stime","1=smart answer time on",stimeopt);
  cmd.AddValue("mcast","1=mcast answers 0=ucast answers",mcastopt);
  cmd.AddValue("cacheinterval","Determines the cache show up time",cacheinterval);
  
  cmd.AddValue("verbose","Verbosity level 0=script+SMF, 1=coap",verbose);
  
  cmd.AddValue("interval","The time to wait between requests. If it is 0-> Server Mode.",interval);
  cmd.AddValue("maxAge","Delete items after max-Age? If >0 it is the defaultage given to the services",maxAge);
  cmd.AddValue("runtime","number of seconds the simulation will last",runtime);
  
  cmd.Parse (argc, argv);


  // This is not properly written, infact when you put 0, it also prints the
  // power info.
  switch(verbose){
    case 0:
      LogComponentEnable("simple_log", LOG_LEVEL_ALL);  // POWER
      LogComponentEnable("smfLog",LOG_LEVEL_ALL);
      break;
    case 1:
      LogComponentEnable ("CoapNodeApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("CoapNode_tx", LOG_LEVEL_INFO);
      LogComponentEnable ("CoapNode_tools", LOG_LEVEL_INFO);
      LogComponentEnable ("CoapNode_rx", LOG_LEVEL_INFO);
      LogComponentEnable ("CoapNode_cache", LOG_LEVEL_INFO);
      LogComponentEnable ("Coap_mDNS", LOG_LEVEL_INFO);
      break;
    default:
      break;
  }
	// disable fragmentation for frames below 2200 bytes
	Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
	// turn off RTS/CTS for frames below 2200 bytes
	Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
	// Fix non-unicast data rate to be the same as that of unicast
	Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",StringValue (phyMode));

	NodeContainer nodes;
	nodes.Create (numNodes);

  /// PHYSICAL STACK  ----------------------------------------------------------

	// The below set of helpers will help us to put together the wifi NICs we want
	WifiHelper wifi;

	if(verbose==2){
    wifi.EnableLogComponents ();  // Turn on all Wifi logging
  }
  // More info about Yans Model here: http://cutebugs.net/files/wns2-yans.pdf
	YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
	// set it to zero; otherwise, gain will be added
	wifiPhy.Set ("RxGain", DoubleValue(RxGain));
	// ns-3 supports RadioTap and Prism tracing extensions for 802.11b
	wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

	YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  // More info about Friis implementation here: https://www.nsnam.org/doxygen/classns3_1_1_friis_propagation_loss_model.html#details
	wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
	wifiPhy.SetChannel (wifiChannel.Create ());

	// Add an upper mac and disable rate control
	WifiMacHelper wifiMac;
	wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
								"DataMode",StringValue (phyMode),
								"ControlMode",StringValue (phyMode));
	// Set it to adhoc mode
	wifiMac.SetType ("ns3::AdhocWifiMac");
	NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, nodes);

  /// POSITION AND MOBILITY  ---------------------------------------------------

	MobilityHelper mobility;
	mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
								 "MinX", DoubleValue (0.0),
								 "MinY", DoubleValue (0.0),
								 "DeltaX", DoubleValue (distance),
								 "DeltaY", DoubleValue (distance),
								 "GridWidth", UintegerValue (sqrt(numNodes)),
								 "LayoutType", StringValue ("RowFirst"));

  if (speed == 0){
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  }else{
    //mobility.SetMobilityModel("ns3::RandomDirection2dMobilityModel");
    //mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel","Bounds", RectangleValue (Rectangle (-30000, 30000, -30000, 30000)), "Speed", StringValue("ns3::ConstantRandomVariable[Constant=300.0]"));
    rectangleSize = 90000;
    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                  "Bounds", RectangleValue (Rectangle (-9000, rectangleSize, -9000, rectangleSize)),
                  "Speed", StringValue("ns3::ConstantRandomVariable[Constant="+ std::to_string(speed) +"]"));

  }
	mobility.Install (nodes);

  /// INTERNET STACK / ROUTING STACK  ------------------------------------------
  Ipv4ListRoutingHelper routingList;
  Ipv4StaticRoutingHelper staticRouting;

  // stream to a file to get the route tables
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>("routetables", std::ios::out);

	NS_LOG_INFO("N_NODES   " << nodes.GetN());	// NodeContainer
	NS_LOG_INFO("N_DEVICES " << devices.GetN()); //NetDeviceContainer

  // 0=NONE,1=OLSR,2=AODV,3=DSDV,4=DSR 5=SMF
  routingList.Add(staticRouting,0);
  NS_LOG_INFO ("\tROUTING PROTOCOL SELECTED " << protocol);
  switch (protocol)
  {
    case 0:
      m_protocolName = "NONE";
      break;
    case 1:
      routingList.Add(smf,10);
      smf.PrintRoutingTableAllEvery(Seconds(5),routingStream);
      m_protocolName = "SMF";
      break;
    case 2:
      routingList.Add(smf,10);
      routingList.Add (olsr, 5);
      olsr.PrintRoutingTableAllEvery(Seconds(5),routingStream);
      m_protocolName = "OLSR_SMF";
      break;
    case 3:
      routingList.Add (olsr, 10);
      olsr.PrintRoutingTableAllEvery(Seconds(5),routingStream);
      m_protocolName = "OLSR";
      break;
    case 4:
      routingList.Add(smf,10);
      routingList.Add (aodv, 5);
      aodv.PrintRoutingTableAllEvery(Seconds(5),routingStream);
      m_protocolName = "AODV_SMF";
      break;
    case 5:
      routingList.Add(smf,10);
      routingList.Add (dsdv, 5);
      dsdv.PrintRoutingTableAllEvery(Seconds(5),routingStream);
      m_protocolName = "DSDV_SMF";
      break;
    case 6:
      m_protocolName = "DSR";
      dsrMain.Install (dsr, nodes);
      break;
    default:
      NS_FATAL_ERROR ("No such protocol:" << protocol);
  }
  NS_LOG_INFO ("\tROUTING PROTOCOL SELECTED " << m_protocolName);

  InternetStackHelper internet;
  if(protocol!=4) {
    NS_LOG_INFO ("\tSETTING ROUTING LIST... " << m_protocolName);
    internet.SetRoutingHelper(routingList);
  }
  internet.Install (nodes); // Install InternetStackHelper

  NS_LOG_INFO ("Assign IP Addresses.");
	Ipv4AddressHelper ipv4;
	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	//ipv4.Assign(devices);
	Ipv4InterfaceContainer ifaces = ipv4.Assign(devices);
	printNodesIp(nodes);

  /// MULTICAST
  // http://stackoverflow.com/questions/15795069/how-do-i-implement-multicast-dynamic-join-prune-using-ns3


  for( uint32_t a = 0; a < nodes.GetN(); a = a + 1 )
	{
		//NS_LOG_INFO ("\tREGISTERING MULTICAST NODE " << a);
		staticRouting.SetDefaultMulticastRoute(nodes.Get(a),devices.Get(a));
	}

/*
  for( uint32_t a = 0; a < nodes.GetN(); a = a + 1 )
  {
    Ptr<Node> anode = nodes.Get(a); // Get pointer to ith node in container
    Ptr<Ipv4> aipv4 = anode->GetObject<Ipv4>(); // Get Ipv4 instance of the node
    Ipv4Address mcastIpSource = aipv4->GetAddress (1,0).GetLocal(); // Get Ipv4InterfaceAddress of xth interface.
    NS_LOG_INFO ("\tNODE " << a << ": " << mcastIpSource);
    for( uint32_t b = 0; b < nodes.GetN(); b = b + 1 )
    {
      if(a!=b){
        Ptr<Node> mcastRouter = nodes.Get (b);  // The node in question
        Ptr<NetDevice> inputdev = devices.Get (b);  // The input NetDevice
        staticRouting.AddMulticastRoute (mcastRouter, mcastIpSource, mdnsGroup, inputdev, inputdev);
        NS_LOG_INFO ("\t\t--> " << b );

      }
    }
  }*/

  ApplicationContainer apps;

  CoapNodeHelper coapnode(5683);
  coapnode.SetAttribute ("startDelay",UintegerValue (60));
  
  // Interval between discoveries
  coapnode.SetAttribute ("interval", TimeValue (Seconds (interval)));
  coapnode.SetAttribute ("mcast", UintegerValue (mcastopt));
  if (cacheopt == 0)	coapnode.SetAttribute ("cache", UintegerValue (0));
  coapnode.SetAttribute ("useMaxAge",UintegerValue (maxAge));   // default maxAge
  coapnode.SetAttribute ("mDNS",UintegerValue(mDnsOn));
  coapnode.SetAttribute ("ping",UintegerValue(pingopt));	  // Tha's useful if you want to verify pings
  coapnode.SetAttribute ("etag",UintegerValue(etagopt));	  
  coapnode.SetAttribute ("stime",UintegerValue(stimeopt));	 
  coapnode.SetAttribute ("cacheInterval",UintegerValue(cacheinterval));	  // Tha's useful if you want to verify pings

  apps = coapnode.Install(nodes);	
  apps.Start (Seconds (0.0));
  
  NS_LOG_INFO ("SIM TIME: " << runtime);

  if(runtime>900){
    apps.Stop (Seconds (runtime - 300));
    NS_LOG_INFO ("APPTIME: " << runtime -300);
  }else{
    apps.Stop (Seconds ((2*runtime)/3));
    NS_LOG_INFO ("APPTIME: " << (2*runtime)/3);

  }
  

  /*
  CoapServerHelper coapserver (5683);
  coapserver.SetAttribute ("multicastResponse", UintegerValue (1));
  apps = coapserver.Install (nodes);
  apps.Start(Seconds(0.1));
  apps.Stop(Seconds(64));
	*/
  // T = 1 from node1, discovery 4 times each 5 seconds
/*
  uint32_t nPackets = 1;
  Time interPacketInterval = Seconds (15);
  uint8_t packetType = COAP_DISCOVERY_TYPE;
  CoapClientHelper coapclient2(multicastGroup,5683);
  coapclient2.SetAttribute ("nPackets", UintegerValue (nPackets));
  coapclient2.SetAttribute ("Interval", TimeValue (interPacketInterval));
  coapclient2.SetAttribute ("packetType", UintegerValue (packetType));
  apps = coapclient2.Install (nodes.Get (12));
  apps.Start (Seconds (3.0));
  apps.Stop (Seconds (10.0));*/

  // T = 25 from node23, temperature 2 times each 2 seconds


  // T = 50 from node13, discovery 3 times each 5 seconds
  /*
  nPackets = 3;
  interPacketInterval = Seconds (3);
  packetType = COAP_DISCOVERY_TYPE;
  CoapClientHelper coapclient3(multicastGroup,5683);
  coapclient3.SetAttribute ("MaxPackets", UintegerValue (nPackets));
  coapclient3.SetAttribute ("Interval", TimeValue (interPacketInterval));
  coapclient3.SetAttribute ("packetType", UintegerValue (packetType));
  apps = coapclient3.Install (nodes.Get (13));
  apps.Start (Seconds (50.0));
  apps.Stop (Seconds (30.0));*/


  // ENERGY INFO
	WifiRadioEnergyModelHelper radioEnergyHelper;
	radioEnergyHelper.Set ("IdleCurrentA", DoubleValue (idleCurrent));
	radioEnergyHelper.Set ("TxCurrentA", DoubleValue (txCurrent));

	// compute the efficiency of the power amplifier (eta) assuming that the provided value for tx current
	// corresponds to the minimum tx power level
	double eta = WifiTxCurrentModel::DbmToW (txPowerStart) / ((txCurrent - idleCurrent) * voltage);

	radioEnergyHelper.SetTxCurrentModel ("ns3::LinearWifiTxCurrentModel",
									   "Voltage", DoubleValue (voltage),
									   "IdleCurrent", DoubleValue (idleCurrent),
									   "Eta", DoubleValue (eta));

	BasicEnergySourceHelper basicSourceHelper;
	basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (initialEnergy));
	basicSourceHelper.Set ("BasicEnergySupplyVoltageV", DoubleValue (voltage));

	// install an energy source on each node
	EnergySourceContainer eSources;
	for (NodeContainer::Iterator n = nodes.Begin(); n != nodes.End(); n++){
		eSources.Add (basicSourceHelper.Install (*n));
		Ptr<WifiNetDevice> wnd;
		for (uint32_t i = 0; i < (*n)->GetNDevices (); ++i){
			wnd = (*n)->GetDevice (i)->GetObject<WifiNetDevice> ();
			// if it is a WifiNetDevice
			if (wnd != 0){
				// this device draws power from the last created energy source
				radioEnergyHelper.Install (wnd, eSources.Get(eSources.GetN()-1));
			}
		}
	}

  eSources.Get(0)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<0>));
	eSources.Get(1)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<1>));
	eSources.Get(2)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<2>));
	eSources.Get(3)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<3>));
	eSources.Get(4)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<4>));
	eSources.Get(5)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<5>));
	eSources.Get(6)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<6>));
	eSources.Get(7)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<7>));
	eSources.Get(8)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<8>));
	eSources.Get(9)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<9>));
	eSources.Get(10)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<10>));
	eSources.Get(11)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<11>));
	eSources.Get(12)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<12>));
	eSources.Get(13)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<13>));
	eSources.Get(14)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<14>));
	eSources.Get(15)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<15>));
	eSources.Get(16)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<16>));
	eSources.Get(17)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<17>));
	eSources.Get(18)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<18>));
	eSources.Get(19)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<19>));
	eSources.Get(20)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<20>));
	eSources.Get(21)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<21>));
	eSources.Get(22)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<22>));
	eSources.Get(23)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<23>));
	eSources.Get(24)->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback(&RemainingEnergyTrace<24>));


	Config::Connect ("/NodeList/0/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<0>));
	Config::Connect ("/NodeList/1/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<1>));
	Config::Connect ("/NodeList/2/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<2>));
	Config::Connect ("/NodeList/3/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<3>));
	Config::Connect ("/NodeList/4/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<4>));
	Config::Connect ("/NodeList/5/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<5>));
	Config::Connect ("/NodeList/6/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<6>));
	Config::Connect ("/NodeList/7/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<7>));
	Config::Connect ("/NodeList/8/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<8>));
	Config::Connect ("/NodeList/9/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<9>));
	Config::Connect ("/NodeList/10/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<10>));
	Config::Connect ("/NodeList/11/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<11>));
	Config::Connect ("/NodeList/12/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<12>));
	Config::Connect ("/NodeList/13/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<13>));
	Config::Connect ("/NodeList/14/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<14>));
	Config::Connect ("/NodeList/15/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<15>));
	Config::Connect ("/NodeList/16/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<16>));
	Config::Connect ("/NodeList/17/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<17>));
	Config::Connect ("/NodeList/18/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<18>));
	Config::Connect ("/NodeList/19/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<19>));
	Config::Connect ("/NodeList/20/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<20>));
	Config::Connect ("/NodeList/21/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<21>));
	Config::Connect ("/NodeList/22/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<22>));
	Config::Connect ("/NodeList/23/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<23>));
	Config::Connect ("/NodeList/24/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<24>));

	///TRACING
	AsciiTraceHelper ascii;
	wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("coap_trace.tr"));
	wifiPhy.EnablePcap ("coap_pcap", devices);

	AnimationInterface anim("coap_animation.xml");
	anim.SetMaxPktsPerTraceFile(3000);
	anim.EnablePacketMetadata (true);

  Simulator::ScheduleDestroy (showSummary);
	Simulator::Stop (Seconds (runtime));
	Simulator::Run ();
	Simulator::Destroy ();

  return 0;
}
