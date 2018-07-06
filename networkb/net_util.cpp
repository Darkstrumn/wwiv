/**************************************************************************/
/*                                                                        */
/*                          WWIV Version 5.x                              */
/*            Copyright (C)2016-2017, WWIV Software Services              */
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
#include "networkb/net_util.h"

#include <string>

#include "core/command_line.h"
#include "core/file.h"
#include "core/log.h"
#include "core/stl.h"
#include "core/strings.h"
#include "core/version.h"
#include "sdk/datetime.h"

using std::string;
using namespace wwiv::core;
using namespace wwiv::stl;
using namespace wwiv::strings;

namespace wwiv {
namespace net {

void rename_pend(const string& directory, const string& filename, uint8_t network_app_num) {
  File pend_file(directory, filename);
  if (!pend_file.Exists()) {
    LOG(INFO) << " pending file does not exist: " << pend_file;
    return;
  }
  const auto pend_filename(pend_file.full_pathname());
  const auto num = filename.substr(1);
  const string prefix = (to_number<int>(num)) ? "1" : "0";

  for (int i = 0; i < 1000; i++) {
    const auto new_filename =
      StringPrintf("%sp%s-%u-%u.net", directory.c_str(), prefix.c_str(), network_app_num, i);
    if (File::Rename(pend_filename, new_filename)) {
      LOG(INFO) << "renamed file: '" << pend_filename << "' to: '" << new_filename << "'";
      return;
    }
  }
  LOG(ERROR) << "all attempts failed to rename_wwivnet_pend";
}

std::string create_pend(const string& directory, bool local, char network_app_id) {
  const uint8_t prefix = (local) ? 0 : 1;
  for (auto i = 0; i < 1000; i++) {
    const auto filename =
      StringPrintf("p%u-%c-%d.net", prefix, network_app_id, i);
    File f(directory, filename);
    if (f.Exists()) {
      continue;
    }
    if (f.Open(File::modeCreateFile | File::modeReadWrite | File::modeExclusive)) {
      LOG(INFO) << "Created pending file: " << filename;
      return filename;
    }
  }
  LOG(ERROR) << "all attempts failed to create_pend";
  return "";
}


string main_type_name(int typ) {
  switch (typ) {
  case main_type_net_info: return "main_type_net_info";
  case main_type_email: return "main_type_email";
  case main_type_post: return "main_type_post";
  case main_type_file: return "main_type_file";
  case main_type_pre_post: return "main_type_pre_post";
  case main_type_external: return "main_type_external";
  case main_type_email_name: return "main_type_email_name";
  case main_type_net_edit: return "main_type_net_edit";
  case main_type_sub_list: return "main_type_sub_list";
  case main_type_extra_data: return "main_type_extra_data";
  case main_type_group_bbslist: return "main_type_group_bbslist";
  case main_type_group_connect: return "main_type_group_connect";
  case main_type_group_binkp: return "main_type_group_binkp";
  case main_type_group_info: return "main_type_group_info";
  case main_type_ssm: return "main_type_ssm";
  case main_type_sub_add_req: return "main_type_sub_add_req";
  case main_type_sub_drop_req: return "main_type_sub_drop_req";
  case main_type_sub_add_resp: return "main_type_sub_add_resp";
  case main_type_sub_drop_resp: return "main_type_sub_drop_resp";
  case main_type_sub_list_info: return "main_type_sub_list_info";
  case main_type_new_post: return "main_type_new_post";
  case main_type_new_external: return "main_type_new_external";
  case main_type_game_pack: return "main_type_game_pack";
  default: return StringPrintf("UNKNOWN type #%d", typ);
  }
}

string net_info_minor_type_name(int typ) {
  switch (typ) {
  case net_info_general_message: return "net_info_general_message";
  case net_info_bbslist: return "net_info_bbslist";
  case net_info_connect: return "net_info_connect";
  case net_info_sub_lst: return "net_info_sub_lst";
  case net_info_wwivnews: return "net_info_wwivnews";
  case net_info_fbackhdr: return "net_info_fbackhdr";
  case net_info_more_wwivnews: return "net_info_more_wwivnews";
  case net_info_categ_net: return "net_info_categ_net";
  case net_info_network_lst: return "net_info_network_lst";
  case net_info_file: return "net_info_file";
  case net_info_binkp: return "net_info_binkp";
  default: return StringPrintf("UNKNOWN type #%d", typ);
  }
}

std::string get_subtype_from_packet_text(const std::string& text) {
  auto iter = text.begin();
  return get_message_field(text, iter, {'\0', '\r', '\n'}, 80);
}

void AddStandardNetworkArgs(wwiv::core::CommandLine& cmdline, const std::string& current_directory) {
  cmdline.add_argument({"net", "Network number to use (i.e. 0).", "0"});
  cmdline.add_argument({"bbsdir", "(optional) BBS directory if other than current directory", current_directory});
  cmdline.add_argument(BooleanCommandLineArgument("skip_net", "Skip invoking network1/network2/network3"));
}

NetworkCommandLine::NetworkCommandLine(wwiv::core::CommandLine& cmdline) {
  cmdline.set_no_args_allowed(true);
  cmdline.AddStandardArgs();
  AddStandardNetworkArgs(cmdline, File::current_directory());

  if (!cmdline.Parse()) {
    initialized_ = false;
  }
  bbsdir_ = cmdline.arg("bbsdir").as_string();
  network_number_ = cmdline.arg("net").as_int();

  config_.reset(new wwiv::sdk::Config(bbsdir_));
  networks_.reset(new wwiv::sdk::Networks(*config_.get()));

  if (!config_->IsInitialized()) {
    LOG(ERROR) << "Unable to load CONFIG.DAT.";
    initialized_ = false;
  }
  if (!networks_->IsInitialized()) {
    LOG(ERROR) << "Unable to load networks.";
    initialized_ = false;
  }
  const auto& nws = networks_->networks();

  if (network_number_ < 0 || network_number_ >= size_int(nws)) {
    LOG(ERROR) << "network number must be between 0 and " << nws.size() << ".";
    initialized_ = false;
    return;
  }
  network_ = nws[network_number_];

  network_name_ = network_.name;
  StringLowerCase(&network_name_);
  LOG(INFO) << cmdline.program_name() << " [" << wwiv_version << beta_version << "]"
    << " for network: " << network_name_;
}

std::unique_ptr<wwiv::core::IniFile> NetworkCommandLine::LoadNetIni(char net_cmd) const {
  const auto net_tag = StrCat("network", net_cmd);
  const auto net_tag_net = StrCat(net_tag, "-", network_name());

  File file(config().root_directory(), "net.ini");
  return std::unique_ptr<IniFile>(new IniFile(file.full_pathname(), { net_tag_net, net_tag }));
}


}  // namespace net
}  // namespace wwiv

