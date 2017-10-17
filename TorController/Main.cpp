// TorController.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <iostream>

#include <boost/algorithm/string/replace.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_array.hpp>

#include "Tools/StringUtils/UnicodeTextProcessing.h"

#include "ControllerWebSvc.h"

#include "Main.h"

//-------------------------------------------------------------------------------------------------
std::string getExeName()
{
    unsigned nameBufLen = 1024;
    boost::scoped_array<TCHAR> nameBuf(new TCHAR[nameBufLen + 1]);
    nameBuf[nameBufLen] = 0;

    unsigned const fileNameLen = GetModuleFileName(NULL, nameBuf.get(), nameBufLen);
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        nameBuf.reset(new TCHAR[fileNameLen + 1]);
        nameBuf[fileNameLen] = 0;

        if (!GetModuleFileName(NULL, nameBuf.get(), fileNameLen))
        {
            return "";
        }
    }
    UnicodeString ustrFileName(nameBuf.get());
    std::string strFileName;
    ustrFileName.toUTF8String(strFileName);

    return strFileName;
}

//-------------------------------------------------------------------------------------------------
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPTSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    ControllerWebSvc webSvc;
   
    const std::string strFileName = getExeName();
    if (strFileName.empty())
    {
        std::cerr << "Can't get module file name" << std::endl;
        return -1;
    }

    std::string strCmdLine;
    UnicodeString(lpCmdLine).toUTF8String(strCmdLine);
    boost::replace_all(strCmdLine, "\\", "/");

    std::vector<std::string> args = boost::program_options::split_unix(strCmdLine);

    std::vector<boost::shared_array<char>> cmdParts;

    boost::shared_array<char> namePart(new char[strFileName.length() + 1]);
    strcpy(namePart.get(), strFileName.c_str());
    cmdParts.push_back(namePart);

    for (const std::string &arg : args)
    {
        boost::shared_array<char> part(new char[arg.length() + 1]);
        strcpy(part.get(), arg.c_str());
        cmdParts.push_back(part);
    }

    boost::scoped_array<char*> argv(new char*[cmdParts.size() + 1]);
    for (unsigned i = 0; i < cmdParts.size(); ++i)
    {
        argv[i] = cmdParts[i].get();
    }
    argv[cmdParts.size()] = NULL;

    return webSvc.run(cmdParts.size(), argv.get());
}
