#include<iostream>
#include<memory>
#include<thread>
#include "MySQLconn.h"
using namespace std;

int query()
{
    MysqlConn conn;
    conn.connect("lovetheresia","123456","db_project01","172.17.0.2:3306");
    string sql = "insert into person values(5,25,'man','tom')";
    bool flag = conn.update(sql);
    cout<<"flag value: "<<flag<<endl;

    sql = "select * from person";
    conn.query(sql);

    while(conn.next())
    {
        cout<<conn.getFieldValue(0)<<", "
            <<conn.getFieldValue(1)<<", "
            <<conn.getFieldValue(2)<<", "
            <<conn.getFieldValue(3)<<endl;
    }

    return 0;

}

int main()
{
    query();

    return 0;
}