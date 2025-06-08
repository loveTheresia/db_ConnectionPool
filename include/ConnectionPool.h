#include<mutex>
#include<condition_variable>
#include<queue>
#include<memory>
#include <json/json.h>
#include "MySQLconn.h"

using namespace std;
class ConnectionPool {

public:
    //懒汉模式
    static ConnectionPool& getInstance() {
        //不管被调用多少次，静态对象都只会被创建一次(线程安全)
        static ConnectionPool instance; 
        return instance;
    }

    ConnectionPool(const ConnectionPool&) = delete; // 禁止拷贝构造 
    ConnectionPool& operator=(const ConnectionPool&) = delete; // 禁止赋值操作  
    ~ConnectionPool();

    shared_ptr<MysqlConn> getConnection();

private:

    ConnectionPool();
    
    bool parseJsonFile(const string& filePath);

    bool produceConnection();
    bool recycleConnection();

    bool addConnection();

    
    string m_host;
    string m_user;
    string m_passwd;
    string m_dbname;
    unsigned short m_port;
    int m_minSize;
    int m_maxSize;
    int m_timeout;                 //线程等待连接的时间
    int m_idleTimeout;             //连接空闲的时间
    int m_connCount;               //连接池中连接的数量
    bool m_isInitialized;          //连接池是否初始化
    bool m_isRunning;              //连接池是否正在运行
    mutex m_mutex;                //互斥锁，保护连接池的线程安全
    condition_variable m_cond;     //条件变量，用于线程等待连接

    queue<MysqlConn*> m_connQueue; //连接池队列

};