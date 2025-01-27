/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2022,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tests/test-common.hpp"

namespace nfd::tests {

shared_ptr<Interest>
makeInterest(const Name& name, bool canBePrefix, std::optional<time::milliseconds> lifetime,
             std::optional<Interest::Nonce> nonce)
{
  auto interest = std::make_shared<Interest>(name);
  interest->setCanBePrefix(canBePrefix);
  if (lifetime) {
    interest->setInterestLifetime(*lifetime);
  }
  interest->setNonce(nonce ? ndn::make_optional(*nonce) : ndn::nullopt);
  return interest;
}

shared_ptr<Data>
makeData(const Name& name)
{
  auto data = std::make_shared<Data>(name);
  return signData(data);
}

Data&
signData(Data& data)
{
  data.setSignatureInfo(ndn::SignatureInfo(tlv::NullSignature));
  data.setSignatureValue(std::make_shared<ndn::Buffer>());
  data.wireEncode();
  return data;
}

lp::Nack
makeNack(Interest interest, lp::NackReason reason)
{
  lp::Nack nack(std::move(interest));
  nack.setReason(reason);
  return nack;
}

ndn::PrefixAnnouncement
makePrefixAnn(const Name& announcedName, time::milliseconds expiration,
              std::optional<ndn::security::ValidityPeriod> validity)
{
  ndn::PrefixAnnouncement pa;
  pa.setAnnouncedName(announcedName);
  pa.setExpiration(expiration);
  pa.setValidityPeriod(validity ? ndn::make_optional(*validity) : ndn::nullopt);
  return pa;
}

ndn::PrefixAnnouncement
makePrefixAnn(const Name& announcedName, time::milliseconds expiration,
              std::pair<time::seconds, time::seconds> validityFromNow)
{
  auto now = time::system_clock::now();
  return makePrefixAnn(announcedName, expiration,
    ndn::security::ValidityPeriod(now + validityFromNow.first, now + validityFromNow.second));
}

ndn::PrefixAnnouncement
signPrefixAnn(ndn::PrefixAnnouncement&& pa, ndn::KeyChain& keyChain,
              const ndn::security::SigningInfo& si, std::optional<uint64_t> version)
{
  pa.toData(keyChain, si, version ? ndn::make_optional(*version) : ndn::nullopt);
  return std::move(pa);
}

} // namespace nfd::tests
