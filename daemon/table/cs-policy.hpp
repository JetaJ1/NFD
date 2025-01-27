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

#ifndef NFD_DAEMON_TABLE_CS_POLICY_HPP
#define NFD_DAEMON_TABLE_CS_POLICY_HPP

#include "cs-entry.hpp"

namespace nfd::cs {

class Cs;

/**
 * \brief Represents a CS replacement policy.
 */
class Policy : noncopyable
{
public: // registry
  template<typename P>
  static void
  registerPolicy(const std::string& policyName = P::POLICY_NAME)
  {
    BOOST_ASSERT(!policyName.empty());
    auto r = getRegistry().insert_or_assign(policyName, [] { return make_unique<P>(); });
    BOOST_VERIFY(r.second);
  }

  /**
   * \brief Returns a cs::Policy identified by \p policyName,
   *        or nullptr if \p policyName is unknown.
   */
  static unique_ptr<Policy>
  create(const std::string& policyName);

  /**
   * \brief Returns a list of available policy names.
   */
  static std::set<std::string>
  getPolicyNames();

public:
  virtual
  ~Policy() = default;

  const std::string&
  getName() const noexcept
  {
    return m_policyName;
  }

  /**
   * \brief Returns a pointer to the associated CS instance.
   */
  Cs*
  getCs() const noexcept
  {
    return m_cs;
  }

  /**
   * \brief Sets the associated CS instance.
   */
  void
  setCs(Cs* cs) noexcept
  {
    m_cs = cs;
  }

  /**
   * \brief Gets hard limit (in number of entries).
   */
  size_t
  getLimit() const noexcept
  {
    return m_limit;
  }

  /** \brief Sets hard limit (in number of entries).
   *  \post getLimit() == nMaxEntries
   *  \post cs.size() <= getLimit()
   *
   *  The policy may evict entries if necessary.
   */
  void
  setLimit(size_t nMaxEntries);

public:
  /** \brief A reference to a CS entry.
   *  \note `operator<` of EntryRef compares the Data name enclosed in the Entry.
   */
  using EntryRef = Table::const_iterator;

  /** \brief %Signal emitted when an entry is being evicted.
   *
   *  A policy implementation should emit this signal to cause CS to erase an entry from its index.
   *  CS should connect to this signal and erase the entry upon signal emission.
   */
  signal::Signal<Policy, EntryRef> beforeEvict;

  /** \brief Invoked by CS after a new entry is inserted.
   *  \post cs.size() <= getLimit()
   *
   *  The policy may evict entries if necessary.
   *  During this process, \p i might be evicted.
   */
  void
  afterInsert(EntryRef i);

  /** \brief Invoked by CS after an existing entry is refreshed by same Data.
   *
   *  The policy may witness this refresh to make better eviction decisions in the future.
   */
  void
  afterRefresh(EntryRef i);

  /** \brief Invoked by CS before an entry is erased due to management command.
   *  \warning CS must not invoke this method if an entry is erased due to eviction.
   */
  void
  beforeErase(EntryRef i);

  /** \brief Invoked by CS before an entry is used to match a lookup.
   *
   *  The policy may witness this usage to make better eviction decisions in the future.
   */
  void
  beforeUse(EntryRef i);

protected:
  /** \brief Invoked after a new entry is created in CS.
   *
   *  When overridden in a subclass, a policy implementation should decide whether to accept \p i.
   *  If \p i is accepted, it should be inserted into a cleanup index.
   *  Otherwise, \p beforeEvict signal should be emitted with \p i to inform CS to erase the entry.
   *  A policy implementation may decide to evict other entries by emitting \p beforeEvict signal,
   *  in order to keep CS size under limit.
   */
  virtual void
  doAfterInsert(EntryRef i) = 0;

  /** \brief Invoked after an existing entry is refreshed by same Data.
   *
   *  When overridden in a subclass, a policy implementation may witness this operation
   *  and adjust its cleanup index.
   */
  virtual void
  doAfterRefresh(EntryRef i) = 0;

  /** \brief Invoked before an entry is erased due to management command.
   *  \note This will not be invoked for an entry being evicted by policy.
   *
   *  When overridden in a subclass, a policy implementation should erase \p i
   *  from its cleanup index without emitted \p afterErase signal.
   */
  virtual void
  doBeforeErase(EntryRef i) = 0;

  /** \brief Invoked before an entry is used to match a lookup.
   *
   *  When overridden in a subclass, a policy implementation may witness this operation
   *  and adjust its cleanup index.
   */
  virtual void
  doBeforeUse(EntryRef i) = 0;

  /** \brief Evicts zero or more entries.
   *  \post CS size does not exceed hard limit
   */
  virtual void
  evictEntries() = 0;

protected:
  explicit
  Policy(std::string_view policyName);

  DECLARE_SIGNAL_EMIT(beforeEvict)

private: // registry
  using CreateFunc = std::function<unique_ptr<Policy>()>;
  using Registry = std::map<std::string, CreateFunc>; // indexed by policy name

  static Registry&
  getRegistry();

private:
  const std::string m_policyName;
  size_t m_limit;
  Cs* m_cs;
};

} // namespace nfd::cs

/** \brief Registers a CS policy.
 *  \param P a subclass of nfd::cs::Policy
 */
#define NFD_REGISTER_CS_POLICY(P)                      \
static class NfdAuto ## P ## CsPolicyRegistrationClass \
{                                                      \
public:                                                \
  NfdAuto ## P ## CsPolicyRegistrationClass()          \
  {                                                    \
    ::nfd::cs::Policy::registerPolicy<P>();            \
  }                                                    \
} g_nfdAuto ## P ## CsPolicyRegistrationVariable

#endif // NFD_DAEMON_TABLE_CS_POLICY_HPP
