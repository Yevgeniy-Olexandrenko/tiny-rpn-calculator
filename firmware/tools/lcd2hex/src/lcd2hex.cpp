#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include "pugixml.hpp"

using Pixels = std::vector<bool>;
using Font = std::vector<Pixels>;

bool Convert(const std::filesystem::path& inputPath, const std::filesystem::path& outputPath)
{
    // load lcd font
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(inputPath.string().c_str());
    std::ofstream output(outputPath);
    if (!result || !output) return false;

    // parse lcd font
    auto nodeFont = doc.child("FONT");
    auto nodeFontSize = nodeFont.child("FONTSIZE");
    auto nodeRange = nodeFont.child("RANGE");
    auto nodeChars = nodeFont.child("CHARS");

    auto fontName = nodeFont.child_value("FONTNAME");
    auto fontWidthPx = nodeFontSize.attribute("WIDTH").as_int();
    auto fontHeightPx = nodeFontSize.attribute("HEIGHT").as_int();
    auto fontAsciiFrom = nodeRange.attribute("FROM").as_int();
    auto fontAsciiTo = nodeRange.attribute("TO").as_int();

    std::cout << "Name  : " << fontName << '\n';
    std::cout << "Size  : " << fontWidthPx << 'x' << fontHeightPx << '\n';
    std::cout << "ASCII : " << fontAsciiFrom << '-' << fontAsciiTo << '\n';
    
    Font font;
    font.resize(fontAsciiTo - fontAsciiFrom + 1);

    for (pugi::xml_node nodeChar = nodeChars.child("CHAR"); nodeChar; nodeChar = nodeChar.next_sibling("CHAR"))
    {
        auto charIndex = nodeChar.attribute("CODE").as_int() - fontAsciiFrom;
        auto pixelsPtr = const_cast<char*>(nodeChar.attribute("PIXELS").value());

        bool bitVal = false;
        while (true)
        {
            auto ch = *pixelsPtr++;
            if (ch == ',' || ch == 0)
            {
                font[charIndex].push_back(bitVal);
                if (font[charIndex].size() == (fontWidthPx * fontHeightPx)) break;
            }
            else if (ch >= '0' && ch <= '9')
            {
                bitVal = (ch == '0');
            }
        }
    }

    // convert to hex font
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

    // generate hex font file
    std::stringstream outputFontName;
    std::stringstream outputFontBytesName;
    outputFontName << fontName << fontWidthPx << 'x' << fontHeightPx;
    outputFontBytesName << outputFontName.str() << "_bytes";

    auto to_hex = [](const uint8_t& byte) -> std::string
    {
        const char hex[] = "0123456789ABCDEF";

        std::stringstream ss;
        ss << "0x";
        ss << hex[(byte & 0xF0) >> 4];
        ss << hex[(byte & 0x0F)];
        return ss.str();
    };

    output << "const uint8_t " << outputFontBytesName.str() << "[] PROGMEM =" << std::endl;
    output << "{" << std::endl;

    int bytesPerLine = 0, bytesTotal = 0;
    int charCode = fontAsciiFrom;
    for (const auto& charBytes : fontBytes)
    {
        output << "    ";
        output << "// character '" << char(charCode) << "' (" << to_hex(charCode) << ")";
        output << std::endl;

        for (const auto& bytesInRow : charBytes)
        {
            for (uint8_t byte : bytesInRow)
            {
                if (bytesPerLine == 0)
                {
                    output << "    ";
                }

                output << to_hex(byte);
                output << ", ";

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
    output << "const Font " << outputFontName.str() << " PROGMEM =" << std::endl;
    output << "{" << std::endl;
    output << "    .bytesInRow  = " << to_hex(bytesInRow) << "," << std::endl;
    output << "    .rowsOfBytes = " << to_hex(rowsOfBytes) << "," << std::endl;
    output << "    .asciiFirst  = " << to_hex(fontAsciiFrom) << "," << std::endl;
    output << "    .asciiLast   = " << to_hex(fontAsciiTo) << "," << std::endl;
    output << "    .bytes = " << outputFontBytesName.str() << " // " << bytesTotal << " bytes" << std::endl;
    output << "};" << std::endl;
    output.close();

    std::cout << "DONE\n";
    return true;
}

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        std::filesystem::path inputPath = argv[1];
        std::filesystem::path outputPath = inputPath;
        outputPath.replace_extension(".h");

        if (!Convert(inputPath, outputPath))
        {
            std::cerr << "Conversion FAILED:\n";
            std::cerr << inputPath.string() << " -> " << outputPath.string() << '\n';
            return -1;
        }
    }
    else
    {
        std::cerr << "Usage:\n";
        std::cerr << argv[0] << " <input_file>" << '\n';
        return -2;
    }
    return 0;
}
