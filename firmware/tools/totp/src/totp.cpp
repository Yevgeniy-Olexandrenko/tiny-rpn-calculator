// -----------------------------------------------------------------------------
// TOTP keys converter for the TinyRPNCalculator sketches.
//
// totp header <keys.txt> <area size> <totp.h>
//     writes C header with the keys area for DATAMEM, built into the firmware,
//     private keys files (*.local.*) are refused, the header goes to git
// totp eep <keys.txt> <input.eep> <output.eep>
//     replaces the keys area in the EEPROM image of a compiled sketch, the area
//     is found by its signature, its size stays the same
//
// Keys file: one key per line, label (1..4 chars of A-Z, 0-9) and Base32 secret
// or otpauth:// URI, empty lines and lines starting with '#' are skipped.
// Keys area layout is described in Extras/TOTP.h of the library.
// -----------------------------------------------------------------------------

#include <cstdint>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <filesystem>

// stubs for the library headers, TOTP.h is used to print current codes
using b08 = bool;
using u08 = uint8_t; using u16 = uint16_t; using u32 = uint32_t;
using s08 = int8_t;  using s16 = int16_t;  using s32 = int32_t;
#define PROGMEM
#define DATAMEM
#define NOINLINE

namespace MEM
{
	void ProgCopy(void* d, const void* s, size_t n) { memcpy(d, s, n); }
	u08  DataRead(const u08* p) { return *p; }
	void DataCopy(void* d, const void* s, size_t n) { memcpy(d, s, n); }
}

#include "../../../libraries/TinyRPNCalculator/src/Extras/TOTP.h"

const char k_signature[] = { 'T', 'O', 'T', 'P' };
const size_t k_keysMax = 9;

struct Entry
{
	std::string label;
	std::vector<uint8_t> key;
};

// -----------------------------------------------------------------------------
// Keys file parsing
// -----------------------------------------------------------------------------

std::string Trim(const std::string& s)
{
	auto b = s.find_first_not_of(" \t\r\n");
	auto e = s.find_last_not_of(" \t\r\n");
	return b == std::string::npos ? "" : s.substr(b, e - b + 1);
}

std::string ToUpper(std::string s)
{
	for (auto& ch : s) ch = char(toupper((unsigned char)ch));
	return s;
}

std::string ToLower(std::string s)
{
	for (auto& ch : s) ch = char(tolower((unsigned char)ch));
	return s;
}

bool DecodeBase32(const std::string& text, std::vector<uint8_t>& out, std::string& error, std::string& warning)
{
	// RFC 4648 alphabet, spaces and dashes between groups are allowed,
	// optional '=' padding can only be at the end
	uint32_t buffer = 0;
	int bits = 0, chars = 0;
	bool padding = false;
	for (char ch : text)
	{
		if (ch == ' ' || ch == '\t' || ch == '-') continue;
		if (ch == '=') { padding = true; continue; }
		if (padding)
		{
			error = "secret has chars after '=' padding";
			return false;
		}

		ch = char(toupper((unsigned char)ch));
		int value;
		if (ch >= 'A' && ch <= 'Z')
			value = ch - 'A';
		else if (ch >= '2' && ch <= '7')
			value = ch - '2' + 26;
		else
		{
			error = "secret is not valid Base32";
			return false;
		}

		buffer = (buffer << 5) | value;
		bits += 5;
		chars++;
		if (bits >= 8)
		{
			bits -= 8;
			out.push_back(uint8_t(buffer >> bits));
		}
	}

	// whole bytes take 0, 2, 4, 5 or 7 chars of the last group of 8,
	// other lengths mean a char is lost or added by mistake
	int tail = chars % 8;
	if (tail == 1 || tail == 3 || tail == 6)
	{
		error = "secret length of " + std::to_string(chars) + " chars is not valid Base32";
		return false;
	}

	// unused bits of the last char are zero in properly encoded secrets,
	// some services leave them random, so it's only a warning
	if (buffer & ((1u << bits) - 1))
		warning = "unused bits of the secret are not zero, check the secret for typos";
	return true;
}

int HexDigit(char ch)
{
	if (ch >= '0' && ch <= '9') return ch - '0';
	ch = char(toupper((unsigned char)ch));
	if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
	return -1;
}

