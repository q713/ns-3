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

#include "jitter-provider.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("JitterProvider");

TypeId
JitterProvider::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::JitterProvider")
          .SetParent<Object> ()
          .AddConstructor<JitterProvider> ()
          .AddAttribute (
              "JitterRandVar",
              "A RandomVariableStream used to pick the additional delay caused through jitter.",
              StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
              MakePointerAccessor (&JitterProvider::m_jitterRandVar),
              MakePointerChecker<RandomVariableStream> ());
  return tid;
}

TypeId
JitterProvider::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

JitterProvider::JitterCallback
JitterProvider::CreateCallback (Ptr<JitterProvider> provider)
{
  NS_ABORT_MSG_IF (provider == 0, "JitterProvider::CreateCallback provider is null");

  JitterProvider *peekedProvider = PeekPointer (provider);
  JitterProvider::JitterCallback callback =
      MakeCallback (
          &JitterProvider::CalculateNextDelay, peekedProvider);

  return callback;
}

Time
JitterProvider::CalculateNextDelay (Ptr<Packet> packet, uint16_t protocol, Address to,
                                    Address from)
{
  if (m_jitterRandVar == 0)
    {
      return NanoSeconds (0);
    }

  Time delay_nanoseconds = NanoSeconds (m_jitterRandVar->GetInteger ());
  NS_LOG_FUNCTION ("apply " << delay_nanoseconds << "nanoseconds of jitter to packet " << packet);
  return delay_nanoseconds;
}

} // namespace ns3
