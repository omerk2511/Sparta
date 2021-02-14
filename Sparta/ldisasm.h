#pragma once

// adapted from https://github.com/Nomade040/length-disassembler
// thanks to @Nomade040!

#include <cstddef>

namespace ldisasm
{
	inline constexpr unsigned char LDISASM_R(unsigned char* b) { return *b >> 4; };
	inline constexpr unsigned char LDISASM_C(unsigned char* b) { return *b & 0xF; };

	constexpr unsigned char LEGACY_PREFIXES[] = { 0xF0, 0xF2, 0xF3, 0x2E, 0x36, 0x3E, 0x26, 0x64, 0x65, 0x66, 0x67 };
	constexpr unsigned char OP1_MOD_RM[] = { 0x62, 0x63, 0x69, 0x6B, 0xC0, 0xC1, 0xC4, 0xC5, 0xC6, 0xC7, 0xD0, 0xD1, 0xD2, 0xD3, 0xF6, 0xF7, 0xFE, 0xFF };
	constexpr unsigned char OP1_IMM8[] = { 0x6A, 0x6B, 0x80, 0x82, 0x83, 0xA8, 0xC0, 0xC1, 0xC6, 0xCD, 0xD4, 0xD5, 0xEB };
	constexpr unsigned char OP1_IMM32[] = { 0x68, 0x69, 0x81, 0xA9, 0xC7, 0xE8, 0xE9 };
	constexpr unsigned char OP2_MOD_RM[] = { 0x0D, 0xA3, 0xA4, 0xA5, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF };

	inline bool find_byte(const unsigned char* arr, const size_t N, const unsigned char x) { for (size_t i = 0; i < N; i++) { if (arr[i] == x) { return true; } }; return false; }

	inline void parse_mod_rm(unsigned char** b, const bool addressPrefix)
	{
		unsigned char modrm = *++ * b;

		if (!addressPrefix || (addressPrefix && **b >= 0x40))
		{
			bool hasSIB = false;
			if (**b < 0xC0 && (**b & 0b111) == 0b100 && !addressPrefix)
				hasSIB = true, (*b)++;

			if (modrm >= 0x40 && modrm <= 0x7F)
				(*b)++;
			else if ((modrm <= 0x3F && (modrm & 0b111) == 0b101) || (modrm >= 0x80 && modrm <= 0xBF))
				*b += (addressPrefix) ? 2 : 4;
			else if (hasSIB && (**b & 0b111) == 0b101)
				*b += (modrm & 0b01000000) ? 1 : 4;
		}
		else if (addressPrefix && modrm == 0x26)
			*b += 2;
	};

	enum class DisassmeblyMode { X86, X86_64 };

	inline size_t get_instruction_length(const void* const address, const DisassmeblyMode mode)
	{
		size_t offset = 0;
		bool operandPrefix = false, addressPrefix = false, rexW = false;
		unsigned char* b = (unsigned char*)(address);

		for (int i = 0; i < 14 && find_byte(LEGACY_PREFIXES, sizeof(LEGACY_PREFIXES), *b) || ((mode == DisassmeblyMode::X86_64) ? (LDISASM_R(b) == 4) : false); i++, b++)
		{
			if (*b == 0x66)
				operandPrefix = true;
			else if (*b == 0x67)
				addressPrefix = true;
			else if (LDISASM_R(b) == 4 && LDISASM_C(b) >= 8)
				rexW = true;
		}

		if (*b == 0x0F)
		{
			b++;
			if (*b == 0x38 || *b == 0x3A)
			{
				if (*b++ == 0x3A)
					offset++;

				parse_mod_rm(&b, addressPrefix);
			}
			else
			{
				if (LDISASM_R(b) == 8)
					offset += 4;
				else if ((LDISASM_R(b) == 7 && LDISASM_C(b) < 4) || *b == 0xA4 || *b == 0xC2 || (*b > 0xC3 && *b <= 0xC6) || *b == 0xBA || *b == 0xAC) //imm8
					offset++;

				if (find_byte(OP2_MOD_RM, sizeof(OP2_MOD_RM), *b) || (LDISASM_R(b) != 3 && LDISASM_R(b) > 0 && LDISASM_R(b) < 7) || *b >= 0xD0 || (LDISASM_R(b) == 7 && LDISASM_C(b) != 7) || LDISASM_R(b) == 9 || LDISASM_R(b) == 0xB || (LDISASM_R(b) == 0xC && LDISASM_C(b) < 8) || (LDISASM_R(b) == 0 && LDISASM_C(b) < 4))
					parse_mod_rm(&b, addressPrefix);
			}
		}
		else
		{
			if ((LDISASM_R(b) == 0xE && LDISASM_C(b) < 8) || (LDISASM_R(b) == 0xB && LDISASM_C(b) < 8) || LDISASM_R(b) == 7 || (LDISASM_R(b) < 4 && (LDISASM_C(b) == 4 || LDISASM_C(b) == 0xC)) || (*b == 0xF6 && !(*(b + 1) & 48)) || find_byte(OP1_IMM8, sizeof(OP1_IMM8), *b)) //imm8
				offset++;
			else if (*b == 0xC2 || *b == 0xCA)
				offset += 2;
			else if (*b == 0xC8)
				offset += 3;
			else if ((LDISASM_R(b) < 4 && (LDISASM_C(b) == 5 || LDISASM_C(b) == 0xD)) || (LDISASM_R(b) == 0xB && LDISASM_C(b) >= 8) || (*b == 0xF7 && !(*(b + 1) & 48)) || find_byte(OP1_IMM32, sizeof(OP1_IMM32), *b)) //imm32,16
				offset += (rexW) ? 8 : (operandPrefix ? 2 : 4);
			else if (LDISASM_R(b) == 0xA && LDISASM_C(b) < 4)
				offset += (rexW) ? 8 : (addressPrefix ? 2 : 4);
			else if (*b == 0xEA || *b == 0x9A)
				offset += operandPrefix ? 4 : 6;

			if (find_byte(OP1_MOD_RM, sizeof(OP1_MOD_RM), *b) || (LDISASM_R(b) < 4 && (LDISASM_C(b) < 4 || (LDISASM_C(b) >= 8 && LDISASM_C(b) < 0xC))) || LDISASM_R(b) == 8 || (LDISASM_R(b) == 0xD && LDISASM_C(b) >= 8))
				parse_mod_rm(&b, addressPrefix);
		}

		return (size_t)((ptrdiff_t)(++b + offset) - (ptrdiff_t)(address));
	}
}
