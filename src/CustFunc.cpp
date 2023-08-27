// CustFunc.cpp : Defines the exported functions for the DLL Add-in.
//

// #include <string>
// #include <sstream>
#include <iostream>
#include <cstdint>
#include <filesystem>

namespace fs = std::filesystem;

// Dialog Stuff
#include "resource.h"

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
bool cfDebug = false;
bool fileDebug = true;
static BOOL MC_Active = FALSE;    // TODO: Start off as TRUE since Mathcad is active when DLL Loads?
static char text[TEXTLENGTH];
int  nVirtKey;

typedef struct
{
    TCHAR funcName[40];
    TCHAR funcCall[40];
    TCHAR funcDesc[MAX_PATH];
} Function;

Function FuncList[] =
{
    { TEXT("realsum"), TEXT("realsum(a,b)"), TEXT("Calculates the sum of two real variables, a and b.") },
    { TEXT("transpose"), TEXT("transpose(X)"), TEXT("Calculates the transpose of a matrix, X.") }
};

TCHAR CatList[40] = TEXT("User Pack");

HHOOK hhk;
// HHOOK hhookMsg;
HINSTANCE hDLLglobal = 0;



//Function Templates
BOOL LoadDocs();    // Get DLL directory and the \docs diretory underneith it
INT_PTR CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK CFDlgProc(HWND, UINT, WPARAM, LPARAM);



/*******************************************************************************************************/
/*   Low-Level Keyboard Hook Call Back Process.                                                        */
/*                                                                                                     */
/*   Trap <Shift><F2> key pressed, but only if the "PTC Mathcad Prime*" window is active.              */
/*   If TRUE - Open the Custom Function Dialog Box through the Windows API,                            */
/*   Otherwise, ignore the keystrokes.                                                                 */
/*   ALWAYS pass the Windows Message on to the next Windows hook!                                      */
/*******************************************************************************************************/
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
 //   static HINSTANCE hInstance;

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
                ptcFound = std::strstr(wnd_title, "PTC Mathcad Prime");              // See if active window starts with "PTC Mathcad Prime"
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
                    
                    DialogBox(hDLLglobal, MAKEINTRESOURCE(IDD_CFDIALOG), hwnd, CFDlgProc);

                    // TODO: we can pass the function string to Mathcad from within the Dialog Box, or
                    //       we can return an index value from the the DialogBox and send the call
                    //       to Mathcad here.  However, the DialogBox may be non-modal, so we might
                    //       need to pass the info from within the Dialog Process (CFDlgProc).
                }
            }
        }
    }
    return CallNextHookEx(hhk, nCode, wParam, lParam);
}

/*******************************************************************************************************/
/*   About Box Dialog Process.                                                                         */
/*                                                                                                     */
/*   May not be used.  Not sure where to cleanly activate it if the user wants to know.                */
/*******************************************************************************************************/
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    /**/
    HWND hwndOwner;
    RECT rc, rcDlg, rcOwner;
    /**/

    switch (message)
    {
        case WM_INITDIALOG :
        {
            /**/
            hwndOwner = GetParent(hDlg);

            GetWindowRect(hwndOwner, &rcOwner);
            GetWindowRect(hDlg, &rcDlg);
            CopyRect(&rc, &rcOwner);

            // Offset the owner and dialog box rectangles so that right and bottom 
            // values represent the width and height, and then offset the owner again 
            // to discard space taken up by the dialog box. 

            OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
            OffsetRect(&rc, -rc.left, -rc.top);
            OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

            // The new position is the sum of half the remaining space and the owner's 
            // original position. 

            SetWindowPos(hDlg,
                HWND_TOP,
                rcOwner.left + (rc.right / 2),
                rcOwner.top + (rc.bottom / 2),
                0, 0,          // Ignores size arguments. 
                SWP_NOSIZE);

            /*   Not Needed.  Our Dialog is always modal, on top, and has focus.
            if (GetDlgCtrlID((HWND)wParam) != IDD_ABOUTBOX)
            {
                SetFocus(GetDlgItem(hDlg, IDD_ABOUTBOX));
                return FALSE;
            }
            */

            return TRUE;
        }

        case WM_COMMAND :
            switch (LOWORD(wParam))
            {
            case IDOK :
                EndDialog(hDlg, 1);
                return TRUE;
            }

        break;
    }
    return FALSE;
}

