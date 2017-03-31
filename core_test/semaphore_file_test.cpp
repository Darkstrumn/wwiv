/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*               Copyright (C)2014-2017, WWIV Software Services           */
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
#include "file_helper.h"
#include "gtest/gtest.h"
#include "core/file.h"
#include "core/semaphore_file.h"
#include "core/strings.h"

#include <chrono>
#include <iostream>
#include <string>

using std::string;
using namespace wwiv::core;
using namespace wwiv::strings;

TEST(SemaphoreFileTest, AlreadyAcqired) {
    FileHelper file;
    string tmp = file.TempDir();
    string fn;
    {
      SemaphoreFile ok = SemaphoreFile::try_acquire(
        FilePath(tmp, "x.sem"), std::chrono::milliseconds(100));
      fn = ok.filename();

      EXPECT_TRUE(File::Exists(fn));

      try {
        SemaphoreFile nok = SemaphoreFile::try_acquire(
          FilePath(tmp, "x.sem"), std::chrono::milliseconds(10));
        FAIL() << "semaphore_not_acquired expected";
      }
      catch (const wwiv::core::semaphore_not_acquired&) {
        // expected to happen.
      }
    }
    EXPECT_FALSE(File::Exists(fn));
}