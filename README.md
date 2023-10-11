# CustFunc add-in DLL for Mathcad Prime

In the very early versions of Mathcad Prime, PTC carried forward the Legacy Mathcad capability of allowing users to write Custom Functions for Mathcad through a compiled C++ DLL.  These DLLs are placed in the ``Custom Functions`` directory and loaded when Mathcad Prime is launched.  In fact, C++ code for legacy Mathcad Custom Functions (formerly UserEFI functions) can be recompiled as Mathcad Prime Custom Functions with no change the the code (just by linking to the appropriate Mathcad Prime libraries).  This is a great feature of Mathcad Prime since many individuals and companies have built and rely function libraries for Legacy Mathcad. 

Each Custom Function DLL contains the mathematical coding for each function to be added.  Additionally, a a ``FUNCTIONINFO`` structure is supplied that tells Mathcad Prime the address of each callable function, its callable "Mathcad" name, its parameter list, the types of each parameter, and return value type.  When loaded by Mathcad Prime, the first thing the DLL does is register a table of possible error messages and all of the ``FUNCTIONINFO`` structures so that the added functions can be called through the Mathcad interface.

However, one legacy feature that has still not been integrated into Mathcad Prime, is the ability to provide companion XML files that integrate Custom Functions documentation into the Mathcad interface, adding them to the list of internal functions under the Insert Functions panel under their own Function Category. 

While Mathcad Prime does contain a **_Functions_** panel for inserting built-in functions, it does not allow for incorporation of categorized Custom Functions into that interface.  For DLLs containing a large number of Custom Functions, this means that the user has to know: 

1.	that the functions are loaded, 
2.	the purpose and description of each function, 
3.	the correct syntax to manually type in each function, 
4.	the type, number, and meaning of the parameters required by each function. 

This information has to be memorized or looked up in a reference document outside of Mathcad Prime for every additional function and seriously degrades the usefulness of the Custom Functions capability.

# The CustFunc DLL

The **CustFunc** add-in DLL does not actually register any new functions, but provides interface integration functionality for any other Custom Function DLLs that are loaded!  The built-in functions, as mentioned above, can be typed into a math region or inserted from the **_Functions_** panel.  The **_Functions_** panel is opened from Functions ribbon (the All Functions button) or by pressing the hot-key, `<F2>`.  Mathcad Prime does not use the hot-key combination `<Shift><F2>`, so **CustFunc** uses this keyboard combo to launch an **_Insert Custom Function_** dialog box, providing the user with a list of categorized Custom Functions for insertion onto the active worksheet at the current cursor location.