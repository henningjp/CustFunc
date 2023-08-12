// CustFunc.cpp : Defines the exported functions for the DLL Add-in.
//

// #include <string>
// #include <sstream>

// Dialog Stuff
//#include "CustFuncPicker.h"

#ifndef NOMINMAX // Kill windows' horrible min() and max() macros
#define NOMINMAX
#endif
#include "MCADINCL.H"
#undef NOMINMAX 

enum { MC_STRING = STRING };  // substitute enumeration variable MC_STRING for STRING, use MC_STRING below
#undef STRING                 // undefine STRING as it may conflict with STRING in other includes

// #include <stdlib.h>
// #include <stdio.h>
#include <string>
// #include "winuser.h"
// #include <iostream>
// #include <fstream>

// RefProp Mathcad Add-in Version
std::string CFVersion = "0.1";       // Mathcad Add-in version number

// Setup Dialog Window for debugging
HWND hwndDlg;  // Dialog handle for pop-up message boxes

enum EC { MUST_BE_REAL = 1, 
          UNKNOWN, 
          NUMBER_OF_ERRORS };                                       // Dummy Code for Error Count

    // table of error messages
    // if user function never returns an
    // error -- you do not need to create this
    // table
char * CFErrorMessageTable[NUMBER_OF_ERRORS] =
{
    (char *)("Argument must be real"),                      //  MUST_BE_REAL
    (char *)("Unknown Error"),                              //  UNKNOWN
    (char *)("Error Count - Not Used")                      //  NUMBER_OF_ERRORS
};

#define SHIFTED 0x8000
#define TEXTLENGTH 10

bool ctrlDown = false;
bool shiftDown = false;
bool bhooked = false;
bool cfDebug = true;
static BOOL MC_Active = FALSE;    // TODO: Start off as TRUE since Mathcad is active when DLL Loads?
static char text[TEXTLENGTH];
int  nVirtKey;

HHOOK hhk;
HHOOK hhookMsg;
/*******************************************************************************************************/
/*   This was an initial attempt from an internet example.  It is not used and can probably go.        */
/*******************************************************************************************************/
/*
LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HCBT_ACTIVATE)
    {
        GetClassName((HWND)wParam, text, TEXTLENGTH);
        //TODO: Pop the text string in a MessageBox to verify the Mathcad Window Class Name

        //TODO: Use enough character checks to create unique class name comparison
        if (text[0] == 'M' && text[1] == 'A')	//Class name of Mathcad begins with MA?  PTC?
        {
            MC_Active = TRUE;
        }
        else
            MC_Active = FALSE;
    }

    return CallNextHookEx(hhookMsg, nCode, wParam, lParam);
}
*/

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if ( nCode == HC_ACTION ) {  // Only process if nCode == HC_ACTION (0) == change in keyboard key state
        PKBDLLHOOKSTRUCT hookStruct = (PKBDLLHOOKSTRUCT) lParam;
        // Check if any key was pressed - KEYDOWN
        if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN) {
            // Check if F2 pressed and <Shift> key is also down; gets shift key state on the fly.
            if (hookStruct->vkCode == VK_F2 && (GetKeyState(VK_SHIFT) & SHIFTED)) {
                char wnd_title[256];
                HWND hwnd = GetForegroundWindow();                     // get handle of currently active window
                GetWindowText(hwnd, wnd_title, sizeof(wnd_title));     // get title of the window
                char *ptcFound = NULL;
                ptcFound = std::strstr(wnd_title, "PTC Mathcad Prime");              // See if active window starts with "PTC"
                if (ptcFound) {                                        // If yes...
                    // Only if Mathcad Window is active, Pop dialog here.
                    if (cfDebug)
                    {
                        std::string msg;
                        msg = "<Shift>-F2 was pressed.\n\n WindowClassName = ";
                        msg.append(wnd_title);                                    // append full title to Message
                        MessageBox(hwndDlg, msg.c_str(), "CustFunc Add-In", 0);   // Open Message box with msg
                    }
                    // TODO: If Mathcad Window is active, Pop Custom Function Dialog Box here.
                }
            }
        }
    }
    return CallNextHookEx(hhk, nCode, wParam, lParam);
}

// This code loads/removes the DLL and installed keyboard hooks  
// ************************************************************************************
// DLL entry point code.  
// ************************************************************************************
// Since we have our own DLL Entry point (not DLLMain) we must call the _CRT_INIT function manually
// to initialize the C Runtime library.
#ifdef _WIN32 
    extern "C" BOOL WINAPI _CRT_INIT(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpReserved);
#endif

// NOTE: The IntelliSense error "Function definition for '_CRT_INIT' not found." above can be ignored.
//       The code will compile just fine and find _CRT_INIT at compile time with linking with the C Run Time libraries

extern "C" BOOL WINAPI  DllEntryPoint (HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
           //
            // DLL is attaching to the address space of the current process.
            //
            if (!_CRT_INIT(hDLL, dwReason, lpReserved))
            {
                return FALSE;
            }
                
            // register the error message table
            // Note: if your function never returns an error -- you do not need to register an error message table
            // if ( !CreateUserErrorMessageTable( hDLL, NUMBER_OF_ERRORS, CPErrorMessageTable ) )
            //    break;

            // and if the errors register OK
            // go ahead and
            // register user function
            // ***  Probably not going to need any user functions ***
            // ***  But will load the Keyboard hook here, or at lease call the routine that does ***

            // Use this pop-up window for debugging if needed
            //===============================================================================
            // std::string msg;
            // msg = "We're here!";
            // msg.append("\n\nPath = ");
            // msg.append(RPPath_loaded);
            // MessageBox(hwndDlg, msg.c_str(), "CustFunc Add-In", 0);

            //
            // ATTACH THE CBT AND KEYBOARD HOOKS HERE
            //
            if (cfDebug) MessageBox(hwndDlg,"Installing Hooks.","CustFunc Add-In", 0);
            if (!bhooked) {
                hhk = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)LowLevelKeyboardProc, hDLL, 0);
                // hhookMsg = SetWindowsHookEx(WH_CBT, (HOOKPROC)CBTProc, hDLL, 0);      <== Not using this one.
                bhooked = true;
            }
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:

            if (!_CRT_INIT(hDLL, dwReason, lpReserved))
            {
                return FALSE;
            }
            break;

        case DLL_PROCESS_DETACH:

            if (!_CRT_INIT(hDLL, dwReason, lpReserved))
            {
                return FALSE;
            }

//            Sleep(1000);   // Attempt to keep CRT_INIT from detaching before all threads are closed

            // DETACH THE KEYBOARD HOOKS HERE
            if (bhooked) {
                if (UnhookWindowsHookEx(hhk)) {
                    bhooked = false;
                    // UnhookWindowsHookEx(hhookMsg);
                    if (cfDebug) MessageBox(hwndDlg, "Removing Keyboard Hooks.", "CustFunc Add-In", 0);
                }
            }
            break;                   
    }
    return TRUE;
}


    

