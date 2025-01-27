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

#ifndef NFD_TOOLS_NFDC_RIB_MODULE_HPP
#define NFD_TOOLS_NFDC_RIB_MODULE_HPP

#include "module.hpp"
#include "command-parser.hpp"

namespace nfd::tools::nfdc {

using ndn::nfd::RibEntry;
using ndn::nfd::Route;

/** \brief Provides access to NFD RIB management.
 *  \sa https://redmine.named-data.net/projects/nfd/wiki/RibMgmt
 */
class RibModule : public Module, noncopyable
{
public:
  /** \brief Register 'route list', 'route show', 'route add', 'route remove' commands.
   */
  static void
  registerCommands(CommandParser& parser);

  /** \brief The 'route list' command.
   */
  static void
  list(ExecuteContext& ctx);

  /** \brief The 'route show' command.
   */
  static void
  show(ExecuteContext& ctx);

  /** \brief The 'route add' command.
   */
  static void
  add(ExecuteContext& ctx);

  /** \brief The 'route remove' command.
   */
  static void
  remove(ExecuteContext& ctx);

  void
  fetchStatus(Controller& controller,
              const std::function<void()>& onSuccess,
              const Controller::DatasetFailCallback& onFailure,
              const CommandOptions& options) override;

  void
  formatStatusXml(std::ostream& os) const override;

  void
  formatStatusText(std::ostream& os) const override;

private:
  using RoutePredicate = std::function<bool(const RibEntry&, const Route&)>;

  static void
  listRoutesImpl(ExecuteContext& ctx, const RoutePredicate& filter);

  /** \brief Format a single status item as XML.
   *  \param os output stream
   *  \param item status item
   */
  void
  formatItemXml(std::ostream& os, const RibEntry& item) const;

  /** \brief Format a RibEntry as text.
   *  \param os output stream
   *  \param entry RIB entry
   */
  static void
  formatEntryText(std::ostream& os, const RibEntry& entry);

  /** \brief Format a Route as text.
   *  \param os output stream
   *  \param entry RIB entry
   *  \param route RIB route within \p entry
   *  \param includePrefix whether to print the name prefix
   */
  static void
  formatRouteText(std::ostream& os, const RibEntry& entry, const Route& route,
                  bool includePrefix);

private:
  std::vector<RibEntry> m_status;
};

} // namespace nfd::tools::nfdc

#endif // NFD_TOOLS_NFDC_RIB_MODULE_HPP
