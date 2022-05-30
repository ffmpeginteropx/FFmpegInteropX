#pragma once

#include "pch.h"
using namespace Platform;


namespace FFmpegInteropX
{
	class LanguageEntry
	{
	public:
		String^ TwoLetterCode() { return twoLetterCode; }
		String^ EnglishName() { return englishName; }

		LanguageEntry(String^ twoLetterCode, String^ englishName)
		{
			this->twoLetterCode = twoLetterCode;
			this->englishName = englishName;
		}

	private:
		String^ twoLetterCode;
		String^ englishName;
	};

	class LanguageTagConverter
	{
	private:
		static std::map<String^, std::shared_ptr<LanguageEntry>> map;

	public:
		static std::shared_ptr<LanguageEntry> TryGetLanguage(String^ languageTag)
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
			map["aar"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("aa", "Afar"));
			map["abk"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ab", "Abkhaz"));
			map["afr"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("af", "Afrikaans"));
			map["aka"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ak", "Akan"));
			map["alb"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("sq", "Albanian"));
			map["amh"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("am", "Amharic"));
			map["ara"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ar", "Arabic"));
			map["arg"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("an", "Aragonese"));
			map["arm"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("hy", "Armenian"));
			map["asm"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("as", "Assamese"));
			map["ava"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("av", "Avaric"));
			map["ave"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ae", "Avestan"));
			map["aym"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ay", "Aymara"));
			map["aze"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("az", "Azerbaijani"));
			map["bak"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ba", "Bashkir"));
			map["bam"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("bm", "Bambara"));
			map["baq"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("eu", "Basque"));
			map["bel"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("be", "Belarusian"));
			map["ben"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("bn", "Bengali"));
			map["bih"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("bh", "Bihari"));
			map["bis"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("bi", "Bislama"));
			map["bod"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("bo", "Tibetan Standard"));
			map["bos"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("bs", "Bosnian"));
			map["bre"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("br", "Breton"));
			map["bul"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("bg", "Bulgarian"));
			map["bur"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("my", "Burmese"));
			map["cat"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ca", "Catalan"));
			map["ces"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("cs", "Czech"));
			map["cha"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ch", "Chamorro"));
			map["che"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ce", "Chechen"));
			map["chi"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("zh", "Chinese"));
			map["chu"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("cu", "Old Church Slavonic"));
			map["chv"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("cv", "Chuvash"));
			map["cor"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("kw", "Cornish"));
			map["cos"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("co", "Corsican"));
			map["cre"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("cr", "Cree"));
			map["cym"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("cy", "Welsh"));
			map["cze"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("cs", "Czech"));
			map["dan"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("da", "Danish"));
			map["deu"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("de", "German"));
			map["div"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("dv", "Divehi"));
			map["dut"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("nl", "Dutch"));
			map["dzo"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("dz", "Dzongkha"));
			map["ell"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("el", "Greek"));
			map["eng"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("en", "English"));
			map["epo"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("eo", "Esperanto"));
			map["est"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("et", "Estonian"));
			map["eus"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("eu", "Basque"));
			map["ewe"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ee", "Ewe"));
			map["fao"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("fo", "Faroese"));
			map["fas"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("fa", "Persian"));
			map["fij"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("fj", "Fijian"));
			map["fin"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("fi", "Finnish"));
			map["fra"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("fr", "French"));
			map["fre"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("fr", "French"));
			map["fry"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("fy", "Western Frisian"));
			map["ful"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ff", "Fula"));
			map["geo"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ka", "Georgian"));
			map["ger"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("de", "German"));
			map["gla"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("gd", "Gaelic"));
			map["gle"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ga", "Irish"));
			map["glg"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("gl", "Galician"));
			map["glv"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("gv", "Manx"));
			map["gre"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("el", "Greek"));
			map["grn"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("gn", "Guaraní"));
			map["guj"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("gu", "Gujarati"));
			map["hat"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ht", "Haitian"));
			map["hau"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ha", "Hausa"));
			map["heb"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("he", "Hebrew"));
			map["her"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("hz", "Herero"));
			map["hin"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("hi", "Hindi"));
			map["hmo"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ho", "Hiri Motu"));
			map["hrv"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("hr", "Croatian"));
			map["hun"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("hu", "Hungarian"));
			map["hye"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("hy", "Armenian"));
			map["ibo"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ig", "Igbo"));
			map["ice"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("is", "Icelandic"));
			map["ido"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("io", "Ido"));
			map["iii"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ii", "Nuosu"));
			map["iku"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("iu", "Inuktitut"));
			map["ile"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ie", "Interlingue"));
			map["ina"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ia", "Interlingua"));
			map["ind"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("id", "Indonesian"));
			map["ipk"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ik", "Inupiaq"));
			map["isl"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("is", "Icelandic"));
			map["ita"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("it", "Italian"));
			map["jav"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("jv", "Javanese"));
			map["jpn"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ja", "Japanese"));
			map["kal"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("kl", "Kalaallisut"));
			map["kan"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("kn", "Kannada"));
			map["kas"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ks", "Kashmiri"));
			map["kat"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ka", "Georgian"));
			map["kau"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("kr", "Kanuri"));
			map["kaz"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("kk", "Kazakh"));
			map["khm"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("km", "Khmer"));
			map["kik"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ki", "Kikuyu"));
			map["kin"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("rw", "Kinyarwanda"));
			map["kir"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ky", "Kyrgyz"));
			map["kom"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("kv", "Komi"));
			map["kon"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("kg", "Kongo"));
			map["kor"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ko", "Korean"));
			map["kua"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("kj", "Kwanyama"));
			map["kur"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ku", "Kurdish"));
			map["lao"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("lo", "Lao"));
			map["lat"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("la", "Latin"));
			map["lav"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("lv", "Latvian"));
			map["lim"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("li", "Limburgish"));
			map["lin"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ln", "Lingala"));
			map["lit"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("lt", "Lithuanian"));
			map["ltz"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("lb", "Luxembourgish"));
			map["lub"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("lu", "Luba-Katanga"));
			map["lug"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("lg", "Ganda"));
			map["mac"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("mk", "Macedonian"));
			map["mah"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("mh", "Marshallese"));
			map["mal"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ml", "Malayalam"));
			map["mao"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("mi", "Maori"));
			map["mar"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("mr", "Marathi"));
			map["may"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ms", "Malay"));
			map["mkd"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("mk", "Macedonian"));
			map["mlg"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("mg", "Malagasy"));
			map["mlt"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("mt", "Maltese"));
			map["mon"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("mn", "Mongolian"));
			map["mri"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("mi", "Maori"));
			map["msa"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ms", "Malay"));
			map["mya"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("my", "Burmese"));
			map["nau"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("na", "Nauru"));
			map["nav"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("nv", "Navajo"));
			map["nbl"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("nr", "Southern Ndebele"));
			map["nde"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("nd", "Northern Ndebele"));
			map["ndo"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ng", "Ndonga"));
			map["nep"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ne", "Nepali"));
			map["nld"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("nl", "Dutch"));
			map["nno"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("nn", "Norwegian Nynorsk"));
			map["nob"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("nb", "Norwegian Bokmål"));
			map["nor"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("no", "Norwegian"));
			map["nya"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ny", "Chichewa"));
			map["oci"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("oc", "Occitan"));
			map["oji"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("oj", "Ojibwe"));
			map["ori"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("or", "Oriya"));
			map["orm"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("om", "Oromo"));
			map["oss"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("os", "Ossetian"));
			map["pan"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("pa", "Panjabi"));
			map["per"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("fa", "Persian"));
			map["pli"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("pi", "Pali"));
			map["pol"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("pl", "Polish"));
			map["por"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("pt", "Portuguese"));
			map["pus"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ps", "Pashto"));
			map["que"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("qu", "Quechua"));
			map["roh"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("rm", "Romansh"));
			map["ron"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ro", "Romanian"));
			map["rum"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ro", "Romanian"));
			map["run"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("rn", "Kirundi"));
			map["rus"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ru", "Russian"));
			map["sag"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("sg", "Sango"));
			map["san"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("sa", "Sanskrit"));
			map["sin"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("si", "Sinhala"));
			map["slk"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("sk", "Slovak"));
			map["slo"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("sk", "Slovak"));
			map["slv"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("sl", "Slovene"));
			map["sme"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("se", "Northern Sami"));
			map["smo"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("sm", "Samoan"));
			map["sna"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("sn", "Shona"));
			map["snd"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("sd", "Sindhi"));
			map["som"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("so", "Somali"));
			map["sot"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("st", "Southern Sotho"));
			map["spa"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("es", "Spanish"));
			map["sqi"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("sq", "Albanian"));
			map["srd"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("sc", "Sardinian"));
			map["srp"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("sr", "Serbian"));
			map["ssw"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ss", "Swati"));
			map["sun"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("su", "Sundanese"));
			map["swa"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("sw", "Swahili"));
			map["swe"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("sv", "Swedish"));
			map["tah"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ty", "Tahitian"));
			map["tam"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ta", "Tamil"));
			map["tat"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("tt", "Tatar"));
			map["tel"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("te", "Telugu"));
			map["tgk"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("tg", "Tajik"));
			map["tgl"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("tl", "Tagalog"));
			map["tha"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("th", "Thai"));
			map["tib"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("bo", "Tibetan Standard"));
			map["tir"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ti", "Tigrinya"));
			map["ton"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("to", "Tonga"));
			map["tsn"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("tn", "Tswana"));
			map["tso"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ts", "Tsonga"));
			map["tuk"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("tk", "Turkmen"));
			map["tur"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("tr", "Turkish"));
			map["twi"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("tw", "Twi"));
			map["uig"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ug", "Uyghur"));
			map["ukr"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("uk", "Ukrainian"));
			map["urd"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ur", "Urdu"));
			map["uzb"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("uz", "Uzbek"));
			map["ven"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("ve", "Venda"));
			map["vie"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("vi", "Vietnamese"));
			map["vol"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("vo", "Volapük"));
			map["wel"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("cy", "Welsh"));
			map["wln"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("wa", "Walloon"));
			map["wol"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("wo", "Wolof"));
			map["xho"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("xh", "Xhosa"));
			map["yid"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("yi", "Yiddish"));
			map["yor"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("yo", "Yoruba"));
			map["zha"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("za", "Zhuang"));
			map["zho"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("zh", "Chinese"));
			map["zul"] = std::shared_ptr<LanguageEntry>(new LanguageEntry("zu", "Zulu"));
		}
	};
}
