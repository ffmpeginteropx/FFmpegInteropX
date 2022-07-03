#pragma once
#include "pch.h"

namespace FFmpegInteropX
{
    using namespace Platform;
    using namespace Platform::Collections;
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Collections;
    using namespace Windows::Media::Core;

    [Windows::UI::Xaml::Data::Bindable]
    public ref class CharacterEncoding sealed
    {
        String^ name, ^ description;
        int codePage;
        static Platform::Collections::Vector<CharacterEncoding^>^ internalMap;
        static Windows::Foundation::Collections::IVectorView<CharacterEncoding^>^ internalView;
        static std::mutex mutex;


    public:
        property String^ Name
        {
            String^ get()
            {
                return name;
            }
        }

        property String^ Description
        {
            String^ get()
            {
                return description;
            }
        }

        property int WindowsCodePage
        {
            int get()
            {
                return codePage;
            }
        }
    internal:
        CharacterEncoding(int p_codePage, String^ p_name, String^ p_description)
        {
            codePage = p_codePage;
            name = p_name;
            description = p_description;
        }


    public:
        static IVectorView<CharacterEncoding^>^ GetCharacterEncodings()
        {
            if (internalView == nullptr)
            {
                mutex.lock();
                try
                {
                    if (internalView == nullptr)
                    {
                        //populate and create the vector.
                        internalMap = ref new Vector<CharacterEncoding^>();
                        internalMap->Append(ref new CharacterEncoding(0, "", "System locale"));
                        internalMap->Append(ref new CharacterEncoding(65001, "CP65001", "Unicode (UTF-8)"));
                        internalMap->Append(ref new CharacterEncoding(1200, "CP1200", "Unicode UTF-16, little endian byte order (BMP of ISO 10646)"));
                        internalMap->Append(ref new CharacterEncoding(1201, "CP1201", "Unicode UTF-16, big endian byte order"));
                        internalMap->Append(ref new CharacterEncoding(12000, "CP12000", "Unicode UTF-32, little endian byte order"));
                        internalMap->Append(ref new CharacterEncoding(12001, "CP12001", "Unicode UTF-32, big endian byte order"));

                        internalMap->Append(ref new CharacterEncoding(154, "CP154", "Cyrillic-asian"));
                        internalMap->Append(ref new CharacterEncoding(1133, "CP1133", "IBM Lao Script"));
                        internalMap->Append(ref new CharacterEncoding(853, "CP853", " DOS for Turkish, Maltese, and Esperanto."));
                        internalMap->Append(ref new CharacterEncoding(1125, "CP1125", "Legacy Latin"));
                        internalMap->Append(ref new CharacterEncoding(37, "IBM037", "IBM EBCDIC US-Canada"));
                        internalMap->Append(ref new CharacterEncoding(437, "IBM437", "OEM United States"));

                        internalMap->Append(ref new CharacterEncoding(500, "IBM500", "IBM EBCDIC International"));
                        internalMap->Append(ref new CharacterEncoding(708, "ASMO-708", "Arabic (ASMO 708)"));
                        internalMap->Append(ref new CharacterEncoding(720, "DOS-720", "Arabic (Transparent ASMO)); Arabic (DOS)"));
                        internalMap->Append(ref new CharacterEncoding(737, "ibm737", "OEM Greek (formerly 437G)); Greek (DOS)"));
                        internalMap->Append(ref new CharacterEncoding(775, "ibm775", "OEM Baltic; Baltic (DOS)"));

                        internalMap->Append(ref new CharacterEncoding(850, "ibm850", "OEM Multilingual Latin 1; Western European (DOS)"));
                        internalMap->Append(ref new CharacterEncoding(852, "ibm852", "OEM Latin 2; Central European (DOS)"));
                        internalMap->Append(ref new CharacterEncoding(855, "IBM855", "OEM Cyrillic (primarily Russian)"));
                        internalMap->Append(ref new CharacterEncoding(857, "ibm857", "OEM Turkish; Turkish (DOS)"));
                        internalMap->Append(ref new CharacterEncoding(858, "IBM00858", "OEM Multilingual Latin 1 + Euro symbol"));
                        internalMap->Append(ref new CharacterEncoding(860, "IBM860", "OEM Portuguese; Portuguese (DOS)"));

                        internalMap->Append(ref new CharacterEncoding(861, "ibm861", "OEM Icelandic; Icelandic (DOS)"));
                        internalMap->Append(ref new CharacterEncoding(862, "DOS-862", "OEM Hebrew; Hebrew (DOS)"));
                        internalMap->Append(ref new CharacterEncoding(863, "IBM863", "OEM French Canadian; French Canadian (DOS)"));
                        internalMap->Append(ref new CharacterEncoding(864, "IBM864", "OEM Arabic; Arabic (864)"));
                        internalMap->Append(ref new CharacterEncoding(865, "IBM865", "OEM Nordic; Nordic (DOS)"));

                        internalMap->Append(ref new CharacterEncoding(866, "cp866", "OEM Russian; Cyrillic (DOS)"));
                        internalMap->Append(ref new CharacterEncoding(869, "ibm869", "OEM Modern Greek; Greek, Modern (DOS)"));
                        internalMap->Append(ref new CharacterEncoding(870, "IBM870", "IBM EBCDIC Multilingual/ROECE (Latin 2)); IBM EBCDIC Multilingual Latin 2"));
                        internalMap->Append(ref new CharacterEncoding(874, "windows-874", "ANSI/OEM Thai (same as 28605, ISO 8859-15)); Thai (Windows)"));
                        internalMap->Append(ref new CharacterEncoding(875, "cp875", "IBM EBCDIC Greek Modern"));
                        internalMap->Append(ref new CharacterEncoding(932, "shift_jis", "ANSI/OEM Japanese; Japanese (Shift-JIS)"));

                        internalMap->Append(ref new CharacterEncoding(936, "gb2312", "ANSI/OEM Simplified Chinese (PRC, Singapore)); Chinese Simplified (GB2312)"));
                        internalMap->Append(ref new CharacterEncoding(949, "ks_c_5601-1987", "ANSI/OEM Korean (Unified Hangul Code)"));
                        internalMap->Append(ref new CharacterEncoding(950, "big5", "ANSI/OEM Traditional Chinese (Taiwan; Hong Kong SAR, PRC)); Chinese Traditional (Big5)"));
                        internalMap->Append(ref new CharacterEncoding(1026, "IBM1026", "IBM EBCDIC Turkish (Latin 5)"));
                        internalMap->Append(ref new CharacterEncoding(1047, "IBM01047", "IBM EBCDIC Latin 1/Open System"));
                        internalMap->Append(ref new CharacterEncoding(1140, "IBM01140", "IBM EBCDIC US-Canada (037 + Euro symbol)); IBM EBCDIC (US-Canada-Euro)"));

                        internalMap->Append(ref new CharacterEncoding(1141, "IBM01141", "IBM EBCDIC Germany (20273 + Euro symbol)); IBM EBCDIC (Germany-Euro)"));
                        internalMap->Append(ref new CharacterEncoding(1142, "IBM01142", "IBM EBCDIC Denmark-Norway (20277 + Euro symbol)); IBM EBCDIC (Denmark-Norway-Euro)"));
                        internalMap->Append(ref new CharacterEncoding(1143, "IBM01143", "IBM EBCDIC Finland-Sweden (20278 + Euro symbol)); IBM EBCDIC (Finland-Sweden-Euro)"));
                        internalMap->Append(ref new CharacterEncoding(1144, "IBM01144", "IBM EBCDIC Italy (20280 + Euro symbol)); IBM EBCDIC (Italy-Euro)"));
                        internalMap->Append(ref new CharacterEncoding(1145, "IBM01145", "IBM EBCDIC Latin America-Spain (20284 + Euro symbol)); IBM EBCDIC (Spain-Euro)"));

                        internalMap->Append(ref new CharacterEncoding(1146, "IBM01146", "IBM EBCDIC United Kingdom (20285 + Euro symbol)); IBM EBCDIC (UK-Euro)"));
                        internalMap->Append(ref new CharacterEncoding(1147, "IBM01147", "IBM EBCDIC France (20297 + Euro symbol)); IBM EBCDIC (France-Euro)"));
                        internalMap->Append(ref new CharacterEncoding(1148, "IBM01148", "IBM EBCDIC International (500 + Euro symbol)); IBM EBCDIC (International-Euro)"));
                        internalMap->Append(ref new CharacterEncoding(1149, "IBM01149", "IBM EBCDIC Icelandic (20871 + Euro symbol)); IBM EBCDIC (Icelandic-Euro)"));
                        internalMap->Append(ref new CharacterEncoding(1250, "windows-1250", "ANSI Central European; Central European (Windows)"));

                        internalMap->Append(ref new CharacterEncoding(1251, "windows-1251", "ANSI Cyrillic; Cyrillic (Windows)"));
                        internalMap->Append(ref new CharacterEncoding(1252, "windows-1252", "ANSI Latin 1; Western European (Windows)"));
                        internalMap->Append(ref new CharacterEncoding(1253, "windows-1253", "ANSI Greek; Greek (Windows)"));
                        internalMap->Append(ref new CharacterEncoding(1254, "windows-1254", "ANSI Turkish; Turkish (Windows)"));
                        internalMap->Append(ref new CharacterEncoding(1255, "windows-1255", "ANSI Hebrew; Hebrew (Windows)"));

                        internalMap->Append(ref new CharacterEncoding(1256, "windows-1256", "ANSI Arabic; Arabic (Windows)"));
                        internalMap->Append(ref new CharacterEncoding(1257, "windows-1257", "ANSI Baltic; Baltic (Windows)"));
                        internalMap->Append(ref new CharacterEncoding(1258, "windows-1258", "ANSI/OEM Vietnamese; Vietnamese (Windows)"));
                        internalMap->Append(ref new CharacterEncoding(1361, "Johab", "Korean (Johab)"));
                        internalMap->Append(ref new CharacterEncoding(10000, "macintosh", "MAC Roman; Western European (Mac)"));

                        internalMap->Append(ref new CharacterEncoding(10001, "x-mac-japanese", "Japanese (Mac)"));
                        internalMap->Append(ref new CharacterEncoding(10002, "x-mac-chinesetrad", "MAC Traditional Chinese (Big5)); Chinese Traditional (Mac)"));
                        internalMap->Append(ref new CharacterEncoding(10003, "x-mac-korean", "Korean (Mac)"));
                        internalMap->Append(ref new CharacterEncoding(10004, "x-mac-arabic", "Arabic (Mac)"));
                        internalMap->Append(ref new CharacterEncoding(10005, "x-mac-hebrew", "Hebrew (Mac)"));
                        internalMap->Append(ref new CharacterEncoding(10006, "x-mac-greek", "Greek (Mac)"));
                        internalMap->Append(ref new CharacterEncoding(10007, "x-mac-cyrillic", "Cyrillic (Mac)"));
                        internalMap->Append(ref new CharacterEncoding(10008, "x-mac-chinesesimp", "MAC Simplified Chinese (GB 2312)); Chinese Simplified (Mac)"));
                        internalMap->Append(ref new CharacterEncoding(10010, "x-mac-romanian", "Romanian (Mac)"));

                        internalMap->Append(ref new CharacterEncoding(10017, "x-mac-ukrainian", "Ukrainian (Mac)"));
                        internalMap->Append(ref new CharacterEncoding(10021, "x-mac-thai", "Thai (Mac)"));
                        internalMap->Append(ref new CharacterEncoding(10029, "x-mac-ce", "MAC Latin 2; Central European (Mac)"));
                        internalMap->Append(ref new CharacterEncoding(10079, "x-mac-icelandic", "Icelandic (Mac)"));
                        internalMap->Append(ref new CharacterEncoding(10081, "x-mac-turkish", "Turkish (Mac)"));
                        internalMap->Append(ref new CharacterEncoding(10082, "x-mac-croatian", "Croatian (Mac)"));
                        internalMap->Append(ref new CharacterEncoding(20000, "x-Chinese_CNS", "CNS Taiwan; Chinese Traditional (CNS)"));

                        internalMap->Append(ref new CharacterEncoding(20001, "x-cp20001", "TCA Taiwan"));
                        internalMap->Append(ref new CharacterEncoding(20002, "x_Chinese-Eten", "Eten Taiwan; Chinese Traditional (Eten)"));
                        internalMap->Append(ref new CharacterEncoding(20003, "x-cp20003", "IBM5550 Taiwan"));
                        internalMap->Append(ref new CharacterEncoding(20004, "x-cp20004", "TeleText Taiwan"));
                        internalMap->Append(ref new CharacterEncoding(20005, "x-cp20005", "Wang Taiwan"));
                        internalMap->Append(ref new CharacterEncoding(20105, "x-IA5", "IA5 (IRV International Alphabet No. 5, 7-bit)); Western European (IA5)"));
                        internalMap->Append(ref new CharacterEncoding(20106, "x-IA5-German", "IA5 German (7-bit)"));
                        internalMap->Append(ref new CharacterEncoding(20107, "x-IA5-Swedish", "IA5 Swedish (7-bit)"));

                        internalMap->Append(ref new CharacterEncoding(20108, "x-IA5-Norwegian", "IA5 Norwegian (7-bit)"));
                        internalMap->Append(ref new CharacterEncoding(20127, "us-ascii", "US-ASCII (7-bit)"));
                        internalMap->Append(ref new CharacterEncoding(20261, "x-cp20261", "T.61"));
                        internalMap->Append(ref new CharacterEncoding(20269, "x-cp20269", "ISO 6937 Non-Spacing Accent"));
                        internalMap->Append(ref new CharacterEncoding(20273, "IBM273", "IBM EBCDIC Germany"));

                        internalMap->Append(ref new CharacterEncoding(20277, "IBM277", "IBM EBCDIC Denmark-Norway"));
                        internalMap->Append(ref new CharacterEncoding(20278, "IBM278", "IBM EBCDIC Finland-Sweden"));
                        internalMap->Append(ref new CharacterEncoding(20280, "IBM280", "IBM EBCDIC Italy"));
                        internalMap->Append(ref new CharacterEncoding(20284, "IBM284", "IBM EBCDIC Latin America-Spain"));
                        internalMap->Append(ref new CharacterEncoding(20285, "IBM285", "IBM EBCDIC United Kingdom"));
                        internalMap->Append(ref new CharacterEncoding(20290, "IBM290", "IBM EBCDIC Japanese Katakana Extended"));
                        internalMap->Append(ref new CharacterEncoding(20297, "IBM297", "IBM EBCDIC France"));
                        internalMap->Append(ref new CharacterEncoding(20420, "IBM420", "IBM EBCDIC Arabic"));

                        internalMap->Append(ref new CharacterEncoding(20423, "IBM423", "IBM EBCDIC Greek"));
                        internalMap->Append(ref new CharacterEncoding(20424, "IBM424", "IBM EBCDIC Hebrew"));
                        internalMap->Append(ref new CharacterEncoding(20833, "x-EBCDIC-KoreanExtended", "IBM EBCDIC Korean Extended"));
                        internalMap->Append(ref new CharacterEncoding(20838, "IBM-Thai", "IBM EBCDIC Thai"));
                        internalMap->Append(ref new CharacterEncoding(20866, "koi8-r", "Russian (KOI8-R)); Cyrillic (KOI8-R)"));
                        internalMap->Append(ref new CharacterEncoding(20871, "IBM871", "IBM EBCDIC Icelandic"));
                        internalMap->Append(ref new CharacterEncoding(20880, "IBM880", "IBM EBCDIC Cyrillic Russian"));

                        internalMap->Append(ref new CharacterEncoding(20905, "IBM905", "IBM EBCDIC Turkish"));
                        internalMap->Append(ref new CharacterEncoding(20924, "IBM00924", "IBM EBCDIC Latin 1/Open System (1047 + Euro symbol)"));
                        internalMap->Append(ref new CharacterEncoding(20932, "EUC-JP", "Japanese (JIS 0208-1990 and 0121-1990)"));
                        internalMap->Append(ref new CharacterEncoding(20936, "x-cp20936", "Simplified Chinese (GB2312)); Chinese Simplified (GB2312-80)"));
                        internalMap->Append(ref new CharacterEncoding(20949, "x-cp20949", "Korean Wansung"));
                        internalMap->Append(ref new CharacterEncoding(21025, "cp1025", "IBM EBCDIC Cyrillic Serbian-Bulgarian"));
                        internalMap->Append(ref new CharacterEncoding(21866, "koi8-u", "Ukrainian (KOI8-U)); Cyrillic (KOI8-U)"));

                        internalMap->Append(ref new CharacterEncoding(28591, "iso-8859-1", "ISO 8859-1 Latin 1; Western European (ISO)"));
                        internalMap->Append(ref new CharacterEncoding(28592, "iso-8859-2", "ISO 8859-2 Central European; Central European (ISO)"));
                        internalMap->Append(ref new CharacterEncoding(28593, "iso-8859-3", "ISO 8859-3 Latin 3"));
                        internalMap->Append(ref new CharacterEncoding(28594, "iso-8859-4", "ISO 8859-4 Baltic"));
                        internalMap->Append(ref new CharacterEncoding(28595, "iso-8859-5", "ISO 8859-5 Cyrillic"));
                        internalMap->Append(ref new CharacterEncoding(28596, "iso-8859-6", "ISO 8859-6 Arabic"));
                        internalMap->Append(ref new CharacterEncoding(28597, "iso-8859-7", "ISO 8859-7 Greek"));

                        internalMap->Append(ref new CharacterEncoding(28598, "iso-8859-8", "ISO 8859-8 Hebrew; Hebrew (ISO-Visual)"));
                        internalMap->Append(ref new CharacterEncoding(28599, "iso-8859-9", "ISO 8859-9 Turkish"));
                        internalMap->Append(ref new CharacterEncoding(28603, "iso-8859-13", "ISO 8859-13 Estonian"));
                        internalMap->Append(ref new CharacterEncoding(28605, "iso-8859-15", "ISO 8859-15 Latin 9"));
                        internalMap->Append(ref new CharacterEncoding(29001, "x-Europa", "Europa 3"));
                        internalMap->Append(ref new CharacterEncoding(38598, "iso-8859-8-i", "ISO 8859-8 Hebrew; Hebrew (ISO-Logical)"));

                        internalMap->Append(ref new CharacterEncoding(50220, "iso-2022-jp", "ISO 2022 Japanese with no halfwidth Katakana; Japanese (JIS)"));
                        internalMap->Append(ref new CharacterEncoding(50221, "csISO2022JP", "ISO 2022 Japanese with halfwidth Katakana; Japanese (JIS-Allow 1 byte Kana)"));
                        internalMap->Append(ref new CharacterEncoding(50222, "iso-2022-jp", "ISO 2022 Japanese JIS X 0201-1989; Japanese (JIS-Allow 1 byte Kana - SO/SI)"));
                        internalMap->Append(ref new CharacterEncoding(50225, "iso-2022-kr", "ISO 2022 Korean"));
                        internalMap->Append(ref new CharacterEncoding(50227, "x-cp50227", "ISO 2022 Simplified Chinese; Chinese Simplified (ISO 2022)"));
                        internalMap->Append(ref new CharacterEncoding(51932, "euc-jp", "EUC Japanese"));
                        internalMap->Append(ref new CharacterEncoding(51936, "EUC-CN", "EUC Simplified Chinese; Chinese Simplified (EUC)"));

                        internalMap->Append(ref new CharacterEncoding(51949, "euc-kr", "EUC Korean"));
                        internalMap->Append(ref new CharacterEncoding(52936, "hz-gb-2312", "HZ-GB2312 Simplified Chinese; Chinese Simplified (HZ)"));
                        internalMap->Append(ref new CharacterEncoding(54936, "GB18030", "Windows XP and later: GB18030 Simplified Chinese (4 byte)); Chinese Simplified (GB18030)"));
                        internalMap->Append(ref new CharacterEncoding(57002, "x-iscii-de", "ISCII Devanagari"));
                        internalMap->Append(ref new CharacterEncoding(57003, "x-iscii-be", "ISCII Bengali"));
                        internalMap->Append(ref new CharacterEncoding(57004, "x-iscii-ta", "ISCII Tamil"));

                        internalMap->Append(ref new CharacterEncoding(57005, "x-iscii-te", "ISCII Telugu"));
                        internalMap->Append(ref new CharacterEncoding(57006, "x-iscii-as", "ISCII Assamese"));
                        internalMap->Append(ref new CharacterEncoding(57007, "x-iscii-or", "ISCII Oriya"));
                        internalMap->Append(ref new CharacterEncoding(57008, "x-iscii-ka", "ISCII Kannada"));
                        internalMap->Append(ref new CharacterEncoding(57009, "x-iscii-ma", "ISCII Malayalam"));
                        internalMap->Append(ref new CharacterEncoding(57010, "x-iscii-gu", "ISCII Gujarati"));
                        internalMap->Append(ref new CharacterEncoding(57011, "x-iscii-pa", "ISCII Punjabi"));
                    }
                }
                catch (...)
                {
                }
                internalView = internalMap->GetView();
                mutex.unlock();
            }
            return internalView;

        }

        static CharacterEncoding^ GetSystemDefault()
        {
            return GetCharacterEncodings()->GetAt(0);
        }
    };
}

