#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>

const std::string k_iFileExt = ".lst";
const std::string k_oFileExt = ".asm";

bool IsRawData(const std::string& line)
{
	if (line.size() >= 62)
	{
		for (int i = 12; i <= 60; i += 3)
		{
			if (line[i] != ' ') return false;
		}
		return true;
	}
	return false;
}

std::string ProcessString(const std::string& line)
{
	std::stringstream stream;
	std::for_each(line.begin(), line.end(), [&](char ch)
		{
			if (ch == '\t')
			{
				auto p = int(stream.tellp());
				auto t = (8 - (p % 8));
				stream << std::string(t, ' ');
			}
			else
				stream << ch;
		}
	);
	return stream.str();
}

bool Convert(const std::filesystem::path& inputPath, const std::filesystem::path& outputPath)
{
	std::ifstream input(inputPath);
	std::ofstream output(outputPath);
	if (!input || !output) return false;

	bool isRawData = false;
	bool isFuncDef = false;
	bool isFuncLab = false;
	bool isAsmCode = false;
	bool isSection = false;
	bool isSpecial = false;

	std::string line;
	while (std::getline(input, line))
	{
		if (IsRawData(line))
		{
			line[60] = ';';
		}

		isFuncLab = !isFuncDef && (line.size() > 2 && line[line.size() - 2] == ')' && line[line.size() - 1] == ':');
		isFuncDef = (line.size() > 2 && line[line.size() - 2] == '>' && line[line.size() - 1] == ':');
		isAsmCode = (line.size() > 9 && line[8] == ':' && line[9] == '\t');
		isSection = (line.find("Disassembly of section") != std::string::npos);
		isSpecial = (!line.empty() && (line.find("...") == 1 || line[0] == '.'));

		line = ProcessString(line);

		if (isFuncDef)
		{
			output << std::endl << line << std::endl;
		}

		if (isFuncLab || isSection)
		{
			output << std::endl << "; " << line << std::endl;
		}

		if (isAsmCode || isSpecial)
		{
			output << line << std::endl;
		}
	}
	input.close();
	output.close();
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

	if (iFilePath.extension() != k_iFileExt)
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
