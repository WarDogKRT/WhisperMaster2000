#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "teamspeak/public_definitions.h"
#include "language_pkg.h"

//(de-)activate printf's
#define DEBUG_LOG true
#define USE_CALL_STACK true
#define USE_EARLY_CALL_STACK false

class error_handler
{
public:
    error_handler();
    ~error_handler() { };

    void init(std::string  sLogPath, unsigned int(*funcLogMessage)(const char*, LogLevel, const char*, uint64));
    void debug_log(const char * pString);
    void error_log(const char * pString);
    void error_log(const char * pString, std::exception &e);
    void error_log(const char * pString, boost::exception &e);
    void message_log(char * pString, LogLevel nMode);
    void message_callstack(char * pString, void* pClassPointer = nullptr);

    void remove_log_file();

    bool file_exists(const std::string& sFilename);
    bool folderExists(const std::string& sFoldername);

protected:
    language_pkg         m_cTranslate;       // language converter

    static unsigned int(*m_pFuncLogMessage)(const char* logMessage, LogLevel severity, const char* channel, uint64 logID);
    static std::string   m_sLogPath;
    static bool          m_bInitDone;
    const  std::string   c_sLogfileName = "/WhisperMaster2000/wm2000_error_log.txt";
    const  std::string   c_sStackfileName = "/WhisperMaster2000/wm2000_callstack_log.txt";

};

