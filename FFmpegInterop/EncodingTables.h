#pragma once
#include <pch.h>
#include <collection.h>

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Media::Core;


namespace FFmpegInterop {
	public ref class EncodingTable sealed
	{
		String^ name;
		int encodingTable;
		static Platform::Collections::Vector<EncodingTable^>^ internalMap;


	public:
		property String^ Name
		{
			String^ get()
			{
				return name;
			}
		}

		property int WindowsEncodingTable
		{
			int get()
			{
				return encodingTable;
			}
		}
	internal:
		EncodingTable(int p_windowsTable, String^ p_name)
		{
			encodingTable = p_windowsTable;
			name = p_name;
		}


	public:
		static IVectorView<EncodingTable^>^ GetTables()
		{
			if (internalMap == nullptr) {
				//populate and create the vector.
				internalMap = ref new Vector<EncodingTable^>();
				internalMap->Append(ref new EncodingTable(65001, "CP65001"));
				internalMap->Append(ref new EncodingTable(65001, "UTF8"));
				internalMap->Append(ref new EncodingTable(65001, "UTF-8"));

				internalMap->Append(ref new EncodingTable(1200, "CP1200"));
				internalMap->Append(ref new EncodingTable(1200, "UTF16LE"));
				internalMap->Append(ref new EncodingTable(1200, "UTF-16LE"));
				internalMap->Append(ref new EncodingTable(1200, "UCS2LE"));
				internalMap->Append(ref new EncodingTable(1200, "UCS-2LE"));

				internalMap->Append(ref new EncodingTable(1201, "CP1201"));
				internalMap->Append(ref new EncodingTable(1201, "UTF16BE"));
				internalMap->Append(ref new EncodingTable(1201, "UTF-16BE"));
				internalMap->Append(ref new EncodingTable(1201, "UCS2BE"));
				internalMap->Append(ref new EncodingTable(1201, "UCS-2BE"));
				internalMap->Append(ref new EncodingTable(1201, "unicodeFFFE"));

				internalMap->Append(ref new EncodingTable(12000, "CP12000"));
				internalMap->Append(ref new EncodingTable(12000, "UTF32LE"));
				internalMap->Append(ref new EncodingTable(12000, "UTF-32LE"));
				internalMap->Append(ref new EncodingTable(12000, "UCS4LE"));
				internalMap->Append(ref new EncodingTable(12000, "UCS-4LE"));

				internalMap->Append(ref new EncodingTable(12001, "CP12001"));
				internalMap->Append(ref new EncodingTable(12001, "UTF32BE"));
				internalMap->Append(ref new EncodingTable(12001, "UTF-32BE"));
				internalMap->Append(ref new EncodingTable(12001, "UCS4BE"));
				internalMap->Append(ref new EncodingTable(12001, "UCS-4BE"));

				/* #ifndef GLIB_COMPILATION
				 * Default is big endian.
				 * See rfc2781 4.3 Interpreting text labelled as UTF-16.
				 */
				internalMap->Append(ref new EncodingTable(1201, "UTF16"));
				internalMap->Append(ref new EncodingTable(1201, "UTF-16"));
				internalMap->Append(ref new EncodingTable(1201, "UCS2"));
				internalMap->Append(ref new EncodingTable(1201, "UCS-2"));
				internalMap->Append(ref new EncodingTable(12001, "UTF32"));
				internalMap->Append(ref new EncodingTable(12001, "UTF-32"));
				internalMap->Append(ref new EncodingTable(12001, "UCS-4"));
				internalMap->Append(ref new EncodingTable(12001, "UCS4"));

				/*
				#else
				Default is little endian, because the platform is */
				internalMap->Append(ref new EncodingTable(1200, "UTF16"));
				internalMap->Append(ref new EncodingTable(1200, "UTF-16"));
				internalMap->Append(ref new EncodingTable(1200, "UCS2"));
				internalMap->Append(ref new EncodingTable(1200, "UCS-2"));
				internalMap->Append(ref new EncodingTable(12000, "UTF32"));
				internalMap->Append(ref new EncodingTable(12000, "UTF-32"));
				internalMap->Append(ref new EncodingTable(12000, "UCS4"));
				internalMap->Append(ref new EncodingTable(12000, "UCS-4"));

				/*
				#endif

				copy from libiconv `iconv -l` */
				/* !IsValidCodePage(367) */
				internalMap->Append(ref new EncodingTable(20127, "ANSI_X3.4-1968"));
				internalMap->Append(ref new EncodingTable(20127, "ANSI_X3.4-1986"));
				internalMap->Append(ref new EncodingTable(20127, "ASCII"));
				internalMap->Append(ref new EncodingTable(20127, "CP367"));
				internalMap->Append(ref new EncodingTable(20127, "IBM367"));
				internalMap->Append(ref new EncodingTable(20127, "ISO-IR-6"));
				internalMap->Append(ref new EncodingTable(20127, "ISO646-US"));
				internalMap->Append(ref new EncodingTable(20127, "ISO_646.IRV:1991"));
				internalMap->Append(ref new EncodingTable(20127, "US"));
				internalMap->Append(ref new EncodingTable(20127, "US-ASCII"));
				internalMap->Append(ref new EncodingTable(20127, "CSASCII"));

				/* !IsValidCodePage(819) */
				internalMap->Append(ref new EncodingTable(1252, "CP819"));
				internalMap->Append(ref new EncodingTable(1252, "IBM819"));
				internalMap->Append(ref new EncodingTable(28591, "ISO-8859-1"));
				internalMap->Append(ref new EncodingTable(28591, "ISO-IR-100"));
				internalMap->Append(ref new EncodingTable(28591, "ISO8859-1"));
				internalMap->Append(ref new EncodingTable(28591, "ISO_8859-1"));
				internalMap->Append(ref new EncodingTable(28591, "ISO_8859-1:1987"));
				internalMap->Append(ref new EncodingTable(28591, "L1"));
				internalMap->Append(ref new EncodingTable(28591, "LATIN1"));
				internalMap->Append(ref new EncodingTable(28591, "CSISOLATIN1"));

				internalMap->Append(ref new EncodingTable(1250, "CP1250"));
				internalMap->Append(ref new EncodingTable(1250, "MS-EE"));
				internalMap->Append(ref new EncodingTable(1250, "WINDOWS-1250"));

				internalMap->Append(ref new EncodingTable(1251, "CP1251"));
				internalMap->Append(ref new EncodingTable(1251, "MS-CYRL"));
				internalMap->Append(ref new EncodingTable(1251, "WINDOWS-1251"));

				internalMap->Append(ref new EncodingTable(1252, "CP1252"));
				internalMap->Append(ref new EncodingTable(1252, "MS-ANSI"));
				internalMap->Append(ref new EncodingTable(1252, "WINDOWS-1252"));

				internalMap->Append(ref new EncodingTable(1253, "CP1253"));
				internalMap->Append(ref new EncodingTable(1253, "MS-GREEK"));
				internalMap->Append(ref new EncodingTable(1253, "WINDOWS-1253"));

				internalMap->Append(ref new EncodingTable(1254, "CP1254"));
				internalMap->Append(ref new EncodingTable(1254, "MS-TURK"));
				internalMap->Append(ref new EncodingTable(1254, "WINDOWS-1254"));

				internalMap->Append(ref new EncodingTable(1255, "CP1255"));
				internalMap->Append(ref new EncodingTable(1255, "MS-HEBR"));
				internalMap->Append(ref new EncodingTable(1255, "WINDOWS-1255"));

				internalMap->Append(ref new EncodingTable(1256, "CP1256"));
				internalMap->Append(ref new EncodingTable(1256, "MS-ARAB"));
				internalMap->Append(ref new EncodingTable(1256, "WINDOWS-1256"));

				internalMap->Append(ref new EncodingTable(1257, "CP1257"));
				internalMap->Append(ref new EncodingTable(1257, "WINBALTRIM"));
				internalMap->Append(ref new EncodingTable(1257, "WINDOWS-1257"));

				internalMap->Append(ref new EncodingTable(1258, "CP1258"));
				internalMap->Append(ref new EncodingTable(1258, "WINDOWS-1258"));

				internalMap->Append(ref new EncodingTable(850, "850"));
				internalMap->Append(ref new EncodingTable(850, "CP850"));
				internalMap->Append(ref new EncodingTable(850, "IBM850"));
				internalMap->Append(ref new EncodingTable(850, "CSPC850MULTILINGUAL"));

				/* !IsValidCodePage(862) */
				internalMap->Append(ref new EncodingTable(862, "862"));
				internalMap->Append(ref new EncodingTable(862, "CP862"));
				internalMap->Append(ref new EncodingTable(862, "IBM862"));
				internalMap->Append(ref new EncodingTable(862, "CSPC862LATINHEBREW"));

				internalMap->Append(ref new EncodingTable(866, "866"));
				internalMap->Append(ref new EncodingTable(866, "CP866"));
				internalMap->Append(ref new EncodingTable(866, "IBM866"));
				internalMap->Append(ref new EncodingTable(866, "CSIBM866"));

				/* !IsValidCodePage(154) */
				internalMap->Append(ref new EncodingTable(154, "CP154"));
				internalMap->Append(ref new EncodingTable(154, "CYRILLIC-ASIAN"));
				internalMap->Append(ref new EncodingTable(154, "PT154"));
				internalMap->Append(ref new EncodingTable(154, "PTCP154"));
				internalMap->Append(ref new EncodingTable(154, "CSPTCP154"));

				/* !IsValidCodePage(1133) */
				internalMap->Append(ref new EncodingTable(1133, "CP1133"));
				internalMap->Append(ref new EncodingTable(1133, "IBM-CP1133"));

				internalMap->Append(ref new EncodingTable(874, "CP874"));
				internalMap->Append(ref new EncodingTable(874, "WINDOWS-874"));

				/* !IsValidCodePage(51932) */
				internalMap->Append(ref new EncodingTable(51932, "CP51932"));
				internalMap->Append(ref new EncodingTable(51932, "MS51932"));
				internalMap->Append(ref new EncodingTable(51932, "WINDOWS-51932"));
				internalMap->Append(ref new EncodingTable(51932, "EUC-JP"));

				internalMap->Append(ref new EncodingTable(932, "CP932"));
				internalMap->Append(ref new EncodingTable(932, "MS932"));
				internalMap->Append(ref new EncodingTable(932, "SHIFFT_JIS"));
				internalMap->Append(ref new EncodingTable(932, "SHIFFT_JIS-MS"));
				internalMap->Append(ref new EncodingTable(932, "SJIS"));
				internalMap->Append(ref new EncodingTable(932, "SJIS-MS"));
				internalMap->Append(ref new EncodingTable(932, "SJIS-OPEN"));
				internalMap->Append(ref new EncodingTable(932, "SJIS-WIN"));
				internalMap->Append(ref new EncodingTable(932, "WINDOWS-31J"));
				internalMap->Append(ref new EncodingTable(932, "WINDOWS-932"));
				internalMap->Append(ref new EncodingTable(932, "CSWINDOWS31J"));

				internalMap->Append(ref new EncodingTable(50221, "CP50221"));
				internalMap->Append(ref new EncodingTable(50221, "ISO-2022-JP"));
				internalMap->Append(ref new EncodingTable(50221, "ISO-2022-JP-MS"));
				internalMap->Append(ref new EncodingTable(50221, "ISO2022-JP"));
				internalMap->Append(ref new EncodingTable(50221, "ISO2022-JP-MS"));
				internalMap->Append(ref new EncodingTable(50221, "MS50221"));
				internalMap->Append(ref new EncodingTable(50221, "WINDOWS-50221"));

				internalMap->Append(ref new EncodingTable(936, "CP936"));
				internalMap->Append(ref new EncodingTable(936, "GBK"));
				internalMap->Append(ref new EncodingTable(936, "MS936"));
				internalMap->Append(ref new EncodingTable(936, "WINDOWS-936"));

				internalMap->Append(ref new EncodingTable(950, "CP950"));
				internalMap->Append(ref new EncodingTable(950, "BIG5"));
				internalMap->Append(ref new EncodingTable(950, "BIG5HKSCS"));
				internalMap->Append(ref new EncodingTable(950, "BIG5-HKSCS"));

				internalMap->Append(ref new EncodingTable(949, "CP949"));
				internalMap->Append(ref new EncodingTable(949, "UHC"));
				internalMap->Append(ref new EncodingTable(949, "EUC-KR"));

				internalMap->Append(ref new EncodingTable(1361, "CP1361"));
				internalMap->Append(ref new EncodingTable(1361, "JOHAB"));

				internalMap->Append(ref new EncodingTable(437, "437"));
				internalMap->Append(ref new EncodingTable(437, "CP437"));
				internalMap->Append(ref new EncodingTable(437, "IBM437"));
				internalMap->Append(ref new EncodingTable(437, "CSPC8CODEPAGE437"));

				internalMap->Append(ref new EncodingTable(737, "CP737"));

				internalMap->Append(ref new EncodingTable(775, "CP775"));
				internalMap->Append(ref new EncodingTable(775, "IBM775"));
				internalMap->Append(ref new EncodingTable(775, "CSPC775BALTIC"));

				internalMap->Append(ref new EncodingTable(852, "852"));
				internalMap->Append(ref new EncodingTable(852, "CP852"));
				internalMap->Append(ref new EncodingTable(852, "IBM852"));
				internalMap->Append(ref new EncodingTable(852, "CSPCP852"));

				/* !IsValidCodePage(853) */
				internalMap->Append(ref new EncodingTable(853, "CP853"));

				internalMap->Append(ref new EncodingTable(855, "855"));
				internalMap->Append(ref new EncodingTable(855, "CP855"));
				internalMap->Append(ref new EncodingTable(855, "IBM855"));
				internalMap->Append(ref new EncodingTable(855, "CSIBM855"));

				internalMap->Append(ref new EncodingTable(857, "857"));
				internalMap->Append(ref new EncodingTable(857, "CP857"));
				internalMap->Append(ref new EncodingTable(857, "IBM857"));
				internalMap->Append(ref new EncodingTable(857, "CSIBM857"));

				/* !IsValidCodePage(858) */
				internalMap->Append(ref new EncodingTable(858, "CP858"));

				internalMap->Append(ref new EncodingTable(860, "860"));
				internalMap->Append(ref new EncodingTable(860, "CP860"));
				internalMap->Append(ref new EncodingTable(860, "IBM860"));
				internalMap->Append(ref new EncodingTable(860, "CSIBM860"));

				internalMap->Append(ref new EncodingTable(861, "861"));
				internalMap->Append(ref new EncodingTable(861, "CP-IS"));
				internalMap->Append(ref new EncodingTable(861, "CP861"));
				internalMap->Append(ref new EncodingTable(861, "IBM861"));
				internalMap->Append(ref new EncodingTable(861, "CSIBM861"));

				internalMap->Append(ref new EncodingTable(863, "863"));
				internalMap->Append(ref new EncodingTable(863, "CP863"));
				internalMap->Append(ref new EncodingTable(863, "IBM863"));
				internalMap->Append(ref new EncodingTable(863, "CSIBM863"));

				internalMap->Append(ref new EncodingTable(864, "CP864"));
				internalMap->Append(ref new EncodingTable(864, "IBM864"));
				internalMap->Append(ref new EncodingTable(864, "CSIBM864"));

				internalMap->Append(ref new EncodingTable(865, "865"));
				internalMap->Append(ref new EncodingTable(865, "CP865"));
				internalMap->Append(ref new EncodingTable(865, "IBM865"));
				internalMap->Append(ref new EncodingTable(865, "CSIBM865"));

				internalMap->Append(ref new EncodingTable(869, "869"));
				internalMap->Append(ref new EncodingTable(869, "CP-GR"));
				internalMap->Append(ref new EncodingTable(869, "CP869"));
				internalMap->Append(ref new EncodingTable(869, "IBM869"));
				internalMap->Append(ref new EncodingTable(869, "CSIBM869"));

				/* !IsValidCodePage(1152) */
				internalMap->Append(ref new EncodingTable(1125, "CP1125"));

				/*
				 * Code Page Identifiers
				 * http://msdn2.microsoft.com/en-us/library/ms776446.aspx
				 */
				internalMap->Append(ref new EncodingTable(37, "IBM037")); /* IBM EBCDIC US-Canada */
				internalMap->Append(ref new EncodingTable(437, "IBM437")); /* OEM United States */
				internalMap->Append(ref new EncodingTable(500, "IBM500")); /* IBM EBCDIC International */
				internalMap->Append(ref new EncodingTable(708, "ASMO-708")); /* Arabic (ASMO 708) */
				/* 709 		Arabic (ASMO-449+, BCON V4) */
				/* 710 		Arabic - Transparent Arabic */
				internalMap->Append(ref new EncodingTable(720, "DOS-720")); /* Arabic (Transparent ASMO)); Arabic (DOS) */
				internalMap->Append(ref new EncodingTable(737, "ibm737")); /* OEM Greek (formerly 437G)); Greek (DOS) */
				internalMap->Append(ref new EncodingTable(775, "ibm775")); /* OEM Baltic; Baltic (DOS) */
				internalMap->Append(ref new EncodingTable(850, "ibm850")); /* OEM Multilingual Latin 1; Western European (DOS) */
				internalMap->Append(ref new EncodingTable(852, "ibm852")); /* OEM Latin 2; Central European (DOS) */
				internalMap->Append(ref new EncodingTable(855, "IBM855")); /* OEM Cyrillic (primarily Russian) */
				internalMap->Append(ref new EncodingTable(857, "ibm857")); /* OEM Turkish; Turkish (DOS) */
				internalMap->Append(ref new EncodingTable(858, "IBM00858")); /* OEM Multilingual Latin 1 + Euro symbol */
				internalMap->Append(ref new EncodingTable(860, "IBM860")); /* OEM Portuguese; Portuguese (DOS) */
				internalMap->Append(ref new EncodingTable(861, "ibm861")); /* OEM Icelandic; Icelandic (DOS) */
				internalMap->Append(ref new EncodingTable(862, "DOS-862")); /* OEM Hebrew; Hebrew (DOS) */
				internalMap->Append(ref new EncodingTable(863, "IBM863")); /* OEM French Canadian; French Canadian (DOS) */
				internalMap->Append(ref new EncodingTable(864, "IBM864")); /* OEM Arabic; Arabic (864) */
				internalMap->Append(ref new EncodingTable(865, "IBM865")); /* OEM Nordic; Nordic (DOS) */
				internalMap->Append(ref new EncodingTable(866, "cp866")); /* OEM Russian; Cyrillic (DOS) */
				internalMap->Append(ref new EncodingTable(869, "ibm869")); /* OEM Modern Greek; Greek, Modern (DOS) */
				internalMap->Append(ref new EncodingTable(870, "IBM870")); /* IBM EBCDIC Multilingual/ROECE (Latin 2)); IBM EBCDIC Multilingual Latin 2 */
				internalMap->Append(ref new EncodingTable(874, "windows-874")); /* ANSI/OEM Thai (same as 28605, ISO 8859-15)); Thai (Windows) */
				internalMap->Append(ref new EncodingTable(875, "cp875")); /* IBM EBCDIC Greek Modern */
				internalMap->Append(ref new EncodingTable(932, "shift_jis")); /* ANSI/OEM Japanese; Japanese (Shift-JIS) */
				internalMap->Append(ref new EncodingTable(932, "shift-jis")); /* alternative name for it */
				internalMap->Append(ref new EncodingTable(936, "gb2312")); /* ANSI/OEM Simplified Chinese (PRC, Singapore)); Chinese Simplified (GB2312) */
				internalMap->Append(ref new EncodingTable(949, "ks_c_5601-1987")); /* ANSI/OEM Korean (Unified Hangul Code) */
				internalMap->Append(ref new EncodingTable(950, "big5")); /* ANSI/OEM Traditional Chinese (Taiwan; Hong Kong SAR, PRC)); Chinese Traditional (Big5) */
				internalMap->Append(ref new EncodingTable(950, "big5hkscs")); /* ANSI/OEM Traditional Chinese (Hong Kong SAR)); Chinese Traditional (Big5-HKSCS) */
				internalMap->Append(ref new EncodingTable(950, "big5-hkscs")); /* alternative name for it */
				internalMap->Append(ref new EncodingTable(1026, "IBM1026")); /* IBM EBCDIC Turkish (Latin 5) */
				internalMap->Append(ref new EncodingTable(1047, "IBM01047")); /* IBM EBCDIC Latin 1/Open System */
				internalMap->Append(ref new EncodingTable(1140, "IBM01140")); /* IBM EBCDIC US-Canada (037 + Euro symbol)); IBM EBCDIC (US-Canada-Euro) */
				internalMap->Append(ref new EncodingTable(1141, "IBM01141")); /* IBM EBCDIC Germany (20273 + Euro symbol)); IBM EBCDIC (Germany-Euro) */
				internalMap->Append(ref new EncodingTable(1142, "IBM01142")); /* IBM EBCDIC Denmark-Norway (20277 + Euro symbol)); IBM EBCDIC (Denmark-Norway-Euro) */
				internalMap->Append(ref new EncodingTable(1143, "IBM01143")); /* IBM EBCDIC Finland-Sweden (20278 + Euro symbol)); IBM EBCDIC (Finland-Sweden-Euro) */
				internalMap->Append(ref new EncodingTable(1144, "IBM01144")); /* IBM EBCDIC Italy (20280 + Euro symbol)); IBM EBCDIC (Italy-Euro) */
				internalMap->Append(ref new EncodingTable(1145, "IBM01145")); /* IBM EBCDIC Latin America-Spain (20284 + Euro symbol)); IBM EBCDIC (Spain-Euro) */
				internalMap->Append(ref new EncodingTable(1146, "IBM01146")); /* IBM EBCDIC United Kingdom (20285 + Euro symbol)); IBM EBCDIC (UK-Euro) */
				internalMap->Append(ref new EncodingTable(1147, "IBM01147")); /* IBM EBCDIC France (20297 + Euro symbol)); IBM EBCDIC (France-Euro) */
				internalMap->Append(ref new EncodingTable(1148, "IBM01148")); /* IBM EBCDIC International (500 + Euro symbol)); IBM EBCDIC (International-Euro) */
				internalMap->Append(ref new EncodingTable(1149, "IBM01149")); /* IBM EBCDIC Icelandic (20871 + Euro symbol)); IBM EBCDIC (Icelandic-Euro) */
				internalMap->Append(ref new EncodingTable(1250, "windows-1250")); /* ANSI Central European; Central European (Windows) */
				internalMap->Append(ref new EncodingTable(1251, "windows-1251")); /* ANSI Cyrillic; Cyrillic (Windows) */
				internalMap->Append(ref new EncodingTable(1252, "windows-1252")); /* ANSI Latin 1; Western European (Windows) */
				internalMap->Append(ref new EncodingTable(1253, "windows-1253")); /* ANSI Greek; Greek (Windows) */
				internalMap->Append(ref new EncodingTable(1254, "windows-1254")); /* ANSI Turkish; Turkish (Windows) */
				internalMap->Append(ref new EncodingTable(1255, "windows-1255")); /* ANSI Hebrew; Hebrew (Windows) */
				internalMap->Append(ref new EncodingTable(1256, "windows-1256")); /* ANSI Arabic; Arabic (Windows) */
				internalMap->Append(ref new EncodingTable(1257, "windows-1257")); /* ANSI Baltic; Baltic (Windows) */
				internalMap->Append(ref new EncodingTable(1258, "windows-1258")); /* ANSI/OEM Vietnamese; Vietnamese (Windows) */
				internalMap->Append(ref new EncodingTable(1361, "Johab")); /* Korean (Johab) */
				internalMap->Append(ref new EncodingTable(10000, "macintosh")); /* MAC Roman; Western European (Mac) */
				internalMap->Append(ref new EncodingTable(10001, "x-mac-japanese")); /* Japanese (Mac) */
				internalMap->Append(ref new EncodingTable(10002, "x-mac-chinesetrad")); /* MAC Traditional Chinese (Big5)); Chinese Traditional (Mac) */
				internalMap->Append(ref new EncodingTable(10003, "x-mac-korean")); /* Korean (Mac) */
				internalMap->Append(ref new EncodingTable(10004, "x-mac-arabic")); /* Arabic (Mac) */
				internalMap->Append(ref new EncodingTable(10005, "x-mac-hebrew")); /* Hebrew (Mac) */
				internalMap->Append(ref new EncodingTable(10006, "x-mac-greek")); /* Greek (Mac) */
				internalMap->Append(ref new EncodingTable(10007, "x-mac-cyrillic")); /* Cyrillic (Mac) */
				internalMap->Append(ref new EncodingTable(10008, "x-mac-chinesesimp")); /* MAC Simplified Chinese (GB 2312)); Chinese Simplified (Mac) */
				internalMap->Append(ref new EncodingTable(10010, "x-mac-romanian")); /* Romanian (Mac) */
				internalMap->Append(ref new EncodingTable(10017, "x-mac-ukrainian")); /* Ukrainian (Mac) */
				internalMap->Append(ref new EncodingTable(10021, "x-mac-thai")); /* Thai (Mac) */
				internalMap->Append(ref new EncodingTable(10029, "x-mac-ce")); /* MAC Latin 2; Central European (Mac) */
				internalMap->Append(ref new EncodingTable(10079, "x-mac-icelandic")); /* Icelandic (Mac) */
				internalMap->Append(ref new EncodingTable(10081, "x-mac-turkish")); /* Turkish (Mac) */
				internalMap->Append(ref new EncodingTable(10082, "x-mac-croatian")); /* Croatian (Mac) */
				internalMap->Append(ref new EncodingTable(20000, "x-Chinese_CNS")); /* CNS Taiwan; Chinese Traditional (CNS) */
				internalMap->Append(ref new EncodingTable(20001, "x-cp20001")); /* TCA Taiwan */
				internalMap->Append(ref new EncodingTable(20002, "x_Chinese-Eten")); /* Eten Taiwan; Chinese Traditional (Eten) */
				internalMap->Append(ref new EncodingTable(20003, "x-cp20003")); /* IBM5550 Taiwan */
				internalMap->Append(ref new EncodingTable(20004, "x-cp20004")); /* TeleText Taiwan */
				internalMap->Append(ref new EncodingTable(20005, "x-cp20005")); /* Wang Taiwan */
				internalMap->Append(ref new EncodingTable(20105, "x-IA5")); /* IA5 (IRV International Alphabet No. 5, 7-bit)); Western European (IA5) */
				internalMap->Append(ref new EncodingTable(20106, "x-IA5-German")); /* IA5 German (7-bit) */
				internalMap->Append(ref new EncodingTable(20107, "x-IA5-Swedish")); /* IA5 Swedish (7-bit) */
				internalMap->Append(ref new EncodingTable(20108, "x-IA5-Norwegian")); /* IA5 Norwegian (7-bit) */
				internalMap->Append(ref new EncodingTable(20127, "us-ascii")); /* US-ASCII (7-bit) */
				internalMap->Append(ref new EncodingTable(20261, "x-cp20261")); /* T.61 */
				internalMap->Append(ref new EncodingTable(20269, "x-cp20269")); /* ISO 6937 Non-Spacing Accent */
				internalMap->Append(ref new EncodingTable(20273, "IBM273")); /* IBM EBCDIC Germany */
				internalMap->Append(ref new EncodingTable(20277, "IBM277")); /* IBM EBCDIC Denmark-Norway */
				internalMap->Append(ref new EncodingTable(20278, "IBM278")); /* IBM EBCDIC Finland-Sweden */
				internalMap->Append(ref new EncodingTable(20280, "IBM280")); /* IBM EBCDIC Italy */
				internalMap->Append(ref new EncodingTable(20284, "IBM284")); /* IBM EBCDIC Latin America-Spain */
				internalMap->Append(ref new EncodingTable(20285, "IBM285")); /* IBM EBCDIC United Kingdom */
				internalMap->Append(ref new EncodingTable(20290, "IBM290")); /* IBM EBCDIC Japanese Katakana Extended */
				internalMap->Append(ref new EncodingTable(20297, "IBM297")); /* IBM EBCDIC France */
				internalMap->Append(ref new EncodingTable(20420, "IBM420")); /* IBM EBCDIC Arabic */
				internalMap->Append(ref new EncodingTable(20423, "IBM423")); /* IBM EBCDIC Greek */
				internalMap->Append(ref new EncodingTable(20424, "IBM424")); /* IBM EBCDIC Hebrew */
				internalMap->Append(ref new EncodingTable(20833, "x-EBCDIC-KoreanExtended")); /* IBM EBCDIC Korean Extended */
				internalMap->Append(ref new EncodingTable(20838, "IBM-Thai")); /* IBM EBCDIC Thai */
				internalMap->Append(ref new EncodingTable(20866, "koi8-r")); /* Russian (KOI8-R)); Cyrillic (KOI8-R) */
				internalMap->Append(ref new EncodingTable(20871, "IBM871")); /* IBM EBCDIC Icelandic */
				internalMap->Append(ref new EncodingTable(20880, "IBM880")); /* IBM EBCDIC Cyrillic Russian */
				internalMap->Append(ref new EncodingTable(20905, "IBM905")); /* IBM EBCDIC Turkish */
				internalMap->Append(ref new EncodingTable(20924, "IBM00924")); /* IBM EBCDIC Latin 1/Open System (1047 + Euro symbol) */
				internalMap->Append(ref new EncodingTable(20932, "EUC-JP")); /* Japanese (JIS 0208-1990 and 0121-1990) */
				internalMap->Append(ref new EncodingTable(20936, "x-cp20936")); /* Simplified Chinese (GB2312)); Chinese Simplified (GB2312-80) */
				internalMap->Append(ref new EncodingTable(20949, "x-cp20949")); /* Korean Wansung */
				internalMap->Append(ref new EncodingTable(21025, "cp1025")); /* IBM EBCDIC Cyrillic Serbian-Bulgarian */
				/* 21027 		(deprecated) */
				internalMap->Append(ref new EncodingTable(21866, "koi8-u")); /* Ukrainian (KOI8-U)); Cyrillic (KOI8-U) */
				internalMap->Append(ref new EncodingTable(28591, "iso-8859-1")); /* ISO 8859-1 Latin 1; Western European (ISO) */
				internalMap->Append(ref new EncodingTable(28591, "iso8859-1")); /* ISO 8859-1 Latin 1; Western European (ISO) */
				internalMap->Append(ref new EncodingTable(28591, "iso_8859-1"));
				internalMap->Append(ref new EncodingTable(28591, "iso_8859_1"));
				internalMap->Append(ref new EncodingTable(28592, "iso-8859-2")); /* ISO 8859-2 Central European; Central European (ISO) */
				internalMap->Append(ref new EncodingTable(28592, "iso8859-2")); /* ISO 8859-2 Central European; Central European (ISO) */
				internalMap->Append(ref new EncodingTable(28592, "iso_8859-2"));
				internalMap->Append(ref new EncodingTable(28592, "iso_8859_2"));
				internalMap->Append(ref new EncodingTable(28593, "iso-8859-3")); /* ISO 8859-3 Latin 3 */
				internalMap->Append(ref new EncodingTable(28593, "iso8859-3")); /* ISO 8859-3 Latin 3 */
				internalMap->Append(ref new EncodingTable(28593, "iso_8859-3"));
				internalMap->Append(ref new EncodingTable(28593, "iso_8859_3"));
				internalMap->Append(ref new EncodingTable(28594, "iso-8859-4")); /* ISO 8859-4 Baltic */
				internalMap->Append(ref new EncodingTable(28594, "iso8859-4")); /* ISO 8859-4 Baltic */
				internalMap->Append(ref new EncodingTable(28594, "iso_8859-4"));
				internalMap->Append(ref new EncodingTable(28594, "iso_8859_4"));
				internalMap->Append(ref new EncodingTable(28595, "iso-8859-5")); /* ISO 8859-5 Cyrillic */
				internalMap->Append(ref new EncodingTable(28595, "iso8859-5")); /* ISO 8859-5 Cyrillic */
				internalMap->Append(ref new EncodingTable(28595, "iso_8859-5"));
				internalMap->Append(ref new EncodingTable(28595, "iso_8859_5"));
				internalMap->Append(ref new EncodingTable(28596, "iso-8859-6")); /* ISO 8859-6 Arabic */
				internalMap->Append(ref new EncodingTable(28596, "iso8859-6")); /* ISO 8859-6 Arabic */
				internalMap->Append(ref new EncodingTable(28596, "iso_8859-6"));
				internalMap->Append(ref new EncodingTable(28596, "iso_8859_6"));
				internalMap->Append(ref new EncodingTable(28597, "iso-8859-7")); /* ISO 8859-7 Greek */
				internalMap->Append(ref new EncodingTable(28597, "iso8859-7")); /* ISO 8859-7 Greek */
				internalMap->Append(ref new EncodingTable(28597, "iso_8859-7"));
				internalMap->Append(ref new EncodingTable(28597, "iso_8859_7"));
				internalMap->Append(ref new EncodingTable(28598, "iso-8859-8")); /* ISO 8859-8 Hebrew; Hebrew (ISO-Visual) */
				internalMap->Append(ref new EncodingTable(28598, "iso8859-8")); /* ISO 8859-8 Hebrew; Hebrew (ISO-Visual) */
				internalMap->Append(ref new EncodingTable(28598, "iso_8859-8"));
				internalMap->Append(ref new EncodingTable(28598, "iso_8859_8"));
				internalMap->Append(ref new EncodingTable(28599, "iso-8859-9")); /* ISO 8859-9 Turkish */
				internalMap->Append(ref new EncodingTable(28599, "iso8859-9")); /* ISO 8859-9 Turkish */
				internalMap->Append(ref new EncodingTable(28599, "iso_8859-9"));
				internalMap->Append(ref new EncodingTable(28599, "iso_8859_9"));
				internalMap->Append(ref new EncodingTable(28603, "iso-8859-13")); /* ISO 8859-13 Estonian */
				internalMap->Append(ref new EncodingTable(28603, "iso8859-13")); /* ISO 8859-13 Estonian */
				internalMap->Append(ref new EncodingTable(28603, "iso_8859-13"));
				internalMap->Append(ref new EncodingTable(28603, "iso_8859_13"));
				internalMap->Append(ref new EncodingTable(28605, "iso-8859-15")); /* ISO 8859-15 Latin 9 */
				internalMap->Append(ref new EncodingTable(28605, "iso8859-15")); /* ISO 8859-15 Latin 9 */
				internalMap->Append(ref new EncodingTable(28605, "iso_8859-15"));
				internalMap->Append(ref new EncodingTable(28605, "iso_8859_15"));
				internalMap->Append(ref new EncodingTable(29001, "x-Europa")); /* Europa 3 */
				internalMap->Append(ref new EncodingTable(38598, "iso-8859-8-i")); /* ISO 8859-8 Hebrew; Hebrew (ISO-Logical) */
				internalMap->Append(ref new EncodingTable(38598, "iso8859-8-i")); /* ISO 8859-8 Hebrew; Hebrew (ISO-Logical) */
				internalMap->Append(ref new EncodingTable(38598, "iso_8859-8-i"));
				internalMap->Append(ref new EncodingTable(38598, "iso_8859_8-i"));
				internalMap->Append(ref new EncodingTable(50220, "iso-2022-jp")); /* ISO 2022 Japanese with no halfwidth Katakana; Japanese (JIS) */
				internalMap->Append(ref new EncodingTable(50221, "csISO2022JP")); /* ISO 2022 Japanese with halfwidth Katakana; Japanese (JIS-Allow 1 byte Kana) */
				internalMap->Append(ref new EncodingTable(50222, "iso-2022-jp")); /* ISO 2022 Japanese JIS X 0201-1989; Japanese (JIS-Allow 1 byte Kana - SO/SI) */
				internalMap->Append(ref new EncodingTable(50225, "iso-2022-kr")); /* ISO 2022 Korean */
				internalMap->Append(ref new EncodingTable(50225, "iso2022-kr")); /* ISO 2022 Korean */
				internalMap->Append(ref new EncodingTable(50227, "x-cp50227")); /* ISO 2022 Simplified Chinese; Chinese Simplified (ISO 2022) */
				/* 50229 		ISO 2022 Traditional Chinese */
				/* 50930 		EBCDIC Japanese (Katakana) Extended */
				/* 50931 		EBCDIC US-Canada and Japanese */
				/* 50933 		EBCDIC Korean Extended and Korean */
				/* 50935 		EBCDIC Simplified Chinese Extended and Simplified Chinese */
				/* 50936 		EBCDIC Simplified Chinese */
				/* 50937 		EBCDIC US-Canada and Traditional Chinese */
				/* 50939 		EBCDIC Japanese (Latin) Extended and Japanese */
				internalMap->Append(ref new EncodingTable(51932, "euc-jp")); /* EUC Japanese */
				internalMap->Append(ref new EncodingTable(51936, "EUC-CN")); /* EUC Simplified Chinese; Chinese Simplified (EUC) */
				internalMap->Append(ref new EncodingTable(51949, "euc-kr")); /* EUC Korean */
				/* 51950 		EUC Traditional Chinese */
				internalMap->Append(ref new EncodingTable(52936, "hz-gb-2312")); /* HZ-GB2312 Simplified Chinese; Chinese Simplified (HZ) */
				internalMap->Append(ref new EncodingTable(54936, "GB18030")); /* Windows XP and later: GB18030 Simplified Chinese (4 byte)); Chinese Simplified (GB18030) */
				internalMap->Append(ref new EncodingTable(57002, "x-iscii-de")); /* ISCII Devanagari */
				internalMap->Append(ref new EncodingTable(57003, "x-iscii-be")); /* ISCII Bengali */
				internalMap->Append(ref new EncodingTable(57004, "x-iscii-ta")); /* ISCII Tamil */
				internalMap->Append(ref new EncodingTable(57005, "x-iscii-te")); /* ISCII Telugu */
				internalMap->Append(ref new EncodingTable(57006, "x-iscii-as")); /* ISCII Assamese */
				internalMap->Append(ref new EncodingTable(57007, "x-iscii-or")); /* ISCII Oriya */
				internalMap->Append(ref new EncodingTable(57008, "x-iscii-ka")); /* ISCII Kannada */
				internalMap->Append(ref new EncodingTable(57009, "x-iscii-ma")); /* ISCII Malayalam */
				internalMap->Append(ref new EncodingTable(57010, "x-iscii-gu")); /* ISCII Gujarati */
				internalMap->Append(ref new EncodingTable(57011, "x-iscii-pa")); /* ISCII Punjabi */

				internalMap->Append(ref new EncodingTable(0, nullptr));
			}
			return internalMap->GetView();

		}

		static EncodingTable^ GetCurrentSystemDefault()
		{
			unsigned int systemEncodingTable = GetACP();
			//maybe use a map?
			auto tables = GetTables();
			for (int i = 0; i < tables->Size; i++)
			{
				auto current = tables->GetAt(i);
				if (current->WindowsEncodingTable == systemEncodingTable)
				{
					return current;
				}
			}

			return nullptr;
		}
	};
}


