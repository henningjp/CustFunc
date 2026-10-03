// CustFunc version number.
//
// This is the ONE place to change the version.  It is included by both the C++ source
// (CustFunc.cpp, for the dialog title) and the version resource (CustFuncVersion.rc,
// for the file Properties > Details tab).  Keep this file simple: only #define lines,
// because the resource compiler also reads it.
#pragma once

#define CF_VERSION_MAJOR 1
#define CF_VERSION_MINOR 3
#define CF_VERSION_PATCH 1

// Numeric form used by the VERSIONINFO FILEVERSION/PRODUCTVERSION fields: 1,3,1,0
#define CF_VERSION_NUMERIC CF_VERSION_MAJOR,CF_VERSION_MINOR,CF_VERSION_PATCH,0

// String form: "1.3.1"
#define CF_STR2(x) #x
#define CF_STR(x) CF_STR2(x)
#define CF_VERSION_STRING CF_STR(CF_VERSION_MAJOR.CF_VERSION_MINOR.CF_VERSION_PATCH)
