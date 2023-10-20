# CustFunc add-in DLL for PTC Mathcad Prime

While Mathcad Prime does contain a Functions panel for inserting built-in functions, it does not allow for incorporation of categorized Custom Functions into that interface.

<details>
<summary><b>Background</b><br><br></summary>
In the very early versions of Mathcad Prime, PTC carried forward the Legacy Mathcad capability of allowing users to write Custom Functions for Mathcad through a compiled C++ DLL.  These DLLs are placed in the ``Custom Functions`` directory and loaded when Mathcad Prime is launched.  In fact, C++ code for legacy Mathcad Custom Functions (formerly UserEFI functions) can be recompiled as Mathcad Prime Custom Functions with no change the the code (just by linking to the appropriate Mathcad Prime libraries).  This is a great feature of Mathcad Prime since many individuals and companies have built and rely function libraries for Legacy Mathcad. 

Each Custom Function DLL contains the mathematical coding for each function to be added.  Additionally, a a ``FUNCTIONINFO`` structure is supplied that tells Mathcad Prime the address of each callable function, its callable "Mathcad" name, its parameter list, the types of each parameter, and return value type.  When loaded by Mathcad Prime, the first thing the DLL does is register a table of possible error messages and all of the ``FUNCTIONINFO`` structures so that the added functions can be called through the Mathcad interface.

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

# The CustFunc Add-in

The **CustFunc** add-in DLL does not actually register any new functions, but provides interface integration functionality for any other Custom Function DLLs that are loaded.  The built-in functions, as mentioned above, can be typed into a math region or inserted from the **_Functions_** panel.  The **_Functions_** panel is opened from Functions ribbon (the All Functions button) or by pressing the hot-key, `<F2>`.  **CustFunc** uses the hot-key combination `<Shift><F2>` to launch an **_Insert Custom Function_** dialog box, providing the user with a list of categorized Custom Functions for insertion onto the active worksheet at the current cursor location.  The **_Insert Custom Function_** dialog box will be populated with any XML function files found in Mathcad Prime's installation directory under `"Custom Functions\docs"`.

![Insert Custom Functions Panel](https://github.com/henningjp/CustFunc/blob/master/images/CustFuncPanel.png)

This modal dialog box is patterned after the legacy `Insert Function` panel.  In addition to the categorized list of functions and the ability to insert them into any worksheet, when the user browsed to and selects Function Name from the list, the actual function text (including expected parameters) is displayed on the dialog followed by a detailed text description of the functions behavior and expected parameters.

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

A number of sample XML files is provided for a few Mathcad Prime add-in custom function libraries.  These include function listings for: 
- **CoolProp** - Fluid properties library found at https://github.com/CoolProp/CoolProp
- **CoolProp/IF97** - Water/Steam properties library found at https://github.com/CoolProp/IF97
- **NIST REFPROP** - Fluid/Mixture properties library found at https://github.com/usnistgov/REFPROP-wrappers

XML files for other add-ins can be added to this sample set.

# CustFunc Enhancements

There are a few enhancements that CustFunc offers over legacy behavior.  These include:

- The extended character set, including greek symbols (e.g. α, ε, Δ, etc.) and other technical symbols, operators, and diacritical marks, can be entered into the function name, local_name, and/or description.  The easiest way to enter these symbols is by copying them from the Microsoft Character Map application and pasting into the XML sub-elements. 
- Textual subscripts can be entered by embedding a period in the function's <local_name> element. 
- Constant values can be input with no parameters by omitting the <parameters> element including a parameter of "const". 

These allow XML files to be provided not only for companion DLLs, but also for included worksheets that contain user functions and constants (entered with a <parameter> element of "const").

# Installing CustFunc

To install CustFunc in your local Mathcad Prime installation:  

1. Download the latest release zip file from this repository.
2. Unzip the release file.
3. Copy the `CustFunc.dll` file to Mathcad Prime's `Custom Functions` directory.
4. Copy sample XML files (or custom XML files) into a `Custom Functions\docs` directory.
5. Restart Mathcad Prime.
6. On any worksheet, press `<Shift><F2>` to pop up the **Insert Custom Functions** panel 

# Code Enhancements

Contributions to this code repository are welcome and encouraged through:  

* Issue reporting under Issues
* Code enhancement suggestions and/or pull requests
* Addition of sample XML file for any other public add-in DLLs
