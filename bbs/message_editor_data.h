/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*             Copyright (C)1998-2020, WWIV Software Services             */
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
#ifndef __INCLUDED_BBS_MESSAGE_EDITOR_DATA_H__
#define __INCLUDED_BBS_MESSAGE_EDITOR_DATA_H__

#include <cstdint>
#include <string>

namespace wwiv {
namespace bbs {


constexpr int MSGED_FLAG_NONE = 0;
constexpr int MSGED_FLAG_NO_TAGLINE = 1;
constexpr int MSGED_FLAG_HAS_REPLY_NAME = 2;
constexpr int MSGED_FLAG_HAS_REPLY_TITLE = 4;

enum class FsedFlags {
  NOFSED, FSED, WORKSPACE
};

class MessageEditorData {
public:
  MessageEditorData() = default;
  ~MessageEditorData() = default;

  [[nodiscard]] bool is_email() const;

  // Title to use. If set it will be used without prompting the user.
  std::string title;
  std::string to_name;  // szDestination (to or sub name)
  std::string sub_name;

  bool need_title{true};
  uint8_t anonymous_flag{0};   // an
  int msged_flags{MSGED_FLAG_NONE};      // used to be flags
  FsedFlags fsed_flags{FsedFlags::NOFSED};       // fsed
  bool silent_mode{false};     // Used for ASV and new email emails.  No questions, etc.

  // legacy filename, used to see if it's email or not.
  std::string aux;

  // output only
  std::string text;
};

} // namespace bbs
} // namespace wwiv

#endif // __INCLUDED_BBS_MESSAGE_EDITOR_DATA_H__