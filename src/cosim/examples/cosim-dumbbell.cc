/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 University of Washington, 2012 INRIA
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

#include <iostream>
#include <iomanip>

#include "ns3/abort.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/bridge-net-device.h"
#include "ns3/simple-channel.h"
#include "ns3/simple-net-device.h"
#include "ns3/cosim.h"
#include "ns3/config-store.h"
#include "ns3/simbricks-trace-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CosimDumbbellExample");

std::vector<std::string> cosimLeftPaths;
std::vector<std::string> cosimRightPaths;

bool
AddCosimLeftPort (std::string arg)
{
  cosimLeftPaths.push_back (arg);
  return true;
}

bool
AddCosimRightPort (std::string arg)
{
  cosimRightPaths.push_back (arg);
  return true;
}

void
ReplacementTimePrinter (std::ostream &os)
{
  os << Simulator::Now ().GetPicoSeconds () << "ps";
}

void
ReplaceTimePrinter (void)
{
  LogSetTimePrinter (&ReplacementTimePrinter);
}

int
main (int argc, char *argv[])
{
  Time linkLatency (MilliSeconds (10));
  DataRate linkRate ("10Mb/s");
  double ecnTh = 200000;
  std::string trace_file_path = "";

  CommandLine cmd (__FILE__);
  cmd.AddValue ("LinkLatency", "Propagation delay through link", linkLatency);
  cmd.AddValue ("LinkRate", "Link bandwidth", linkRate);
  cmd.AddValue ("EcnTh", "ECN Threshold queue size", ecnTh);
  cmd.AddValue ("CosimPortLeft", "Add a cosim ethernet port to the bridge",
                MakeCallback (&AddCosimLeftPort));
  cmd.AddValue ("CosimPortRight", "Add a cosim ethernet port to the bridge",
                MakeCallback (&AddCosimRightPort));
  cmd.AddValue ("EnableTracing", "Path to a file into which the trace shall be written",
                trace_file_path);
  cmd.Parse (argc, argv);

  //LogComponentEnable ("SimBricksTraceHelper", LOG_LEVEL_ALL);
  //LogComponentEnable ("CosimNetDevice", LOG_LEVEL_ALL);
  //LogComponentEnable ("BridgeNetDevice", LOG_LEVEL_ALL);
  //LogComponentEnable ("CosimDumbbellExample", LOG_LEVEL_ALL);
  //LogComponentEnable ("SimpleChannel", LOG_LEVEL_ALL);
  //LogComponentEnable ("SimpleNetDevice", LOG_LEVEL_INFO);
  //LogComponentEnable ("RedQueueDisc", LOG_LEVEL_ALL);
  //LogComponentEnable ("DropTailQueue", LOG_LEVEL_ALL);
  //LogComponentEnable ("DevRedQueue", LOG_LEVEL_ALL);
  //LogComponentEnable ("Queue", LOG_LEVEL_ALL);
  //LogComponentEnable ("TrafficControlLayer", LOG_LEVEL_ALL);
  //LogComponentEnable ("Config", LOG_LEVEL_ALL);

  //LogComponentEnableAll (LOG_PREFIX_TIME);
  //LogComponentEnableAll (LOG_PREFIX_NODE);
  //Simulator::Schedule (Seconds (0), &ReplaceTimePrinter);

  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));
  //GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  NS_LOG_INFO ("Create Nodes");
  Ptr<Node> nodeLeft = CreateObject<Node> ();
  Ptr<Node> nodeRight = CreateObject<Node> ();
  NodeContainer nodes (nodeLeft);
  nodes.Add (nodeRight);
  NS_LOG_INFO ("Node Num: " << nodes.GetN ());

  NS_LOG_INFO ("Create BridgeDevice");
  Ptr<BridgeNetDevice> bridgeLeft = CreateObject<BridgeNetDevice> ();
  Ptr<BridgeNetDevice> bridgeRight = CreateObject<BridgeNetDevice> ();
  bridgeLeft->SetAddress (Mac48Address::Allocate ());
  bridgeRight->SetAddress (Mac48Address::Allocate ());
  nodeLeft->AddDevice (bridgeLeft);
  nodeRight->AddDevice (bridgeRight);

  NS_LOG_INFO ("Create simple channel link between the two");
  Ptr<SimpleChannel> ptpChan = CreateObject<SimpleChannel> ();

  SimpleNetDeviceHelper pointToPointSR;
  pointToPointSR.SetQueue ("ns3::DevRedQueue", "MaxSize", StringValue ("2666p"));
  pointToPointSR.SetQueue ("ns3::DevRedQueue", "MinTh", DoubleValue (ecnTh));
  pointToPointSR.SetDeviceAttribute ("DataRate", DataRateValue (linkRate));
  pointToPointSR.SetChannelAttribute ("Delay", TimeValue (linkLatency));

  //ptpChan->SetAttribute ("Delay", TimeValue (linkLatency));

  //Ptr<SimpleNetDevice> ptpDevLeft = CreateObject<SimpleNetDevice> ();
  //Ptr<SimpleNetDevice> ptpDevRight = CreateObject<SimpleNetDevice> ();
  //Ptr<PointToPointNetDevice> ptpDevLeft = CreateObject<PointToPointNetDevice> ();
  //Ptr<PointToPointNetDevice> ptpDevRight = CreateObject<PointToPointNetDevice> ();
  //ptpDevLeft = pointToPointSR.Install (nodeLeft, nodeRight).Get(0);
  //ptpDevRight = pointToPointSR.Install (nodeLeft, nodeRight).Get(1);
  //NetDeviceContainer ptpDev = pointToPointSR.Install (nodeLeft, nodeRight);

  NetDeviceContainer ptpDev = pointToPointSR.Install (nodes, ptpChan);

  //ptpDevLeft->SetAttribute ("DataRate", DataRateValue(linkRate));
  //ptpDevRight->SetAttribute ("DataRate", DataRateValue(linkRate));
  //ptpDevLeft->SetAddress (Mac48Address::Allocate ());
  //ptpDevRight->SetAddress (Mac48Address::Allocate ());
  //ptpChan->Add (ptpDevLeft);
  //ptpChan->Add (ptpDevRight);
  //ptpDevLeft->SetChannel (ptpChan);
  //ptpDevRight->SetChannel (ptpChan);
  //nodeLeft->AddDevice (ptpDevLeft);
  //nodeRight->AddDevice (ptpDevRight);
  //bridgeLeft->AddBridgePort (ptpDevLeft);
  //bridgeRight->AddBridgePort (ptpDevRight);
  NS_LOG_INFO ("num node device" << nodeLeft->GetNDevices () << " type: ");
  bridgeLeft->AddBridgePort (nodeLeft->GetDevice (1));

  bridgeRight->AddBridgePort (nodeRight->GetDevice (1));

  NS_LOG_INFO ("Create CosimDevices and add them to bridge");
  NS_LOG_INFO ("cosim path :" << cosimLeftPaths[0]);

  for (std::string cpp : cosimLeftPaths)
    {
      Ptr<CosimNetDevice> device = CreateObject<CosimNetDevice> ();
      device->SetAttribute ("UnixSocket", StringValue (cpp));
      nodeLeft->AddDevice (device);
      bridgeLeft->AddBridgePort (device);
      device->Start ();
    }
  for (std::string cpp : cosimRightPaths)
    {
      Ptr<CosimNetDevice> device = CreateObject<CosimNetDevice> ();
      device->SetAttribute ("UnixSocket", StringValue (cpp));
      nodeRight->AddDevice (device);
      bridgeRight->AddBridgePort (device);
      device->Start ();
    }

  // Enable SimBricks Tracing
  std::set<std::pair<int, int>> interesting_node_device_pairs{{0, 2}, {1, 2}};
  SimBricksTraceHelper &simBricksTraceHelper = SimBricksTraceHelper::GetTracehelper ();
  if (not trace_file_path.empty ())
    {
      Ptr<OutputStreamWrapper> outStream = simBricksTraceHelper.CreateFileStream (trace_file_path);
      simBricksTraceHelper.EnableAsciiLoggingForNodeContainer (
          outStream, nodes, "/$ns3::NodeListPriv", interesting_node_device_pairs);
    }

  // Print Ns3 Config to a file
  //Config::SetDefault ("ns3::ConfigStore::Filename",
  //                    StringValue ("/local/jakobg/tracing-experiments/wrkdir/config.txt"));
  //Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
  //Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  //ConfigStore configStore;
  //configStore.ConfigureDefaults ();
  //configStore.ConfigureAttributes ();

  NS_LOG_INFO ("Run Emulation.");
  Simulator::Run ();

  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
