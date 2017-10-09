// TorController.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <iostream>

#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_array.hpp>
#include <boost/tokenizer.hpp>

#include "Tools/StringUtils/UnicodeTextProcessing.h"

#include "ControllerWebSvc.h"


#include "Main.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

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

    std::vector<boost::shared_array<char>> cmdParts;

    boost::shared_array<char> namePart(new char[strFileName.length() + 1]);
    strcpy(namePart.get(), strFileName.c_str());
    cmdParts.push_back(namePart);

    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

    boost::char_separator<char> sep(" \t");
    tokenizer tokens(strCmdLine, sep);
    BOOST_FOREACH(const std::string &token, tokens)
    {
        boost::shared_array<char> part(new char[token.length() + 1]);
        strcpy(part.get(), token.c_str());
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



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TORCONTROLLER));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDC_TORCONTROLLER);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;

    hInst = hInstance; // Store instance handle in our global variable

    hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
        case WM_COMMAND:
            wmId = LOWORD(wParam);
            wmEvent = HIWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
                case IDM_ABOUT:
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                    break;
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code here...
            EndPaint(hWnd, &ps);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        case WM_INITDIALOG:
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}
