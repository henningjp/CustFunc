// CustFunc.cpp : Defines the exported functions for the DLL Add-in.
//

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <filesystem>
// Using TinyXML2 from: https://github.com/leethomason/tinyxml2 
// There's some new VC++ XML functionality, but TinyXML2 is far easier to use, for now.
#include <tinyxml2.h>

namespace fs = std::filesystem;

// Dialog Resource
#include "resource.h"

#ifndef NOMINMAX // Kill windows' horrible min() and max() macros
#define NOMINMAX
#endif
#include "MCADINCL.H"
#undef NOMINMAX 

enum { MC_STRING = STRING };  // substitute enumeration variable MC_STRING for STRING, use MC_STRING below
#undef STRING                 // undefine STRING as it may conflict with STRING in other includes


// RefProp Mathcad Add-in Version
std::string CFVersion = "0.1";       // Mathcad Add-in version number

// Setup Dialog Window for debugging
HWND hwndDlg;  // Generic Dialog handle for pop-up message boxes (MessageBox) when needed

//enum EC { MUST_BE_REAL = 1, 
//          UNKNOWN, 
//          NUMBER_OF_ERRORS };                                       // Dummy Code for Error Count

    // table of error messages
    // if user function never returns an
    // error -- you do not need to create this
    // table
//char * CFErrorMessageTable[NUMBER_OF_ERRORS] =
//{
//    (char *)("Argument must be real"),                      //  MUST_BE_REAL
//    (char *)("Unknown Error"),                              //  UNKNOWN
//    (char *)("Error Count - Not Used")                      //  NUMBER_OF_ERRORS
//};

#define SHIFTED 0x8000
#define TEXTLENGTH 10

bool ctrlDown = false;
bool shiftDown = false;
bool bhooked = false;
bool cfDebug = false;
bool fileDebug = true;
//static BOOL MC_Active = FALSE;    // TODO: Start off as TRUE since Mathcad is active when DLL Loads?

// Global Category and Function indices;
int iCategory;
int iFunction;
bool SendFunction = false;
// int  nVirtKey;

/***************************************************/
/*      Dynamically allocated ListBox Vectors      */
/***************************************************/

typedef struct
{
    std::wstring Name;
    std::wstring LocalName;
    std::wstring Params;
    std::wstring Description;
} FuncDef;

typedef std::vector<FuncDef> FuncVec;

typedef struct
{
    std::wstring CatName;
    FuncVec Functions;
} Category;

std::vector<Category> CatVec;



HHOOK hhk;
// HHOOK hhookMsg;
HINSTANCE hDLLglobal = 0;



//Function Templates
BOOL LoadDocs();    // Get DLL directory and the \docs directory underneath it
INT_PTR CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK CFDlgProc(HWND, UINT, WPARAM, LPARAM);
void SendAffine(wchar_t);



