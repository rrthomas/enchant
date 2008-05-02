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

#if defined(_MSC_VER)
#pragma once
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#endif
#include <io.h>
#include <assert.h>
#include <glib.h>
#include <string>

struct EnchantTestFixture
{
    std::string savedRegistryHomeDir;
    std::string savedUserRegistryModuleDir;
    std::string savedMachineRegistryModuleDir;
    std::string savedUserRegistryConfigDir;
    std::string savedMachineRegistryConfigDir;

    //Setup
    EnchantTestFixture()
    {
        CleanUpFiles(); //just in case we stopped the process in the middle.
        MoveEnchantUserFilesOutOfTheWay();

#ifdef _WIN32
        savedRegistryHomeDir = GetRegistryHomeDir();
        ClearRegistryHomeDir();

        savedUserRegistryModuleDir = GetUserRegistryModuleDir();
        ClearUserRegistryModuleDir();

        savedMachineRegistryModuleDir = GetMachineRegistryModuleDir();
        ClearMachineRegistryModuleDir();

        savedUserRegistryConfigDir = GetUserRegistryConfigDir();
        ClearUserRegistryConfigDir();

        savedMachineRegistryConfigDir = GetMachineRegistryConfigDir();
        ClearMachineRegistryConfigDir();
#endif
    }

    //Teardown
    ~EnchantTestFixture(){
#ifdef _WIN32
        if(savedRegistryHomeDir.empty()) {
            ClearRegistryHomeDir();
        }
        else {
            SetRegistryHomeDir(savedRegistryHomeDir);
        }
                
        if(savedUserRegistryModuleDir.empty()) {
            ClearUserRegistryModuleDir();
        }
        else{
            SetUserRegistryModuleDir(savedUserRegistryModuleDir);
        }

        if(savedMachineRegistryModuleDir.empty())
        {
            ClearMachineRegistryModuleDir();
        }
        else{
            SetMachineRegistryModuleDir(savedMachineRegistryModuleDir);
        }

        if(savedUserRegistryConfigDir.empty())
        {
            ClearUserRegistryConfigDir();
        }
        else{
            SetUserRegistryConfigDir(savedUserRegistryConfigDir);
        }

        if(savedMachineRegistryConfigDir.empty()) {
            ClearMachineRegistryConfigDir();
        }
        else {
            SetMachineRegistryConfigDir(savedMachineRegistryConfigDir);
        }

#endif
        RestoreEnchantUserFiles();
        CleanUpFiles();
    }
    void CleanUpFiles()
    {
        //clean up personal dictionaries from home dir
        DeleteDirAndFiles(GetTempUserEnchantDir());
        DeleteDirAndFiles(AddToPath(GetDirectoryOfThisModule(), "lib"));
        DeleteDirAndFiles(AddToPath(GetDirectoryOfThisModule(), "share"));
    }

    std::string GetTempUserEnchantDir()
    {
        return GetEnchantHomeDirFromBase(GetDirectoryOfThisModule());
    }

    void MoveEnchantUserFilesOutOfTheWay()
    {
        GetFilesOutOfTheWay(GetEnchantHomeDirFromBase(g_get_user_config_dir()));
        GetFilesOutOfTheWay(GetEnchantHomeDirFromBase(g_get_home_dir()));
    }

    void RestoreEnchantUserFiles()
    {
        RestoreFiles(GetEnchantHomeDirFromBase(g_get_user_config_dir()));
        RestoreFiles(GetEnchantHomeDirFromBase(g_get_home_dir()));
    }

#define OUT_OF_THE_WAY ".real";

    void GetFilesOutOfTheWay(const std::string& dir)
    {
        std::string toTheSideDir = dir + OUT_OF_THE_WAY;
        
        if(DirExists(dir) && !DirExists(toTheSideDir))
        {
            MoveDir(dir, toTheSideDir);
        }

        DeleteDirAndFiles(dir);
    }

