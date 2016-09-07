#include "db_access.h"

CDbAcess::CDbAcess()
{
    mysql_init(&mysql);

}

bool CDbAcess::connect(string ip,int port,string username, string password,string db)
{
  if(!mysql_real_connect(&mysql, ip.c_str(), username.c_str(), password.c_str(), db.c_str(), port, 0, 0))
  {
     cerr << "mysql_real_connect: "  << mysql_error(&mysql)  << endl ;
     return false;
  }

  if (mysql_set_character_set(&mysql, "GBK") != 0)
  {
        cerr << "mysql_set_character_set: "  << mysql_error(&mysql)  << endl ;
  }
  return true;
}

///
///
///
bool CDbAcess::executeSql(string sql)
{
    if (mysql_query(&mysql, sql.c_str()) != 0)
    {
        cerr << "mysql_query: " << mysql_error(&mysql) << endl;
        return false;
    }
    return true;
}


///
///
///
bool CDbAcess::updateCol(string tableName,string col,string colvalues, string selectkey, string selectvalue)
{
    string sql="update "+tableName+" set "+ col + " = '" + colvalues+"'";
    if(!selectkey.empty())
      sql += " WHERE " + selectkey + " = '" + selectvalue + "' ";
    return executeSql(sql);
}

///
///
///
string CDbAcess::selectData(string table,string col,string selectkey, string selectvalue  )
{

}
string CDbAcess::selectData(string sql)
{
  if(executeSql(sql))
   {
       MYSQL_RES* result;
       result = mysql_store_result(&mysql);
       MYSQL_ROW row = mysql_fetch_row(result);
       return row[0];
   }
   return "";
}

///
///
///
int CDbAcess::countNumber()
{
    string sql="select member_number from customer_info;";
    mysql_query(&mysql, sql.c_str());

    MYSQL_RES* result;
    result = mysql_store_result(&mysql);
    return mysql_num_rows(result);
}
