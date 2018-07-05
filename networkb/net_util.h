/**************************************************************************/
/*                                                                        */
/*                          WWIV Version 5.x                              */
/*           Copyright (C)2016-2017, WWIV Software Services               */
/*                                                                        */
/*    Licensed  under the  Apache License, Version  2.0 (the "License");  */
/*    you may not use this  file  except in compliance with the License.  */
/*    You may obtain a copy of the License at                             */
/*                                                                        */
/*                http://www.apache.org/licenses/LICENSE-2.0              */
/*                                                                        */
/*    Unless  required  by  applicable  law  or agreed to  in  writing,   */
/*    software  distributed  under  the  License  is  distributed on an   */
/*    "AS IS"  BASIS, WITHOUT  WARRANTIES  OR  CONDITIONS OF ANY  KIND,   */
/*    either  express  or implied.  See  the  License for  the specific   */
/*    language governing permissions and limitations under the License.   */
/**************************************************************************/
#ifndef __INCLUDED_NETWORKB_NET_UTIL_H__
#define __INCLUDED_NETWORKB_NET_UTIL_H__

#include <memory>
#include <set>
#include <string>
#include <vector>

#include "core/command_line.h"
#include "core/file.h"
#include "core/inifile.h"
#include "sdk/config.h"
#include "sdk/net.h"
#include "sdk/networks.h"

namespace wwiv {
namespace net {

void rename_pend(const std::string& directory, const std::string& filename,
                 uint8_t network_app_num);
std::string create_pend(const std::string& directory, bool local, char network_app_id);

std::string main_type_name(int typ);
std::string net_info_minor_type_name(int typ);

/**
 * Gets the subtype from a main_type_new_post message packet's text.
 * Returns empty string on error.
 */
std::string get_subtype_from_packet_text(const std::string& text);

template <typename C, typename I>
static std::string get_message_field(const C& c, I& iter, std::set<char> stop, std::size_t max) {
  // No need to continue if we're already at the end.
  if (iter == c.end()) {
    return "";
  }

  const auto begin = iter;
  std::size_t count = 0;
  while (stop.find(*iter) == std::end(stop) && ++count < max && iter != c.end()) {
    iter++;
  }
  std::string result(begin, iter);

  // Stop over stop chars
  while (iter != c.end() && stop.find(*iter) != std::end(stop)) {
    iter++;
  }

  return result;
}

void AddStandardNetworkArgs(wwiv::core::CommandLine& cmdline, const std::string& current_directory);

class NetworkCommandLine {
public:
  NetworkCommandLine(wwiv::core::CommandLine& cmdline);

  bool IsInitialized() const { return initialized_; }
  const std::string bbsdir() const { return bbsdir_; }
  const wwiv::sdk::Config& config() const { return *config_.get(); }
  const wwiv::sdk::Networks& networks() const { return *networks_.get(); }
  const std::string network_name() const { return network_name_; }
  const int network_number() const { return network_number_; }
  const net_networks_rec& network() const { return network_; }

  std::unique_ptr<wwiv::core::IniFile> LoadNetIni(char net_cmd) const;

private:
  std::string bbsdir_;
  std::unique_ptr<wwiv::sdk::Config> config_;
  std::unique_ptr<wwiv::sdk::Networks> networks_;
  std::string network_name_;
  int network_number_ = 0;
  bool initialized_ = true;
  net_networks_rec network_;
};

} // namespace net
} // namespace wwiv

#endif // __INCLUDED_NETWORKB_NET_UTIL_H__
