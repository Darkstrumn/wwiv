/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*             Copyright (C)1998-2021, WWIV Software Services             */
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
/*                                                                        */
/**************************************************************************/
#include "common/pause.h"

#include "common/common_events.h"
#include "common/context.h"
#include "common/input.h"
#include "core/datetime.h"
#include "core/eventbus.h"
#include "core/os.h"
#include "core/scope_exit.h"
#include "core/strings.h"
#include "local_io/keycodes.h"
#include <chrono>

char str_pause[81], str_quit[81];

using std::chrono::milliseconds;
using namespace wwiv::common;
using namespace wwiv::core;
using namespace wwiv::os;
using namespace wwiv::sdk;
using namespace wwiv::strings;

namespace wwiv::common {

TempDisablePause::TempDisablePause(Output& out)
    : Transaction([&out] {
    if (out.sess().disable_pause()) {
      out.sess().disable_pause(false);
      out.user().SetStatusFlag(User::pauseOnPage);
    }
  }, nullptr) {
  if (out.user().HasPause()) {
    out.sess().disable_pause(true);
    out.user().ClearStatusFlag(User::pauseOnPage);
  }
}

char Output::GetKeyForPause() {
  char ch = 0;
  while (ch == 0 && !sess().hangup()) {
    ch = bin.bgetch();
    sleep_for(milliseconds(50));
    bus().invoke<CheckForHangupEvent>();
  }
  const auto k = to_upper_case<int>(ch);
  switch (k) {
  case ESC:
  case 'Q':
  case 'N':
    if (!bin.bkbhit()) {
      bin.nsp(-1);
    }
    break;
  case 'C':
  case '=':
    if (user().HasPause()) {
      bin.nsp(1);
      user().ToggleStatusFlag(User::pauseOnPage);
    }
    break;
  default:
    break;
  }
  return ch;
}

static bool okansi(const wwiv::sdk::User& user) { return user.HasAnsi(); }

void Output::pausescr() {
  bin.clearnsp();
  bus().invoke<PauseProcessingInstanceMessages>();
  ScopeExit at_exit(
      [] { wwiv::core::bus().invoke<ResetProcessingInstanceMessages>(); });
  auto* const ss = str_pause;
  int i1;
  const auto i2 = i1 = strlen(ss);
  const auto com_freeze = sess().incom();

  if (!sess().incom() && sess().outcom()) {
    sess().incom(true);
  }

  if (okansi(user())) {
    ResetColors();

    i1 = ssize(stripcolors(ss));
    const auto i = curatr();
    SystemColor(user().color(3));
    bputs(ss);
    Left(i1);
    SystemColor(i);

    const auto tstart = time_t_now();

    clear_lines_listed();
    auto warned = 0;
    char ch;
    do {
      while (!bin.bkbhit() && !sess().hangup()) {
        const auto tstop = time_t_now();
        const auto ttotal = static_cast<time_t>(difftime(tstop, tstart));
        if (ttotal == 120) {
          if (!warned) {
            warned = 1;
            bputch(CG);
            SystemColor(user().color(6));
            bputs(ss);
            for (auto i3 = 0; i3 < i2; i3++) {
              if (ss[i3] == 3 && i1 > 1) {
                i1 -= 2;
              }
            }
            Left(i1);
            SystemColor(i);
          }
        } else {
          if (ttotal > 180) {
            bputch(CG);
            for (auto i3 = 0; i3 < i1; i3++) {
              bputch(' ');
            }
            Left(i1);
            SystemColor(i);
            return;
          }
        }
        sleep_for(milliseconds(50));
        bus().invoke<CheckForHangupEvent>();
      }
      ch = GetKeyForPause();
    } while (!ch && !sess().hangup());
    for (int i3 = 0; i3 < i1; i3++) {
      bputch(' ');
    }
    Left(i1);
    SystemColor(i);
  } else {
    // nonansi code path
    for (int i3 = 0; i3 < i2; i3++) {
      if ((ss[i3] == CC) && i1 > 1) {
        i1 -= 2;
      }
    }
    bputs(ss);
    GetKeyForPause();
    for (int i3 = 0; i3 < i1; i3++) {
      bs();
    }
  }

  if (!com_freeze) {
    sess().incom(false);
  }
}


} // namespace wwiv::common