bool DecodePercent(const std::string& s, std::string& out)
{
	for (size_t i = 0; i < s.size(); ++i)
	{
		if (s[i] == '%')
		{
			if (i + 2 >= s.size()) return false;
			int hi = HexDigit(s[i + 1]);
			int lo = HexDigit(s[i + 2]);
			if (hi < 0 || lo < 0) return false;
			out += char(hi << 4 | lo);
			i += 2;
		}
		else
			out += s[i];
	}
	return true;
}

bool ParseUri(const std::string& uri, std::string& secret, std::string& error)
{
	// otpauth://totp/Issuer:account?secret=...&issuer=...&algorithm=SHA1&digits=6&period=30
	const std::string prefix = "otpauth://totp/";
	if (ToLower(uri.substr(0, prefix.size())) != prefix)
	{
		error = "only otpauth://totp/ URIs are supported";
		return false;
	}

	auto query = uri.find('?');
	if (query == std::string::npos)
	{
		error = "URI has no parameters";
		return false;
	}

	std::stringstream params(uri.substr(query + 1));
	std::string param;
	while (std::getline(params, param, '&'))
	{
		auto eq = param.find('=');
		if (eq == std::string::npos) continue;
		std::string name = ToLower(param.substr(0, eq));
		std::string value;
		if (!DecodePercent(param.substr(eq + 1), value))
		{
			error = "URI parameter " + name + " has invalid %XX encoding";
			return false;
		}

		if (name == "secret")
			secret = value;
		else if (name == "algorithm" && ToUpper(value) != "SHA1")
			error = "algorithm " + value + " is not supported, only SHA1";
		else if (name == "digits" && value != "6")
			error = value + " digits are not supported, only 6";
		else if (name == "period" && value != "30")
			error = "period of " + value + " seconds is not supported, only 30";
	}

	if (error.empty() && secret.empty()) error = "URI has no secret";
	return error.empty();
}

bool ParseLine(const std::string& line, Entry& entry, std::string& error, std::string& warning)
{
	auto space = line.find_first_of(" \t");
	if (space == std::string::npos)
	{
		error = "expected label and secret";
		return false;
	}

	// label in the font of the calculator menus
	entry.label = ToUpper(line.substr(0, space));
	if (entry.label.size() > TOTP::LABEL_SIZE)
	{
		error = "label is longer than " + std::to_string(TOTP::LABEL_SIZE) + " chars";
		return false;
	}
	for (char ch : entry.label)
	{
		if (!(ch >= 'A' && ch <= 'Z') && !(ch >= '0' && ch <= '9'))
		{
			error = "label can only have chars A-Z and 0-9";
			return false;
		}
	}
	entry.label.resize(TOTP::LABEL_SIZE, ' ');

	// secret as Base32 or inside of URI
	std::string secret = Trim(line.substr(space));
	if (ToLower(secret.substr(0, 10)) == "otpauth://")
	{
		std::string uri = secret;
		if (!ParseUri(uri, secret, error)) return false;
	}
	if (!DecodeBase32(secret, entry.key, error, warning)) return false;
	if (entry.key.empty() || entry.key.size() > TOTP::KEY_MAX)
	{
		error = "key must have 1.." + std::to_string(TOTP::KEY_MAX) +
			" bytes, got " + std::to_string(entry.key.size());
		return false;
	}
	return true;
}

bool ReadKeys(const std::filesystem::path& path, std::vector<Entry>& entries)
{
	std::ifstream input(path);
	if (!input)
	{
		std::cerr << "Can't open " << path.string() << std::endl;
		return false;
	}

	std::string line;
	for (int number = 1; std::getline(input, line); ++number)
	{
		line = Trim(line);
		if (line.empty() || line[0] == '#') continue;

		Entry entry;
		std::string error, warning;
		if (!ParseLine(line, entry, error, warning))
		{
			std::cerr << path.filename().string() << "(" << number << "): " << error << std::endl;
			return false;
		}
		if (!warning.empty())
		{
			std::cerr << path.filename().string() << "(" << number << "): warning: " << warning << std::endl;
		}
		entries.push_back(entry);
	}
	return true;
}

