#include "ConnectionPool.h"
#include<fstream>
#include<thread>
using namespace std;
using namespace Json;
bool ConnectionPool::parseJsonFile(const string& filePath)
{
    ifstream ifs("../dbconf.json");
    if (!ifs.is_open()) {
        cerr << "Failed to open JSON file: " << filePath << endl;
        return false;
    }
    Reader rd;
    Value root;
    rd.parse(ifs, root);
    if(root.isObject()) {
        m_host = root["host"].asString();
        m_user = root["user"].asString();
        m_passwd = root["passwd"].asString();
        m_dbname = root["dbname"].asString();
        m_port = root["port"].asUInt();
        m_minSize = root["minSize"].asInt();
        m_maxSize = root["maxSize"].asInt();
        m_timeout = root["timeout"].asInt();
        m_idleTimeout = root["idleTimeout"].asInt();
        return true;
    } else {
        cerr << "Invalid JSON format in file: " << filePath << endl;
    }
    return false;
}

ConnectionPool::ConnectionPool() 
    : m_host(""), m_user(""), m_passwd(""), m_dbname(""),
      m_port(0), m_minSize(0), m_maxSize(0), m_timeout(0),
      m_idleTimeout(0), m_connCount(0), m_isInitialized(false), m_isRunning(false)
{
    if (!parseJsonFile("dbconfig.json")) {
        cerr << "Failed to initialize connection pool from JSON file." << endl;
        return;
    }
    
    // Initialize the connection pool with the specified minimum size
    for (int i = 0; i < m_minSize; ++i) {
        addConnection();
    }
    //创建生产者线程和回收者线程
    thread producer(&ConnectionPool::produceConnection, this);
    thread recycle(&ConnectionPool::recycleConnection, this);
    producer.detach();//线程分离函数，让线程转为后台执行，与主线程分离
    recycle.detach();
   
}

ConnectionPool::~ConnectionPool()
{
    //队列的连接需要一个个先析构掉
    while(m_connQueue.empty())
    {
        MysqlConn* conn = m_connQueue.front();
        m_connQueue.pop();
        delete conn;
    }

}

bool ConnectionPool::produceConnection()
{
    while(true)
    {
        //由lock对象自动管理m_mutex的加锁和解锁（只有一个线程能抢到这个锁并向下执行）
        unique_lock<mutex> lock(m_mutex);
        while(m_connQueue.size() >= m_maxSize) {
            //阻塞不会占用系统资源
           m_cond.wait(lock); 
        }
        addConnection();
        //由于生产者和回收这公用一个条件变量，所以只能唤醒所有线程，再根据各自函数内的判断条件来决定谁能拿到锁
        //
        m_cond.notify_all();
    }
}

bool ConnectionPool::recycleConnection()
{
    while(true)
    {
        this_thread::sleep_for(chrono::seconds(1));//回收者线程不需要高频执行，可以让线程休眠一段时间
        lock_guard<mutex> locker(m_mutex);
        while(m_connQueue.size() > m_maxSize)
        {
            //从队列头（最早放入的连接，有最长的等待时间）释放连接
            MysqlConn* conn = m_connQueue.front();
            if(conn->getAliveTime() >= m_idleTimeout)//如果等待时间超时了，就释放
            {
                m_connQueue.pop();
                delete conn;
            }
            else break;
        }
    }
}

bool ConnectionPool::addConnection()
{
    MysqlConn* conn = new MysqlConn;
        if (conn->connect(m_host, m_user, m_passwd, m_dbname, m_port)) {
            conn->refresh_aliveTime();
            m_connQueue.push(conn);
        
        } else { 
            delete conn;
            cerr << "Failed to create initial connection." << endl;
            return false;
        } 
   
    return true;
}

shared_ptr<MysqlConn> ConnectionPool::getConnection()
{
    unique_lock<mutex> locker(m_mutex);
    //如果连接池没有连接，就让进程阻塞一会
    while(m_connQueue.empty())
    {
        //让调用者最长等待一个设定好的时间
        if(cv_status::timeout == m_cond.wait_for(locker,chrono::milliseconds(m_timeout)));
        {
            //时间到了依旧没等到连接，那就继续等
            if(m_connQueue.empty())
            continue;

        }
    }
    //等到了连接,用智能指针把连接取出，通过重载智能指针的删除器，然连接用完后自动回到队列中
    shared_ptr<MysqlConn> conn(m_connQueue.front(),[this](MysqlConn* conn){
        lock_guard<mutex> locker(m_mutex);
        m_connQueue.push(conn);
        conn->refresh_aliveTime();
    });
    m_connQueue.pop();
    m_cond.notify_all();

    return conn;
}