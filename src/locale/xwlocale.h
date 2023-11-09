// XWUI locale functions
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWLOCALE_H_
#define _XWLOCALE_H_

/////////////////////////////////////////////////////////////////////
// XWUILocale - locale functions

namespace XWUILocale
{
    // get localized language name from locale
    bool    getLangName(const char* locale, std::string& nameOut, std::wstring& localNameOut);

    // locales
    bool    setLocale(const char* locale);
    void    setDefaultLocale();    
    bool    isLocaleSupported(const char* locale);    

    // supported locales
    void    getSupportedLocales(std::vector<std::string>& localesOut);    
    void    getSupportedLocalesW(std::vector<std::wstring>& localesOut);    

    // add user localizations
    void    setLocaleStrings(const char* locale, const char** strings, const wchar_t** translations, size_t count);

    // selected locale
    const WCHAR*    getSelectedLocaleW();
    const char*     getSelectedLocale();

    // translate text
    const WCHAR*    translateText(const char* text, const char* descr);

    // release locale data
    void    releaseLocaleResources();
};

// XWUILocale
/////////////////////////////////////////////////////////////////////

// localization macro
#define _LTEXT(text, descr)     XWUILocale::translateText((text), (descr))

/////////////////////////////////////////////////////////////////////

#endif // _XWLOCALE_H_