// -----------------------------------------------------------------------------
// Keys area
// -----------------------------------------------------------------------------

bool BuildArea(const std::vector<Entry>& entries, size_t size, std::vector<uint8_t>& area)
{
	if (size < TOTP::SIGNATURE_SIZE || size > 255)
	{
		std::cerr << "Keys area size must be " << int(TOTP::SIGNATURE_SIZE) << "..255 bytes" << std::endl;
		return false;
	}

	// digit keys 1..9 select the key on the calculator
	if (entries.size() > k_keysMax)
	{
		std::cerr << "Too many keys: " << entries.size() << ", maximum is " << k_keysMax << std::endl;
		return false;
	}

	// signature, records, zeros as the end of records
	area.assign(size, 0);
	memcpy(area.data(), k_signature, sizeof(k_signature));
	area[TOTP::SIGNATURE_SIZE - 1] = uint8_t(size);

	size_t used = TOTP::SIGNATURE_SIZE;
	for (const auto& entry : entries)
	{
		used += 1 + TOTP::LABEL_SIZE + entry.key.size();
	}
	if (used > size)
	{
		std::cerr << "Keys don't fit the area: " << used << " of " << size << " bytes" << std::endl;
		return false;
	}

	size_t p = TOTP::SIGNATURE_SIZE;
	for (const auto& entry : entries)
	{
		area[p++] = uint8_t(entry.key.size());
		for (char ch : entry.label) area[p++] = uint8_t(ch);
		for (uint8_t b : entry.key) area[p++] = b;
	}

	std::cout << "Keys area: " << entries.size() << " keys, " << used << " of " << size << " bytes" << std::endl;
	return true;
}

void PrintCodes(const std::vector<Entry>& entries)
{
	// current codes to compare with a phone
	time_t now = time(nullptr);
	u32 counter = u32(now / TOTP::PERIOD);
	int left = int(TOTP::PERIOD - now % TOTP::PERIOD);

	std::cout << "Current codes, valid for " << left << " s:" << std::endl;
	for (const auto& entry : entries)
	{
		TOTP::Key key{};
		key.length = u08(entry.key.size());
		memcpy(key.label, entry.label.data(), TOTP::LABEL_SIZE);
		memcpy(key.data, entry.key.data(), entry.key.size());

		u32 code = TOTP::GetCode(key, counter);
		std::cout << "  " << entry.label << "  ";
		std::cout << std::setfill('0') << std::setw(3) << code / 1000 << ' ';
		std::cout << std::setfill('0') << std::setw(3) << code % 1000 << std::endl;
	}
}

// -----------------------------------------------------------------------------
// Header output
// -----------------------------------------------------------------------------

std::string ToHex(uint8_t byte)
{
	const char hex[] = "0123456789ABCDEF";
	return std::string("0x") + hex[byte >> 4] + hex[byte & 15];
}

bool WriteHeader(const std::filesystem::path& path, const std::filesystem::path& keysPath,
	const std::vector<Entry>& entries, const std::vector<uint8_t>& area)
{
	std::ofstream output(path);
	if (!output)
	{
		std::cerr << "Can't write " << path.string() << std::endl;
		return false;
	}

	std::string source = keysPath.parent_path().filename().string() + "/" + keysPath.filename().string();
	output << "#pragma once" << std::endl;
	output << std::endl;
	output << "// -----------------------------------------------------------------------------" << std::endl;
	output << "// TOTP keys area, generated by firmware/tools/totp from " << source << std::endl;
	output << "// -----------------------------------------------------------------------------" << std::endl;
	output << std::endl;
	output << "const u08 totp_keys[" << area.size() << "] DATAMEM =" << std::endl;
	output << "{" << std::endl;
	output << "\t'T', 'O', 'T', 'P', " << area.size() << "," << std::endl;

	size_t used = TOTP::SIGNATURE_SIZE;
	for (const auto& entry : entries)
	{
		output << std::endl;
		output << "\t// " << Trim(entry.label) << std::endl;
		output << "\t" << entry.key.size() << ",";
		for (char ch : entry.label) output << " '" << ch << "',";
		output << std::endl;

		// key bytes, 10 per line
		for (size_t i = 0; i < entry.key.size(); ++i)
		{
			output << (i % 10 ? " " : "\t") << ToHex(entry.key[i]) << ",";
			if (i % 10 == 9 || i + 1 == entry.key.size()) output << std::endl;
		}
		used += 1 + TOTP::LABEL_SIZE + entry.key.size();
	}

	// the rest of the area is zeros
	if (used < area.size())
	{
		output << std::endl;
		output << "\t// end of records" << std::endl;
		output << "\t0" << std::endl;
	}
	output << "};" << std::endl;
	return true;
}

