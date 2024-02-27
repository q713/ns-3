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
#include "ns3/applications-module.h"
#include "ns3/application.h"
#include "ns3/bulk-send-helper.h"
#include "ns3/bulk-send-application.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/bridge-net-device.h"
#include "ns3/bridge-module.h"
#include "ns3/simple-channel.h"
#include "ns3/simple-net-device.h"
#include "ns3/cosim.h"
#include "ns3/simbricks-trace-helper.h"
#include "ns3/config-store.h"
#include "ns3/jitter-provider.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CosimDumbbellHybridExample");

std::vector<std::string> cosimLeftPaths;
std::vector<std::string> cosimRightPaths;
std::vector<uint64_t> DummyRecvBytes;
std::ofstream DummyRecvTput;

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

int
main (int argc, char *argv[])
{
  Time linkLatency (MilliSeconds (10));
  DataRate linkRate ("10Mb/s");
  double ecnTh = 200000;
  std::string trace_file_path = "";
  int num_ns3_host_pairs = 1;
  uint32_t mtu = 1500;
  bool addJitter = false;

  Time flowStartupWindow = MilliSeconds (500);
  Time convergenceTime = MilliSeconds (500);
  Time measurementWindow = Seconds (8);
  Time progressInterval = MilliSeconds (500);
  Time tputInterval = Seconds (1);
  Time ns3_hosts_rtt (NanoSeconds (50000)); // 50 us

  CommandLine cmd (__FILE__);
  cmd.AddValue ("LinkLatency", "Propagation delay through link", linkLatency);
  cmd.AddValue ("LinkRate", "Link bandwidth", linkRate);
  cmd.AddValue ("EcnTh", "ECN Threshold queue size", ecnTh);
  cmd.AddValue ("Mtu", "Ethernet mtu", mtu);
  cmd.AddValue ("NumNs3HostPairs", "Amount of ns3 hosts used to ehaust bottelneck link",
                num_ns3_host_pairs);
  cmd.AddValue (
      "Rtt",
      "Rtt between dummy ns3 server and client, which is propagation delay multiplied by #links",
      ns3_hosts_rtt);
  cmd.AddValue ("CosimPortLeft", "Add a cosim ethernet port to the bridge",
                MakeCallback (&AddCosimLeftPort));
  cmd.AddValue ("CosimPortRight", "Add a cosim ethernet port to the bridge",
                MakeCallback (&AddCosimRightPort));
  cmd.AddValue ("EnableTracing", "Path to a file into which the trace shall be written",
                trace_file_path);
  cmd.AddValue ("EnableJitter", "bool flag to determine whether jitter shall be applied",
                addJitter);
  cmd.AddFL cmd.Parse (argc, argv);

  NS_ABORT_MSG_IF (cosimLeftPaths.empty (), "must provide at least one cosim left path");
  NS_ABORT_MSG_IF (cosimRightPaths.empty (), "must provide at least one cosim rigth path");
  NS_ABORT_MSG_IF (cosimLeftPaths.size () != cosimRightPaths.size (),
                   "must have same amount of cosim left and right paths");
  auto num_simbricks_host_pairs = cosimLeftPaths.size ();
  auto total_num_host_pairs = num_simbricks_host_pairs + num_ns3_host_pairs;

  //LogComponentEnable("CosimDumbbellHybridExample", LOG_LEVEL_ALL);
  //LogComponentEnable("CosimNetDevice", LOG_LEVEL_ALL);
  //LogComponentEnable("BridgeNetDevice", LOG_LEVEL_ALL);
  //LogComponentEnable("SimpleChannel", LOG_LEVEL_ALL);
  //LogComponentEnable("SimpleNetDevice", LOG_LEVEL_INFO);
  //LogComponentEnable ("RedQueueDisc", LOG_LEVEL_ALL);
  //LogComponentEnable ("DropTailQueue", LOG_LEVEL_ALL);
  //LogComponentEnable ("DevRedQueue", LOG_LEVEL_ALL);
  //LogComponentEnable ("Queue", LOG_LEVEL_ALL);
  //LogComponentEnable ("TrafficControlLayer", LOG_LEVEL_ALL);
  //LogComponentEnableAll(LOG_PREFIX_TIME);
  //LogComponentEnableAll(LOG_PREFIX_NODE);

  // Configurations for ns3 hosts
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpDctcp"));
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (mtu - 52));
  Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (2));
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
  ptpChan->SetAttribute ("Delay", TimeValue (linkLatency));
  if (addJitter)
    {
      Ptr<JitterProvider> jitterProvider = CreateObject<JitterProvider> ();
      // TODO: set random variable stream vor jitter provider to create random jitter
      ptpChan->SetJitterCallback (
          MakeCallback (&JitterProvider::CalculateNextDelay, jitterProvider));
    }

  SimpleNetDeviceHelper pointToPointSR;
  pointToPointSR.SetQueue ("ns3::DevRedQueue", "MaxSize", StringValue ("2666p"));
  pointToPointSR.SetQueue ("ns3::DevRedQueue", "MinTh", DoubleValue (ecnTh));
  pointToPointSR.SetDeviceAttribute ("DataRate", DataRateValue (linkRate));
  pointToPointSR.SetChannelAttribute ("Delay", TimeValue (linkLatency));

  NetDeviceContainer ptpDev = pointToPointSR.Install (nodes, ptpChan);
  NS_LOG_INFO ("num node device" << nodeLeft->GetNDevices () << " type: ");
  bridgeLeft->AddBridgePort (nodeLeft->GetDevice (1));
  bridgeRight->AddBridgePort (nodeRight->GetDevice (1));

  NS_LOG_INFO ("Create detailed hosts and add them to bridge");
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

  // Add dummy ns3 hosts
  NS_LOG_INFO ("Create dummy ns3 hosts and add them to bridge");
  NodeContainer DumLeftNode, DumRightNode;
  DumLeftNode.Create (num_ns3_host_pairs);
  DumRightNode.Create (num_ns3_host_pairs);

  std::vector<NetDeviceContainer> DumLeftDev;
  DumLeftDev.reserve (num_ns3_host_pairs);
  std::vector<NetDeviceContainer> DumRightDev;
  DumRightDev.reserve (num_ns3_host_pairs);

  SimpleNetDeviceHelper pointToPointHost;
  pointToPointHost.SetDeviceAttribute ("DataRate", StringValue ("10Gbps"));
  pointToPointHost.SetChannelAttribute ("Delay", TimeValue (linkLatency));

  Time ns3_host_linkLatency = ns3_hosts_rtt / 4;
  for (int i = 0; i < num_ns3_host_pairs; i++)
    {
      // Add left side
      Ptr<Node> left_host_node = DumLeftNode.Get (i);
      Ptr<SimpleChannel> ptpChanL = CreateObject<SimpleChannel> ();
      //ptpChanL->SetAttribute ("Delay", TimeValue (ns3_host_linkLatency));
      pointToPointHost.Install (left_host_node, ptpChanL);
      // add the netdev to bridge port
      bridgeLeft->AddBridgePort (pointToPointHost.Install (nodeLeft, ptpChanL).Get (0));

      // Add right side
      Ptr<Node> right_host_node = DumRightNode.Get (i);
      Ptr<SimpleChannel> ptpChanR = CreateObject<SimpleChannel> ();
      //ptpChanR->SetAttribute ("Delay", TimeValue (ns3_host_linkLatency));
      pointToPointHost.Install (right_host_node, ptpChanR);
      // add the netdev to bridge port
      bridgeRight->AddBridgePort (pointToPointHost.Install (nodeRight, ptpChanR).Get (0));
    }

  // Network configurations for ns3 hosts
  InternetStackHelper stack;
  stack.Install (DumLeftNode);
  stack.Install (DumRightNode);

  std::vector<Ipv4InterfaceContainer> ipRight;
  ipRight.reserve (num_ns3_host_pairs);

  Ipv4AddressHelper ipv4;
  std::string base_ip = "0.0.0." + std::to_string (num_simbricks_host_pairs * 2 + 1);
  ipv4.SetBase ("192.168.64.0", "255.255.255.0", base_ip.c_str ());
  for (int i = 0; i < num_ns3_host_pairs; i++)
    {
      Ipv4InterfaceContainer le;
      le = ipv4.Assign (DumLeftNode.Get (i)->GetDevice (0));
      NS_LOG_INFO ("Left IP: " << le.GetAddress (0));
    }

  base_ip = "0.0.0." + std::to_string (num_simbricks_host_pairs * 2 + 1 + num_ns3_host_pairs);
  ipv4.SetBase ("192.168.64.0", "255.255.255.0", base_ip.c_str ());
  for (int i = 0; i < num_ns3_host_pairs; i++)
    {
      ipRight.push_back (ipv4.Assign (DumRightNode.Get (i)->GetDevice (0)));
      NS_LOG_INFO ("Right IP: " << ipRight[i].GetAddress (0));
    }

  // Create MyApp to ns3 hosts
  std::vector<Ptr<PacketSink>> RightSinks;
  RightSinks.reserve (num_ns3_host_pairs);

  std::vector<Ptr<Socket>> sockets;
  sockets.reserve (num_ns3_host_pairs);

  for (int i = 0; i < num_ns3_host_pairs; i++)
    {
      uint16_t port = 50000 + i;
      Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
      PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
      ApplicationContainer sinkApp = sinkHelper.Install (DumRightNode.Get (i));
      Ptr<PacketSink> packetSink = sinkApp.Get (0)->GetObject<PacketSink> ();
      RightSinks.push_back (packetSink);
      sinkApp.Start (Seconds (1));

      Address bulkAddress (InetSocketAddress (ipRight[i].GetAddress (0), port));
      BulkSendHelper bulkHelper ("ns3::TcpSocketFactory", bulkAddress);
      ApplicationContainer bulkApp = bulkHelper.Install (DumLeftNode.Get (i));
      bulkApp.Start (Seconds (1));

      Ptr<Socket> ns3TcpSocket =
          Socket::CreateSocket (DumLeftNode.Get (i), TcpSocketFactory::GetTypeId ());
      sockets.push_back (ns3TcpSocket);
    }

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