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

#include "ns3/simbricks-trace-helper.h"

#include "ns3/ipv4-header.h"
#include "ns3/ethernet-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SimBricksTraceHelper");

Ptr<OutputStreamWrapper>
SimBricksTraceHelper::CreateFileStream (std::string filename, std::ios::openmode filemode)
{
  NS_LOG_FUNCTION (filename << filemode);
  Ptr<OutputStreamWrapper> StreamWrapper = Create<OutputStreamWrapper> (filename, filemode);
  NS_ABORT_MSG_IF (StreamWrapper == 0,
                   "SimBricksTraceHelper::CreateFileStream StreamWrapoper is null");
  return StreamWrapper;
}

void
SimBricksTraceHelper::PrintPacketToStream (bool manual_eth, bool manual_ip,
                                           Ptr<OutputStreamWrapper> stream,
                                           const std::string &context, Ptr<const Packet> p,
                                           const std::string &prefix)
{
  NS_ABORT_MSG_IF (stream == 0, "SimBricksTraceHelper::PrintPacketToStreamManual stream is null");
  NS_ABORT_MSG_IF (p == 0, "SimBricksTraceHelper::PrintPacketToStreamManual packet is null");

  NS_LOG_FUNCTION (stream << *p << context << prefix);

  std::ostream &out = *(stream->GetStream ());
  out << prefix;
  out << " " << Simulator::Now ().GetPicoSeconds ();
  out << " " << context;

  if (manual_eth)
    {
      EthernetHeader eth_header;
      if (p->PeekHeader (eth_header))
        {
          out << " ns3::EthernetHeader(";
          eth_header.Print (out);
          out << ")";
        }
    }
  if (manual_ip)
    {
      Ipv4Header ip_header;
      if (p->PeekHeader (ip_header))
        {
          out << " ns3::Ipv4Header(";
          ip_header.Print (out);
          out << ")";
        }
    }

  out << " " << *p;
  out << std::endl;
}

void
SimBricksTraceHelper::EnableAsciiLoggingForSimpleNetDevice (Ptr<OutputStreamWrapper> outStream,
                                                            Ptr<SimpleNetDevice> simpleNetDevice,
                                                            const std::string path_prefix)
{
  NS_ABORT_MSG_IF (outStream == 0,
                   "SimBricksTraceHelper::EnableAsciiLoggingForSimpleNetDevice: outStream is null");
  NS_ABORT_MSG_IF (simpleNetDevice == 0,
                   "SimBricksTraceHelper::EnableAsciiLoggingForSimpleNetDevice: netDevice is null");

  std::ostringstream config_path_prefix;
  config_path_prefix << path_prefix << "/$ns3::SimpleNetDevice";

  //Ptr<Queue<Packet>> queue = simpleNetDevice->GetObject<Queue<Packet>>();
  //queue->

  std::ostringstream config_path;
  config_path << config_path_prefix.str () << "/PhyRxDrop";
  Config::Connect (config_path.str (),
                   MakeBoundCallback (&SimBricksTraceHelper::DropSinkWithContext, outStream));

  config_path.str ("");
  config_path << config_path_prefix.str () << "/RxPacketFromNetwork";
  Config::Connect (config_path.str (),
                   MakeBoundCallback (&SimBricksTraceHelper::EnqueueSinkWithContext<true, false>, outStream));

  config_path.str ("");
  config_path << config_path_prefix.str () << "/TxPacketToAttachedDevice";
  Config::Connect (config_path.str (),
                   MakeBoundCallback (&SimBricksTraceHelper::DequeueSinkWithContext<true, false>, outStream));

  config_path_prefix << "/TxQueue";
  config_path.str ("");
  config_path << config_path_prefix.str () << "/Enqueue";
  Config::Connect (config_path.str (),
                   MakeBoundCallback (&SimBricksTraceHelper::EnqueueSinkWithContext<true, false>, outStream));

  config_path.str ("");
  config_path << config_path_prefix.str () << "/Dequeue";
  Config::Connect (config_path.str (),
                   MakeBoundCallback (&SimBricksTraceHelper::DequeueSinkWithContext<true, false>, outStream));

  config_path.str ("");
  config_path << config_path_prefix.str () << "/Drop";
  Config::Connect (config_path.str (),
                   MakeBoundCallback (&SimBricksTraceHelper::DropSinkWithContext, outStream));

  config_path.str ("");
  config_path << config_path_prefix.str () << "/DropBeforeEnqueue";
  Config::Connect (config_path.str (),
                   MakeBoundCallback (&SimBricksTraceHelper::DropSinkWithContext, outStream));

  config_path.str ("");
  config_path << config_path_prefix.str () << "/DropAfterDequeue";
  Config::Connect (config_path.str (),
                   MakeBoundCallback (&SimBricksTraceHelper::DropSinkWithContext, outStream));
}

