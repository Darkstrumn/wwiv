//**************************************************************************/
/*                                                                        */
/*                                WWIV Version 5                          */
/*             Copyright (C)1998-2016, WWIV Software Services             */
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
#include "local_io/local_io_unix_console.h"

#include <algorithm>
#include <chrono>
#include <cstdarg>
#include <iostream>
#include <string>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/poll.h>

#include "core/file.h"
#include "core/os.h"
#include "core/strings.h"
#include "local_io/wconstants.h"

using std::cout;
using std::string;
using namespace wwiv::strings;

#define TTY "/dev/tty"

#if !defined(TCGETS) && defined(TIOCGETA)
#define TCGETS TIOCGETA
#endif
#if !defined(TCSETS) && defined(TIOCSETA)
#define TCSETS TIOCSETA
#endif

static void set_terminal(bool initMode) {
  static struct termios foo;
  static struct termios boo;

  if (initMode) {
    tcgetattr(fileno(stdin), &foo);
    memcpy(&boo, &foo, sizeof(struct termios));
    foo.c_lflag &= ~ICANON;
    tcsetattr(fileno(stdin), TCSANOW, &foo);
  } else {
    tcsetattr(fileno(stdin), TCSANOW, &boo);
  }
}

UnixConsoleIO::UnixConsoleIO() {
  if ((ttyf = fdopen(::open(TTY, O_RDWR), "r")) == nullptr) {
    ttyf = stdin;
  } else {
    setbuf(ttyf, NULL);
  }

  int f = fileno(ttyf);
  set_terminal(true);
  struct termios ttyb;

  ioctl(f, TCGETS, &ttysav);
  ioctl(f, TCGETS, &ttyb);
  ttyb.c_lflag &= ~(ECHO | ISIG);
  ioctl(f, TCSETS, &ttyb);
}

UnixConsoleIO::~UnixConsoleIO() {
  int f = fileno(ttyf);
  ioctl(f, TCSETS, &ttysav);

  if (ttyf != stdin) {
    fclose(ttyf);
  }
  set_terminal(false);
}

void UnixConsoleIO::LocalGotoXY(int x, int y) {
  x = std::max<int>(x, 0);
  x = std::min<int>(x, 79);
  y = std::max<int>(y, 0);
  y += GetTopLine();
  y = std::min<int>(y, GetScreenBottom());

  if (x_only) {
    capture_->set_wx(x);
    return;
  }
  m_cursorPositionX = static_cast< short >(x);
  m_cursorPositionY = static_cast< short >(y);

  cout << "\x1b[" << y << ";" << x << "H";
  cout.flush();
}

int UnixConsoleIO::WhereX() const noexcept {
  if (x_only) {
    return capture_->wx();
  }
  return m_cursorPositionX;
}

int UnixConsoleIO::WhereY() const noexcept {
  return m_cursorPositionY;
}

void UnixConsoleIO::LocalLf() {
  cout << "\n";
  cout.flush();
  m_cursorPositionY++;
  if (m_cursorPositionY > 24) {
    m_cursorPositionY = 24;
  }
}

void UnixConsoleIO::LocalCr() {
  cout << "\r";
  cout.flush();
  m_cursorPositionX = 0;
}

void UnixConsoleIO::LocalCls() {
  cout << "\x1b[2J";
  cout.flush();
  m_cursorPositionX = 0;
  m_cursorPositionY = 0;
}

void UnixConsoleIO::LocalBackspace() {
  cout << "\b";
  cout.flush();
  if (m_cursorPositionX >= 0) {
    m_cursorPositionX--;
  } else if (m_cursorPositionY != GetTopLine()) {
    m_cursorPositionX = 79;
    m_cursorPositionY--;
  }
}

void UnixConsoleIO::LocalPutchRaw(unsigned char ch) {
  ::write(fileno(stdout), &ch, 1);
  if (m_cursorPositionX <= 79) {
    m_cursorPositionX++;
    return;
  }
  m_cursorPositionX = 0;
  if (m_cursorPositionY != GetScreenBottom()) {
    m_cursorPositionY++;
  }
}

void UnixConsoleIO::LocalPutch(unsigned char ch) {
  if (x_only) {
    int wx = capture_->wx();
    if (ch > 31) {
      wx = (wx + 1) % 80;
    } else if (ch == RETURN || ch == CL) {
      wx = 0;
    } else if (ch == BACKSPACE) {
      if (wx) {
        wx--;
      }
    }
    capture_->set_wx(wx);
    return;
  }
  if (ch > 31) {
    LocalPutchRaw(ch);
  } else if (ch == CM) {
    LocalCr();
  } else if (ch == CJ) {
    LocalLf();
  } else if (ch == CL) {
    LocalCls();
  } else if (ch == BACKSPACE) {
    LocalBackspace();
  } else if (ch == CG) {
    // No bell
  }
}

void UnixConsoleIO::LocalPuts(const string& s) {
  for (char ch : s) {
    LocalPutch(ch);
  }
}

void UnixConsoleIO::LocalXYPuts(int x, int y, const string& text) {
  LocalGotoXY(x, y);
  LocalFastPuts(text);
}

void UnixConsoleIO::LocalFastPuts(const string& text) {
  m_cursorPositionX += text.length();
  m_cursorPositionX %= 80;

  ::write(fileno(stdout), text.c_str(), text.size());
  // TODO: set current attributes
}

void UnixConsoleIO::set_protect(int l) {
  SetTopLine(l);
}

void UnixConsoleIO::savescreen() {}
void UnixConsoleIO::restorescreen() {}


void UnixConsoleIO::tleft(bool bCheckForTimeOut) {}

bool UnixConsoleIO::LocalKeyPressed() { 
  struct pollfd p;

  p.fd = fileno(stdin);
  p.events = POLLIN;
  poll(&p, 1, 0);
  return (p.revents & POLLIN);
}
void UnixConsoleIO::SaveCurrentLine(char *cl, char *atr, char *xl, char *cc) {}

unsigned char UnixConsoleIO::LocalGetChar() {
  unsigned char ch = 0;
  int dlay = 0;
  struct pollfd p;

  p.fd = fileno(stdin);
  p.events = POLLIN;

  if (poll(&p, 1, dlay)) {
    if (p.revents & POLLIN) {
      ::read(fileno(stdin), &ch, 1);
      // Don't ask why this needs to be here...but it does
      if (ch == SOFTRETURN) {
        ch = RETURN;
      }
    }
  }
  return ch;
}

void UnixConsoleIO::MakeLocalWindow(int x, int y, int xlen, int ylen) {}
void UnixConsoleIO::SetCursor(int cursorStyle) {}
void UnixConsoleIO::LocalClrEol() {}
void UnixConsoleIO::LocalWriteScreenBuffer(const char *pszBuffer) {}
int UnixConsoleIO::GetDefaultScreenBottom() { return 25; }
void UnixConsoleIO::UpdateNativeTitleBar(const std::string& system_name, int instance_number) {}
