// XWUI locale functions
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "../resources/xwui_resources.h"

#include "xwlangnames.h"
#include "xwlocale.h"

/////////////////////////////////////////////////////////////////////
// strings

// locale strings
struct XWUILocaleStrings
{
    const char*         locale;
    const char**        strings;
    const wchar_t**     values;
    size_t              count;
};

// supported locales
static XWUILocaleStrings g_XWUILocaleStrings[] =
{
    {
        "en",
        xwui_strings_en_us_strings,
        xwui_strings_en_us_values,
        xwui_strings_en_us_strings_count
    },
    {
        "en-us",
        xwui_strings_en_us_strings,
        xwui_strings_en_us_values,
        xwui_strings_en_us_strings_count
    },
    {
        "ru",
        xwui_strings_ru_ru_strings,
        xwui_strings_ru_ru_values,
        xwui_strings_ru_ru_strings_count
    },
    {
        "ru-ru",
        xwui_strings_ru_ru_strings,
        xwui_strings_ru_ru_values,
        xwui_strings_ru_ru_strings_count
    }
};

// locale count
#define XWUI_LOCALE_COUNT       sizeof(g_XWUILocaleStrings) / sizeof(g_XWUILocaleStrings[0])

/////////////////////////////////////////////////////////////////////
// locale data 

// string map
typedef std::map<std::string, std::wstring> XWUILocaleStringMapT;

// user string map
typedef std::map<std::string, std::vector<XWUILocaleStrings> > XWUIUserStringMapT;

// locale data
struct XWUILocaleData
{
    std::string             locale;
    std::wstring            localew;
    XWUILocaleStringMapT    strings;
    XWUIUserStringMapT      ustrings;
};

// data reference
static XWUILocaleData* g_pXWUILocaleData = 0;

// get locale instance
inline XWUILocaleData* _xwuiLocaleData(bool initDefault = false)
{
    if(g_pXWUILocaleData == 0)
    {
        // NOTE: we need to allocate static data on the heap for memory
        //       leak checking to work properly.
        g_pXWUILocaleData = new XWUILocaleData();
    }

    // init default if needed and not initialized yet
    if(initDefault && g_pXWUILocaleData->locale.length() == 0)
    {
        XWUILocale::setDefaultLocale();
    }

    return g_pXWUILocaleData;
}

/////////////////////////////////////////////////////////////////////
// helper functions
XWUILocaleStrings*  _findXWUILocaleStrings(const char* locale);
bool _normalizeLocaleName(const char* locale, std::string& nameOut);
void _addStrings(XWUILocaleData* xwuiLocale, const XWUILocaleStrings& localeStrings);

/////////////////////////////////////////////////////////////////////
// XWUILocale - locale functions

// get localized language name from locale
bool XWUILocale::getLangName(const char* locale, std::string& nameOut, std::wstring& localNameOut)
{
    // normalize name
    std::string localeName;
    if(!_normalizeLocaleName(locale, localeName)) return false;

    // get name
    return XWUILangNames::getLangName(localeName.c_str(), nameOut, localNameOut);
}

// locales
bool XWUILocale::setLocale(const char* locale)
{
    // ignore if not supported
    if(!isLocaleSupported(locale)) return false;

    // normalize name
    std::string localeName;
    if(!_normalizeLocaleName(locale, localeName)) return false;

    // active locale
    XWUILocaleData* xwuiLocale = _xwuiLocaleData();
    if(xwuiLocale == 0) return false;

    // reset current strings
    xwuiLocale->strings.clear();

    // name
    xwuiLocale->locale = localeName;

    // wide char version
    XWUtils::asciiToUnicode(localeName.c_str(), 0, xwuiLocale->localew);

    // NOTE: order is important here, we add strings in the order they were added

    // add XWUI strings first
    XWUILocaleStrings* localeStrings = _findXWUILocaleStrings(localeName.c_str());
    if(localeStrings)
    {
        // copy strings
        _addStrings(xwuiLocale, *localeStrings);
    }

    // check if there are user defined strings
    XWUIUserStringMapT::iterator it = xwuiLocale->ustrings.find(localeName.c_str());
    if(it != xwuiLocale->ustrings.end())
    {
        // loop over all strings
        for(std::vector<XWUILocaleStrings>::iterator sit = it->second.begin();
            sit != it->second.end(); ++sit)
        {
            // copy strings
            _addStrings(xwuiLocale, *sit);
        }
    }

    return true;
}

void XWUILocale::setDefaultLocale()
{
    // get default locale
    const wchar_t* localew = XWUtils::sGetDefaultLocaleName();
    if(localew)
    {
        std::string locale;

        // convert to ascii
        XWUtils::unicodeToAscii(localew, 0, locale);

        // set locale
        if(setLocale(locale.c_str())) return;
    }

    // locale not set, fallback to English
    setLocale("en");
}

bool XWUILocale::isLocaleSupported(const char* locale)
{
    // normalize name
    std::string localeName;
    if(!_normalizeLocaleName(locale, localeName)) return false;

    // find strings
    if(_findXWUILocaleStrings(localeName.c_str()) != 0) return true;

    // locale data
    XWUILocaleData* xwuiLocale = _xwuiLocaleData();
    if(xwuiLocale == 0) return false;

    // check user supported locales
    return (xwuiLocale->ustrings.count(localeName.c_str()) != 0);
}