void
SimBricksTraceHelper::EnableAsciiLoggingForCosimNetDevice (Ptr<OutputStreamWrapper> outStream,
                                                           Ptr<CosimNetDevice> cosimNetDevice,
                                                           const std::string path_prefix)
{
  NS_ABORT_MSG_IF (outStream == 0,
                   "SimBricksTraceHelper::EnableAsciiLoggingForCosimNetDevice: outStream is null");
  NS_ABORT_MSG_IF (cosimNetDevice == 0,
                   "SimBricksTraceHelper::EnableAsciiLoggingForCosimNetDevice: netDevice is null");

  std::ostringstream config_path_prefix;
  config_path_prefix << path_prefix << "/$ns3::CosimNetDevice";

  std::ostringstream config_path;
  config_path << config_path_prefix.str () << "/RxPacketFromAdapter";
  Config::Connect (
      config_path.str (),
      MakeBoundCallback (&SimBricksTraceHelper::EnqueueSinkWithContext<true, true>, outStream));

  config_path.str ("");
  config_path << config_path_prefix.str () << "/TxPacketToNetwork";
  Config::Connect (
      config_path.str (),
      MakeBoundCallback (&SimBricksTraceHelper::DequeueSinkWithContext<true, true>, outStream));

  config_path.str ("");
  config_path << config_path_prefix.str () << "/RxPacketFromNetwork";
  Config::Connect (
      config_path.str (),
      MakeBoundCallback (&SimBricksTraceHelper::EnqueueSinkWithContext<true, false>, outStream));

  config_path.str ("");
  config_path << config_path_prefix.str () << "/TxPacketToAdapter";
  Config::Connect (
      config_path.str (),
      MakeBoundCallback (&SimBricksTraceHelper::DequeueSinkWithContext<false, false>, outStream));

  config_path.str ("");
  config_path << config_path_prefix.str () << "/DropPacket";
  Config::Connect (
      config_path.str (),
      MakeBoundCallback (&SimBricksTraceHelper::DropSinkWithContext<false, false>, outStream));
}

void
SimBricksTraceHelper::EnableAsciiLoggingForNetDevice (Ptr<OutputStreamWrapper> outStream,
                                                      Ptr<NetDevice> netDevice,
                                                      const std::string path_prefix)
{
  NS_ABORT_MSG_IF (outStream == 0,
                   "SimBricksTraceHelper::EnableAsciiLoggingForNetDevice: outStream is null");
  NS_ABORT_MSG_IF (netDevice == 0,
                   "SimBricksTraceHelper::EnableAsciiLoggingForNetDevice: netDevice is null");

  { // handel simple net device
    Ptr<SimpleNetDevice> simpleNetDevice = netDevice->GetObject<SimpleNetDevice> ();
    if (simpleNetDevice != 0)
      {
        EnableAsciiLoggingForSimpleNetDevice (outStream, simpleNetDevice, path_prefix);
        return;
      }
  }

  { // handel cosim net device
    Ptr<CosimNetDevice> cosimNetDevice = netDevice->GetObject<CosimNetDevice> ();
    if (cosimNetDevice != 0)
      {
        EnableAsciiLoggingForCosimNetDevice (outStream, cosimNetDevice, path_prefix);
        return;
      }
  }

  // In case no handler is defined log info and do nothing
  NS_LOG_INFO ("EnableAsciiLoggingForSimpleNetDevice: could not downcast device to a device for "
               "which ascii logging can be set");
  return;
}

void
SimBricksTraceHelper::EnableAsciiLoggingForNodeContainer (Ptr<OutputStreamWrapper> outStream,
                                                          NodeContainer &nodes,
                                                          const std::string prefix)
{
  NS_ABORT_MSG_IF (outStream == 0,
                   "SimBricksTraceHelper::EnableAsciiLoggingForNodeContainer: outStream is null");

  for (auto node_it = nodes.Begin (); node_it != nodes.End (); node_it++)
    {
      Ptr<Node> node = *node_it;
      NS_ABORT_MSG_IF (node == 0, "EnableAsciiLoggingForNodeContainer: node is null");
      uint32_t nodeid = node->GetId ();

      for (uint32_t index = 0; index < node->GetNDevices (); index++)
        {
          Ptr<NetDevice> netDevice = node->GetDevice (index);
          uint32_t device_id = index;

          std::stringstream prefix_path;
          prefix_path << prefix << "/NodeList/" << nodeid << "/$ns3::Node/DeviceList/" << device_id;

          EnableAsciiLoggingForNetDevice (outStream, netDevice, prefix_path.str ());
        }
    }
}

} // namespace ns3
