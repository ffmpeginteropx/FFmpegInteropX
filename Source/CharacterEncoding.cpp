#include "pch.h"
#include "CharacterEncoding.h"
#include "CharacterEncoding.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    CharacterEncoding::CharacterEncoding() {}

    Windows::Foundation::Collections::IVectorView<FFmpegInteropX::CharacterEncoding> CharacterEncoding::GetCharacterEncodings()
    {
        if (internalView == nullptr)
        {
            mutex.lock();
            try
            {
                if (internalView == nullptr)
                {
                    //populate and create the vector.
                    internalMap = winrt::single_threaded_observable_vector<winrt::FFmpegInteropX::CharacterEncoding>();
                    internalMap.Append(winrt::make_self<CharacterEncoding>(65001, L"CP65001", L"Unicode (UTF-8)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1200, L"CP1200", L"Unicode UTF-16, little endian byte order (BMP of ISO 10646)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1201, L"CP1201", L"Unicode UTF-16, big endian byte order").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(12000, L"CP12000", L"Unicode UTF-32, little endian byte order").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(12001, L"CP12001", L"Unicode UTF-32, big endian byte order").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(154, L"CP154", L"Cyrillic-asian").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1133, L"CP1133", L"IBM Lao Script").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(853, L"CP853", L" DOS for Turkish, Maltese, and Esperanto.").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1125, L"CP1125", L"Legacy Latin").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(37, L"IBM037", L"IBM EBCDIC US-Canada").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(437, L"IBM437", L"OEM United States").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(500, L"IBM500", L"IBM EBCDIC International").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(708, L"ASMO-708", L"Arabic (ASMO 708)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(720, L"DOS-720", L"Arabic (Transparent ASMO)); Arabic (DOS)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(737, L"ibm737", L"OEM Greek (formerly 437G)); Greek (DOS)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(775, L"ibm775", L"OEM Baltic; Baltic (DOS)").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(850, L"ibm850", L"OEM Multilingual Latin 1; Western European (DOS)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(852, L"ibm852", L"OEM Latin 2; Central European (DOS)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(855, L"IBM855", L"OEM Cyrillic (primarily Russian)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(857, L"ibm857", L"OEM Turkish; Turkish (DOS)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(858, L"IBM00858", L"OEM Multilingual Latin 1 + Euro symbol").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(860, L"IBM860", L"OEM Portuguese; Portuguese (DOS)").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(861, L"ibm861", L"OEM Icelandic; Icelandic (DOS)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(862, L"DOS-862", L"OEM Hebrew; Hebrew (DOS)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(863, L"IBM863", L"OEM French Canadian; French Canadian (DOS)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(864, L"IBM864", L"OEM Arabic; Arabic (864)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(865, L"IBM865", L"OEM Nordic; Nordic (DOS)").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(866, L"cp866", L"OEM Russian; Cyrillic (DOS)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(869, L"ibm869", L"OEM Modern Greek; Greek, Modern (DOS)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(870, L"IBM870", L"IBM EBCDIC Multilingual/ROECE (Latin 2)); IBM EBCDIC Multilingual Latin 2").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(874, L"windows-874", L"ANSI/OEM Thai (same as 28605, ISO 8859-15)); Thai (Windows)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(875, L"cp875", L"IBM EBCDIC Greek Modern").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(932, L"shift_jis", L"ANSI/OEM Japanese; Japanese (Shift-JIS)").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(936, L"gb2312", L"ANSI/OEM Simplified Chinese (PRC, Singapore)); Chinese Simplified (GB2312)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(949, L"ks_c_5601-1987", L"ANSI/OEM Korean (Unified Hangul Code)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(950, L"big5", L"ANSI/OEM Traditional Chinese (Taiwan; Hong Kong SAR, PRC)); Chinese Traditional (Big5)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1026, L"IBM1026", L"IBM EBCDIC Turkish (Latin 5)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1047, L"IBM01047", L"IBM EBCDIC Latin 1/Open System").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1140, L"IBM01140", L"IBM EBCDIC US-Canada (037 + Euro symbol)); IBM EBCDIC (US-Canada-Euro)").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(1141, L"IBM01141", L"IBM EBCDIC Germany (20273 + Euro symbol)); IBM EBCDIC (Germany-Euro)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1142, L"IBM01142", L"IBM EBCDIC Denmark-Norway (20277 + Euro symbol)); IBM EBCDIC (Denmark-Norway-Euro)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1143, L"IBM01143", L"IBM EBCDIC Finland-Sweden (20278 + Euro symbol)); IBM EBCDIC (Finland-Sweden-Euro)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1144, L"IBM01144", L"IBM EBCDIC Italy (20280 + Euro symbol)); IBM EBCDIC (Italy-Euro)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1145, L"IBM01145", L"IBM EBCDIC Latin America-Spain (20284 + Euro symbol)); IBM EBCDIC (Spain-Euro)").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(1146, L"IBM01146", L"IBM EBCDIC United Kingdom (20285 + Euro symbol)); IBM EBCDIC (UK-Euro)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1147, L"IBM01147", L"IBM EBCDIC France (20297 + Euro symbol)); IBM EBCDIC (France-Euro)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1148, L"IBM01148", L"IBM EBCDIC International (500 + Euro symbol)); IBM EBCDIC (International-Euro)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1149, L"IBM01149", L"IBM EBCDIC Icelandic (20871 + Euro symbol)); IBM EBCDIC (Icelandic-Euro)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1250, L"windows-1250", L"ANSI Central European; Central European (Windows)").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(1251, L"windows-1251", L"ANSI Cyrillic; Cyrillic (Windows)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1252, L"windows-1252", L"ANSI Latin 1; Western European (Windows)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1253, L"windows-1253", L"ANSI Greek; Greek (Windows)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1254, L"windows-1254", L"ANSI Turkish; Turkish (Windows)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1255, L"windows-1255", L"ANSI Hebrew; Hebrew (Windows)").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(1256, L"windows-1256", L"ANSI Arabic; Arabic (Windows)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1257, L"windows-1257", L"ANSI Baltic; Baltic (Windows)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1258, L"windows-1258", L"ANSI/OEM Vietnamese; Vietnamese (Windows)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(1361, L"Johab", L"Korean (Johab)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(10000, L"macintosh", L"MAC Roman; Western European (Mac)").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(10001, L"x-mac-japanese", L"Japanese (Mac)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(10002, L"x-mac-chinesetrad", L"MAC Traditional Chinese (Big5)); Chinese Traditional (Mac)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(10003, L"x-mac-korean", L"Korean (Mac)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(10004, L"x-mac-arabic", L"Arabic (Mac)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(10005, L"x-mac-hebrew", L"Hebrew (Mac)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(10006, L"x-mac-greek", L"Greek (Mac)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(10007, L"x-mac-cyrillic", L"Cyrillic (Mac)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(10008, L"x-mac-chinesesimp", L"MAC Simplified Chinese (GB 2312)); Chinese Simplified (Mac)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(10010, L"x-mac-romanian", L"Romanian (Mac)").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(10017, L"x-mac-ukrainian", L"Ukrainian (Mac)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(10021, L"x-mac-thai", L"Thai (Mac)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(10029, L"x-mac-ce", L"MAC Latin 2; Central European (Mac)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(10079, L"x-mac-icelandic", L"Icelandic (Mac)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(10081, L"x-mac-turkish", L"Turkish (Mac)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(10082, L"x-mac-croatian", L"Croatian (Mac)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20000, L"x-Chinese_CNS", L"CNS Taiwan; Chinese Traditional (CNS)").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(20001, L"x-cp20001", L"TCA Taiwan").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20002, L"x_Chinese-Eten", L"Eten Taiwan; Chinese Traditional (Eten)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20003, L"x-cp20003", L"IBM5550 Taiwan").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20004, L"x-cp20004", L"TeleText Taiwan").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20005, L"x-cp20005", L"Wang Taiwan").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20105, L"x-IA5", L"IA5 (IRV International Alphabet No. 5, 7-bit)); Western European (IA5)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20106, L"x-IA5-German", L"IA5 German (7-bit)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20107, L"x-IA5-Swedish", L"IA5 Swedish (7-bit)").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(20108, L"x-IA5-Norwegian", L"IA5 Norwegian (7-bit)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20127, L"us-ascii", L"US-ASCII (7-bit)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20261, L"x-cp20261", L"T.61").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20269, L"x-cp20269", L"ISO 6937 Non-Spacing Accent").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20273, L"IBM273", L"IBM EBCDIC Germany").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(20277, L"IBM277", L"IBM EBCDIC Denmark-Norway").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20278, L"IBM278", L"IBM EBCDIC Finland-Sweden").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20280, L"IBM280", L"IBM EBCDIC Italy").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20284, L"IBM284", L"IBM EBCDIC Latin America-Spain").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20285, L"IBM285", L"IBM EBCDIC United Kingdom").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20290, L"IBM290", L"IBM EBCDIC Japanese Katakana Extended").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20297, L"IBM297", L"IBM EBCDIC France").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20420, L"IBM420", L"IBM EBCDIC Arabic").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(20423, L"IBM423", L"IBM EBCDIC Greek").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20424, L"IBM424", L"IBM EBCDIC Hebrew").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20833, L"x-EBCDIC-KoreanExtended", L"IBM EBCDIC Korean Extended").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20838, L"IBM-Thai", L"IBM EBCDIC Thai").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20866, L"koi8-r", L"Russian (KOI8-R)); Cyrillic (KOI8-R)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20871, L"IBM871", L"IBM EBCDIC Icelandic").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20880, L"IBM880", L"IBM EBCDIC Cyrillic Russian").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(20905, L"IBM905", L"IBM EBCDIC Turkish").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20924, L"IBM00924", L"IBM EBCDIC Latin 1/Open System (1047 + Euro symbol)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20932, L"EUC-JP", L"Japanese (JIS 0208-1990 and 0121-1990)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20936, L"x-cp20936", L"Simplified Chinese (GB2312)); Chinese Simplified (GB2312-80)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(20949, L"x-cp20949", L"Korean Wansung").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(21025, L"cp1025", L"IBM EBCDIC Cyrillic Serbian-Bulgarian").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(21866, L"koi8-u", L"Ukrainian (KOI8-U)); Cyrillic (KOI8-U)").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(28591, L"iso-8859-1", L"ISO 8859-1 Latin 1; Western European (ISO)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(28592, L"iso-8859-2", L"ISO 8859-2 Central European; Central European (ISO)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(28593, L"iso-8859-3", L"ISO 8859-3 Latin 3").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(28594, L"iso-8859-4", L"ISO 8859-4 Baltic").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(28595, L"iso-8859-5", L"ISO 8859-5 Cyrillic").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(28596, L"iso-8859-6", L"ISO 8859-6 Arabic").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(28597, L"iso-8859-7", L"ISO 8859-7 Greek").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(28598, L"iso-8859-8", L"ISO 8859-8 Hebrew; Hebrew (ISO-Visual)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(28599, L"iso-8859-9", L"ISO 8859-9 Turkish").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(28603, L"iso-8859-13", L"ISO 8859-13 Estonian").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(28605, L"iso-8859-15", L"ISO 8859-15 Latin 9").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(29001, L"x-Europa", L"Europa 3").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(38598, L"iso-8859-8-i", L"ISO 8859-8 Hebrew; Hebrew (ISO-Logical)").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(50220, L"iso-2022-jp", L"ISO 2022 Japanese with no halfwidth Katakana; Japanese (JIS)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(50221, L"csISO2022JP", L"ISO 2022 Japanese with halfwidth Katakana; Japanese (JIS-Allow 1 byte Kana)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(50222, L"iso-2022-jp", L"ISO 2022 Japanese JIS X 0201-1989; Japanese (JIS-Allow 1 byte Kana - SO/SI)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(50225, L"iso-2022-kr", L"ISO 2022 Korean").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(50227, L"x-cp50227", L"ISO 2022 Simplified Chinese; Chinese Simplified (ISO 2022)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(51932, L"euc-jp", L"EUC Japanese").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(51936, L"EUC-CN", L"EUC Simplified Chinese; Chinese Simplified (EUC)").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(51949, L"euc-kr", L"EUC Korean").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(52936, L"hz-gb-2312", L"HZ-GB2312 Simplified Chinese; Chinese Simplified (HZ)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(54936, L"GB18030", L"Windows XP and later: GB18030 Simplified Chinese (4 byte)); Chinese Simplified (GB18030)").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(57002, L"x-iscii-de", L"ISCII Devanagari").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(57003, L"x-iscii-be", L"ISCII Bengali").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(57004, L"x-iscii-ta", L"ISCII Tamil").as< winrt::FFmpegInteropX::CharacterEncoding>());

                    internalMap.Append(winrt::make_self<CharacterEncoding>(57005, L"x-iscii-te", L"ISCII Telugu").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(57006, L"x-iscii-as", L"ISCII Assamese").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(57007, L"x-iscii-or", L"ISCII Oriya").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(57008, L"x-iscii-ka", L"ISCII Kannada").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(57009, L"x-iscii-ma", L"ISCII Malayalam").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(57010, L"x-iscii-gu", L"ISCII Gujarati").as< winrt::FFmpegInteropX::CharacterEncoding>());
                    internalMap.Append(winrt::make_self<CharacterEncoding>(57011, L"x-iscii-pa", L"ISCII Punjabi").as< winrt::FFmpegInteropX::CharacterEncoding>());
                }
            }
            catch (...)
            {
            }
            internalView = internalMap.GetView();
            mutex.unlock();
        }
        return internalView;
    }

    CharacterEncoding::CharacterEncoding(int p_codePage, hstring const& p_name, hstring const& p_description)
        :codePage(p_codePage),
        name(p_name),
        description(p_description)
    {
    }

    FFmpegInteropX::CharacterEncoding CharacterEncoding::GetSystemDefault()
    {
        return GetCharacterEncodings().GetAt(0);
    }

    hstring CharacterEncoding::Name()
    {
        return this->name;
    }

    hstring CharacterEncoding::Description()
    {
        return this->description;
    }

    int32_t CharacterEncoding::WindowsCodePage()
    {
        return this->codePage;
    }

    winrt::Windows::Foundation::Collections::IVector<winrt::FFmpegInteropX::CharacterEncoding> CharacterEncoding::internalMap{ nullptr };
    winrt::Windows::Foundation::Collections::IVectorView<winrt::FFmpegInteropX::CharacterEncoding> CharacterEncoding::internalView{ nullptr };
    std::mutex CharacterEncoding::mutex;

}