/*******************************************************************************************************/
/*   Send Affine Unit String to Mathcad                                                                */
/*                                                                                                     */
/*   Send Keystrokes (using SendInput) to PTC Mathcad Prime window to reproduce °F or °C unit symbols. */
/*   Calling function should pass a 'C' or 'F' character as needed.                                    */
/*******************************************************************************************************/
void SendAffine(wchar_t chFC)
{
    INPUT keys[20] = {};
    ZeroMemory(keys, sizeof(keys));
    
    // Now enter "°F or "°C in newly created math region *******************************************
    keys[0].type = INPUT_KEYBOARD;
    keys[0].ki.wVk = 0;
    keys[0].ki.wScan = L'"';                          // Double Quote
    keys[0].ki.dwFlags = KEYEVENTF_UNICODE;

    keys[1].type = INPUT_KEYBOARD;
    keys[1].ki.wVk = 0;
    keys[1].ki.wScan = L'"';                          // Double Quote (UP)
    keys[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;

    keys[2].type = INPUT_KEYBOARD;
    keys[2].ki.wVk = 0;
    keys[2].ki.wScan = 0x00B0;                        // Degree Sym
    keys[2].ki.dwFlags = KEYEVENTF_UNICODE;

    keys[3].type = INPUT_KEYBOARD;
    keys[3].ki.wVk = 0;
    keys[3].ki.wScan = 0x00B0;                        // Degree Sym (UP)
    keys[3].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;

     // Now send the 'C' or 'F' character from the parameter ***************************************
    keys[4].type = INPUT_KEYBOARD;
    //keys[8].ki.wVk = (isC) ? 'C' : 'F';             // (Not using vKey)
    keys[4].ki.wVk = 0;                               // Send UNICODE Instead (no Shift req'd)
    keys[4].ki.wScan = chFC;                          // 'C' or 'F' Char
    keys[4].ki.dwFlags = KEYEVENTF_UNICODE;

    keys[5].type = INPUT_KEYBOARD;
    //keys[8].ki.wVk = (isC) ? 'C' : 'F';             // (Not using vKey)
    keys[5].ki.wVk = 0;                               // Send UNICODE Instead (no Shift req'd)
    keys[5].ki.wScan = chFC;                          // 'C' or 'F' Char (UP)
    keys[5].ki.dwFlags = KEYEVENTF_KEYUP;

    // Issue KEYDOWN/KEYUP for <Control> & <Shift> as user is likely still holding them down. ********
    // Have to release both left and right, in case user is using keys on either side
    keys[6].type = INPUT_KEYBOARD;
    keys[6].ki.wVk = VK_RCONTROL;                      // Right <Ctrl>
    keys[6].ki.dwFlags = KEYEVENTF_EXTENDEDKEY;        // Right <Ctrl> key is actually an extended key
                                                       // dwFlags must be set to KEYEVENTF_EXTENDEDKEY

    keys[7] = keys[6];                                 // Right <Ctrl>
    keys[7].ki.dwFlags |= KEYEVENTF_KEYUP;             // (UP)   OR the KEYUP flag with EXTENDEDKEY

    keys[8].type = INPUT_KEYBOARD;
    keys[8].ki.wVk = VK_LCONTROL;                      // Left <Ctrl>

    keys[9] = keys[8];                                 // Left <Ctrl>
    keys[9].ki.dwFlags = KEYEVENTF_KEYUP;              // (UP)   set the KEYUP flag

    keys[10].type = INPUT_KEYBOARD;                    // 
    keys[10].ki.wVk = VK_RSHIFT;                       // Right <Shift>

    keys[11] = keys[10];                               // Right <Shift>
    keys[11].ki.dwFlags = KEYEVENTF_KEYUP;             // (UP)   set the KEYUP flag

    keys[12].type = INPUT_KEYBOARD;                    // 
    keys[12].ki.wVk = VK_LSHIFT;                       // Left <Shift>

    keys[13] = keys[12];                               // Left <Shift>
    keys[13].ki.dwFlags = KEYEVENTF_KEYUP;             // (UP)   set the KEYUP flag

    // Now send <DEL> key todelete the double quotes around the °X symbol **************************
    // Since the <Ctrl> and <Shift> keys are released above, this will actually work
    keys[14].type = INPUT_KEYBOARD;
    keys[14].ki.wVk = VK_DELETE;                       // <DEL>

    keys[15] = keys[14];                               // <DEL>
    keys[15].ki.dwFlags = KEYEVENTF_KEYUP;             // (UP)


    // Now Set Label to Unit with <Ctrl>U  *********************************************************
    keys[16].type = INPUT_KEYBOARD;
    keys[16].ki.wVk = VK_LCONTROL;                    // <Control>

    keys[17].type = INPUT_KEYBOARD;
    keys[17].ki.wVk = 'U';                            // 'U' (Down - lower case)

    keys[18].type = INPUT_KEYBOARD;
    keys[18].ki.wVk = 'U';                            // 'U' (UP - lower case)
    keys[18].ki.dwFlags = KEYEVENTF_KEYUP;

    keys[19].type = INPUT_KEYBOARD;
    keys[19].ki.wVk = VK_LCONTROL;                    // <Control> (UP)
    keys[19].ki.dwFlags = KEYEVENTF_KEYUP;

    UINT uSent = SendInput(ARRAYSIZE(keys), keys, sizeof(INPUT));
    if (uSent != ARRAYSIZE(keys))
    {
        MessageBox(hwndDlg, L"Send °C or °F to Mathcad Failed!", L"Keyboard Hook Process", 0);
    }

}

/*******************************************************************************************************/
/*   Send Function to Mathcad                                                                          */
/*                                                                                                     */
/*   Send Function Keystrokes (using SendInput) to PTC Mathcad Prime window.                           */
/*   Calling function passes the Mathcad window handle and the current category (iC) and function (if) */
/*   indices.                                                                                          */
/*******************************************************************************************************/

int SendFunction2Mathcad(HWND mcad, int iC, int iF)
{
    std::wstring FuncString = CatVec[iC].Functions[iF].LocalName;     // Get Function string
    std::wstring strp = CatVec[iC].Functions[iF].Params;
    for (auto& c : strp) c = toupper(c);                              // make temp UCase version of Params
    if (strp != L"CONST")                                             // if Params <> const
        FuncString.append(L"(").append(CatVec[iC].Functions[iF].Params);  // append parameters, no closing paren

    std::vector<INPUT> keys;         // Initialize Vector of INPUT keystrokes
    static bool quoteSet = false;    // Initialize quote pair tracking to false
    for (auto ch : FuncString)      // for each character in the Function String
    {
        switch (ch)
        {
            case L' ':               // Skip spaces.  Mathcad won't like them in a Math region.
                break;

            case L'.':               // Handle subscripts with a dot.  Send <Ctrl>- toggle
            {
                INPUT input = { 0 };                               // Initialize temp input stroke

                input.type = INPUT_KEYBOARD;
                input.ki.wVk = VK_LCONTROL;                        // Left <Ctrl>
                keys.push_back(input);

                input.ki.wVk = VK_SUBTRACT;                        // '-' key
                keys.push_back(input);

                input.ki.dwFlags = KEYEVENTF_KEYUP;                // '-' key (KEYUP)
                keys.push_back(input);

                input.ki.wVk = VK_LCONTROL;                        // Left <Ctrl>
                input.ki.dwFlags = KEYEVENTF_KEYUP;                // (UP)   set the KEYUP flag
                keys.push_back(input);
            }
            break;

            case L'"':
            if (quoteSet)                                      // This is a matching quote pair; Mathcad will have
            {                                                  //    already supplied it, so Right Arrow over it.

                INPUT input = { 0 };                               // Initialize temp input stroke

                input.type = INPUT_KEYBOARD;
                input.ki.wVk = VK_RIGHT;                           // '->' key
                input.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;          // It's an extended key on 101-keyboards
                keys.push_back(input);

                input.ki.dwFlags |= KEYEVENTF_KEYUP;               // '->' key (KEYUP)
                keys.push_back(input);

                quoteSet = false;                                  // clear flag and
                break;                                             // break out of switch
            }
            else                                               // This is a first quote, actually send it
            {
                quoteSet = true;                               //     set flag and fall through
                [[fallthrough]];
            }


            default:                 // Handle any other characters as a UNICODE wchar_t
            {
                INPUT input = { 0 };                               // Initialize temp input stroke

                input.type = INPUT_KEYBOARD;
                input.ki.dwFlags = KEYEVENTF_UNICODE;              // Send UNICODE Event (KEYDOWN)
                input.ki.wScan = ch;                               //   with current character, ch
                keys.push_back(input);

                input.ki.dwFlags |= KEYEVENTF_KEYUP;               // Send UNICODE Event (KEYUP)
                keys.push_back(input);
            }
        }   // Process next character in string...
    }  // FOR Loop over all characters in string

    if ((int)keys.size() > 0)
    {
        // Add two more right arror to keystrokes list to get to the end of the function string.
        // skip if entering a constant
        if (strp != L"CONST")
        {
            INPUT input = { 0 };
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = VK_RIGHT;                   // '->' key
            input.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;  // It's an extended key on 101-keyboards
            keys.push_back(input);
            input.ki.dwFlags |= KEYEVENTF_KEYUP;       // '->' key (KEYUP)
            keys.push_back(input);
            input.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;  // '->' key (Send extended KEYDOWN) again
            keys.push_back(input);
            input.ki.dwFlags |= KEYEVENTF_KEYUP;       // '->' key (KEYUP) again
            keys.push_back(input);
        }

        SetActiveWindow(mcad);                                              // Make sure user hasn't clicked away
        UINT uSent = SendInput((UINT)keys.size(), keys.data(), sizeof(INPUT));    // send input to current window

        if (uSent != (UINT)keys.size())                                     // if error
        {                                                                   //    pop a message to user
            FuncString.insert(0, L"Send UNICODE Function string, [").append(L"] to Mathcad Failed!");
            MessageBox(hwndDlg, FuncString.c_str(), L"Custom Function Dialog Process", 0);
            return 1;
        }                                                                   // NOTE: This should never happen
    }

    return 0;
}

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

    if ( nCode == HC_ACTION ) {  // Only process if nCode == HC_ACTION (0) == change in keyboard key state
        PKBDLLHOOKSTRUCT hookStruct = (PKBDLLHOOKSTRUCT) lParam;

        // A bit of code here to make sure that Mathcad Prime is the active window; Ignore keyboard otherwise.
        wchar_t * wnd_title = nullptr;                           // Default to empty window title
        wchar_t * ptcFound = NULL;                               // Default to PTC Mathcad not active
        HWND hwnd = GetForegroundWindow();                       // get handle of currently active window
        DWORD cTextLen = GetWindowTextLength(hwnd);              // get length of window title string
        if (cTextLen > 0)
        {
            cTextLen++;                                          // Add one for null terminator, just in case
            // Allocate memory for the string and compy the string into memory
            wnd_title = (PWSTR)VirtualAlloc((LPVOID)NULL, cTextLen, MEM_COMMIT, PAGE_READWRITE);
            if (wnd_title != NULL)                               // IF wnd_title not null,
            {
                GetWindowText(hwnd, wnd_title, cTextLen);        //         get title of the window: was size sizeof(wnd_title)
                ptcFound = std::wcsstr(wnd_title, L"PTC Mathcad Prime"); // See if active window starts with "PTC Mathcad Prime"
            }
            else
                wnd_title = L"<no title>";                       // otherwise, set to "<no title>"
        }
        if (ptcFound)                                            // If yes (not NULL)...
        {
            // Key checks below only processed if Mathcad Window is active
            switch (wParam)
            {
            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:            // Check if any key was pressed - KEYDOWN

            {

                // Check if F2 pressed and <Shift> key is also down; gets shift key state on the fly.
                if (hookStruct->vkCode == VK_F2 && (GetKeyState(VK_SHIFT) & SHIFTED)) {

                    // If Mathcad Window is active, Pop Custom Function Dialog Box here.
                    DialogBox(hDLLglobal, MAKEINTRESOURCE(IDD_CFDIALOG), hwnd, CFDlgProc);
                    //Get function string index set by DialogBox and SendInput to Mathcad window
                    if(SendFunction)                                                 // If user pressed Insert button
                        int ierr = SendFunction2Mathcad(hwnd, iCategory, iFunction); //    Send selected string to Mathcad
                    SendFunction = false;                                     // Reset SendFunction flag
                }

                // Check if <Ctrl><Shift>">" was pressed to insert "°F" at Mathcad Cursor Location
                if (hookStruct->vkCode == VK_OEM_PERIOD && (GetKeyState(VK_SHIFT) & SHIFTED) && (GetKeyState(VK_CONTROL) & SHIFTED))
                {
                    SendAffine(L'F');      // Send keystrokes for °F with unit label
                    return 1;
                }
                // Check if <Ctrl><Shift>"<" was pressed to insert "°C" at Mathcad Cursor Location
                if (hookStruct->vkCode == VK_OEM_COMMA && (GetKeyState(VK_SHIFT) & SHIFTED) && (GetKeyState(VK_CONTROL) & SHIFTED))
                {
                    SendAffine(L'C');      // Send keystrokes for °C with unit label
                    return 1;
                }
            }
            break;    // WM_SYSKEYDOWN and WM_KEYDOWN

            // Add other Message cases here if needed

            } // END switch (wParam)
        } // END ptcFound
    } // END HC_ACTION

    return CallNextHookEx(hhk, nCode, wParam, lParam);

}  // Callback 

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
    static int iFindex = 0, iCindex = 0;
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
        SendMessage(hwndCList, WM_SETREDRAW, FALSE, 0);    // Temporarily turn off Redraw
        for (int ic = 0; ic < (int)CatVec.size(); ic++)
        {
            int pos = (int)SendMessage(hwndCList, LB_ADDSTRING, 0, (LPARAM)CatVec[ic].CatName.c_str());
            SendMessage(hwndCList, LB_SETITEMDATA, pos, (LPARAM)ic);
        }
        SendMessage(hwndCList, WM_SETREDRAW, TRUE, 0);     // Turn Redraw back on
        SendMessage(hwndCList, LB_SETCURSEL, iCindex, 0);  // Set Selector to last Category item clicked (init 0)
        iCategory = (int)SendMessage(hwndCList, LB_GETITEMDATA, iCindex, 0);

        // Add items to the Function List
        HWND hwndFList = GetDlgItem(hDlg, IDC_LISTFUNC);   // Get the Function ListBox handle
        SendMessage(hwndFList, WM_SETREDRAW, FALSE, 0);    // Temporarily turn off Redraw
        for (int i = 0; i < (int)CatVec[iCategory].Functions.size(); i++)  // For all function in the first
        {
            int pos = (int)SendMessage(hwndFList, LB_ADDSTRING, 0, (LPARAM)CatVec[iCategory].Functions[i].Name.c_str());
            SendMessage(hwndFList, LB_SETITEMDATA, pos, (LPARAM)i);
        };
        SendMessage(hwndFList, WM_SETREDRAW, TRUE, 0);     // Turn Redraw back on
        SendMessage(hwndFList, LB_SETCURSEL, iFindex, 0);  // Set Selector to last Function item clicked (init 0)
        iFunction = (int)SendMessage(hwndFList, LB_GETITEMDATA, iFindex, 0);

         //Set Text for the Function Call Edit Control (IDC_EDITFUNC) to the first function call string
        std::wstring tLocal = CatVec[iCategory].Functions[iFunction].LocalName;
        tLocal.append(L"(").append(CatVec[iCategory].Functions[iFunction].Params).append(L")");
        SetWindowText(GetDlgItem(hDlg, IDC_EDITFUNC), (LPCWSTR)tLocal.c_str());

       //Set Text for the Description Edit Control (IDC_EDITDESC) to the first function description string
        SetWindowText(GetDlgItem(hDlg, IDC_EDITDESC), (LPCWSTR)CatVec[iCategory].Functions[iFunction].Description.c_str());

        // Set input focus to the Category List Box
        SetFocus(hwndCList);

        return TRUE;
    }

    case WM_COMMAND:                 // User clicked on something in the Dialog Box.
        switch (LOWORD(wParam))      // Let's see what it was and what we should do about it.
        {
        case IDC_LISTCAT:            // Made selection in the Categories ListBox (IDC_LISCAT)
            if (HIWORD(wParam) == LBN_SELCHANGE)            // If they made a change in selection,
            {                                               // Get the index of the selected item
                iCindex = (int)SendMessage(GetDlgItem(hDlg, IDC_LISTCAT), LB_GETCURSEL, 0, 0);
                iCategory = (int)SendMessage(GetDlgItem(hDlg, IDC_LISTCAT), LB_GETITEMDATA, iCindex, 0);
                                                            // Update Function Listbox for new category
                HWND hwndFList = GetDlgItem(hDlg, IDC_LISTFUNC);   // Get the Function ListBox handle
                SendMessage(hwndFList, WM_SETREDRAW, FALSE, 0);    // Temporarily turn off Redraw
                SendMessage(hwndFList, LB_RESETCONTENT, 0, 0);     // Reset the contents of the Functions List Box
                for (int i = 0; i < (int)CatVec[iCategory].Functions.size(); i++)  // For all function in the first
                {                                                                  // ... add in the new category Functions
                    int pos = (int)SendMessage(hwndFList, LB_ADDSTRING, 0, (LPARAM)CatVec[iCategory].Functions[i].Name.c_str());
                    SendMessage(hwndFList, LB_SETITEMDATA, pos, (LPARAM)i);
                };
                SendMessage(hwndFList, WM_SETREDRAW, TRUE, 0);     // Turn Redraw back on
                SendMessage(hwndFList, LB_SETCURSEL, 0, 0);        // Set Selector to first Function item
                iFunction = (int)SendMessage(hwndFList, LB_GETITEMDATA, 0, 0);

                //Set Text for the Function Call Edit Control (IDC_EDITFUNC) to the first function call string
                std::wstring tLocal = CatVec[iCategory].Functions[iFunction].LocalName;
                tLocal.append(L"(").append(CatVec[iCategory].Functions[iFunction].Params).append(L")");
                SetWindowText(GetDlgItem(hDlg, IDC_EDITFUNC), (LPCWSTR)tLocal.c_str());

                //Set Text for the Description Edit Control (IDC_EDITDESC) to the first function description string
                SetWindowText(GetDlgItem(hDlg, IDC_EDITDESC), (LPCWSTR)CatVec[iCategory].Functions[iFunction].Description.c_str());

                // Set input focus to the Category List Box
                SetFocus(GetDlgItem(hDlg, IDC_LISTCAT));
            }
            return FALSE;
            break;

        case IDC_LISTFUNC:           // Made selection in the Functions ListBox (IDC_LISTFUNC)
            if (HIWORD(wParam) == LBN_SELCHANGE)            // If they made a change in selection,
            {                                               // Get the index of the selected item
                iFindex = (int)SendMessage(GetDlgItem(hDlg, IDC_LISTFUNC), LB_GETCURSEL, 0, 0);
                iFunction = (int)SendMessage(GetDlgItem(hDlg, IDC_LISTFUNC), LB_GETITEMDATA, iFindex, 0);
                                                            // Update function call and description fields
                std::wstring tLocal = CatVec[iCategory].Functions[iFunction].LocalName;
                tLocal.append(L"(").append(CatVec[iCategory].Functions[iFunction].Params).append(L")");
                SetWindowText(GetDlgItem(hDlg, IDC_EDITFUNC), (LPCWSTR)tLocal.c_str());
                SetWindowText(GetDlgItem(hDlg, IDC_EDITDESC), (LPCWSTR)CatVec[iCategory].Functions[iFunction].Description.c_str());
            }
            return FALSE;   // return FALSE (0) - Local action, don't close the window.
            break;


        // If IDINSERT pushed, Set flag that a function was selected and should be sent when
        //    the Dialog closes, then fall through to IDCANCEL, which closes the Dialog.
        //    When flag is set, each character in the function name and parameter string will 
        //    be sent as a WM_KEYDOWN/WM_KEYUP pair through the Win32 function SendInput.
        //    The keystrokes will be received by the active Mathcad Prime HWND.
        case IDINSERT:                // User selected the Insert Button
            SendFunction = true;      //    Set SendFunction flag and ...
            [[fallthrough]];          //    fall through to IDCANCEL
        case IDCANCEL:                // User selected the Cancel button, do nothing and
            EndDialog(hDlg, 1);       //    close the Dialog Box.
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
/*   XML Error Utility Function                                                                        */
/*   Pops a MessageBox with the Error Message from TinyXLM2 as set in the enumeration XMLError         */
/*******************************************************************************************************/
void PopXMLError(tinyxml2::XMLError errnum)
{
    std::wstring errmsg = L"XML File Error: ";
    switch (errnum)
    {
    case tinyxml2::XML_SUCCESS: errmsg.append(L"No Error"); break;
    case tinyxml2::XML_NO_ATTRIBUTE: errmsg.append(L"No Attribute"); break;
    case tinyxml2::XML_WRONG_ATTRIBUTE_TYPE: errmsg.append(L"Wrong Attribute Type"); break;
    case tinyxml2::XML_ERROR_FILE_NOT_FOUND: errmsg.append(L"File Not Found"); break;
    case tinyxml2::XML_ERROR_FILE_COULD_NOT_BE_OPENED: errmsg.append(L"File could not be opened"); break;
    case tinyxml2::XML_ERROR_FILE_READ_ERROR: errmsg.append(L"File Read Error"); break;
    case tinyxml2::XML_ERROR_PARSING_ELEMENT: errmsg.append(L"Error Parsing Element"); break;
    case tinyxml2::XML_ERROR_PARSING_ATTRIBUTE: errmsg.append(L"Error Parsing Attribute"); break;
    case tinyxml2::XML_ERROR_PARSING_TEXT: errmsg.append(L"Error Parsing Text"); break;
    case tinyxml2::XML_ERROR_PARSING_CDATA: errmsg.append(L"Error Parsing CData"); break;
    case tinyxml2::XML_ERROR_PARSING_COMMENT: errmsg.append(L"Error Parsing Comment"); break;
    case tinyxml2::XML_ERROR_PARSING_DECLARATION: errmsg.append(L"Error Parsing Declaration"); break;
    case tinyxml2::XML_ERROR_PARSING_UNKNOWN: errmsg.append(L"Unknown Parsing Error"); break;
    case tinyxml2::XML_ERROR_EMPTY_DOCUMENT: errmsg.append(L"Empty Document"); break;
    case tinyxml2::XML_ERROR_MISMATCHED_ELEMENT: errmsg.append(L"Mismatched Element"); break;
    case tinyxml2::XML_ERROR_PARSING: errmsg.append(L"Parsing Error"); break;
    case tinyxml2::XML_CAN_NOT_CONVERT_TEXT: errmsg.append(L"Cannot Convert Text"); break;
    case tinyxml2::XML_NO_TEXT_NODE: errmsg.append(L"No Text Node"); break;
    case tinyxml2::XML_ELEMENT_DEPTH_EXCEEDED: errmsg.append(L"Element Depth Exceeded"); break;
    default: errmsg.append(L"Unknown Error"); break;
    }
    MessageBox(hwndDlg, errmsg.c_str(), L"TinyXML2 Error", MB_ICONERROR);
    return;
}

/*******************************************************************************************************/
/*   Convert multi-byte utf8 char string to wide char string (utf-16)                                  */
/*                                                                                                     */
/*   TinyXML2 only reads UTF-8 encoded single byte characters.  ASCII version of Windows dialogs and   */
/*   message boxes do not handle this encoding in ASCII mode so we have to use the UNICODE versions,   */
/*   which only take pointers to wide character strings (wchar_t*) as inputs.  This means that we have */
/*   to convert the UTF-8 single byte strings to wide strings while preserving the UTF-8 encoding.     */
/*   Win32 provides a function for this, MutliByteToWideChar, but we've wrapped a function around it   */
/*   here to make it easier to use; counting characters, allocating memory, and returning the          */
/*   converted wide-string.                                                                            */
/*******************************************************************************************************/
/*   TODO: Might want to consider some pop-up error message boxes here. Currently just returns null.   */
wchar_t* utf8_to_wchar(const char* input)
{
    wchar_t* Buffer;                  // pointer to buffer to wide-character Buffer (null)
    int BuffSize = 0, Result = 0;     // Init Buffer and Result sizes

    BuffSize = MultiByteToWideChar(CP_UTF8, 0, input, -1, NULL, 0);  // get length of input char* string in "chars" (not bytes)
    Buffer = (wchar_t*)malloc(sizeof(wchar_t) * BuffSize);           // allocate number of characters needed to wide string
    if (Buffer)                                                      // IF the buffer size is > 0
    {
        Result = MultiByteToWideChar(CP_UTF8, 0, input, -1, Buffer, BuffSize); // Convert (char*)input to wide (wchar_t*)Buffer
    }
                                                                     //
    return ((Result > 0) && (Result <= BuffSize)) ? Buffer : NULL;   // IF non-zero length and we didn't overflow the Buffer
                                                                     //    return the wide string (wchar_t*)Buffer, otherwise null
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
/*               Convert UTF-8 MB strings to wide multi-byte strings                                   */
/*               Load into ListBox data structure                                                      */
/*           END For Loop                                                                              */
/*       END For Loop                                                                                  */
/*   END IF                                                                                            */
/*******************************************************************************************************/
BOOL LoadDocs()    // Get DLL directory and the \docs directory underneath it
{
    TCHAR DllPath[MAX_PATH] = { 0 };

    if (GetModuleFileName(hDLLglobal, DllPath, _countof(DllPath)) != 0)     // Can we get this DLL's path?
    {

        fs::path fsName = DllPath;   // Assign DllPath string to an fs::path variable
        fs::path docsPath = fsName.parent_path().append("docs");  // This is the path we are looking for

        // Build and display debug message box.  Can be deleted later.
        if (cfDebug)
        {
            std::wstring msg = L"DLL Name = \n";
            msg.append(fsName.wstring());
            msg.append(L"\n\nPath = \n");
            msg.append(fsName.parent_path().wstring());
            msg.append(L"\n\nDocs Path = \n");
            msg.append(docsPath.wstring());
            MessageBox(hwndDlg, msg.c_str(), L"Getting DLL Directory", 0);
        }

        std::wstring dbgmsg = L"";
        std::wstring xmlmsg = L"";
        std::wstring CatNew = L"";
        int iCat = -1;
        int iFunc = 0;
        FuncDef tFunc;            // This is a temporary Function Definition 
        FuncVec tFuncVec;         // This is a temporary Function Vector
        Category tCat = { L"USER", tFuncVec };   // This is a temporary Category description, initialized to "USER"
        std::string tempString;

        dbgmsg.append(L"The docs directory");
        if ( fs::exists(docsPath) && fs::is_directory(docsPath))                                   // if "Custom Functions\docs" directory found
        {
            long int fc = (long int)std::count_if(fs::directory_iterator(docsPath), {}, isXML);          // Count XML files in it

            if (fc < 1)                                                                                  // If no XML files found
            {
                dbgmsg.append(L", \n\n").append(docsPath.wstring()).append(L"\n\n contains no XML files.");
                MessageBox(hwndDlg, dbgmsg.c_str(), L"Getting DLL docs Directory", MB_ICONERROR);         //    Popup Error Message
            }                                                                                            //    User Needs to Know (maybe)
            else                                                                                         // Otherwise => XML Files Found
            {
                std::wstring::size_type n = 0;
                const std::wstring s = L"\\n";
                const std::wstring t = L"\r\n";

                dbgmsg.append(L" contains ").append(std::to_wstring(fc)).append((fc > 1) ? L" XML files:" : L" XML file:");

                tinyxml2::XMLDocument doc;                                                               // Create an XML Document structure

                for(const auto& entry : fs::directory_iterator(docsPath))                                // For Each File Found
                {
                    if (isXML(entry.path()))                                                             // If it's an XML File
                    {
                        tinyxml2::XMLError eResult = doc.LoadFile( entry.path().string().c_str());         // Load the XML File into XMLDocument
                        if (eResult != tinyxml2::XML_SUCCESS)
                            PopXMLError(eResult);                                                          //   Pop an Error Message if not successful

                        tinyxml2::XMLElement* p_root_element = doc.RootElement();                          // This is the <FUNCTIONS> Tag
                        tinyxml2::XMLElement* p_function = p_root_element->FirstChildElement("function");  // First <function> Tag

                        while (p_function)                                                                 // While <function> Tag valid
                        {
                            if(NULL != p_function->FirstChildElement("name"))                              // If <name> Tag exists
                            {                                                                                 // Get <name> text
                                tFunc.Name = utf8_to_wchar(p_function->FirstChildElement("name")->GetText()); // Convert utf8 to wchar_t & assign to wstring
                            }
                            else                                                                           // otherwise...
                            {                                                                              //     Skip this function entry
                                p_function = p_function->NextSiblingElement("function");                       //    Get the next function
                                continue;                                                                      //    Continue with next loop
                            }                                                                              // END <name> Tag

                            if (NULL != p_function->FirstChildElement("local_name"))                       // if <local_name> Tag exists
                                tFunc.LocalName = utf8_to_wchar(p_function->FirstChildElement("local_name")->GetText());  // Get <local_name> text (convert from utf-8)
                            else                                                                           // Otherwise...
                                tFunc.LocalName = tFunc.Name;                                              //    Set <local_name> to <name>
                                                                                                           // END <local_name> Tag

                            if (NULL != p_function->FirstChildElement("params"))                           // if <params> Tag exists
                                tFunc.Params = utf8_to_wchar(p_function->FirstChildElement("params")->GetText()); // Get <params> text (convert from utf-8)
                            else                                                                           // otherwise...
                                tFunc.Params = L"0";                                                       //        Assume function takes (0)

                            if (NULL != p_function->FirstChildElement("description"))                      // if <description> Tag exists
                            {
                                tFunc.Description = utf8_to_wchar(p_function->FirstChildElement("description")->GetText()); // Get <description> text
                                n = 0;
                                while ((n = tFunc.Description.find(s, n)) != std::wstring::npos)
                                {
                                    tFunc.Description.replace(n, s.size(), t);
                                    n += t.size();
                                }
                            }
                            else
                                tFunc.Description = L"<no description provided>";

                            if (NULL != p_function->FirstChildElement("category"))                         // if <category> Tag exists
                                CatNew = utf8_to_wchar(p_function->FirstChildElement("category")->GetText()); //    Get last read <category> text
                            else                                                                           // otherwise
                                CatNew = tCat.CatName;                                                     //       Assume previous function category (or default)

                            if (CatNew != tCat.CatName)                                                    //    If category changed
                            {
                                iCat++;                                                                    //      increment counter
                                tFuncVec.clear();                                                          //      clear out temp function vector
                                tFuncVec.push_back(tFunc);                                                 //      add new function to vector
                                tCat.CatName = CatNew;                                                     //      put last read category in temp struct
                                tCat.Functions = tFuncVec;                                                 //      put new func vector in temp struct
                                CatVec.push_back(tCat);                                                    //      add new category to Vector
                             }
                            else                                                                           //    Otherwise
                            {
                                CatVec[iCat].Functions.push_back(tFunc);                                   //      Just add new function to current category
                            }
                            p_function = p_function->NextSiblingElement("function");                       //    Get the next function
                        }                                                                                  // Until there are no more functions in the file
                    }                                                                                   // ENDIF isXML?
                }                                                                                    // END FOR Directory iterator

                for (int ic = 0; ic < CatVec.size(); ic++)                                           // Build Debug message with all functions read (in case we need it)
                {
                    for (int i = 0; i < CatVec[ic].Functions.size(); i++)
                    {
                        dbgmsg.append(L"\n\t(").append(CatVec[ic].CatName).append(L")\t");
                        dbgmsg.append(CatVec[ic].Functions[i].Name);
                    }
                }
                if (cfDebug) MessageBox(hwndDlg, dbgmsg.c_str(), L"Getting DLL docs Directory", 0);  // IF cfDebug flag is true, pop-up debug message box
            }
        }
        else
        {
            dbgmsg.append(L", \n\n").append(docsPath.wstring()).append(L"\n\n... does not Exist!");  // Pop-up message box if docs directory does not exist.
            MessageBox(hwndDlg, dbgmsg.c_str(), L"Getting DLL docs Directory", MB_ICONERROR);

        }
    }
    else
    {
        return false;  // Return false if we can't get the path of the current DLL, but just in case...
    }

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

            // Get DLL directory and the \docs directory underneath it
            if (!LoadDocs()) break;    // If error, just break here and don't load the keyboard hooks.

            //
            // Attach the Keyboard Hook here and register the Keyboard Hook Callback Process
            //
            if (cfDebug) MessageBox(hwndDlg, L"Installing Hooks.", L"CustFunc Add-In", 0);
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

            // DETACH THE KEYBOARD HOOKS HERE WHEN Mathcad Prime SHUTS DOWN
            if (bhooked) {
                if (UnhookWindowsHookEx(hhk)) {
                    bhooked = false;
                    if (cfDebug) MessageBox(hwndDlg, L"Removing Keyboard Hooks.", L"CustFunc Add-In", 0);
                }
            }
            break;                   
    }
    return TRUE;
}


    

