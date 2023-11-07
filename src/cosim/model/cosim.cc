/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 * Copyright 2020 Max Planck Institute for Software Systems
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "cosim.h"

#include "ns3/node.h"
#include "ns3/channel.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/integer.h"
#include "ns3/string.h"
#include "ns3/ethernet-header.h"
#include "ns3/simulator.h"
#include "ns3/tag.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CosimNetDevice");

/**
 * \brief Get the type ID.
 * \return the object TypeId
 */
TypeId
CosimNetDevice::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::CosimNetDevice")
          .SetParent<NetDevice> ()
          .SetGroupName ("CosimNetDevice")
          .AddConstructor<CosimNetDevice> ()
          .AddAttribute (
              "UnixSocket", "The path to the Ethernet Unix socket", StringValue ("/tmp/cosim-eth"),
              MakeStringAccessor (&CosimNetDevice::m_a_uxSocketPath), MakeStringChecker ())
          .AddAttribute ("SyncDelay", "Max delay between outgoing messages before sync is sent",
                         TimeValue (NanoSeconds (500.)),
                         MakeTimeAccessor (&CosimNetDevice::m_a_syncDelay), MakeTimeChecker ())
          .AddAttribute ("PollDelay", "Delay between polling for messages in non-sync mode",
                         TimeValue (NanoSeconds (100.)),
                         MakeTimeAccessor (&CosimNetDevice::m_a_pollDelay), MakeTimeChecker ())
          .AddAttribute ("EthLatency", "Max delay between outgoing messages before sync is sent",
                         TimeValue (NanoSeconds (500.)),
                         MakeTimeAccessor (&CosimNetDevice::m_a_ethLatency), MakeTimeChecker ())
          .AddAttribute ("Sync", "Request synchronous interaction with device", IntegerValue (1),
                         MakeIntegerAccessor (&CosimNetDevice::m_a_sync),
                         MakeIntegerChecker<int32_t> ())
          .AddTraceSource ("RxPacketFromAdapter",
                           "Trace source indicating a packet has been "
                           "received from a simulator adapter within a CosimNetDevice",
                           MakeTraceSourceAccessor (&CosimNetDevice::m_rxPacketFromAdapterTrace),
                           "ns3::Packet::TracedCallback")
          .AddTraceSource (
              "TxPacketToNetwork",
              "Trace source indicating a packet has been "
              "send to the actual network a device is attached to within a CosimNetDevice",
              MakeTraceSourceAccessor (&CosimNetDevice::m_txPacketToNetworkTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource ("RxPacketFromNetwork",
                           "Trace source indicating a packet has been "
                           "received from the network within a CosimNetDevice",
                           MakeTraceSourceAccessor (&CosimNetDevice::m_rxPacketFromNetworkTrace),
                           "ns3::Packet::TracedCallback")
          .AddTraceSource ("TxPacketToAdapter",
                           "Trace source indicating a packet has been "
                           "send to the device attached to the simulation within a CosimNetDevice",
                           MakeTraceSourceAccessor (&CosimNetDevice::m_txPacketToAdapterTrace),
                           "ns3::Packet::TracedCallback")
          .AddTraceSource ("DropPacket",
                           "Trace source indicating a packet has been "
                           "dropped by a CosimNetDevice",
                           MakeTraceSourceAccessor (&CosimNetDevice::m_dropPacketTrace),
                           "ns3::Packet::TracedCallback");

  return tid;
}

CosimNetDevice::CosimNetDevice ()
    : m_mtu (1500), m_node (0), m_rxCallback (0), m_promiscRxCallback (0)
{
  NS_LOG_FUNCTION_NOARGS ();
}

CosimNetDevice::~CosimNetDevice ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
CosimNetDevice::Start ()
{
  SimbricksNetIfDefaultParams (&m_adapter.m_bifparam);
  m_adapter.m_bifparam.sock_path = m_a_uxSocketPath.c_str ();

  m_adapter.m_bifparam.sync_interval = m_a_syncDelay.ToInteger (Time::PS);
  m_adapter.m_pollDelay = m_a_pollDelay;
  m_adapter.m_bifparam.link_latency = m_a_ethLatency.ToInteger (Time::PS);
  m_adapter.m_bifparam.sync_mode = (enum SimbricksBaseIfSyncMode) m_a_sync;
  m_adapter.SetReceiveCallback (MakeCallback (&CosimNetDevice::AdapterRx, this));
  m_adapter.Start ();
}

void
CosimNetDevice::Stop ()
{
  m_adapter.Stop ();
}

void
CosimNetDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION (index);
  m_ifIndex = index;
}

