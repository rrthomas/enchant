/* Copyright (c) 2007 Eric Scott Albright
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __ENCHANTTESTFIXTURE
#define __ENCHANTTESTFIXTURE

#include <assert.h>
#include <glib.h>
#ifdef G_OS_UNIX
#include <fcntl.h> /* For creat; FIXME: should not be needed */
#endif
#include <glib/gstdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <locale>
#include <codecvt>

#include "enchant.h"
#include "enchant-provider.h"

struct EnchantTestFixture
{
    //Setup
    EnchantTestFixture()
    {
        CleanUpFiles(); //just in case we stopped the process in the middle.
        CreateDirectory(GetTempUserEnchantDir());
    }

    //Teardown
    ~EnchantTestFixture(){
        CleanUpFiles();
    }
    void CleanUpFiles()
    {
        DeleteDirAndFiles(GetTempUserEnchantDir());
        DeleteDirAndFiles(AddToPath(LIBDIR_SUBDIR, "enchant-" ENCHANT_MAJOR_VERSION));
        DeleteDirAndFiles("share");
    }

    std::string GetTempUserEnchantDir()
    {
        return getenv("ENCHANT_CONFIG_DIR");
    }

    std::string GetEnchantConfigDir()
    {
        GSList *config_dirs = enchant_get_conf_dirs();
        const char *pkgdatadir = (char *)g_slist_nth(config_dirs, 0)->data;
        return std::string(pkgdatadir);
    }

    static void DeleteDirAndFiles(const std::string& dir)
    {
        GDir* gdir = g_dir_open(dir.c_str(), 0, NULL);
        if(gdir != NULL)
        {
            const gchar* filename;
            for(;;){
                filename = g_dir_read_name(gdir);
                if(filename == NULL)
                {
                    break;
                }
                std::string filepath = AddToPath(dir, filename);
                if(g_file_test(filepath.c_str(), G_FILE_TEST_IS_DIR)){
                    DeleteDirAndFiles(filepath);
                }
                else {
                    DeleteFile(filepath);
                }
            } 
            g_dir_close(gdir);
        }
        g_rmdir(dir.c_str());
    }

    static std::string GetTemporaryFilename(const std::string & prefix){
        char* tempFileName = tempnam(".", prefix.c_str());
        std::string temp(tempFileName);
        free(tempFileName);
        return temp;
    }

    static void CreateDirectory(const std::string& filepath)
    {
        g_mkdir_with_parents(filepath.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
    }
    static void CreateFile(const std::string& filepath)
    {
        int fh = g_creat(filepath.c_str(), S_IRUSR | S_IWUSR);
        if(fh != -1) {
            close(fh);
        }
    }
    static void DeleteFile(const std::string& filepath)
    {
        if(FileExists(filepath)){
            g_remove(filepath.c_str());
        }
    }
    static bool FileExists(const std::string& filepath)
    {
        return(g_access(filepath.c_str(), 0)==0);
    }

    std::string Convert(const std::wstring & ws)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convert;
        return convert.to_bytes(ws);
    }

    std::wstring Convert(const std::string & s)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convert;
        return convert.from_bytes(s);
    }

  static std::string AddToPath(const std::string & path, const std::string & fileOrDirName)
    {
        std::string result;
        gchar* filename = g_build_filename(path.c_str(), fileOrDirName.c_str(), NULL);
        result = std::string(filename);
        g_free(filename);

        return result;
    }
};

#endif
