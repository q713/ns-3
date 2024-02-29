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

#ifndef SIMBRICKS_JITTER_PROVIDER_H_
#define SIMBRICKS_JITTER_PROVIDER_H_

#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/address.h"
#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/callback.h"

namespace ns3 {

class JitterProvider : public Object
{
public:
  typedef Callback<Time, Ptr<Packet>, uint16_t, Address, Address>
      JitterCallback;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  static JitterCallback CreateCallback (Ptr<JitterProvider> provider);

  Time CalculateNextDelay (Ptr<Packet> packet, uint16_t protocol, Address to,
                           Address from);

  void SetRandomVariableStream(Ptr<RandomVariableStream> randVar);

private:
  Ptr<RandomVariableStream> m_jitterRandVar;
};

} // namespace ns3

#endif // SIMBRICKS_JITTER_PROVIDER_H_
