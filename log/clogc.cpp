#include"clogc.h"

namespace chu_log
{

     string CLog::m_log_path=getLogFilePath();


    void  CLog::setPath(string path)
    {
        m_log_path=path;
    }

    string CLog::getPath()
    {
        return m_log_path;
    }

    string CLog::getLogFilePath()
    {
        /*char* szPath;
        GetModuleFileNameA( NULL, szPath, MAX_PATH ) ;
        ZeroMemory(strrchr(szPath,_T('\\')), strlen(strrchr(szPath,_T('\\') ) )*sizeof(CHAR)) ;
        strcat(szPath,"\\");
        strcat(szPath,LOG_FILE_NAME);
        return szPath;*/
        return LOG_FILE_NAME;
    }


    string CLog::getSystemTime()
    {
        //get local time
        time_t now_time;
        time(&now_time);
        tm* local_time = localtime(&now_time);

        //time to string
        char szTime[30] = {'\0'};
        strftime(szTime, 30, "[%Y-%m-%d %H:%M:%S] ", local_time);
        string time = szTime;
        return time;
    }
}
