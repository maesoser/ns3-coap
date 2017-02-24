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
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/energy-module.h"

#include "ns3/csma-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/ipv6-routing-table-entry.h"
#include "ns3/sixlowpan-module.h"

#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace ns3;

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

/*
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
*/
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

  // Assigned coAP multicat group
  uint32_t verbose = 0;  // switch verbose level
  uint32_t runtime = 3000;
  uint32_t rectangleSize = distance*2*(sqrt(numNodes)-1); // for RandomWalk2dMobilityModel
  uint32_t mDnsOn = 0;
  uint32_t pingopt = 0;
  uint32_t cacheopt = 0;

  // Command parsing
  CommandLine cmd;
  cmd.AddValue("distance", "Distance between nodes (m)", distance);
  cmd.AddValue("speed", "Speed of the nodes (m/s)", speed);
  cmd.AddValue("gain", "Rx Gain (dB)", RxGain);
  cmd.AddValue("txpower","Transmission Power (dB)",txPowerEnd);
  cmd.AddValue("energy","Initial Energy (J))",initialEnergy);
  cmd.AddValue("voltage","Battery voltage (v)",voltage);
  cmd.AddValue("idlecurrent","Current consumed when iddle (A)",idleCurrent);
  cmd.AddValue("txcurrent","Current consumed when transmitting (A)",txCurrent);
  cmd.AddValue("verbose","Verbosity level 0=ENERGY, 1=LINK, 2=WIFI",verbose);
  cmd.AddValue("interval","The time to wait between packets. If it is 0-> Server Mode.",interval);
  cmd.AddValue("maxAge","Delete items after max-Age? If >0 it is the age given to the services",maxAge);
  cmd.AddValue("runtime","number of seconds the simulatiun will last",runtime);
  cmd.AddValue("mdns","1=mDNS 0=coAP discovery",mDnsOn);
  cmd.AddValue("ping","1=ping on 0=cping off",pingopt);
  cmd.AddValue("cache","1=cache on 0=cache off",cacheopt);

  cmd.Parse (argc, argv);



  // This is not properly written, infact when you put 0, it also prints the
  // power info.
LogComponentEnable("smfLog",LOG_LEVEL_ALL);

  switch(verbose){
    case 0:
      LogComponentEnable("simple_log", LOG_LEVEL_ALL);  // POWER
      break;
    case 1:
      LogComponentEnable ("CoapNodeApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("CoapNode_tx", LOG_LEVEL_INFO);
      LogComponentEnable ("CoapNode_tools", LOG_LEVEL_INFO);
      LogComponentEnable ("CoapNode_rx", LOG_LEVEL_INFO);
      LogComponentEnable ("CoapNode_cache", LOG_LEVEL_INFO);
      LogComponentEnable ("Coap_mDNS", LOG_LEVEL_INFO);
      break;
    case 2:
      break;
    default:
      break;
  }
  
	NodeContainer nodes;
	nodes.Create(numNodes);

	NS_LOG_INFO ("Create IPv6 Internet Stack");
	InternetStackHelper internetv6;
	internetv6.Install(nodes);

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
	mobility.Install(nodes);

	NS_LOG_INFO ("Create channels.");
	CsmaHelper csma;
	csma.SetChannelAttribute ("DataRate", DataRateValue (5000000));
	csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
	csma.SetDeviceAttribute ("Mtu", UintegerValue (150));
	NetDeviceContainer devices = csma.Install (nodes);

	SixLowPanHelper sixlowpan;
	sixlowpan.SetDeviceAttribute ("ForceEtherType", BooleanValue (true) );
	NetDeviceContainer sixlw = sixlowpan.Install (devices);

	NS_LOG_INFO ("Create networks and assign IPv6 Addresses.");
	Ipv6AddressHelper ipv6;
	ipv6.SetBase (Ipv6Address ("2001:1::"), Ipv6Prefix (64));
	Ipv6InterfaceContainer ifaces = ipv6.Assign (sixlw);
	ifaces.SetForwarding (1, true);
	ifaces.SetDefaultRouteInAllNodes (1);

	
	CoapNodeHelper coapnode(5683);
	coapnode.SetAttribute ("startDelay",UintegerValue (60));
	coapnode.SetAttribute ("interval", TimeValue (Seconds (interval)));
	coapnode.SetAttribute ("multicastResponse", UintegerValue (1));
	if (cacheopt == 0){
		coapnode.SetAttribute ("cache", UintegerValue (0));
	}
	coapnode.SetAttribute ("useMaxAge",UintegerValue (maxAge));
	coapnode.SetAttribute ("mDNS",UintegerValue(mDnsOn));
	coapnode.SetAttribute ("ping",UintegerValue(pingopt));
	/*
	ApplicationContainer apps;
	apps = coapnode.Install(nodes);
	apps.Start (Seconds (0.0));
	if(runtime>900){
		apps.Stop (Seconds (runtime - 300));
	}else{
		apps.Stop (Seconds (runtime/3));
	}
	*/

/*
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
	*/

	//Config::Connect ("/NodeList/0/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<0>));
	//Config::Connect ("/NodeList/1/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<1>));
	//Config::Connect ("/NodeList/2/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<2>));
	//Config::Connect ("/NodeList/3/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<3>));
	//Config::Connect ("/NodeList/4/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<4>));
	//Config::Connect ("/NodeList/5/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<5>));
	//Config::Connect ("/NodeList/6/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<6>));
	//Config::Connect ("/NodeList/7/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<7>));
	//Config::Connect ("/NodeList/8/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<8>));
	//Config::Connect ("/NodeList/9/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<9>));
	//Config::Connect ("/NodeList/10/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<10>));
	//Config::Connect ("/NodeList/11/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<11>));
	//Config::Connect ("/NodeList/12/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<12>));
	//Config::Connect ("/NodeList/13/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<13>));
	//Config::Connect ("/NodeList/14/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<14>));
	//Config::Connect ("/NodeList/15/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<15>));
	//Config::Connect ("/NodeList/16/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<16>));
	//Config::Connect ("/NodeList/17/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<17>));
	//Config::Connect ("/NodeList/18/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<18>));
	//Config::Connect ("/NodeList/19/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<19>));
	//Config::Connect ("/NodeList/20/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<20>));
	//Config::Connect ("/NodeList/21/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<21>));
	//Config::Connect ("/NodeList/22/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<22>));
	//Config::Connect ("/NodeList/23/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<23>));
	//Config::Connect ("/NodeList/24/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<24>));
	
	///TRACING
	//AsciiTraceHelper ascii;
	//csma.EnableAsciiAll (ascii.CreateFileStream ("coap_trace.tr"));
	//csma.EnablePcap ("coap_pcap", devices);

	//AnimationInterface anim("coap_animation.xml");
	//anim.SetMaxPktsPerTraceFile(3000);
	//anim.EnablePacketMetadata (true);

  Simulator::ScheduleDestroy (showSummary);
	Simulator::Stop (Seconds (runtime));
	Simulator::Run ();
	Simulator::Destroy ();

  return 0;
}
