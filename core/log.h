/**************************************************************************/
/*                                                                        */
/*                          WWIV Version 5.x                              */
/*             Copyright (C)2014-2017, WWIV Software Services             */
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
#ifndef __INCLUDED_CORE_LOG_H__
#define __INCLUDED_CORE_LOG_H__

#ifdef _WIN32

#ifdef MOUSE_MOVED
#undef MOUSE_MOVED
#endif  // MOUSE_MOVED

#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOATOM
#define NOCLIPBOARD
#define NODRAWTEXT
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
#ifndef NOMINMAX
#define NOMINMAX
#endif  // NOMINMAX
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NOCRYPT
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  // WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#endif  // _WIN32

#define ELPP_NO_DEFAULT_LOG_FILE
#define ELPP_CUSTOM_COUT std::cerr
#define ELPP_WINSOCK2
#define ELPP_DISABLE_DEFAULT_CRASH_HANDLING
// We need the thread safe logger.
#define ELPP_THREAD_SAFE
#define ELPP_FORCE_USE_STD_THREAD

#ifdef _MSC_VER
#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable : ALL_CODE_ANALYSIS_WARNINGS)
#endif  // _MSC_VER
#include "deps/easylogging/easylogging++.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif  // _MSC_VER

#include <functional>
#include <map>
#include <string>
#include <sstream>

#define WWIV_INIT_LOGGER() SHARE_EASYLOGGINGPP(wwiv::core::Logger::getLoggerStorage())

namespace wwiv {
namespace core {

/**
 * Logger class for WWIV.
 * Usage:
 * 
 * Once near your main() method, invoke Logger::Init(argc, argv) to initialize
 * the logger. This will initialize the logger filename to be your
 * executable's name with .log appended. You should also also invoke ExitLogger
 * when exiting your binary,
 *
 * Example:
 *
 *   Logger::Init(argc, argv);
 *   wwiv::core::ScopeExit at_exit(Logger::ExitLogger);
 *
 * In code, just use "LOG(INFO) << messages" and it will end up in the information logs.
 */
class Logger {
public:
  Logger();
  ~Logger();

  /** Initializes the WWIV Loggers.  Must be invoked once per binary. */
  static void Init(int argc, char** argv, bool startup_log);
  static void Init(int argc, char** argv) { Init(argc, argv, true); };
  static void StartupLog(int argc, char* argv[]);
  static void ExitLogger();
  static el::base::type::StoragePointer getLoggerStorage();
  static void DisableFileLoging();

private:
  static std::string exit_filename;
};

}
}

#endif  // __INCLUDED_CORE_LOG_H__
