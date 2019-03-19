#include "speech_engine.h"

#ifdef _WIN32
#pragma warning (disable : 4100)  /* Disable Unreferenced parameter warning */
#include <Windows.h>
#endif

#include <stringapiset.h>

#include <qvector.h>
#include <qlocale.h>


// static member initialisation
QTextToSpeech *speech_engine::m_pcSpeechEngine = nullptr;
std::wstring   speech_engine::m_wLanguage = L"";

/* ----------------------------------------------------------------------------
* constructor
*/
speech_engine::speech_engine()
{
    //*************************************************************************************
    // Init text snippets
    //*************************************************************************************

    // template
    //-------------------------------------------------------------------------------------
    this->m_cLangPkg.add(L"en.temp", std::wstring(L""));
    this->m_cLangPkg.add(L"de.temp", std::wstring(L""));

    // NoClientPofile
    //-------------------------------------------------------------------------------------
    this->m_cLangPkg.add(L"en.NoClientPofile", std::wstring(L"No clients in profile"));
    this->m_cLangPkg.add(L"de.NoClientPofile", std::wstring(L"Keine Nutzer im Profil"));

    // NoClientFreq
    //-------------------------------------------------------------------------------------
    this->m_cLangPkg.add(L"en.NoClientFreq", std::wstring(L"No clients on frequency"));
    this->m_cLangPkg.add(L"de.NoClientFreq", std::wstring(L"Kein Nutzer auf Frequenz"));



    // test
    //-------------------------------------------------------------------------------------
    this->m_cLangPkg.add(L"en.test", std::wstring(L"This is a test"));
    this->m_cLangPkg.add(L"de.test", std::wstring(L"Das ist ein test"));




    //*************************************************************************************
    // Init SpeechEngine, if not already done
    //*************************************************************************************
    if(m_wLanguage.length() == 0)
        set_language("en");
}

/* ----------------------------------------------------------------------------
* check if engine is available or not
*/
bool speech_engine::check_engine()
{
    bool bResult = true;

    std::string sTestText = "";
    auto vcEngines = m_pcSpeechEngine->availableEngines();
    sTestText += "Engines:\n";
    for (int ii = 0; ii < vcEngines.length(); ii++)
        sTestText += "   " + vcEngines[ii].toStdString() + "\n";

    auto vcLocales = m_pcSpeechEngine->availableLocales();
    sTestText += "Locales:\n";
    for (int ii = 0; ii < vcLocales.length(); ii++)
        sTestText += "   " + vcLocales[ii].name().toStdString() + "\n";

    auto vcVoices = m_pcSpeechEngine->availableVoices();
    sTestText += "Voices:\n";
    for (int ii = 0; ii < vcVoices.length(); ii++)
        sTestText += "   " + vcVoices[ii].name().toStdString() + "\n";

    //check if current setting is valid
    if ((vcEngines.length() == 0) || (vcLocales.length() == 0) || (vcVoices.length() == 0))
        bResult = false;

    return bResult;
}

/* ----------------------------------------------------------------------------
* destructor
*/
speech_engine::~speech_engine()
{
    if (m_pcSpeechEngine != nullptr)
        delete m_pcSpeechEngine;
}

/* ----------------------------------------------------------------------------
* set language
*/
void speech_engine::set_language(const char* sNewLanguage)
{
    //if no speech engine is active, create one
    if (m_pcSpeechEngine == nullptr)
        m_pcSpeechEngine = new QTextToSpeech();

    //get possible locals
    auto vcLocales = m_pcSpeechEngine->availableLocales();

    // find corresponding language
    int iLanguageIdx = get_possible_language(utf8_decode(std::string(sNewLanguage)));

    if (iLanguageIdx < 0)
    {
        // if no language was not found, set a default language
        m_wLanguage = L"en";
    }
    else
    {
        //save used language for search in m_cLangPkg and set speech engine
        m_wLanguage = vcLocales[iLanguageIdx].name().section('_', 0, 0).toStdWString();
        m_pcSpeechEngine->setLocale(vcLocales[iLanguageIdx]);
    }

    //check if Engine is available
    if (!check_engine())
    {
        m_cErrHandler.message_log("TTS is not available.", LogLevel_WARNING);
        delete m_pcSpeechEngine;
        m_pcSpeechEngine = nullptr;
    }
}

/* ----------------------------------------------------------------------------
* get possible language
*/
int speech_engine::get_possible_language(std::wstring sNewLanguage)
{
    //if no speech engine is active, create one
    if (m_pcSpeechEngine == nullptr)
        m_pcSpeechEngine = new QTextToSpeech();

    //get possible locals
    auto vcLocales = m_pcSpeechEngine->availableLocales();

    //if no language was found
    if ( vcLocales.length() == 0)
        return -1;

    // find corresponding language
    int iLanguageIdx = 0;
    for (; iLanguageIdx < vcLocales.length(); iLanguageIdx++)
        if (vcLocales[iLanguageIdx].name().indexOf(QString::fromStdWString(sNewLanguage), Qt::CaseInsensitive) >= 0)
            break;

    //if no matching language was found, use first one
    if (iLanguageIdx >= vcLocales.length())
        iLanguageIdx = 0;

    return iLanguageIdx;
}

/* ----------------------------------------------------------------------------
* set volume (0-100%) of voice
*/
void speech_engine::set_volume(double dVolume)
{
    //if no speech engine is active, bail out early
    if (m_pcSpeechEngine == nullptr)
        return;

    //saturate to limits
    if (dVolume < 0.0)
        dVolume = 0.0;
    else if(dVolume > 100.0)
        dVolume = 100.0;

    //set volume
    m_pcSpeechEngine->setVolume(dVolume/100);
}

/* ----------------------------------------------------------------------------
* say specific line of text
*/
void speech_engine::say(const char * sStringName)
{
    //if no speech engine is active, bail out early
    if (m_pcSpeechEngine == nullptr)
        return;

    //try to use user prefered language, but...
    std::wstring temp_lang = m_wLanguage;
    temp_lang.append(L".");
    temp_lang.append(utf8_decode(std::string(sStringName)));

    //...fall back language is always english
    std::wstring temp_default = L"en.";
    temp_default.append(utf8_decode(std::string(sStringName)));

    try
    {
        QString sSpeechText = QString::fromStdWString(this->m_cLangPkg.get(temp_lang, this->m_cLangPkg.get(temp_default, temp_lang)));
        this->m_pcSpeechEngine->say(sSpeechText);
    }
    catch (std::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (boost::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (...)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__);
    }

    return;
}

/* ----------------------------------------------------------------------------
* Convert an UTF8 string to a wide Unicode String
*/
std::wstring speech_engine::utf8_decode(const std::string &str)
{
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}
