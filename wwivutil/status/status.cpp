/**************************************************************************/
/*                                                                        */
/*                          WWIV Version 5.x                              */
/*             Copyright (C)2015-2020, WWIV Software Services             */
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
#include "wwivutil/status/status.h"

#include "sdk/status.h"
#include "core/command_line.h"
#include "core/file.h"
#include "core/strings.h"
#include "sdk/config.h"
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using std::cerr;
using std::cout;
using std::endl;
using std::make_unique;
using std::setw;
using std::string;
using std::unique_ptr;
using std::vector;
using wwiv::core::BooleanCommandLineArgument;
using namespace wwiv::sdk;
using namespace wwiv::strings;

namespace wwiv::wwivutil {

static int show_qscan(const Config& config) {
  StatusMgr mgr(config.datadir(), [](int) {});
  const auto status = mgr.GetStatus();
  cout << "QScan Pointer : " << status->GetQScanPointer() << std::endl;
  return 0;
}

static int set_qscan(const Config& config, uint32_t qscan) {
  StatusMgr mgr(config.datadir(), [](int) {});
  mgr.Run([=](WStatus& s) {
    s.SetQScanPointer(qscan);
  });

  const auto status = mgr.GetStatus();
  cout << "QScan Pointer : " << status->GetQScanPointer() << std::endl;
  return 0;
}

class StatusVersionCommand : public UtilCommand {
public:
  StatusVersionCommand(): UtilCommand("qscan", "Sets or Gets the qscan high water mark.") {}
  std::string GetUsage() const override final {
    std::ostringstream ss;
    ss << "Usage: " << std::endl << std::endl;
    ss << "  get : Gets the qscptr information." << std::endl << std::endl;
    return ss.str();
  }
  int Execute() override final {
    if (remaining().empty()) {
      std::cout << GetUsage() << GetHelp() << endl;
      return 2;
    }
    const auto set_or_get = ToStringLowerCase(remaining().front());

    if (set_or_get == "get") {
      return show_qscan(*this->config()->config());
    }
    if (set_or_get == "set") {
      if (remaining().size() < 2) {
        std::cout << "qscan set requires a value" << endl;
        return 2;
      }
      const auto s = remaining().at(1);
      const auto v = to_number<uint32_t>(s);
      if (v <= 0) {
        std::cout << "invalid value: '" << s << "' (" << v << ")" << endl;
        return 1;
      }
      std::cout << "Setting qscanptr to : " << v << endl;
      set_qscan(*this->config()->config(), v);
    }
    return 1;
  }

  bool AddSubCommands() override final {
    return true;
  }

};

bool StatusCommand::AddSubCommands() {
  add(make_unique<StatusVersionCommand>());
  return true;
}


}  // namespace wwivutil::wwiv
