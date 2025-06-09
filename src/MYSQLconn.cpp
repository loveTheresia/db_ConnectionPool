#include "MySQLconn.h"
using namespace std;

MysqlConn::MysqlConn()
{
    m_conn = mysql_init(nullptr);

    //设置默认编码
    mysql_set_character_set(m_conn, "utf8");

};

MysqlConn::~MysqlConn()
{
    if(m_conn != nullptr)mysql_close(m_conn);

    if(m_result != nullptr) {
        freeResult();
    }
};

bool MysqlConn::connect(const std::string& host, const std::string& user, 
                 const std::string& password, const std::string& dbname, 
                 unsigned int port)
{
    if (m_conn == nullptr) {
        cerr << "MySQL connection is not initialized." << endl;
        return false;
    }

    MYSQL* ptr = mysql_real_connect(m_conn, host.c_str(), user.c_str(), password.c_str(), 
                       dbname.c_str(), port, nullptr, 0);

    return ptr != nullptr;
}

bool MysqlConn::update(const std::string& sql)
{
    if (m_conn == nullptr) {
        cerr << "MySQL connection is not initialized." << endl;
        return false;
    }

    if (mysql_query(m_conn, sql.c_str())) {
        cerr << "MySQL update error: " << mysql_error(m_conn) << endl;
        return false;
    }

    return true;
}

bool MysqlConn::query(const std::string& sql)
{
    if (m_conn == nullptr) {
        cerr << "MySQL connection is not initialized." << endl;
        return false;
    }

    if (mysql_query(m_conn, sql.c_str())) {
        cerr << "MySQL query error: " << mysql_error(m_conn) << endl;
        return false;
    }
    //在查询之前先清空数据集(如果本没有数据，也不会浪费资源)
    freeResult();
    m_result = mysql_store_result(m_conn);
    if (m_result == nullptr) {
        cerr << "MySQL store result error: " << mysql_error(m_conn) << endl;
        return false;
    }
    //初始化
    m_row = mysql_fetch_row(m_result);
    return m_row != nullptr;
};

bool MysqlConn::next()
{
    if (m_result == nullptr) {
        cerr << "No result set available." << endl;
        return false;
    }
    //m_row是一个二级指针，指向一级指针的地址
    m_row = mysql_fetch_row(m_result);
    return m_row != nullptr;
}

string MysqlConn::getFieldValue(int index)
{
    if (m_row == nullptr || index < 0 || index >= mysql_num_fields(m_result)) {
        cerr << "Invalid row or index." << endl;
        return "";
    }

    const char* value = m_row[index];
    //函数返回一个一级指针，指向m_result对应列的长度
    unsigned long len = mysql_fetch_lengths(m_result)[index];
    return value ? string(value, len) : "";
}

bool MysqlConn::transactionBegin()
{
    if (m_conn == nullptr) {
        cerr << "MySQL connection is not initialized." << endl;
        return false;
    }
    //将事务提交切换成手动模式（置0）
    if (mysql_autocommit(m_conn, 0)) {
        cerr << "Failed to begin transaction: " << mysql_error(m_conn) << endl;
        return false;
    }
    return true;
}

bool MysqlConn::transactionCommit()
{
    if (m_conn == nullptr) {
        cerr << "MySQL connection is not initialized." << endl;
        return false;
    }

    if (mysql_commit(m_conn)) {
        cerr << "Failed to commit transaction: " << mysql_error(m_conn) << endl;
        return false;
    }
    return true;
}

bool MysqlConn::transactionRollback()
{
    if (m_conn == nullptr) {
        cerr << "MySQL connection is not initialized." << endl;
        return false;
    }

    if (mysql_rollback(m_conn)) {
        cerr << "Failed to rollback transaction: " << mysql_error(m_conn) << endl;
        return false;
    }
    return true;
};

void MysqlConn::refresh_aliveTime()
{
    m_alivetime = steady_clock::now();
}

long long MysqlConn::getAliveTime()
{
    nanoseconds res = steady_clock::now() - m_alivetime;    //获得空闲时间（纳秒精度）
    milliseconds millsec = duration_cast<milliseconds>(res);//转换成毫秒精度

    return millsec.count();

}

void MysqlConn::freeResult()
{
    if (m_result != nullptr) {
        mysql_free_result(m_result);
        m_result = nullptr;
    }
}
