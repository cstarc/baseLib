
//************************
//** use for output log into file
//** author :chuhuaxing
//************************

#ifndef CLOGC_H_INCLUDED
#define CLOGC_H_INCLUDED

#define LOG_FILE_NAME "log.txt"

#include <fstream>
#include <string>
#include <ctime>

using namespace std;

namespace chu_log
{
    class CLog
    {
    private:
        static string m_log_path ;

    public:

        //*********
        //write into log file
        //other specific log can use this to build just like writeFuncBegin...
        //*********
        template <class T>
        static void writeLog(T log)
        {
            ofstream fout(m_log_path.c_str(),ios::app);
            //fout.seekp(ios::end);
            fout << getSystemTime()<< log <<endl;
            fout.close();
        }

        //*********
        //输出一行当前函数开始的标志,宏传入__FUNCTION__
        //*********
        template <class T>
        static void writeFuncBegin(T func)
        {
            string log;
            log=log+ "--------------------"+func+"  Begin--------------------";
            writeLog(log);
        }

        //*********
        //输出一行当前函数结束的标志，宏传入__FUNCTION__
        //*********
        template <class T>
        static void writeFuncEnd(T func)
        {
            string log;
            log=log+ "--------------------"+func+"  End--------------------";
            writeLog(log);
        }

        static void  setPath(string path);  //set log file path to param path
        static string getPath();            //get current log file path

    private:
        static string getSystemTime();     //time type:  y-m-d h:m:s
        static string getLogFilePath();
    };




}

#endif // CLOGC_H_INCLUDED