/*******************************************************************************************************/
/*   Custom Function Dialog Box - Dialog Process (Call Back Process).                                  */
/*                                                                                                     */
/*   Processes the user's Button clicks and List Box selections, making changes to the Dialog and/or   */
/*   closing the Dialog Box with any returning actions (like passing function strings to Mathcad).     */
/*******************************************************************************************************/
INT_PTR CALLBACK CFDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    /**/
    HWND hwndOwner;
    RECT rc, rcDlg, rcOwner;
    int iFindex = 0;
    /**/

    switch (message)
    {
    case WM_INITDIALOG:
    {
        /**/
        hwndOwner = GetParent(hDlg);

        GetWindowRect(hwndOwner, &rcOwner);
        GetWindowRect(hDlg, &rcDlg);
        CopyRect(&rc, &rcOwner);

        // Offset the owner and dialog box rectangles so that right and bottom 
        // values represent the width and height, and then offset the owner again 
        // to discard space taken up by the dialog box. 

        OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
        OffsetRect(&rc, -rc.left, -rc.top);
        OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

        // The new position is the sum of half the remaining space and the owner's 
        // original position. 

        SetWindowPos(hDlg,
            HWND_TOP,
            rcOwner.left + (rc.right / 2),
            rcOwner.top + (rc.bottom / 2),
            0, 0,          // Ignores size arguments. 
            SWP_NOSIZE);

        // Add items to the Category List
        HWND hwndCList = GetDlgItem(hDlg, IDC_LISTCAT);    // Get the Category ListBox handle
        int pos = (int)SendMessage(hwndCList, LB_ADDSTRING, 0, (LPARAM)CatList);
        SendMessage(hwndCList, LB_SETCURSEL, 0, 0);

        // Add items to the Function List
        HWND hwndFList = GetDlgItem(hDlg, IDC_LISTFUNC);   // Get the Function ListBox handle
        SendMessage(hwndFList, WM_SETREDRAW, FALSE, 0);
        for (int i = 0; i < ARRAYSIZE(FuncList); i++)
        {
            int pos = (int)SendMessage(hwndFList, LB_ADDSTRING, 0, (LPARAM)FuncList[i].funcName);
            SendMessage(hwndFList, LB_SETITEMDATA, pos, (LPARAM)i);
        };
        SendMessage(hwndFList, LB_SETCURSEL, 0, 0);        // Set Selector to first Function item
        SendMessage(hwndFList, WM_SETREDRAW, TRUE, 0);

        //HWND hwndFCall = GetDlgItem(hDlg, IDC_EDITFUNC);   // Get Edit Control handle for Call String
        //Set Text for the Function Call Edit Control (IDC_EDITFUNC) to the first function call string
        SetWindowText(GetDlgItem(hDlg, IDC_EDITFUNC), (LPCSTR)FuncList[0].funcCall);

        //HWND hwndFDesc = GetDlgItem(hDlg, IDC_EDITDESC);   // Get Edit Control handle for Desc String
        //Set Text for the Descritption Edit Control (IDC_EDITDESC) to the first function description string
        SetWindowText(GetDlgItem(hDlg, IDC_EDITDESC), (LPCSTR)FuncList[0].funcDesc);

        // Set input focus to the Category List Box
        SetFocus(hwndCList);

        /*
        if (GetDlgCtrlID((HWND)wParam) != IDD_ABOUTBOX)
        {
            SetFocus(GetDlgItem(hDlg, IDD_ABOUTBOX));
            return FALSE;
        }
        */

        return TRUE;
    }

    case WM_COMMAND:                 // User clicked on something in the Dialog Box.
        switch (LOWORD(wParam))      // Let's see what it was and what we should do about it.
        {
        case IDC_LISTFUNC:           // Made selection in the Functions ListBox (IDC_LISTFUNC)
            if (HIWORD(wParam) == LBN_SELCHANGE)            // If they made a change in selection,
            {                                               // Get the index of the selected item
                iFindex = (int)SendMessage(GetDlgItem(hDlg, IDC_LISTFUNC), LB_GETCURSEL, 0, 0);
                                                            // Update function call and description fields
                SetWindowText(GetDlgItem(hDlg, IDC_EDITFUNC), (LPCSTR)FuncList[iFindex].funcCall);
                SetWindowText(GetDlgItem(hDlg, IDC_EDITDESC), (LPCSTR)FuncList[iFindex].funcDesc);
            }
            return FALSE;   // return FALSE (0) - Local action, don't close the window.

        case IDINSERT:
        case IDCANCEL:                // User selected the Cancel button, do nothing and
            EndDialog(hDlg, 1);       // close the Dialog Box.
            return TRUE;
        }

        break;
    }
    return FALSE;
}

/*******************************************************************************************************/
/*   Stub Process to see if passed filesystem::path is an XML file; returns TRUE or FALSE              */
/*******************************************************************************************************/
bool isXML(const fs::path& p)
{
    std::string ext = p.extension().string();
    for (auto& c : ext) c = toupper(c);
    return (ext == ".XML");
}

