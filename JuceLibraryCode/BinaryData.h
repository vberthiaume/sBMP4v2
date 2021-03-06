/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   blackLight_svg;
    const int            blackLight_svgSize = 4846;

    extern const char*   sBMP4KnobUniform_svg;
    const int            sBMP4KnobUniform_svgSize = 3638;

    extern const char*   blackLight_png;
    const int            blackLight_pngSize = 43010;

    extern const char*   blackSquare_svg;
    const int            blackSquare_svgSize = 3496;

    extern const char*   redSquare_svg;
    const int            redSquare_svgSize = 6235;

    extern const char*   redLight_png;
    const int            redLight_pngSize = 50181;

    extern const char*   redLight_svg;
    const int            redLight_svgSize = 4623;

    extern const char*   sBMP4Knob_svg;
    const int            sBMP4Knob_svgSize = 3463;

    extern const char*   blackTexture_jpg;
    const int            blackTexture_jpgSize = 75789;

    extern const char*   redTexture_png;
    const int            redTexture_pngSize = 137180;

    extern const char*   blackMetal_jpg;
    const int            blackMetal_jpgSize = 31507;

    extern const char*   metalKnobFitted_png;
    const int            metalKnobFitted_pngSize = 408619;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 12;

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
