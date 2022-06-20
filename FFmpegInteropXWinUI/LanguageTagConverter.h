#pragma once

#include "pch.h"

namespace FFmpegInteropX
{
	class LanguageEntry
	{
	public:
		winrt::hstring TwoLetterCode() { return twoLetterCode; }
		winrt::hstring EnglishName() { return englishName; }

		LanguageEntry(winrt::hstring const& twoLetterCode, winrt::hstring const& englishName)
		{
			this->twoLetterCode = twoLetterCode;
			this->englishName = englishName;
		}

	private:
		winrt::hstring twoLetterCode{};
		winrt::hstring englishName{};
	};

	class LanguageTagConverter
	{
	private:
		static std::map<winrt::hstring, std::shared_ptr<LanguageEntry>> map;

	public:
		static std::shared_ptr<LanguageEntry> TryGetLanguage(winrt::hstring const& languageTag)
		{
			auto result = map.find(languageTag);
			if (result != map.end())
			{
				return result->second;
			}

			return nullptr;
		}

		static void Initialize()
		{
			map[L"aar"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"aa", L"Afar"));
			map[L"abk"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ab", L"Abkhaz"));
			map[L"afr"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"af", L"Afrikaans"));
			map[L"aka"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ak", L"Akan"));
			map[L"alb"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"sq", L"Albanian"));
			map[L"amh"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"am", L"Amharic"));
			map[L"ara"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ar", L"Arabic"));
			map[L"arg"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"an", L"Aragonese"));
			map[L"arm"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"hy", L"Armenian"));
			map[L"asm"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"as", L"Assamese"));
			map[L"ava"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"av", L"Avaric"));
			map[L"ave"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ae", L"Avestan"));
			map[L"aym"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ay", L"Aymara"));
			map[L"aze"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"az", L"Azerbaijani"));
			map[L"bak"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ba", L"Bashkir"));
			map[L"bam"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"bm", L"Bambara"));
			map[L"baq"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"eu", L"Basque"));
			map[L"bel"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"be", L"Belarusian"));
			map[L"ben"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"bn", L"Bengali"));
			map[L"bih"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"bh", L"Bihari"));
			map[L"bis"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"bi", L"Bislama"));
			map[L"bod"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"bo", L"Tibetan Standard"));
			map[L"bos"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"bs", L"Bosnian"));
			map[L"bre"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"br", L"Breton"));
			map[L"bul"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"bg", L"Bulgarian"));
			map[L"bur"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"my", L"Burmese"));
			map[L"cat"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ca", L"Catalan"));
			map[L"ces"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"cs", L"Czech"));
			map[L"cha"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ch", L"Chamorro"));
			map[L"che"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ce", L"Chechen"));
			map[L"chi"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"zh", L"Chinese"));
			map[L"chu"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"cu", L"Old Church Slavonic"));
			map[L"chv"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"cv", L"Chuvash"));
			map[L"cor"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"kw", L"Cornish"));
			map[L"cos"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"co", L"Corsican"));
			map[L"cre"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"cr", L"Cree"));
			map[L"cym"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"cy", L"Welsh"));
			map[L"cze"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"cs", L"Czech"));
			map[L"dan"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"da", L"Danish"));
			map[L"deu"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"de", L"German"));
			map[L"div"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"dv", L"Divehi"));
			map[L"dut"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"nl", L"Dutch"));
			map[L"dzo"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"dz", L"Dzongkha"));
			map[L"ell"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"el", L"Greek"));
			map[L"eng"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"en", L"English"));
			map[L"epo"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"eo", L"Esperanto"));
			map[L"est"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"et", L"Estonian"));
			map[L"eus"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"eu", L"Basque"));
			map[L"ewe"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ee", L"Ewe"));
			map[L"fao"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"fo", L"Faroese"));
			map[L"fas"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"fa", L"Persian"));
			map[L"fij"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"fj", L"Fijian"));
			map[L"fin"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"fi", L"Finnish"));
			map[L"fra"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"fr", L"French"));
			map[L"fre"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"fr", L"French"));
			map[L"fry"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"fy", L"Western Frisian"));
			map[L"ful"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ff", L"Fula"));
			map[L"geo"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ka", L"Georgian"));
			map[L"ger"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"de", L"German"));
			map[L"gla"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"gd", L"Gaelic"));
			map[L"gle"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ga", L"Irish"));
			map[L"glg"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"gl", L"Galician"));
			map[L"glv"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"gv", L"Manx"));
			map[L"gre"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"el", L"Greek"));
			map[L"grn"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"gn", L"Guaraní"));
			map[L"guj"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"gu", L"Gujarati"));
			map[L"hat"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ht", L"Haitian"));
			map[L"hau"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ha", L"Hausa"));
			map[L"heb"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"he", L"Hebrew"));
			map[L"her"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"hz", L"Herero"));
			map[L"hin"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"hi", L"Hindi"));
			map[L"hmo"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ho", L"Hiri Motu"));
			map[L"hrv"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"hr", L"Croatian"));
			map[L"hun"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"hu", L"Hungarian"));
			map[L"hye"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"hy", L"Armenian"));
			map[L"ibo"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ig", L"Igbo"));
			map[L"ice"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"is", L"Icelandic"));
			map[L"ido"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"io", L"Ido"));
			map[L"iii"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ii", L"Nuosu"));
			map[L"iku"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"iu", L"Inuktitut"));
			map[L"ile"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ie", L"Interlingue"));
			map[L"ina"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ia", L"Interlingua"));
			map[L"ind"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"id", L"Indonesian"));
			map[L"ipk"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ik", L"Inupiaq"));
			map[L"isl"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"is", L"Icelandic"));
			map[L"ita"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"it", L"Italian"));
			map[L"jav"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"jv", L"Javanese"));
			map[L"jpn"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ja", L"Japanese"));
			map[L"kal"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"kl", L"Kalaallisut"));
			map[L"kan"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"kn", L"Kannada"));
			map[L"kas"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ks", L"Kashmiri"));
			map[L"kat"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ka", L"Georgian"));
			map[L"kau"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"kr", L"Kanuri"));
			map[L"kaz"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"kk", L"Kazakh"));
			map[L"khm"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"km", L"Khmer"));
			map[L"kik"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ki", L"Kikuyu"));
			map[L"kin"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"rw", L"Kinyarwanda"));
			map[L"kir"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ky", L"Kyrgyz"));
			map[L"kom"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"kv", L"Komi"));
			map[L"kon"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"kg", L"Kongo"));
			map[L"kor"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ko", L"Korean"));
			map[L"kua"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"kj", L"Kwanyama"));
			map[L"kur"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ku", L"Kurdish"));
			map[L"lao"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"lo", L"Lao"));
			map[L"lat"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"la", L"Latin"));
			map[L"lav"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"lv", L"Latvian"));
			map[L"lim"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"li", L"Limburgish"));
			map[L"lin"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ln", L"Lingala"));
			map[L"lit"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"lt", L"Lithuanian"));
			map[L"ltz"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"lb", L"Luxembourgish"));
			map[L"lub"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"lu", L"Luba-Katanga"));
			map[L"lug"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"lg", L"Ganda"));
			map[L"mac"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"mk", L"Macedonian"));
			map[L"mah"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"mh", L"Marshallese"));
			map[L"mal"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ml", L"Malayalam"));
			map[L"mao"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"mi", L"Maori"));
			map[L"mar"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"mr", L"Marathi"));
			map[L"may"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ms", L"Malay"));
			map[L"mkd"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"mk", L"Macedonian"));
			map[L"mlg"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"mg", L"Malagasy"));
			map[L"mlt"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"mt", L"Maltese"));
			map[L"mon"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"mn", L"Mongolian"));
			map[L"mri"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"mi", L"Maori"));
			map[L"msa"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ms", L"Malay"));
			map[L"mya"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"my", L"Burmese"));
			map[L"nau"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"na", L"Nauru"));
			map[L"nav"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"nv", L"Navajo"));
			map[L"nbl"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"nr", L"Southern Ndebele"));
			map[L"nde"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"nd", L"Northern Ndebele"));
			map[L"ndo"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ng", L"Ndonga"));
			map[L"nep"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ne", L"Nepali"));
			map[L"nld"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"nl", L"Dutch"));
			map[L"nno"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"nn", L"Norwegian Nynorsk"));
			map[L"nob"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"nb", L"Norwegian Bokmål"));
			map[L"nor"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"no", L"Norwegian"));
			map[L"nya"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ny", L"Chichewa"));
			map[L"oci"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"oc", L"Occitan"));
			map[L"oji"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"oj", L"Ojibwe"));
			map[L"ori"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"or", L"Oriya"));
			map[L"orm"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"om", L"Oromo"));
			map[L"oss"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"os", L"Ossetian"));
			map[L"pan"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"pa", L"Panjabi"));
			map[L"per"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"fa", L"Persian"));
			map[L"pli"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"pi", L"Pali"));
			map[L"pol"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"pl", L"Polish"));
			map[L"por"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"pt", L"Portuguese"));
			map[L"pus"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ps", L"Pashto"));
			map[L"que"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"qu", L"Quechua"));
			map[L"roh"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"rm", L"Romansh"));
			map[L"ron"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ro", L"Romanian"));
			map[L"rum"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ro", L"Romanian"));
			map[L"run"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"rn", L"Kirundi"));
			map[L"rus"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ru", L"Russian"));
			map[L"sag"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"sg", L"Sango"));
			map[L"san"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"sa", L"Sanskrit"));
			map[L"sin"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"si", L"Sinhala"));
			map[L"slk"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"sk", L"Slovak"));
			map[L"slo"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"sk", L"Slovak"));
			map[L"slv"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"sl", L"Slovene"));
			map[L"sme"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"se", L"Northern Sami"));
			map[L"smo"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"sm", L"Samoan"));
			map[L"sna"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"sn", L"Shona"));
			map[L"snd"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"sd", L"Sindhi"));
			map[L"som"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"so", L"Somali"));
			map[L"sot"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"st", L"Southern Sotho"));
			map[L"spa"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"es", L"Spanish"));
			map[L"sqi"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"sq", L"Albanian"));
			map[L"srd"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"sc", L"Sardinian"));
			map[L"srp"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"sr", L"Serbian"));
			map[L"ssw"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ss", L"Swati"));
			map[L"sun"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"su", L"Sundanese"));
			map[L"swa"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"sw", L"Swahili"));
			map[L"swe"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"sv", L"Swedish"));
			map[L"tah"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ty", L"Tahitian"));
			map[L"tam"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ta", L"Tamil"));
			map[L"tat"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"tt", L"Tatar"));
			map[L"tel"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"te", L"Telugu"));
			map[L"tgk"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"tg", L"Tajik"));
			map[L"tgl"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"tl", L"Tagalog"));
			map[L"tha"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"th", L"Thai"));
			map[L"tib"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"bo", L"Tibetan Standard"));
			map[L"tir"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ti", L"Tigrinya"));
			map[L"ton"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"to", L"Tonga"));
			map[L"tsn"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"tn", L"Tswana"));
			map[L"tso"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ts", L"Tsonga"));
			map[L"tuk"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"tk", L"Turkmen"));
			map[L"tur"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"tr", L"Turkish"));
			map[L"twi"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"tw", L"Twi"));
			map[L"uig"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ug", L"Uyghur"));
			map[L"ukr"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"uk", L"Ukrainian"));
			map[L"urd"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ur", L"Urdu"));
			map[L"uzb"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"uz", L"Uzbek"));
			map[L"ven"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"ve", L"Venda"));
			map[L"vie"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"vi", L"Vietnamese"));
			map[L"vol"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"vo", L"Volapük"));
			map[L"wel"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"cy", L"Welsh"));
			map[L"wln"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"wa", L"Walloon"));
			map[L"wol"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"wo", L"Wolof"));
			map[L"xho"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"xh", L"Xhosa"));
			map[L"yid"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"yi", L"Yiddish"));
			map[L"yor"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"yo", L"Yoruba"));
			map[L"zha"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"za", L"Zhuang"));
			map[L"zho"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"zh", L"Chinese"));
			map[L"zul"] = std::shared_ptr<LanguageEntry>(new LanguageEntry(L"zu", L"Zulu"));
		}
	};
}
