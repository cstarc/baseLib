#ifndef DB_ACCESS_H_INCLUDED
#define DB_ACCESS_H_INCLUDED
#include <iostream>
#include <string>
#include <winsock2.h>
#include <mysql.h>

using namespace std;
class CDbAcess
{
private:
    MYSQL mysql;

private:

public:
    CDbAcess();
    ~CDbAcess(){mysql_close(&mysql);}
    bool connect(string ip="127.0.0.1",int port=3306,string username="testuser", string password="testuser123",string db="testdb");

    bool executeSql(string sql);  //excute sql sentence
    bool updateCol(string tableName,string col,string colvalues, string selectkey="", string selectvalue="");  //update one col in table
    bool deleteData(string sql);
    bool insertData();
    int countNumber();
    string selectData(string table,string col,string selectkey, string selectvalue );  //select one piece of data
    string selectData(string sql);
};



#endif // DB_ACCESS_H_INCLUDED
