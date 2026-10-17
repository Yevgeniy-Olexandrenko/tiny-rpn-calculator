#include <vector>
#include <string>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include "pugixml.hpp"

const std::string k_iFileExt = ".lcd";
const std::string k_oFileExt = ".h";

using Pixels = std::vector<bool>;
using Font = std::vector<Pixels>;

// pixel colors are 0xBBGGRR values: a dark color is a lit pixel
bool IsPixelOn(long color)
{
    return ((color & 0xFF) + ((color >> 8) & 0xFF) + ((color >> 16) & 0xFF)) < 3 * 128;
}

// parses comma separated pixel colors, returns false on a malformed value
bool ParsePixels(const char* text, Pixels& pixels)
{
    for (const char* p = text; *p;)
    {
        char* end;
        long color = std::strtol(p, &end, 10);
        if (end == p) return false;
        pixels.push_back(IsPixelOn(color));
        while (std::isspace((unsigned char)*end)) ++end;
        if (*end == ',') ++end;
        else if (*end) return false;
        p = end;
    }
    return true;
}

std::string ToHex(int byte)
{
    const char hex[] = "0123456789ABCDEF";
    std::string s = "0x";
    s += hex[(byte >> 4) & 0x0F];
    s += hex[byte & 0x0F];
    return s;
}

bool Fail(const std::string& message)
{
    std::cerr << message << std::endl;
    return false;
}