// -----------------------------------------------------------------------------
// EEPROM image in Intel HEX format
// -----------------------------------------------------------------------------

struct HexRecord
{
	uint8_t  type;
	uint16_t address;
	std::vector<uint8_t> data;
};

bool ReadHex(const std::filesystem::path& path, std::vector<HexRecord>& records)
{
	std::ifstream input(path);
	if (!input)
	{
		std::cerr << "Can't open " << path.string() << std::endl;
		return false;
	}

	std::string line;
	for (int number = 1; std::getline(input, line); ++number)
	{
		line = Trim(line);
		if (line.empty()) continue;

		// :LLAAAATT<data>CC, checksum makes the sum of all bytes zero
		std::vector<uint8_t> bytes;
		bool valid = (line[0] == ':' && line.size() % 2 == 1 && line.size() >= 11);
		for (size_t i = 1; valid && i < line.size(); i += 2)
		{
			if (!isxdigit((unsigned char)line[i]) || !isxdigit((unsigned char)line[i + 1])) valid = false;
			else bytes.push_back(uint8_t(std::stoi(line.substr(i, 2), nullptr, 16)));
		}

		uint8_t sum = 0;
		for (uint8_t b : bytes) sum += b;
		if (!valid || bytes.size() != 5u + bytes[0] || sum != 0)
		{
			std::cerr << path.filename().string() << "(" << number << "): invalid Intel HEX record" << std::endl;
			return false;
		}

		HexRecord record;
		record.type = bytes[3];
		record.address = uint16_t(bytes[1] << 8 | bytes[2]);
		record.data.assign(bytes.begin() + 4, bytes.end() - 1);
		if (record.type > 1)
		{
			std::cerr << path.filename().string() << "(" << number << "): unsupported record type " << int(record.type) << std::endl;
			return false;
		}
		records.push_back(record);
	}
	return true;
}

bool WriteHex(const std::filesystem::path& path, const std::vector<HexRecord>& records)
{
	std::ofstream output(path);
	if (!output)
	{
		std::cerr << "Can't write " << path.string() << std::endl;
		return false;
	}

	const char hex[] = "0123456789ABCDEF";
	for (const auto& record : records)
	{
		std::vector<uint8_t> bytes =
		{
			uint8_t(record.data.size()), uint8_t(record.address >> 8), uint8_t(record.address), record.type
		};
		bytes.insert(bytes.end(), record.data.begin(), record.data.end());

		uint8_t sum = 0;
		for (uint8_t b : bytes) sum += b;
		bytes.push_back(uint8_t(-sum));

		output << ':';
		for (uint8_t b : bytes) output << hex[b >> 4] << hex[b & 15];
		output << std::endl;
	}
	return true;
}

bool IsKeysArea(const std::vector<int>& image, int address)
{
	// signature, then records that fit the area size (the same "TOTP"
	// chars can be met in other data, menu strings for example)
	for (int i = 0; i < int(sizeof(k_signature)); ++i)
	{
		if (image[address + i] != k_signature[i]) return false;
	}

	int size = image[address + TOTP::SIGNATURE_SIZE - 1];
	if (size < TOTP::SIGNATURE_SIZE || address + size > int(image.size())) return false;
	for (int i = 0; i < size; ++i)
	{
		if (image[address + i] < 0) return false;
	}

	int end = address + size;
	for (int p = address + TOTP::SIGNATURE_SIZE; p < end;)
	{
		int length = image[p];
		if (length == 0) break;
		if (length > TOTP::KEY_MAX || p + 1 + TOTP::LABEL_SIZE + length > end) return false;
		p += 1 + TOTP::LABEL_SIZE + length;
	}
	return true;
}