// supported locales
void XWUILocale::getSupportedLocales(std::vector<std::string>& localesOut)
{
    // reset output
    localesOut.clear();

    // loop over locales
    for(size_t idx = 0; idx < XWUI_LOCALE_COUNT; ++idx)
    {
        // copy names
        localesOut.push_back(g_XWUILocaleStrings[idx].locale);
    }

    // locale data
    XWUILocaleData* xwuiLocale = _xwuiLocaleData();
    if(xwuiLocale == 0) return;

    // add user supported locales
    for(XWUIUserStringMapT::iterator it = xwuiLocale->ustrings.begin(); 
        it != xwuiLocale->ustrings.end(); ++it)
    {
        // check if already added
        if(std::find(localesOut.begin(), localesOut.end(), it->first) == localesOut.end())
        {
            localesOut.push_back(it->first);
        }
    }
}

void XWUILocale::getSupportedLocalesW(std::vector<std::wstring>& localesOut)
{
    // reset output
    localesOut.clear();

    // get ascii names
    std::vector<std::string> asciiLocales;
    getSupportedLocales(asciiLocales);

    // convert locale names
    std::wstring localew;
    for(size_t idx = 0; idx < asciiLocales.size(); ++idx)
    {
        // wide char version
        XWUtils::asciiToUnicode(asciiLocales[idx].c_str(), 0, localew);

        // copy names
        localesOut.push_back(localew);
    }
}

// add user localizations
void XWUILocale::setLocaleStrings(const char* locale, const char** strings, const wchar_t** translations, size_t count)
{
    // check input
    XWASSERT(locale);
    XWASSERT(strings);
    XWASSERT(translations);
    XWASSERT(count);
    if(locale == 0 || strings == 0 || translations == 0 || count == 0) return;

    // locale data
    XWUILocaleData* localeData = _xwuiLocaleData();

    // double check
    XWASSERT(localeData);
    if(localeData == 0) return;

    // normalize name
    std::string localeName;
    if(!_normalizeLocaleName(locale, localeName)) return;

    // locale strings
    XWUILocaleStrings localeStrings;
    localeStrings.locale = 0; // not needed
    localeStrings.strings = strings;
    localeStrings.values = translations;
    localeStrings.count = count;

    // check whether strings for this locale already exist
    XWUIUserStringMapT::iterator it = localeData->ustrings.find(localeName.c_str());
    if(it != localeData->ustrings.end())
    {
        // append
        it->second.push_back(localeStrings);

    } else
    {
        std::vector<XWUILocaleStrings> vect;
        vect.push_back(localeStrings);

        // insert new
        localeData->ustrings.insert(XWUIUserStringMapT::value_type(localeName.c_str(), vect));
    }
}

// selected locale
const WCHAR* XWUILocale::getSelectedLocaleW()
{
    // active locale
    XWUILocaleData* locale = _xwuiLocaleData(true);

    // double check
    XWASSERT(locale);
    if(locale == 0) return 0;

    // locale name
    return locale->localew.c_str();
}

// selected locale
const char* XWUILocale::getSelectedLocale()
{
    // active locale
    XWUILocaleData* locale = _xwuiLocaleData(true);

    // double check
    XWASSERT(locale);
    if(locale == 0) return 0;

    // locale name
    return locale->locale.c_str();
}

// translate text
const WCHAR* XWUILocale::translateText(const char* text, const char* descr)
{
    // check input
    XWASSERT(text);
    if(text == 0) return 0;

    // active locale
    XWUILocaleData* locale = _xwuiLocaleData(true);

    // double check
    XWASSERT(locale);
    if(locale == 0) return 0;

    // search string
    XWUILocaleStringMapT::iterator it = locale->strings.find(text);

    // check if found
    XWTRACE1("XWUILocale: unknown string requested: \"%s\"", text);
    if(it != locale->strings.end()) 
    {
        return it->second.c_str();
    }

    // convert text to WCHAR
    std::wstring wtext;
    XWUtils::asciiToUnicode(text, 0, wtext);

    // insert to map
    locale->strings.insert(XWUILocaleStringMapT::value_type(text, wtext));

    // return string
    return locale->strings.at(text).c_str();
}

// release locale data
void XWUILocale::releaseLocaleResources()
{
    if(g_pXWUILocaleData)
    {
        delete g_pXWUILocaleData;
        g_pXWUILocaleData = 0;
    }
}

/////////////////////////////////////////////////////////////////////
// helper function
XWUILocaleStrings* _findXWUILocaleStrings(const char* locale)
{
    XWASSERT(locale);
    if(locale == 0) return 0;

    // loop over locales
    for(size_t idx = 0; idx < XWUI_LOCALE_COUNT; ++idx)
    {
        // match locale
        if(::_stricmp(g_XWUILocaleStrings[idx].locale, locale) == 0)
            return g_XWUILocaleStrings + idx;
    }

    // not found
    return 0;
}

bool _normalizeLocaleName(const char* locale, std::string& nameOut)
{
    XWASSERT(locale);
    if(locale == 0) return false;

    // copy locale
    nameOut = locale;

    // remove case if any
    std::transform(nameOut.begin(), nameOut.end(), nameOut.begin(), ::tolower);

    // replace underscore with dash for consistency
    std::replace(nameOut.begin(), nameOut.end(), '_', '-');

    return (nameOut.length() > 0);
}

void _addStrings(XWUILocaleData* xwuiLocale, const XWUILocaleStrings& localeStrings)
{
    // copy strings
    for(size_t idx = 0; idx < localeStrings.count; ++idx)
    {
        // NOTE: we need to replace value if it already exists
        XWUILocaleStringMapT::iterator it = xwuiLocale->strings.find(localeStrings.strings[idx]);
        if(it != xwuiLocale->strings.end())
        {
            // update
            it->second = localeStrings.values[idx];

        } else
        {
            // insert translation string 
            xwuiLocale->strings.insert(XWUILocaleStringMapT::value_type(
                localeStrings.strings[idx], localeStrings.values[idx]));
        }
    }
}

// XWUILocale
/////////////////////////////////////////////////////////////////////