    void RestoreFiles(const std::string& dir)
    {
        std::string toTheSideDir = dir + OUT_OF_THE_WAY;
        if(DirExists(toTheSideDir))
        {
            DeleteDirAndFiles(dir);
            MoveDir(toTheSideDir, dir);
        }
    }

    std::string GetEnchantConfigDir()
    {
        return AddToPath(AddToPath(GetDirectoryOfThisModule(), "share"), "enchant");
    }

    static bool DirExists(const std::string& dir)
    {
        return g_file_test(dir.c_str(), G_FILE_TEST_IS_DIR) != 0;
    }

    static void MoveDir(const std::string& from, const std::string& to)
    {
        int result = g_rename(from.c_str(), to.c_str());
        assert(result);
        if(result)
        {
           perror("failed");
        }
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
        g_mkdir_with_parents(filepath.c_str(), S_IREAD | S_IWRITE | S_IEXEC);
    }
    static void CreateFile(const std::string& filepath)
    {
        int fh = g_creat(filepath.c_str(), _S_IREAD | _S_IWRITE);
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
        gchar* str = g_utf16_to_utf8((gunichar2*)ws.c_str(), (glong)ws.length(), NULL, NULL, NULL);
        std::string s(str);
        g_free(str);
        return s;
    }

    std::wstring Convert(const std::string & s)
    {
        gunichar2* str = g_utf8_to_utf16(s.c_str(), (glong)s.length(), NULL, NULL, NULL);
        std::wstring ws((wchar_t*)str);
        g_free(str);
        return ws;
    }

    std::string GetDirectoryOfThisModule()
    {
        std::string result;
#if defined(_WIN32)
        // should be in the same directory as our test fixture
        WCHAR szFilename[MAX_PATH];
        GetModuleFileName(NULL, (LPWSTR) &szFilename, sizeof(szFilename));
        PathRemoveFileSpec((LPWSTR)&szFilename);

        result = Convert(szFilename);
#elif defined(ENABLE_BINRELOC) 
        gchar* prefix = gbr_find_prefix(NULL);
        result = std::string(prefix);
        g_free(prefix);
#endif
        return result;
      }

    std::string GetRegistryHomeDir()
    {
        return Convert(GetRegistryValue(HKEY_CURRENT_USER, L"Software\\Enchant\\Config", L"Home_Dir"));
    }

    void SetRegistryHomeDir(const std::string & dir)
    {
        SetRegistryValue(HKEY_CURRENT_USER, L"Software\\Enchant\\Config", L"Home_Dir", dir);
    }

    void ClearRegistryHomeDir()
    {
        ClearRegistryValue(HKEY_CURRENT_USER, L"Software\\Enchant\\Config", L"Home_Dir");
    }

    std::string GetUserRegistryModuleDir()
    {
        return Convert(GetRegistryValue(HKEY_CURRENT_USER, L"Software\\Enchant\\Config", L"Module_Dir"));

    }

    void SetUserRegistryModuleDir(const std::string & dir)
    {
        SetRegistryValue(HKEY_CURRENT_USER, L"Software\\Enchant\\Config", L"Module_Dir", dir);
    }

    void ClearUserRegistryModuleDir()
    {
        ClearRegistryValue(HKEY_CURRENT_USER, L"Software\\Enchant\\Config", L"Module_Dir");
    }

    std::string GetMachineRegistryModuleDir()
    {
        return Convert(GetRegistryValue(HKEY_LOCAL_MACHINE, L"Software\\Enchant\\Config", L"Module_Dir"));

    }

    void SetMachineRegistryModuleDir(const std::string & dir)
    {
        SetRegistryValue(HKEY_LOCAL_MACHINE, L"Software\\Enchant\\Config", L"Module_Dir", dir);
    }

    void ClearMachineRegistryModuleDir()
    {
        ClearRegistryValue(HKEY_LOCAL_MACHINE, L"Software\\Enchant\\Config", L"Module_Dir");
    }

    std::string GetUserRegistryConfigDir()
    {
        return Convert(GetRegistryValue(HKEY_CURRENT_USER, L"Software\\Enchant\\Config", L"Data_Dir"));
    }