uint32_t
CosimNetDevice::GetIfIndex (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_ifIndex;
}

Ptr<Channel>
CosimNetDevice::GetChannel (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return 0;
}

void
CosimNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION (address);
  m_address = Mac48Address::ConvertFrom (address);
}

Address
CosimNetDevice::GetAddress (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_address;
}

bool
CosimNetDevice::SetMtu (const uint16_t mtu)
{
  NS_LOG_FUNCTION (mtu);
  m_mtu = mtu;
  return true;
}

uint16_t
CosimNetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_mtu;
}

bool
CosimNetDevice::IsLinkUp (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}

void
CosimNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  NS_LOG_FUNCTION_NOARGS ();
}

bool
CosimNetDevice::IsBroadcast (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}

Address
CosimNetDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return Mac48Address ("ff:ff:ff:ff:ff:ff");
}

bool
CosimNetDevice::IsMulticast (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}

Address
CosimNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION (this << multicastGroup);
  return Mac48Address::GetMulticast (multicastGroup);
}

Address
CosimNetDevice::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION (this << addr);
  return Mac48Address::GetMulticast (addr);
}

bool
CosimNetDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return false;
}

bool
CosimNetDevice::IsPointToPoint (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return false;
}

bool
CosimNetDevice::Send (Ptr<Packet> packet, const Address &dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (packet << dest << protocolNumber);
  return SendFrom (packet, m_address, dest, protocolNumber);
}

bool
CosimNetDevice::SendFrom (Ptr<Packet> packet, const Address &source, const Address &dest,
                          uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (*packet << source << dest << protocolNumber);

  m_rxPacketFromNetworkTrace (packet);

  EthernetHeader header (false);
  header.SetSource (Mac48Address::ConvertFrom (source));
  header.SetDestination (Mac48Address::ConvertFrom (dest));
  header.SetLengthType (protocolNumber);

  packet->AddHeader (header);

  m_txPacketToAdapterTrace (packet);

  NS_LOG_FUNCTION (*packet);

  m_adapter.Transmit (packet);
  return true;
}

Ptr<Node>
CosimNetDevice::GetNode (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_node;
}

void
CosimNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (node);
  m_node = node;
}

bool
CosimNetDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}

void
CosimNetDevice::SetReceiveCallback (ReceiveCallback cb)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_rxCallback = cb;
}

void
CosimNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_promiscRxCallback = cb;
}

bool
CosimNetDevice::SupportsSendFrom (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}

void
CosimNetDevice::AdapterRx (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION_NOARGS ();

  uint32_t nid = m_node->GetId ();
  Simulator::ScheduleWithContext (nid, Seconds (0.0),
                                  MakeEvent (&CosimNetDevice::RxInContext, this, packet));
}

void
CosimNetDevice::RxInContext (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (*packet);

  PacketType packetType;
  Mac48Address destination;
  Mac48Address source;
  uint16_t protocol;

  EthernetHeader header (false);

  m_rxPacketFromAdapterTrace (packet);
  // packet is shorter than header -> drop
  if (packet->GetSize () < header.GetSerializedSize ())
    {
      m_dropPacketTrace (packet);
      return;
    }
  m_txPacketToNetworkTrace (packet);

  packet->RemoveHeader (header);

  destination = header.GetDestination ();
  source = header.GetSource ();
  protocol = header.GetLengthType ();

  if (header.GetDestination ().IsBroadcast ())
    packetType = NS3_PACKET_BROADCAST;
  else if (header.GetDestination ().IsGroup ())
    packetType = NS3_PACKET_MULTICAST;
  else if (destination == m_address)
    packetType = NS3_PACKET_HOST;
  else
    packetType = NS3_PACKET_OTHERHOST;

  if (!m_promiscRxCallback.IsNull ())
    {
      m_promiscRxCallback (this, packet, protocol, source, destination, packetType);
    }

  if (packetType != NS3_PACKET_OTHERHOST)
    {
      m_rxCallback (this, packet, protocol, source);
    }
}

} // namespace ns3