/*******************************************************************************************************/
/*   Load the XML Docs if they exist.                                                                  */
/*                                                                                                     */
/*   First check and see if there is a "docs" directory under "Mathcad Prime X.X.X.X/Custom Functions" */
/*   IF there are XML files in the "docs" directory                                                    */
/*       For Each XML file                                                                             */
/*           Open each one and process with TinyXML2                                                   */
/*           For Each Function                                                                         */
/*               Get name, local_name, params, category, and description                               */
/*               Load into ListBox data structure                                                      */
/*           END For Loop                                                                              */
/*       END For Loop                                                                                  */
/*   END IF                                                                                            */
/*******************************************************************************************************/
BOOL LoadDocs()    // Get DLL directory and the \docs diretory underneith it
{
    TCHAR DllPath[MAX_PATH] = { 0 };

    if (GetModuleFileName(hDLLglobal, DllPath, _countof(DllPath)) != 0)     // Can we get this DLL's path?
    {

        fs::path fsName = DllPath;   // Assign DllPath string to an fs::path variable
        fs::path docsPath = fsName.parent_path().append("docs");  // This is the path we are looking for

        // Build and display debug message box.  Can be deleted later.
        if (cfDebug)
        {
            std::string msg = "DLL Name = \n";
            msg.append(fsName.string());
            msg.append("\n\nPath = \n");
            msg.append(fsName.parent_path().string());
            msg.append("\n\nDocs Path = \n");
            msg.append(docsPath.string());
            MessageBox(hwndDlg, msg.c_str(), "Getting DLL Directory", 0);
        }

        long int mbIcon = 0;
        std::string dbgmsg = "";
        dbgmsg.append("The docs directory");
        if ( fs::exists(docsPath) && fs::is_directory(docsPath))
        {
            mbIcon = MB_ICONASTERISK;
            long int fc = (long int)std::count_if(fs::directory_iterator(docsPath), {}, isXML);
            if (fc < 1)
            {
                dbgmsg.append(", \n\n").append(docsPath.string()).append("\n\n contains no XML files.");
                mbIcon = MB_ICONERROR;
            }
            else
            {
                dbgmsg.append(" contains ").append(std::to_string(fc)).append((fc > 1) ? " XML files:" : " XML file:");
                for(const auto& entry : fs::directory_iterator(docsPath))
                {
                    if (isXML(entry.path()))
                    {
                        dbgmsg.append("\n\t").append(entry.path().filename().string());

                    }
                }

            }
        }
        else
        {
            dbgmsg.append(", \n\n").append(docsPath.string()).append("\n\n... does not Exist!");
            mbIcon = MB_ICONERROR;
        }
        MessageBox(hwndDlg, dbgmsg.c_str(), "Getting DLL docs Directory", mbIcon);
    }
    else
    {
        return false;  // Return false if we can't get the path of the current DLL, but just in case...
    }

    // TODO: [x] Look for XML files in docsPath
    //       Open them one at a time With TinyXML2
    //       Build List Box struct based on XML content found in docsPath


    return true;
}

// ************************************************************************************
// DLL entry point code.  
// This code loads/removes the DLL and installed keyboard hooks  
// ************************************************************************************
// Since we have our own DLL Entry point (not DLLMain) we must call the _CRT_INIT function manually
// to initialize the C Runtime library.  Ignore the IntelliSense warning on _CRT_INIT not found.
#ifdef _WIN32 
    extern "C" BOOL WINAPI _CRT_INIT(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpReserved);
#endif

extern "C" BOOL WINAPI  DllEntryPoint (HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            //
             // DLL is attaching to the address space of the current process.
             //
            hDLLglobal = hDLL;

            if (!_CRT_INIT(hDLL, dwReason, lpReserved))
            {
                return FALSE;
            }

            //
            // Register the Error Message Table
            // 
            // **** Note: Normally we would register this table, but are not adding any Mathcad functions
            // ****       here and will be throwing no errors to the user, so this step is skipped.
            // if ( !CreateUserErrorMessageTable( hDLL, NUMBER_OF_ERRORS, CPErrorMessageTable ) )
            //    break;

            // Register User Function(s)
            // ***  NOTE: Again, this DLL will probably not need any user functions,
            // ***        but instead, will load any XML docs found under Custom Functions
            // ***        and install the Keyboard hook here when the DLL Process is attached.

            // Get DLL directory and the \docs diretory underneith it
            if (!LoadDocs()) break;    // If error, just break here and don't load the keyboard hooks.

            //
            // Attach the Keyboard Hook here and register the Keyboard Hook Callback Process
            //
            if (cfDebug) MessageBox(hwndDlg, "Installing Hooks.", "CustFunc Add-In", 0);
            if (!bhooked) {
                hhk = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)LowLevelKeyboardProc, hDLL, 0);
                bhooked = true;
            }
            break;
        }

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


    