bool PatchEeprom(std::vector<HexRecord>& records, const std::vector<Entry>& entries)
{
	// EEPROM image, -1 for bytes not present in the file
	std::vector<int> image(0x10000, -1);
	for (const auto& record : records)
	{
		if (record.type != 0) continue;
		for (size_t i = 0; i < record.data.size(); ++i)
		{
			image[(record.address + i) & 0xFFFF] = record.data[i];
		}
	}

	// find exactly one keys area
	int found = -1;
	for (int a = 0; a + TOTP::SIGNATURE_SIZE <= int(image.size()); ++a)
	{
		if (!IsKeysArea(image, a)) continue;
		if (found >= 0)
		{
			std::cerr << "More than one keys area in the EEPROM image" << std::endl;
			return false;
		}
		found = a;
	}
	if (found < 0)
	{
		std::cerr << "No keys area in the EEPROM image, is the sketch built with TOTP?" << std::endl;
		return false;
	}

	int size = image[found + TOTP::SIGNATURE_SIZE - 1];
	std::cout << "Keys area found at EEPROM address " << ToHex(uint8_t(found >> 8)) << ToHex(uint8_t(found)).substr(2) << std::endl;

	std::vector<uint8_t> area;
	if (!BuildArea(entries, size, area)) return false;
	for (int i = 0; i < size; ++i) image[found + i] = area[i];

	// put changed bytes back to the records, checksums are computed on write
	for (auto& record : records)
	{
		if (record.type != 0) continue;
		for (size_t i = 0; i < record.data.size(); ++i)
		{
			record.data[i] = uint8_t(image[(record.address + i) & 0xFFFF]);
		}
	}
	return true;
}

// -----------------------------------------------------------------------------
// Enter point
// -----------------------------------------------------------------------------

int Usage(const char* app)
{
	std::string name = std::filesystem::path(app).filename().string();
	std::cerr << "Usage:" << std::endl;
	std::cerr << name << " header <keys.txt> <area size> <totp.h>" << std::endl;
	std::cerr << name << " eep <keys.txt> <input.eep> <output.eep>" << std::endl;
	return -1;
}

int main(int argc, char* argv[])
{
	if (argc != 5) return Usage(argv[0]);

	std::string mode = argv[1];
	std::filesystem::path keysPath{ argv[2] };

	std::vector<Entry> entries;
	if (mode == "header")
	{
		std::filesystem::path headerPath{ argv[4] };
		size_t size = size_t(atoi(argv[3]));

		// the header is committed to git, real secrets must stay in local files
		if (keysPath.filename().string().find(".local.") != std::string::npos)
		{
			std::cerr << "Keys of " << keysPath.filename().string() << " are private, use eep mode for them" << std::endl;
			std::cerr << "Conversion FAILED" << std::endl;
			return -1;
		}

		std::vector<uint8_t> area;
		if (!ReadKeys(keysPath, entries) ||
			!BuildArea(entries, size, area) ||
			!WriteHeader(headerPath, keysPath, entries, area))
		{
			std::cerr << "Conversion FAILED" << std::endl;
			return -1;
		}

		std::cout << "Conversion DONE:" << std::endl;
		std::cout << keysPath.filename().string() << " -> " << headerPath.filename().string() << std::endl;
	}
	else if (mode == "eep")
	{
		std::filesystem::path inputPath{ argv[3] };
		std::filesystem::path outputPath{ argv[4] };

		std::vector<HexRecord> records;
		if (!ReadKeys(keysPath, entries) ||
			!ReadHex(inputPath, records) ||
			!PatchEeprom(records, entries) ||
			!WriteHex(outputPath, records))
		{
			std::cerr << "Conversion FAILED" << std::endl;
			return -1;
		}

		std::cout << "Conversion DONE:" << std::endl;
		std::cout << keysPath.filename().string() << " + " << inputPath.filename().string();
		std::cout << " -> " << outputPath.filename().string() << std::endl;
	}
	else
		return Usage(argv[0]);

	PrintCodes(entries);
	return 0;
}
