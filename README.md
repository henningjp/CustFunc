# CustFunc add-in DLL for PTC Mathcad Prime

While Mathcad Prime does contain a Functions panel for inserting built-in functions, it does not allow for incorporation of categorized Custom Functions into that interface.

<details>
<summary><b>Background</b><br><br></summary>
In the very early versions of Mathcad Prime, PTC carried forward the Legacy Mathcad capability of allowing users to write Custom Functions for Mathcad through a compiled C++ DLL.  These DLLs are placed in the ``Custom Functions`` directory and loaded when Mathcad Prime is launched.  In fact, C++ code for legacy Mathcad Custom Functions (formerly UserEFI functions) can be recompiled as Mathcad Prime Custom Functions with no change the the code (just by linking to the appropriate Mathcad Prime libraries).  This is a great feature of Mathcad Prime since many individuals and companies have built and rely function libraries for Legacy Mathcad. 

Each Custom Function DLL contains the mathematical coding for each function to be added.  Additionally, a ``FUNCTIONINFO`` structure is supplied that tells Mathcad Prime the address of each callable function, its callable "Mathcad" name, its parameter list, the types of each parameter, and return value type.  When loaded by Mathcad Prime, the first thing the DLL does is register a table of possible error messages and all of the ``FUNCTIONINFO`` structures so that the added functions can be called through the Mathcad interface.

However, one legacy feature that has still not been integrated into Mathcad Prime, is the ability to provide companion XML files that integrate Custom Functions documentation into the Mathcad interface, adding them to the list of internal functions under the Insert Functions panel under their own Function Category. 
</details>

<details>
<summary><b>Why Is This A Problem?</b><br><br></summary>

For DLLs containing a large number of Custom Functions, this means that the user has to know: 

1.	that the functions are loaded, 
2.	the purpose and description of each function, 
3.	the correct syntax to manually type in each function, 
4.	the type, number, and meaning of the parameters required by each function. 

This information has to be memorized or looked up in a reference document outside of Mathcad Prime for every additional function and seriously degrades the usefulness of the Custom Functions capability.  
</details>

# The CustFunc Add-in v1.2

The **CustFunc** add-in DLL does not actually register any new functions, but provides interface integration functionality for any other Custom Function DLLs that are loaded.  Mathcad Prime's built-in functions, as mentioned above, can be typed into a math region or inserted from the **_Functions_** panel.  The **_Functions_** panel is opened from Functions ribbon (the All Functions button) or by pressing the hot-key, `<F2>`.  **CustFunc** uses the hot-key `<F3>` to launch an **_Insert Custom Function_** dialog box, providing the user with a list of categorized Custom Functions for insertion onto the active worksheet at the current cursor location.  The **_Insert Custom Function_** dialog box will be populated with any XML function files found in Mathcad Prime's installation directory under `"Custom Functions\docs"`.

