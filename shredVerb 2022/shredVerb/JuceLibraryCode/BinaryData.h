/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   DEFAULT_v4_1_xml;
    const int            DEFAULT_v4_1_xmlSize = 13375;

    extern const char*   DEFAULT_v2_42_xml;
    const int            DEFAULT_v2_42_xmlSize = 12993;

    extern const char*   DIRECT_EDIT_2_0_xml;
    const int            DIRECT_EDIT_2_0_xmlSize = 9394;

    extern const char*   DEFAULT_v4_xml;
    const int            DEFAULT_v4_xmlSize = 12993;

    extern const char*   DEFAULT_v5_xml;
    const int            DEFAULT_v5_xmlSize = 16110;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 5;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