bool Convert(const std::filesystem::path& inputPath, const std::filesystem::path& outputPath)
{
    // load lcd font
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(inputPath.c_str());
    if (!result) return Fail(std::string("XML error: ") + result.description());

    // parse lcd font
    auto nodeFont = doc.child("FONT");
    auto nodeFontSize = nodeFont.child("FONTSIZE");
    auto nodeRange = nodeFont.child("RANGE");
    auto nodeChars = nodeFont.child("CHARS");

    auto fontName = nodeFont.child_value("FONTNAME");
    auto fontWidthPx = nodeFontSize.attribute("WIDTH").as_int();
    auto fontHeightPx = nodeFontSize.attribute("HEIGHT").as_int();
    auto fontAsciiFrom = nodeRange.attribute("FROM").as_int(-1);
    auto fontAsciiTo = nodeRange.attribute("TO").as_int(-1);

    std::cout << "Name  : " << fontName << '\n';
    std::cout << "Size  : " << fontWidthPx << 'x' << fontHeightPx << '\n';
    std::cout << "ASCII : " << fontAsciiFrom << '-' << fontAsciiTo << '\n';

    // the font header stores sizes and codes as bytes
    if (!*fontName) return Fail("Font name is missing");
    if (fontWidthPx < 1 || fontWidthPx > 255 || fontHeightPx < 1 || fontHeightPx > 255 * 8)
        return Fail("Font size must be 1..255 x 1..2040 pixels");
    if (fontAsciiFrom < 0 || fontAsciiTo > 255 || fontAsciiFrom > fontAsciiTo)
        return Fail("ASCII range must be within 0..255, FROM <= TO");

    Font font(fontAsciiTo - fontAsciiFrom + 1);
    const size_t pixelsPerChar = size_t(fontWidthPx) * fontHeightPx;

    for (pugi::xml_node nodeChar = nodeChars.child("CHAR"); nodeChar; nodeChar = nodeChar.next_sibling("CHAR"))
    {
        auto charCode = nodeChar.attribute("CODE").as_int(-1);
        if (charCode < fontAsciiFrom || charCode > fontAsciiTo)
            return Fail("Character code " + std::to_string(charCode) + " is out of the ASCII range");

        auto& pixels = font[charCode - fontAsciiFrom];
        if (!pixels.empty())
            return Fail("Character " + std::to_string(charCode) + " is defined twice");

        if (!ParsePixels(nodeChar.attribute("PIXELS").value(), pixels) || pixels.size() != pixelsPerChar)
            return Fail("Character " + std::to_string(charCode) + " must have " + std::to_string(pixelsPerChar) + " pixel values");
    }

    for (size_t i = 0; i < font.size(); ++i)
    {
        if (font[i].empty())
            return Fail("Character " + std::to_string(fontAsciiFrom + i) + " is missing");
    }

    // convert to hex font, pixels go column by column from the top
    int rowsOfBytes = int((fontHeightPx + 7) / 8);
    int bytesInRow = fontWidthPx;

    using RowBytes = std::vector<uint8_t>;
    using CharBytes = std::vector<RowBytes>;

    std::vector<CharBytes> fontBytes;
    for (const auto& pixels : font)
    {
        CharBytes charBytes;
        for (int ri = 0; ri < rowsOfBytes; ++ri)
        {
            RowBytes rowBytes;
            for (int bi = 0; bi < bytesInRow; ++bi)
            {
                uint8_t byte = 0;
                for (int bit = 0; bit < 8; ++bit)
                {
                    int pixelsRow = bi * fontHeightPx;
                    int pixelsCol = ri * 8 + bit;

                    if (pixelsCol < fontHeightPx)
                    {
                        if (pixels[pixelsRow + pixelsCol])
                        {
                            byte |= (1 << bit);
                        }
                    }
                }
                rowBytes.push_back(byte);
            }
            charBytes.push_back(rowBytes);
        }
        fontBytes.push_back(charBytes);
    }

    // generate hex font file in the format of the sketches' data.h
    std::stringstream output;
    std::string outputFontName = fontName + std::to_string(fontWidthPx) + 'x' + std::to_string(fontHeightPx);
    std::string outputFontBytesName = outputFontName + "_bytes";

    output << "const u08 " << outputFontBytesName << "[] PROGMEM =" << std::endl;
    output << "{" << std::endl;

    int bytesPerLine = 0, bytesTotal = 0;
    int charCode = fontAsciiFrom;
    for (const auto& charBytes : fontBytes)
    {
        // a control character would break the comment line
        output << "    // character ";
        if (std::isprint(charCode)) output << "'" << char(charCode) << "' ";
        output << "(" << ToHex(charCode) << ")" << std::endl;

        for (const auto& rowBytes : charBytes)
        {
            for (uint8_t byte : rowBytes)
            {
                output << (bytesPerLine == 0 ? "    " : " ") << ToHex(byte) << ",";

                if (++bytesPerLine == fontWidthPx)
                {
                    output << std::endl;
                    bytesPerLine = 0;
                }

                bytesTotal++;
            }
        }

        if (++charCode <= fontAsciiTo) output << std::endl;
    }

    if (bytesPerLine) output << std::endl;
    output << "};" << std::endl;

    output << std::endl;
    output << "const TXT::Font " << outputFontName << " PROGMEM =" << std::endl;
    output << "{" << std::endl;
    output << "    .bytesInRow  = " << ToHex(bytesInRow) << "," << std::endl;
    output << "    .rowsOfBytes = " << ToHex(rowsOfBytes) << "," << std::endl;
    output << "    .asciiFirst  = " << ToHex(fontAsciiFrom) << "," << std::endl;
    output << "    .asciiLast   = " << ToHex(fontAsciiTo) << "," << std::endl;
    output << "    .bytes = " << outputFontBytesName << " // " << bytesTotal << " bytes" << std::endl;
    output << "};" << std::endl;

    // the output file is written only when the whole font is converted
    std::ofstream file(outputPath);
    file << output.str();
    file.close();
    if (!file) return Fail("Can't write the output file");
    return true;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::filesystem::path app{ argv[0] };
        std::cerr << "Usage:" << std::endl;
        std::cerr << app.filename().string() << " <input_file>" << std::endl;
        return -1;
    }

    std::filesystem::path iFilePath{ argv[1] };
    std::filesystem::path oFilePath{ iFilePath };
    oFilePath.replace_extension(k_oFileExt);

    std::string iFileExt = iFilePath.extension().string();
    std::transform(iFileExt.begin(), iFileExt.end(), iFileExt.begin(), [](unsigned char c) { return char(std::tolower(c)); });
    if (iFileExt != k_iFileExt)
    {
        std::cerr << "Unsupported file type:" << std::endl;
        std::cerr << "Must be " << k_iFileExt << " instead of " << iFilePath.extension().string() << std::endl;
        return -1;
    }

    if (!Convert(iFilePath, oFilePath))
    {
        std::cerr << "Conversion FAILED:" << std::endl;
        std::cerr << iFilePath.filename().string() << " -> " << oFilePath.filename().string() << std::endl;
        return -1;
    }

    std::cout << "Conversion DONE:" << std::endl;
    std::cout << iFilePath.filename().string() << " -> " << oFilePath.filename().string() << std::endl;
    return 0;
}
