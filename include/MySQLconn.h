#include<iostream>
#include<mysql/mysql.h>
#include<chrono>
using namespace std;
using namespace chrono;

class MysqlConn
{
public:
    //初始化数据库连接
    MysqlConn();
    //释放数据库连接
    ~MysqlConn();
    //连接数据库
    bool connect(const std::string& host, const std::string& user, 
                 const std::string& password, const std::string& dbname, 
                 unsigned int port = 3306);
    //更新数据库
    bool update(const std::string& sql);
    //查询数据库
   bool query(const std::string& sql);

    //遍历查询得到的结果集
    bool next();
    //获取结果集的字段值
    std::string getFieldValue(int index);

    //事务操作
    bool transactionBegin();

    //提交事务
    bool transactionCommit();

    //回滚事务
    bool transactionRollback();

    //刷新空闲时间
    void refresh_aliveTime();

    //计算存活总时长
    long long getAliveTime();
    
private:
        void freeResult(); //释放结果集
        
        MYSQL* m_conn = nullptr;
        MYSQL_RES* m_result = nullptr; //查询结果集
        MYSQL_ROW m_row = nullptr;     //当前行数据

        steady_clock::time_point m_alivetime;   //总存活时间
};