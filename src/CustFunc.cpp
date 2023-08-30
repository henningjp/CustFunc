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
HWND hwndDlg;  // Dialog handle for pop-up message boxes

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
static BOOL MC_Active = FALSE;    // TODO: Start off as TRUE since Mathcad is active when DLL Loads?
static char text[TEXTLENGTH];
int  nVirtKey;
int iCategory;
int iFunction;

/***************************************************/
/*      Dynamically allocated ListBox Vectors      */
/***************************************************/

typedef struct
{
    std::string Name;
    std::string LocalName;
    std::string Params;
    std::string Description;
} FuncDef;

typedef std::vector<FuncDef> FuncVec;

typedef struct
{
    std::string CatName;
    FuncVec Functions;
} Category;

std::vector<Category> CatVec;



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
                    // If Mathcad Window is active, Pop Custom Function Dialog Box here.
                    
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
    int iFindex = 0, iCindex = 0;
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
        SendMessage(hwndCList, LB_SETCURSEL, 0, 0);        // Set default to the first item in the list
        iCategory = (int)SendMessage(hwndCList, LB_GETITEMDATA, 0, 0);

        // Add items to the Function List
        HWND hwndFList = GetDlgItem(hDlg, IDC_LISTFUNC);   // Get the Function ListBox handle
        SendMessage(hwndFList, WM_SETREDRAW, FALSE, 0);    // Temporarily turn off Redraw
        for (int i = 0; i < (int)CatVec[0].Functions.size(); i++)  // For all function in the first
        {
            int pos = (int)SendMessage(hwndFList, LB_ADDSTRING, 0, (LPARAM)CatVec[iCategory].Functions[i].Name.c_str());
            SendMessage(hwndFList, LB_SETITEMDATA, pos, (LPARAM)i);
        };
        SendMessage(hwndFList, WM_SETREDRAW, TRUE, 0);     // Turn Redraw back on
        SendMessage(hwndFList, LB_SETCURSEL, 0, 0);        // Set Selector to first Function item
        iFunction = (int)SendMessage(hwndFList, LB_GETITEMDATA, 0, 0);

         //Set Text for the Function Call Edit Control (IDC_EDITFUNC) to the first function call string
        std::string tLocal = CatVec[iCategory].Functions[iFunction].LocalName;
        tLocal.append("(").append(CatVec[iCategory].Functions[iFunction].Params).append(")");
        SetWindowText(GetDlgItem(hDlg, IDC_EDITFUNC), (LPCSTR)tLocal.c_str());

       //Set Text for the Descritption Edit Control (IDC_EDITDESC) to the first function description string
        SetWindowText(GetDlgItem(hDlg, IDC_EDITDESC), (LPCSTR)CatVec[iCategory].Functions[iFunction].Description.c_str());

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
                std::string tLocal = CatVec[iCategory].Functions[iFunction].LocalName;
                tLocal.append("(").append(CatVec[iCategory].Functions[iFunction].Params).append(")");
                SetWindowText(GetDlgItem(hDlg, IDC_EDITFUNC), (LPCSTR)tLocal.c_str());

                //Set Text for the Descritption Edit Control (IDC_EDITDESC) to the first function description string
                SetWindowText(GetDlgItem(hDlg, IDC_EDITDESC), (LPCSTR)CatVec[iCategory].Functions[iFunction].Description.c_str());

                // Set input focus to the Category List Box
                SetFocus(GetDlgItem(hDlg, IDC_LISTCAT));
            }
            return FALSE;
            break;

        case IDC_LISTFUNC:           // Made selection in the Functions ListBox (IDC_LISTFUNC)
            if (HIWORD(wParam) == LBN_SELCHANGE)            // If they made a change in selection,
            {                                               // Get the index of the selected item
                iFindex = (int)SendMessage(GetDlgItem(hDlg, IDC_LISTFUNC), LB_GETCURSEL, 0, 0);
                int iFunction = (int)SendMessage(GetDlgItem(hDlg, IDC_LISTFUNC), LB_GETITEMDATA, iFindex, 0);
                                                            // Update function call and description fields
                std::string tLocal = CatVec[iCategory].Functions[iFunction].LocalName;
                tLocal.append("(").append(CatVec[iCategory].Functions[iFunction].Params).append(")");
                SetWindowText(GetDlgItem(hDlg, IDC_EDITFUNC), (LPCSTR)tLocal.c_str());
                SetWindowText(GetDlgItem(hDlg, IDC_EDITDESC), (LPCSTR)CatVec[iCategory].Functions[iFunction].Description.c_str());
            }
            return FALSE;   // return FALSE (0) - Local action, don't close the window.
            break;


        //TODO: If IDINSERT pushed, pass selected function LocalName(plus params template) to Mathcad at the
        //      cursor location.  We can do this by
        //      1. Placing string in CLIPBOARD and sending Mathcad a WM_PASTE message
        //         (Probs require capturing and replacing an user data on CLIPBOARD)
        //      2. Send each character in the function name as a WM_KEYDOWN/WM_KEYUP pair through
        //         the Windows messaging queue targeting the Mathcad Prime HWND (more tedious?
        //         Might not support extended character set if needed.)
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
/*   XML Error Utility Function                                                                        */
/*   Pops a MessageBox with the Error Message from TinyXLM2 as set in the enumeration XMLError         */
/*******************************************************************************************************/
void PopXMLError(tinyxml2::XMLError errnum)
{
    std::string errmsg = "XML File Error: ";
    switch (errnum)
    {
    case tinyxml2::XML_SUCCESS: errmsg.append("No Error"); break;
    case tinyxml2::XML_NO_ATTRIBUTE: errmsg.append("No Attribute"); break;
    case tinyxml2::XML_WRONG_ATTRIBUTE_TYPE: errmsg.append("Wrong Attribute Type"); break;
    case tinyxml2::XML_ERROR_FILE_NOT_FOUND: errmsg.append("File Not Found"); break;
    case tinyxml2::XML_ERROR_FILE_COULD_NOT_BE_OPENED: errmsg.append("File could not be opened"); break;
    case tinyxml2::XML_ERROR_FILE_READ_ERROR: errmsg.append("File Read Error"); break;
    case tinyxml2::XML_ERROR_PARSING_ELEMENT: errmsg.append("Error Parsing Element"); break;
    case tinyxml2::XML_ERROR_PARSING_ATTRIBUTE: errmsg.append("Error Parsing Attribute"); break;
    case tinyxml2::XML_ERROR_PARSING_TEXT: errmsg.append("Error Parsing Text"); break;
    case tinyxml2::XML_ERROR_PARSING_CDATA: errmsg.append("Error Parsing CData"); break;
    case tinyxml2::XML_ERROR_PARSING_COMMENT: errmsg.append("Error Parsing Comment"); break;
    case tinyxml2::XML_ERROR_PARSING_DECLARATION: errmsg.append("Error Parsing Declaration"); break;
    case tinyxml2::XML_ERROR_PARSING_UNKNOWN: errmsg.append("Unknown Parsing Error"); break;
    case tinyxml2::XML_ERROR_EMPTY_DOCUMENT: errmsg.append("Empty Document"); break;
    case tinyxml2::XML_ERROR_MISMATCHED_ELEMENT: errmsg.append("Mismatched Element"); break;
    case tinyxml2::XML_ERROR_PARSING: errmsg.append("Parsing Error"); break;
    case tinyxml2::XML_CAN_NOT_CONVERT_TEXT: errmsg.append("Cannot Convert Text"); break;
    case tinyxml2::XML_NO_TEXT_NODE: errmsg.append("No Text Node"); break;
    case tinyxml2::XML_ELEMENT_DEPTH_EXCEEDED: errmsg.append("Element Depth Exceeded"); break;
    default: errmsg.append("Unknown Error"); break;
    }
    MessageBox(hwndDlg, errmsg.c_str(), "TinyXML2 Error", MB_ICONERROR);
    return;
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
// TODO: Extended character set (including ² & ³ ) are misrepresented in dialog box.  Probably need to
//       convert to wchar (UNICODE Font) to handle the extra bits either when reading in the strings
//       from XML or using DialogBoxW to handle these UNICODE strings in the Dialog Box correclty.
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

        std::string dbgmsg = "";
        std::string xmlmsg = "";
        std::string CatLast = "";
        int iCat = -1;
        int iFunc = 0;
        FuncDef tFunc;
        FuncVec tFuncVec;
        Category tCat = { "empty", tFuncVec };

        dbgmsg.append("The docs directory");
        if ( fs::exists(docsPath) && fs::is_directory(docsPath))                                   // if "Custom Functions\docs" direcotry found
        {
            long int fc = (long int)std::count_if(fs::directory_iterator(docsPath), {}, isXML);          // Count XML files in it

            if (fc < 1)                                                                                  // If no XML files found
            {
                dbgmsg.append(", \n\n").append(docsPath.string()).append("\n\n contains no XML files.");
                MessageBox(hwndDlg, dbgmsg.c_str(), "Getting DLL docs Directory", MB_ICONERROR);         //    Popup Error Message
            }                                                                                            //    User Needs to Know (maybe)
            else                                                                                         // Otherwise => XML Files Found
            {
                dbgmsg.append(" contains ").append(std::to_string(fc)).append((fc > 1) ? " XML files:" : " XML file:");

                tinyxml2::XMLDocument doc;                                                               // Create an XML Document structure

                for(const auto& entry : fs::directory_iterator(docsPath))                                // For Each File Found
                {
                    if (isXML(entry.path()))                                                             // If it's an XML File
                    {
                        tinyxml2::XMLError eResult = doc.LoadFile( entry.path().string().c_str());         // Load the XML File into XMLDocument
                        if (eResult != tinyxml2::XML_SUCCESS)
                            PopXMLError(eResult);                                                          //   Pop an Error Message if not succeessful

                        tinyxml2::XMLElement* p_root_element = doc.RootElement();                          // This is the <FUNCTIONS> Tag
                        tinyxml2::XMLElement* p_function = p_root_element->FirstChildElement("function");  // First <function> Tag

                        while (p_function)                                                                 // While <function> Tag valid
                        {
                            tFunc.Name = p_function->FirstChildElement("name")->GetText();                 //    Get <name> text
                            tFunc.LocalName = p_function->FirstChildElement("local_name")->GetText();      //    Get <local_name> text
                            tFunc.Params = p_function->FirstChildElement("params")->GetText();             //    Get <params> text
                            tFunc.Description = p_function->FirstChildElement("description")->GetText();   //    Get <description> text
                            CatLast = p_function->FirstChildElement("category")->GetText();                //    Get last read <category> text

                            if (CatLast != tCat.CatName)                                                   //    If category changed
                            {
                                iCat++;                                                                    //      increment counter
                                tFuncVec.clear();                                                          //      clear out temp function vector
                                tFuncVec.push_back(tFunc);                                                 //      add new function to vector
                                tCat.CatName = CatLast;                                                    //      put last read category in temp struct
                                tCat.Functions = tFuncVec;                                                 //      put new func vector in temp struct
                                CatVec.push_back(tCat);                                                    //      add new category to Vector
                             }
                            else                                                                           //    Otherwise
                            {
                                CatVec[iCat].Functions.push_back(tFunc);                                   //      Just add new function to current category
                            }
                            p_function = p_function->NextSiblingElement("function");                       //    Get the next function
                        }                                                                                  // Until there are no more functions in the file
                    }
                }
                for (int ic = 0; ic < CatVec.size(); ic++)
                {
                    for (int i = 0; i < CatVec[ic].Functions.size(); i++)
                    {
                        dbgmsg.append("\n\t(").append(CatVec[ic].CatName).append(")\t");
                        dbgmsg.append(CatVec[ic].Functions[i].Name);
                    }
                }
                if (cfDebug) MessageBox(hwndDlg, dbgmsg.c_str(), "Getting DLL docs Directory", 0);
            }
        }
        else
        {
            dbgmsg.append(", \n\n").append(docsPath.string()).append("\n\n... does not Exist!");
            MessageBox(hwndDlg, dbgmsg.c_str(), "Getting DLL docs Directory", MB_ICONERROR);

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

            // DETACH THE KEYBOARD HOOKS HERE WHEN Mathcad Prime SHUTS DOWN
            if (bhooked) {
                if (UnhookWindowsHookEx(hhk)) {
                    bhooked = false;
                    if (cfDebug) MessageBox(hwndDlg, "Removing Keyboard Hooks.", "CustFunc Add-In", 0);
                }
            }
            break;                   
    }
    return TRUE;
}


    