    void SetUserRegistryConfigDir(const std::string & dir)
    {
        SetRegistryValue(HKEY_CURRENT_USER, L"Software\\Enchant\\Config", L"Data_Dir", dir);
    }

    void ClearUserRegistryConfigDir()
    {
        ClearRegistryValue(HKEY_CURRENT_USER, L"Software\\Enchant\\Config", L"Data_Dir");
    }

    std::string GetMachineRegistryConfigDir()
    {
        return Convert(GetRegistryValue(HKEY_LOCAL_MACHINE, L"Software\\Enchant\\Config", L"Data_Dir"));
    }

    void SetMachineRegistryConfigDir(const std::string & dir)
    {
        SetRegistryValue(HKEY_LOCAL_MACHINE, L"Software\\Enchant\\Config", L"Data_Dir", dir);
    }

    void ClearMachineRegistryConfigDir()
    {
        ClearRegistryValue(HKEY_LOCAL_MACHINE, L"Software\\Enchant\\Config", L"Data_Dir");
    }

    std::wstring GetRegistryValue(HKEY baseKey, const std::wstring& key, const std::wstring& valueName)
  {
      std::wstring result;

      WCHAR data[2048];
      DWORD dwSize = sizeof(data) * sizeof(WCHAR);
      HKEY hkey;

      if(RegOpenKeyEx(baseKey, 
                      key.c_str(), 
                      0, KEY_WRITE, &hkey) == ERROR_SUCCESS)
      {
         if(RegQueryValueEx(hkey, 
                            valueName.c_str(),
                            0, 
                            NULL,
                           (LPBYTE)&data, &dwSize) 
                                    == ERROR_SUCCESS){
            result = std::wstring(data,dwSize);
          }
      }
      return result;
  }

  void SetRegistryValue(HKEY baseKey, 
                       const std::wstring& key, 
                       const std::wstring& valueName, 
                       const std::string& value)
  {
      SetRegistryValue(baseKey, key, valueName, Convert(value));
  }
  void SetRegistryValue(HKEY baseKey, 
                       const std::wstring& key, 
                       const std::wstring& valueName, 
                       const std::wstring& value)
  {
    HKEY hkey;
    if(RegCreateKeyEx(baseKey, 
                      key.c_str(), 
                      0, NULL,
                      REG_OPTION_NON_VOLATILE,
                      KEY_WRITE,
                      NULL, &hkey,
                      NULL) == ERROR_SUCCESS)
    {
      RegSetValueEx(hkey, 
                    valueName.c_str(), 
                    0, 
                    REG_SZ, 
                    (LPBYTE)value.c_str(), 
                    (DWORD)((value.length()+1)*sizeof(wchar_t)) );
    
      RegCloseKey(hkey);
    }
  }

  void ClearRegistryValue(HKEY baseKey,
      const std::wstring& key,
      const std::wstring& valueName)
  {
    HKEY hkey;
    if(RegOpenKeyEx(baseKey, 
                    key.c_str(), 
                    0, KEY_WRITE, &hkey) == ERROR_SUCCESS)
    {
        RegDeleteValue(hkey, valueName.c_str());
        RegCloseKey(hkey);
    }
  }


  static std::string AddToPath(const std::string & path, const std::string & fileOrDirName)
    {
        std::string result;
        gchar* filename = g_build_filename(path.c_str(), fileOrDirName.c_str(), NULL);
        result = std::string(filename);
        g_free(filename);

        return result;
    }

    static std::string GetEnchantHomeDirFromBase(const std::string& basePath)
    {
#ifdef XP_TARGET_COCOA
        return AddToPath(AddToPath(AddToPath(basePath,"Library"), 
                                                       "Application Support"), 
                                                       "Enchant");
#elif defined(_WIN32)
        return AddToPath(basePath,"enchant");
#else
        return AddToPath(basePath,".enchant");
#endif
    }
};

#endif
