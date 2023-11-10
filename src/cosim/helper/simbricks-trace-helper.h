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

#ifndef SIMBRICKS_TRACE_HELPER_H
#define SIMBRICKS_TRACE_HELPER_H

#include "ns3/abort.h"
#include "ns3/core-module.h"
#include "ns3/simple-net-device.h"
#include "ns3/cosim.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"

namespace ns3 {

class SimBricksTraceHelper
{
  static void PrintPacketToStream (bool manual_eth, bool manual_ip, Ptr<OutputStreamWrapper> stream,
                                   const std::string &context, Ptr<const Packet> packet,
                                   const std::string &prefix);

  // TODO: add functionality to specify more fine grained which packets shall be
  //       logged and which not --> by port and or ip

public:
  SimBricksTraceHelper(Time::Unit time_unit) {
    //
    // Our default trace sinks are going to use packet printing, so we have to
    // make sure that is turned on.
    //
    Time::SetResolution (time_unit);
    Packet::EnablePrinting ();
  }

  Ptr<OutputStreamWrapper> CreateFileStream (std::string filename,
                                             std::ios::openmode filemode = std::ios::out);

  //
  // Trace sinks that write the actual log message into a filestream
  //

  template <bool Manual_Eth = false, bool Manual_Ip = false>
  static void
  DropSinkWithContext (Ptr<OutputStreamWrapper> stream, std::string context, Ptr<const Packet> p)
  {
    NS_ABORT_MSG_IF (stream == 0, "SimBricksTraceHelper::DropSinkWithContext stream is null");
    NS_ABORT_MSG_IF (p == 0, "SimBricksTraceHelper::DropSinkWithContext packet is null");

    const std::string prefix = "d ";
    SimBricksTraceHelper::PrintPacketToStream (Manual_Eth, Manual_Ip, stream, context, p, prefix);
  }

  template <bool Manual_Eth = false, bool Manual_Ip = false>
  static void
  EnqueueSinkWithContext (Ptr<OutputStreamWrapper> stream, std::string context, Ptr<const Packet> p)
  {
    NS_ABORT_MSG_IF (stream == 0, "SimBricksTraceHelper::EnqueueSinkWithContext stream is null");
    NS_ABORT_MSG_IF (p == 0, "SimBricksTraceHelper::EnqueueSinkWithContext packet is null");

    const std::string prefix = "+ ";
    SimBricksTraceHelper::PrintPacketToStream (Manual_Eth, Manual_Ip, stream, context, p, prefix);
  }

  template <bool Manual_Eth = false, bool Manual_Ip = false>
  static void
  DequeueSinkWithContext (Ptr<OutputStreamWrapper> stream, std::string context, Ptr<const Packet> p)
  {
    NS_ABORT_MSG_IF (stream == 0, "SimBricksTraceHelper::DequeueSinkWithContext stream is null");
    NS_ABORT_MSG_IF (p == 0, "SimBricksTraceHelper::DequeueSinkWithContext packet is null");

    const std::string prefix = "- ";
    SimBricksTraceHelper::PrintPacketToStream (Manual_Eth, Manual_Ip, stream, context, p, prefix);
  }

  //
  // Helper functions to make use of various trace sources within different components
  //

  void EnableAsciiLoggingForSimpleNetDevice (Ptr<OutputStreamWrapper> outStream,
                                             Ptr<SimpleNetDevice> simpleNetDevice,
                                             const std::string path_prefix = "");

  void EnableAsciiLoggingForCosimNetDevice (Ptr<OutputStreamWrapper> outStream,
                                            Ptr<CosimNetDevice> cosimNetDevice,
                                            const std::string path_prefix = "");

  void EnableAsciiLoggingForNetDevice (Ptr<OutputStreamWrapper> outStream, Ptr<NetDevice> netDevice,
                                       const std::string path_prefix = "");

  void EnableAsciiLoggingForNodeContainer (Ptr<OutputStreamWrapper> outStream, NodeContainer &nodes,
                                           const std::string prefix = "");
};

} // namespace ns3

#endif // SIMBRICKS_TRACE_HELPER_H
