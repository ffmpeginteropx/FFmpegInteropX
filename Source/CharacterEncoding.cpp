#include "pch.h"
#include "CharacterEncoding.h"
#include "CharacterEncoding.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    Windows::Foundation::Collections::IVectorView<FFmpegInteropX::CharacterEncoding> CharacterEncoding::GetCharacterEncodings()
    {
        if (internalView == nullptr)
        {
            std::lock_guard lock(mutex);
            try
            {
                if (internalView == nullptr)
                {
                    //populate and create the vector.
                    //observable vector is used to work around a bug in xaml/c++winrt which makes binding not work
                    internalMap = winrt::single_threaded_observable_vector<winrt::FFmpegInteropX::CharacterEncoding>();
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(0, L"", L"System locale"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(65001, L"CP65001", L"Unicode (UTF-8)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1200, L"CP1200", L"Unicode UTF-16, little endian byte order (BMP of ISO 10646)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1201, L"CP1201", L"Unicode UTF-16, big endian byte order"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(12000, L"CP12000", L"Unicode UTF-32, little endian byte order"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(12001, L"CP12001", L"Unicode UTF-32, big endian byte order"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(154, L"CP154", L"Cyrillic-asian"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1133, L"CP1133", L"IBM Lao Script"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(853, L"CP853", L" DOS for Turkish, Maltese, and Esperanto."));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1125, L"CP1125", L"Legacy Latin"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(37, L"IBM037", L"IBM EBCDIC US-Canada"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(437, L"IBM437", L"OEM United States"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(500, L"IBM500", L"IBM EBCDIC International"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(708, L"ASMO-708", L"Arabic (ASMO 708)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(720, L"DOS-720", L"Arabic (Transparent ASMO)); Arabic (DOS)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(737, L"ibm737", L"OEM Greek (formerly 437G)); Greek (DOS)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(775, L"ibm775", L"OEM Baltic; Baltic (DOS)"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(850, L"ibm850", L"OEM Multilingual Latin 1; Western European (DOS)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(852, L"ibm852", L"OEM Latin 2; Central European (DOS)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(855, L"IBM855", L"OEM Cyrillic (primarily Russian)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(857, L"ibm857", L"OEM Turkish; Turkish (DOS)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(858, L"IBM00858", L"OEM Multilingual Latin 1 + Euro symbol"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(860, L"IBM860", L"OEM Portuguese; Portuguese (DOS)"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(861, L"ibm861", L"OEM Icelandic; Icelandic (DOS)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(862, L"DOS-862", L"OEM Hebrew; Hebrew (DOS)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(863, L"IBM863", L"OEM French Canadian; French Canadian (DOS)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(864, L"IBM864", L"OEM Arabic; Arabic (864)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(865, L"IBM865", L"OEM Nordic; Nordic (DOS)"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(866, L"cp866", L"OEM Russian; Cyrillic (DOS)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(869, L"ibm869", L"OEM Modern Greek; Greek, Modern (DOS)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(870, L"IBM870", L"IBM EBCDIC Multilingual/ROECE (Latin 2)); IBM EBCDIC Multilingual Latin 2"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(874, L"windows-874", L"ANSI/OEM Thai (same as 28605, ISO 8859-15)); Thai (Windows)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(875, L"cp875", L"IBM EBCDIC Greek Modern"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(932, L"shift_jis", L"ANSI/OEM Japanese; Japanese (Shift-JIS)"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(936, L"gb2312", L"ANSI/OEM Simplified Chinese (PRC, Singapore)); Chinese Simplified (GB2312)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(949, L"ks_c_5601-1987", L"ANSI/OEM Korean (Unified Hangul Code)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(950, L"big5", L"ANSI/OEM Traditional Chinese (Taiwan; Hong Kong SAR, PRC)); Chinese Traditional (Big5)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1026, L"IBM1026", L"IBM EBCDIC Turkish (Latin 5)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1047, L"IBM01047", L"IBM EBCDIC Latin 1/Open System"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1140, L"IBM01140", L"IBM EBCDIC US-Canada (037 + Euro symbol)); IBM EBCDIC (US-Canada-Euro)"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1141, L"IBM01141", L"IBM EBCDIC Germany (20273 + Euro symbol)); IBM EBCDIC (Germany-Euro)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1142, L"IBM01142", L"IBM EBCDIC Denmark-Norway (20277 + Euro symbol)); IBM EBCDIC (Denmark-Norway-Euro)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1143, L"IBM01143", L"IBM EBCDIC Finland-Sweden (20278 + Euro symbol)); IBM EBCDIC (Finland-Sweden-Euro)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1144, L"IBM01144", L"IBM EBCDIC Italy (20280 + Euro symbol)); IBM EBCDIC (Italy-Euro)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1145, L"IBM01145", L"IBM EBCDIC Latin America-Spain (20284 + Euro symbol)); IBM EBCDIC (Spain-Euro)"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1146, L"IBM01146", L"IBM EBCDIC United Kingdom (20285 + Euro symbol)); IBM EBCDIC (UK-Euro)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1147, L"IBM01147", L"IBM EBCDIC France (20297 + Euro symbol)); IBM EBCDIC (France-Euro)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1148, L"IBM01148", L"IBM EBCDIC International (500 + Euro symbol)); IBM EBCDIC (International-Euro)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1149, L"IBM01149", L"IBM EBCDIC Icelandic (20871 + Euro symbol)); IBM EBCDIC (Icelandic-Euro)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1250, L"windows-1250", L"ANSI Central European; Central European (Windows)"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1251, L"windows-1251", L"ANSI Cyrillic; Cyrillic (Windows)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1252, L"windows-1252", L"ANSI Latin 1; Western European (Windows)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1253, L"windows-1253", L"ANSI Greek; Greek (Windows)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1254, L"windows-1254", L"ANSI Turkish; Turkish (Windows)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1255, L"windows-1255", L"ANSI Hebrew; Hebrew (Windows)"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1256, L"windows-1256", L"ANSI Arabic; Arabic (Windows)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1257, L"windows-1257", L"ANSI Baltic; Baltic (Windows)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1258, L"windows-1258", L"ANSI/OEM Vietnamese; Vietnamese (Windows)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(1361, L"Johab", L"Korean (Johab)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(10000, L"macintosh", L"MAC Roman; Western European (Mac)"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(10001, L"x-mac-japanese", L"Japanese (Mac)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(10002, L"x-mac-chinesetrad", L"MAC Traditional Chinese (Big5)); Chinese Traditional (Mac)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(10003, L"x-mac-korean", L"Korean (Mac)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(10004, L"x-mac-arabic", L"Arabic (Mac)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(10005, L"x-mac-hebrew", L"Hebrew (Mac)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(10006, L"x-mac-greek", L"Greek (Mac)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(10007, L"x-mac-cyrillic", L"Cyrillic (Mac)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(10008, L"x-mac-chinesesimp", L"MAC Simplified Chinese (GB 2312)); Chinese Simplified (Mac)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(10010, L"x-mac-romanian", L"Romanian (Mac)"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(10017, L"x-mac-ukrainian", L"Ukrainian (Mac)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(10021, L"x-mac-thai", L"Thai (Mac)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(10029, L"x-mac-ce", L"MAC Latin 2; Central European (Mac)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(10079, L"x-mac-icelandic", L"Icelandic (Mac)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(10081, L"x-mac-turkish", L"Turkish (Mac)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(10082, L"x-mac-croatian", L"Croatian (Mac)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20000, L"x-Chinese_CNS", L"CNS Taiwan; Chinese Traditional (CNS)"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20001, L"x-cp20001", L"TCA Taiwan"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20002, L"x_Chinese-Eten", L"Eten Taiwan; Chinese Traditional (Eten)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20003, L"x-cp20003", L"IBM5550 Taiwan"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20004, L"x-cp20004", L"TeleText Taiwan"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20005, L"x-cp20005", L"Wang Taiwan"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20105, L"x-IA5", L"IA5 (IRV International Alphabet No. 5, 7-bit)); Western European (IA5)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20106, L"x-IA5-German", L"IA5 German (7-bit)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20107, L"x-IA5-Swedish", L"IA5 Swedish (7-bit)"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20108, L"x-IA5-Norwegian", L"IA5 Norwegian (7-bit)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20127, L"us-ascii", L"US-ASCII (7-bit)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20261, L"x-cp20261", L"T.61"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20269, L"x-cp20269", L"ISO 6937 Non-Spacing Accent"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20273, L"IBM273", L"IBM EBCDIC Germany"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20277, L"IBM277", L"IBM EBCDIC Denmark-Norway"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20278, L"IBM278", L"IBM EBCDIC Finland-Sweden"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20280, L"IBM280", L"IBM EBCDIC Italy"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20284, L"IBM284", L"IBM EBCDIC Latin America-Spain"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20285, L"IBM285", L"IBM EBCDIC United Kingdom"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20290, L"IBM290", L"IBM EBCDIC Japanese Katakana Extended"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20297, L"IBM297", L"IBM EBCDIC France"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20420, L"IBM420", L"IBM EBCDIC Arabic"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20423, L"IBM423", L"IBM EBCDIC Greek"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20424, L"IBM424", L"IBM EBCDIC Hebrew"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20833, L"x-EBCDIC-KoreanExtended", L"IBM EBCDIC Korean Extended"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20838, L"IBM-Thai", L"IBM EBCDIC Thai"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20866, L"koi8-r", L"Russian (KOI8-R)); Cyrillic (KOI8-R)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20871, L"IBM871", L"IBM EBCDIC Icelandic"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20880, L"IBM880", L"IBM EBCDIC Cyrillic Russian"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20905, L"IBM905", L"IBM EBCDIC Turkish"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20924, L"IBM00924", L"IBM EBCDIC Latin 1/Open System (1047 + Euro symbol)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20932, L"EUC-JP", L"Japanese (JIS 0208-1990 and 0121-1990)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20936, L"x-cp20936", L"Simplified Chinese (GB2312)); Chinese Simplified (GB2312-80)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(20949, L"x-cp20949", L"Korean Wansung"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(21025, L"cp1025", L"IBM EBCDIC Cyrillic Serbian-Bulgarian"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(21866, L"koi8-u", L"Ukrainian (KOI8-U)); Cyrillic (KOI8-U)"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(28591, L"iso-8859-1", L"ISO 8859-1 Latin 1; Western European (ISO)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(28592, L"iso-8859-2", L"ISO 8859-2 Central European; Central European (ISO)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(28593, L"iso-8859-3", L"ISO 8859-3 Latin 3"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(28594, L"iso-8859-4", L"ISO 8859-4 Baltic"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(28595, L"iso-8859-5", L"ISO 8859-5 Cyrillic"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(28596, L"iso-8859-6", L"ISO 8859-6 Arabic"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(28597, L"iso-8859-7", L"ISO 8859-7 Greek"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(28598, L"iso-8859-8", L"ISO 8859-8 Hebrew; Hebrew (ISO-Visual)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(28599, L"iso-8859-9", L"ISO 8859-9 Turkish"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(28603, L"iso-8859-13", L"ISO 8859-13 Estonian"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(28605, L"iso-8859-15", L"ISO 8859-15 Latin 9"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(29001, L"x-Europa", L"Europa 3"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(38598, L"iso-8859-8-i", L"ISO 8859-8 Hebrew; Hebrew (ISO-Logical)"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(50220, L"iso-2022-jp", L"ISO 2022 Japanese with no halfwidth Katakana; Japanese (JIS)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(50221, L"csISO2022JP", L"ISO 2022 Japanese with halfwidth Katakana; Japanese (JIS-Allow 1 byte Kana)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(50222, L"iso-2022-jp", L"ISO 2022 Japanese JIS X 0201-1989; Japanese (JIS-Allow 1 byte Kana - SO/SI)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(50225, L"iso-2022-kr", L"ISO 2022 Korean"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(50227, L"x-cp50227", L"ISO 2022 Simplified Chinese; Chinese Simplified (ISO 2022)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(51932, L"euc-jp", L"EUC Japanese"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(51936, L"EUC-CN", L"EUC Simplified Chinese; Chinese Simplified (EUC)"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(51949, L"euc-kr", L"EUC Korean"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(52936, L"hz-gb-2312", L"HZ-GB2312 Simplified Chinese; Chinese Simplified (HZ)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(54936, L"GB18030", L"Windows XP and later: GB18030 Simplified Chinese (4 byte)); Chinese Simplified (GB18030)"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(57002, L"x-iscii-de", L"ISCII Devanagari"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(57003, L"x-iscii-be", L"ISCII Bengali"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(57004, L"x-iscii-ta", L"ISCII Tamil"));

                    internalMap.Append(FFmpegInteropX::CharacterEncoding(57005, L"x-iscii-te", L"ISCII Telugu"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(57006, L"x-iscii-as", L"ISCII Assamese"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(57007, L"x-iscii-or", L"ISCII Oriya"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(57008, L"x-iscii-ka", L"ISCII Kannada"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(57009, L"x-iscii-ma", L"ISCII Malayalam"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(57010, L"x-iscii-gu", L"ISCII Gujarati"));
                    internalMap.Append(FFmpegInteropX::CharacterEncoding(57011, L"x-iscii-pa", L"ISCII Punjabi"));
                }
            }
            catch (...)
            {
            }
            internalView = internalMap.GetView();
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
