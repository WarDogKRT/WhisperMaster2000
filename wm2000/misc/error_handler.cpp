#include "error_handler.h"

#include <QtWidgets/QMessageBox>
#include <fstream>
#include <io.h>         // For access().
#include <sys/types.h>  // For stat().
#include <sys/stat.h>   // For stat().

#include "boost\exception\diagnostic_information.hpp"


//static variable
unsigned int(*error_handler::m_pFuncLogMessage)(const char* logMessage, LogLevel severity, const char* channel, uint64 logID) = nullptr;
bool error_handler::m_bInitDone = false;

#if USE_CALL_STACK
#define CALL_STACK if(USE_CALL_STACK) this->message_callstack(__FUNCSIG__, (void*)this);
std::string   error_handler::m_sLogPath = "c:/Users/micha/AppData/Roaming/TS3Client/plugins";
#else
#define CALL_STACK
std::string   error_handler::m_sLogPath = "";
#endif


/* ----------------------------------------------------------------------------
* constructor
*/
error_handler::error_handler()
{
    // clean up call stack
    if (USE_CALL_STACK)
    {
        std::string sFilePathStack = this->m_sLogPath + c_sStackfileName;
        if (file_exists(sFilePathStack) && !m_bInitDone)
        {
            // remove file
            remove(sFilePathStack.c_str());
            m_bInitDone = true;
        }
    }
};


/* ----------------------------------------------------------------------------
* init static variables (has to be done once)
*/
void error_handler::init(std::string sLogPath, unsigned int(*funcLogMessage)(const char*, LogLevel, const char*, uint64))
{
    this->m_pFuncLogMessage = funcLogMessage;
    this->m_sLogPath        = sLogPath;
    CALL_STACK
}


/* ----------------------------------------------------------------------------
* write log message to log file
*/
void error_handler::message_callstack(char * pString, void* pClassPointer)
{
    // write to file if path is valid and we have a error message 
    if (folderExists(this->m_sLogPath))
    {
        std::string sFilePath = this->m_sLogPath + c_sStackfileName;

        FILE *pFile;
        fopen_s(&pFile, sFilePath.c_str(), "a");
        if (pFile != nullptr)
        {
            if(pClassPointer == nullptr)
                fprintf_s(pFile, "%s\n", pString);
            else
                fprintf_s(pFile, "%s\n", pString);
            fclose(pFile);
        }
    }
    else
    {
        if (DEBUG_LOG) printf("CallStack path invalid \"%s\"\n", this->m_sLogPath.c_str());
    }

    return;
}


/* ----------------------------------------------------------------------------
* write log message to log file
*/
void error_handler::message_log(char * pString, LogLevel nMode)
{
    CALL_STACK
    // write to debug console
    if (DEBUG_LOG) printf("Write message: \"%s\"\n", pString);

    // write to TS3 Logger, if function pointer is available   
    if(this->m_pFuncLogMessage != nullptr) this->m_pFuncLogMessage(pString, nMode, "Whispermaster2000", 0);

    // write to file if path is valid and we have a error message 
    if (folderExists(this->m_sLogPath) && ((nMode == LogLevel_ERROR) || ((nMode == LogLevel_DEVEL) && DEBUG_LOG)))
    {
        std::string sFilePath = this->m_sLogPath + c_sLogfileName;

        FILE *pFile;
        fopen_s(&pFile, sFilePath.c_str(), "a");
        if (pFile != nullptr)
        {
            fprintf_s(pFile, "%s\n", pString);
            fclose(pFile);
        }
    }
    else
    {
        if (DEBUG_LOG) printf("Logger path invalid \"%s\"\n", this->m_sLogPath.c_str());
    }

    return;
}


/* ----------------------------------------------------------------------------
* write error log message to log file
*/
void error_handler::error_log(const char * pString)
{
    CALL_STACK
    const size_t nErrBuffSize = 512;
    char cErrBuffer[nErrBuffSize];

    //write error
    sprintf_s(cErrBuffer, nErrBuffSize, this->m_cTranslate.translate(L"ErrUnknown").c_str(), pString);
    message_log(cErrBuffer, LogLevel_ERROR);
    return;
}
void error_handler::error_log(const char * pString, std::exception &e)
{
    CALL_STACK
    const size_t nErrBuffSize = 512;
    char cErrBuffer[nErrBuffSize];

    //write error
    sprintf_s(cErrBuffer, nErrBuffSize, this->m_cTranslate.translate(L"ErrStdUnknown").c_str(), pString, e.what());
    message_log(cErrBuffer, LogLevel_ERROR);
    return;
}
void error_handler::error_log(const char * pString, boost::exception &e)
{
    CALL_STACK
    const size_t nErrBuffSize = 512;
    char cErrBuffer[nErrBuffSize];

    //write error
    sprintf_s(cErrBuffer, nErrBuffSize, this->m_cTranslate.translate(L"ErrBoostUnknown").c_str(), pString, boost::diagnostic_information_what(e));
    message_log(cErrBuffer, LogLevel_ERROR);
    return;
}


/* ----------------------------------------------------------------------------
* write debug log message to log file
*/
void error_handler::debug_log(const char * pString)
{
    CALL_STACK
    const size_t nErrBuffSize = 512;
    char cErrBuffer[nErrBuffSize];

    //write error
    sprintf_s(cErrBuffer, nErrBuffSize, "Debug: %s", pString);
    message_log(cErrBuffer, LogLevel_DEBUG);
    return;
}


/* ----------------------------------------------------------------------------
* delete log file
*/
void error_handler::remove_log_file()
{
    CALL_STACK
    std::string sFilePathLog = this->m_sLogPath + c_sLogfileName;
    if (file_exists(sFilePathLog))
    {
        // read all messages from old file
        std::ifstream t(sFilePathLog);
        std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
        t.close();

        //inform user about the error
        QMessageBox cMsgBox(QMessageBox::Information, QString("Information"), QString(this->m_cTranslate.translate(L"ErrExtMessageBox").c_str()), QMessageBox::Ok);
        cMsgBox.setDetailedText(QString(str.c_str()));
        cMsgBox.exec();

        // remove file
        remove(sFilePathLog.c_str());
    }
    return ;
}


/* ----------------------------------------------------------------------------
* check if a file exists
*/
bool error_handler::file_exists(const std::string& sFilename)
{
    if (sFilename.size() == 0)
        return false;

    std::ifstream f(sFilename.c_str());
    return f.good();
}


/* ----------------------------------------------------------------------------
* check if a folder exists
*/
bool error_handler::folderExists(const std::string& sFoldername)
{
    bool bResult = false;
    if (_access(sFoldername.c_str(), 0) == 0)
    {

        struct stat status;
        stat(sFoldername.c_str(), &status);

        bResult = (status.st_mode & S_IFDIR) != 0;
    }
    return bResult;
}