![Insert Custom Functions Panel](https://github.com/henningjp/CustFunc/blob/master/images/CustFuncPanel.png)

This modal dialog box is patterned after the legacy `Insert Function` panel.  In addition to the categorized list of functions and the ability to insert them into any worksheet, when the user browses to and selects a Function Name from the list, the actual function text (including expected parameters) is displayed on the dialog followed by a detailed text description of the functions behavior and expected parameters.

# Creating XML Function Files

A number of sample XML function files can be found in the `SampleXML` directory, including `UserPack.xml`, which can be used as a template for creating your own function descriptions.  The format of the XML files is simple and demonstrated in this listing of `UserPack.xml`.

<details>
<summary><b>XML File Format</b><br><br></summary>

--- 

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- Custom Function file for the Mathcad Prime Demo User Pack functions found in the -->
<!--    installed Custom Functions folder.                                            -->
<!-- To use, compile and build the UserPack demo functions and place the userpack.DLL -->
<!--   in the Custom Functions folder. Then  place this XML file in the Mathcad Prime -->
<!--installation directory under "Custom Functions/docs/".                            -->
<FUNCTIONS>
    <!-- Below are the demo UserPack functions -->
    <!-- First function: realsum -->
    <function>
        <name>realsum</name>                <!-- Function Name used in the dialog box  -->
        <local_name>realsum</local_name>    <!-- Actual function name to be inserted   -->
        <params>a,b</params>                <!-- Parameter list                        -->
        <category>User Pack</category>      <!-- Category under which to list function -->
        <description>Calculates the sum of two real variables, a and b.</description>
    </function>
    <!-- Second function: transpose -->
    <function>
        <name>transpose</name>
        <local_name>transpose</local_name>
        <params>A</params>
        <category>User Pack</category>
        <description>Calculates the transpose of a matrix, A.</description>
    </function>
</FUNCTIONS>
```

As many functions as needed for a specific DLL can be loaded in this XML file, typically under the same `<category>` description.  Each set of functions for distinct DLLs should use a different `<category>` to keep each DLL set small and separate.  This format is 100% compatible with Legacy Mathcad XML files, which can be used directly with **CustFunc**.

> **_NOTE:_** The first line is required and allows extended character sets to be used in the tag elements.

> **_NOTE:_** The `<name>` tag can contain a "user friendly" name and will be inserted into the **CustFunc** dialog box in the Function Name ListBox.  The `<local_name>` is the actual function syntax that will be inserted on the Mathcad Prime worksheet.  These names can be the same.  If `<local_name>` is omitted, the `<name>` tag will be used as the actual function syntax. 

---

</details>

A number of sample XML files are provided for a few Mathcad Prime add-in custom function libraries.  These include function listings for: 
- **CoolProp** - Fluid properties library found at https://github.com/CoolProp/CoolProp
- **CoolProp/IF97** - Water/Steam properties library found at https://github.com/CoolProp/IF97
- **NIST REFPROP** - Fluid/Mixture properties library found at https://github.com/usnistgov/REFPROP-wrappers

XML files for other add-ins can be added to this sample set.

# Sample XML Files

Sample XML files are located in the `\sampleXML` folder of this repository.  They include:  

   |     **File**          | Units | Description                                         |
   |-----------------------|:-----:|-----------------------------------------------------|
   | **CoolProp_EN.xml**   | SI    | _For use with /CoolProp add-in_                     |
   | **if97_SI_EN.xml**    | SI    | _Use for if97 with default SI units per /CoolProp/IF97_<br>_Accesses `if97_Units_SI.mcdx` include file_ |
   | **if97_IAPWS_EN.xml** | IAPWS | _use for IF97 compiled with `IAPWS\_UNITS` defined_<br>_Accesses `if97_Units_IAPWS.mcdx` include file_<br>_(IAPWS units are modified SI with MPa and kJ.)_ |
   | **Refprop_EN.xml**    | modSI | _for use with /Refprop add-in_<br>_(modSI units are modified SI with MPa and kJ.)_ |
   | **UserPack.xml**      | -     | _Trivial example for accessing Mathcad's UserPack example_ |


# CustFunc Enhancements

There are a few enhancements that CustFunc offers over legacy Insert Function behavior.  These include:

- The extended character set, including greek symbols (e.g. α, ε, Δ, etc.) and other technical symbols, operators, and diacritical marks, can be entered into the function name, local_name, and/or description.  The easiest way to enter these symbols is by copying them from the Microsoft Character Map application and pasting into the XML sub-elements. 
- Textual subscripts can be entered by embedding a period in the function's <local_name> element. 
- Constant values can be input with no parameters by omitting the `<parameters>` element or using a `<parameter>` value of "const". For example:
   ```XML
  <function>
    <name>Constants: Critical Temperature</name>
    <local_name>T.c</local_name>
    <params>const</params>
    <category>IF97 (Water Properties) w/ Units</category>
    <description>Returns the Temperature [K] at the critical point for water as defined in "IAPWS-IF97".\nNOTE: Mathcad units handling on inputs and return value - included in "if97_Units.mcdx".</description>
  </function>
   ```
- Include (Reference) Worksheets can be inserted by providing the `<local_name>` element as `INCLUDE` and providing the worksheet name to be included in the `<params>` element.  If only a file name is given, *_CustFunc_* looks for the file in the \docs directory with the rest of the XML files.  Alternatively, a fully qualified path can be provided to another local or shared filesystem location.  For example:
   ```XML
  <function>
    <name>Add if97_Units_SI.mcdx Include File</name>
    <local_name>INCLUDE</local_name>
    <!-- If no path is provided in the filename below, CustFunc will look for it in -->
    <!-- <Mathcad Prime>"/Custom Functions/docs"                                    -->
    <params>if97_Units_SI.mcdx</params>
    <category>IF97 (Water Properties) w/ Units</category>
    <description>Include worksheet if97_Units.mcdx from docs directory or shared area if path is provided in XML.\nThe worksheet MUST be included in order to use the remaining functions in this category.</description>
  </function>
   ```

These enhancements allow XML files to be provided not only for companion DLLs, but also for included worksheets that contain user functions and constants (entered with a `<parameter>` element of "const").

# Installing CustFunc

To install CustFunc in your local Mathcad Prime installation:  

1. In the right column of this repository, open the latest Release page.
2. Download the `CustFunc.dll` to your local machine (typically in Downloads directory)
3. Copy the `CustFunc.dll` file to Mathcad Prime's `Custom Functions` directory.
4. Downlaod any of the sample XML files from the repository or extract them from the SourceCode.zip file for this release.
5. Copy sample XML files (or create your own custom XML files) into the `Custom Functions\docs` directory (create this `\docs` directory if it doesn't already exist).
6. Copy frequently used XMCD include files into the `Custom Functions\docs` directory or edit the XML files to insert them using full path to another local or shared location.
7. Restart Mathcad Prime.
8. On any worksheet, press `<F3>` to pop up the **Insert Custom Functions** panel.  At least one XML file must exist in the `docs` directory or an error message will pop up indicating that no XML files were found.

# Use of XMCD Include Files

CustFunc works with **Include Worksheet** functions in addition to Custom Function DLL's and can now use XML entries that automatically insert the include statement directly into a worksheet with the path to the desired include file (without having to remember and/or browse to its stored location every time).  The example IF97 and REFPROP XML files above will work with the .xmcd include files found in their respective repositories to provide Mathcad wrapper functions that:
1. Call their respective add-in DLL Custom Functions
2. Provide more standard math notation for the function names
3. Handle input values with units and convert them to the units requried by the Custom Functions in the add-in DLL
4. Apply Mathcad units to the return values from the Custom Functions in the add-in DLL  

Accessed Include Worksheets to date are listed here:  

   |     **File**              | Units | Description                                         |
   |---------------------------|:-----:|-----------------------------------------------------|
   | **if97_Units_SI.mcdx**    | SI    | _Use for if97 with default SI units per /CoolProp/IF97_<br>_Accesses `if97_Units_SI.mcdx` include file_ |
   | **if97_Units_IAPWS.mcdx** | IAPWS | _use for IF97 compiled with `IAPWS\_UNITS` defined_<br>_Accesses `if97_Units_IAPWS.mcdx` include file_<br>_(IAPWS units are modified SI with MPa and kJ.)_ |
   | **RefProp_units.mcdx**    | modSI | _for use with the Mathcad /Refprop add-in_                      |

These include files are available in their respective add-in repositories on GitHub; [CoolProp/IF97](https://github.com/CoolProp/IF97) and [usnistgov/REFPROP-wrappers](https://github.com/usnistgov/REFPROP-wrappers).


# Participating

Contributions to this code repository are welcome and encouraged through:  

* Issue reporting under Issues
* Discussions about usage and Code improvement suggestions
* Collaboration through Pull Requests (PRs) with new features/enhancements/fixes
* Addition of sample XML files for any other public add-in DLLs

# Change Log

- **v1.1** [ 07/24/24 ]
   - Switched hot key from `<Shift><F2>` to `<F3>`.  Original hot-key combination conflicted with hot key used by Citrix when running Mathcad Prime in a Citrix virtual environment.  `<F3>` is not used by Mathcad Prime and is right next to the hot key for Mathcad's built-in functions (`<F2>`).

- **v1.2** [ 08/15/24 ]
  - Added ability to insert Include Worksheets through the **CustFunc** interface, allowing reference worksheets to be included from a specified local or shared filesystem location.
  - Updated the example if97_EN.XML file and split into two files:  
    **if97_SI_EN.XML** &emsp;&emsp;&emsp;&emsp; _(use for default SI units per /CoolProp/IF97)_  
    **if97_IAPWS_EN.XML** &emsp;&emsp; _(use if IF97 is compiled with IAPWS\_UNITS defined)_  
