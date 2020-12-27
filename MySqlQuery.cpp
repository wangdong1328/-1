
#include "MySqlQuery.h"
#include <cassert>


MySQLConnection::MySQLConnection()
{
    m_bIsConnected = false;
    m_pMySQLConn = NULL;
    m_sHostname = "";
    m_sUsername = "";
    m_sPassword = "";
    m_wPort = 0;
    m_sSchemaName = "";
}

MySQLConnection::~MySQLConnection()
{
    if(m_pMySQLConn != nullptr)
    {
        std::clog << "Closing MySQL Connection" << std::endl;
        mysql_close(m_pMySQLConn);
    }
}

bool MySQLConnection::Connect(const std::string &sHostname, const uint16_t &wPort, 
	const std::string &sUsername, const std::string &sPassword, const std::string &sDB = NULL)
{
    // If we're already connected, we should close the first connection
    Disconnect();

    m_sHostname = sHostname;
    m_sUsername = sUsername;
    m_sPassword = sPassword;
    m_wPort = wPort;
    m_sSchemaName = sDB;
    m_bIsConnected = false;

    MYSQL *pMySQLConnRet = nullptr;
    m_pMySQLConn = mysql_init(m_pMySQLConn);

    std::clog << "Connection to " << m_sUsername << "@" << m_sHostname << ":" << wPort << "..." << std::endl;

    pMySQLConnRet = mysql_real_connect(m_pMySQLConn, m_sHostname.c_str(), m_sUsername.c_str(), m_sPassword.c_str(), m_sSchemaName.c_str(), m_wPort, NULL, 0);

    if(nullptr == pMySQLConnRet)
    {
        m_bIsConnected = false;
        std::cerr << "Connection failed: " << mysql_error(m_pMySQLConn);
    } 
    else 
    {
        m_bIsConnected = true;
        std::clog << "Connected!" << std::endl;
    }

    return m_bIsConnected;
}

void MySQLConnection::Disconnect()
{
    if(m_bIsConnected)
    {
        mysql_close(m_pMySQLConn);
        m_pMySQLConn = nullptr;
        std::clog << "Disconnected from MySQL DB!" << std::endl;
    }


    m_bIsConnected = false;

}

bool MySQLConnection::SelectDB(const std::string &sSchemaName)
{
    if(!m_bIsConnected)
    {
        std::cerr << "Not connected to MySQL DB!" << std::endl;
        return false;
    }

    if(mysql_select_db(m_pMySQLConn, sSchemaName.c_str()) != 0)
    {
        std::cerr << "Failed to select DB! Error: " << mysql_error(m_pMySQLConn) << std::endl;
        return false;
    } 
    else 
    {
        m_sSchemaName = sSchemaName.c_str();
        std::clog << "Selected database \"" << sSchemaName << "\"" << std::endl;
        return true;
    }
}

const std::string MySQLConnection::GetLastError() const
{
    if(!m_bIsConnected)
    {
        std::cerr << "Not connected to MySQL DB!" << std::endl;
        return "Not connected";
    }

    return (char*)mysql_error(m_pMySQLConn);
}

MYSQL *MySQLConnection::getConn()
{
    return m_pMySQLConn;
}

bool MySQLConnection::IsConnected()
{
    return m_bIsConnected;
}

const std::string MySQLConnection::EscapeString(const std::string &_strValue) const
{
    if(!m_bIsConnected)
    {
        std::cerr << "Not connected to MySQL DB!" << std::endl;
        return "";
    }

    char *pValue = new char[(_strValue.length()*2)+1];
    mysql_real_escape_string(m_pMySQLConn, pValue, _strValue.c_str(), _strValue.length());

    std::string sRet = pValue;
    delete[] pValue;
    pValue = nullptr;

    return sRet;
}


bool MySQLConnection::StartAutoCommit()
{
	if (nullptr == m_pMySQLConn)
	{
		std::cerr << "MySQL Error: " << std::endl;
		return false;
	}

	mysql_autocommit(m_pMySQLConn, 0);
	return false;
}

bool MySQLConnection::TransctionCommit()
{
	if (nullptr == m_pMySQLConn)
	{
		return false;
	}

	mysql_commit(m_pMySQLConn);
	return false;
}

bool MySQLConnection::TransctionRollback()
{
	if (nullptr == m_pMySQLConn)
	{
		return false;
	}

	mysql_rollback(m_pMySQLConn);
	return false;
}

bool MySQLConnection::EndAutoCommit()
{
	if (nullptr == m_pMySQLConn)
	{
		std::cerr << "MySQL Error: " << std::endl;
		return false;
	}
	mysql_autocommit(m_pMySQLConn, 1);
	return true;
}



MySQLQuery::MySQLQuery(MySQLConnection *pConn, const std::string &strStatement)
{
    m_sqlConn = pConn;
    m_sStatement = strStatement;
    m_iResultRowCount = 0;

    int argCount = std::count(m_sStatement.begin(), m_sStatement.end(), '?');
    for(int i = 1; i <= argCount; i++)
    {
        m_mArgMap.insert(std::pair<int, std::string>(i, ""));
    }
}

MySQLQuery::~MySQLQuery()
{
}

bool MySQLQuery::setString(const unsigned int &idx, const std::string &value)
{

    if(idx > m_mArgMap.size())
    {
        std::cerr << "Index exceeds total arg count in statement" << std::endl;
        return false;
    }

    std::stringstream ss;
    std::string escapedValue = m_sqlConn->EscapeString(value);
    ss << "\"" << escapedValue << "\"";
    m_mArgMap[idx] = ss.str();

    return true;
}

bool MySQLQuery::setInt(const unsigned int &idx, const int &value)
{
    if(idx > m_mArgMap.size())
    {
        std::cerr << "Index exceeds total arg count in statement" << std::endl;
        return false;
    }

    std::stringstream ss;
    ss << value;
    m_mArgMap[idx] = ss.str();

    return true;
}

bool MySQLQuery::setDouble(const unsigned int &idx, const double &_dValue)
{
    if(idx > m_mArgMap.size())
    {
        std::cerr << "Index exceeds total arg count in statement" << std::endl;
        return false;
    }

    std::stringstream ss;
    ss << _dValue;
    m_mArgMap[idx] = ss.str();

    return true;
}

bool MySQLQuery::setTime(const unsigned int &idx, const time_t &value)
{
    if(idx > m_mArgMap.size())
    {
        std::cerr << "Index exceeds total arg count in statement" << std::endl;
        return false;
    }

    std::stringstream ss;
    ss << value;
    m_mArgMap[idx] = ss.str();

    return true;
}

bool MySQLQuery::setNull(const unsigned int &idx)
{
    if(idx > m_mArgMap.size())
    {
        std::cerr << "Index exceeds total arg count in statement" << std::endl;
        return false;
    }

    m_mArgMap[idx] = "NULL";
    return true;
}

const std::string MySQLQuery::getFieldName(const unsigned int &iField)
{
    if(iField < 1)
    {
        std::cerr << "The field index has to be over 1!" << std::endl;
        return "";
    }
    else if(m_mFieldMap.size() < iField)
    {
        std::cerr << "There are only " << m_mFieldMap.size() << " fields available!" << std::endl;
        return "";
    }

    std::string sFieldName = m_mFieldMap[iField];
    return sFieldName;
}

const std::string MySQLQuery::getString(const unsigned int &row, const unsigned int &iField)
{
    if(GetResultRowCount() < 1)
    {
        std::cerr << "The query didn't return any rows!" << std::endl;
        return "";
    } 
    else if(GetResultRowCount() < row)
    {
        std::cerr << "There are only " << GetResultRowCount() << " rows available!" << std::endl;
        return "";
    } 
    else if(row < 1)
    {
        std::cerr << "The selected row has to be > 1" << std::endl;
        return "";
    }

    TResultRow rSelectedRow;
    rSelectedRow = m_mResultMap[row-1];

    std::string sValue = rSelectedRow[iField];

    return sValue;
}

const std::string MySQLQuery::getString(const unsigned int &iRow, const std::string &strField)
{
    if(GetResultRowCount() < 1)
    {
        std::cerr << "The query didn't return any rows!" << std::endl;
        return "";
    }
    else if(GetResultRowCount() < iRow)
    {
        std::cerr << "There are only " << GetResultRowCount() << " rows available!" << std::endl;
        return "";
    } 
    else if(iRow < 1)
    {
        std::cerr << "The selected row has to be > 1" << std::endl;
        return "";
    }

    TResultRow rSelectedRow;
    rSelectedRow = m_mResultMap[iRow-1];

    int iFieldID = m_mFieldStringToIntMap[strField];
    std::string sValue = rSelectedRow[iFieldID];

    return sValue;
}

int MySQLQuery::getInt(const unsigned int &iRow, const unsigned int &iField)
{
    if(GetResultRowCount() < 1)
    {
        std::cerr << "The query didn't return any rows!" << std::endl;
        return -1;
    } 
    else if(GetResultRowCount() < iRow)
    {
        std::cerr << "There are only " << GetResultRowCount() << " rows available!" << std::endl;
        return -1;
    } 
    else if(iRow < 1)
    {
        std::cerr << "The selected row has to be > 1" << std::endl;
        return -1;
    }

    TResultRow rSelectedRow;
    rSelectedRow = m_mResultMap[iRow-1];

    int iValue = atoi(rSelectedRow[iField].c_str());

    return iValue;
}

int MySQLQuery::getInt(const unsigned int &iRow, const std::string &strField)
{
    if(GetResultRowCount() < 1)
    {
        std::cerr << "The query didn't return any rows!" << std::endl;
        return -1;
    } 
    else if(GetResultRowCount() < iRow)
    {
        std::cerr << "There are only " << GetResultRowCount() << " rows available!" << std::endl;
        return -1;
    } 
    else if(iRow < 1)
    {
        std::cerr << "The selected row has to be > 1" << std::endl;
        return -1;
    }

    TResultRow rSelectedRow;
    rSelectedRow = m_mResultMap[iRow-1];

    int iFieldID = m_mFieldStringToIntMap[strField];
    int iValue = atoi(rSelectedRow[iFieldID].c_str());

    return iValue;
}

double MySQLQuery::getDouble(const unsigned int &iRow, const unsigned int &iField)
{
    if(GetResultRowCount() < 1)
    {
        std::cerr << "The query didn't return any rows!" << std::endl;
        return -1.0;
    } 
    else if(GetResultRowCount() < iRow)
    {
        std::cerr << "There are only " << GetResultRowCount() << " rows available!" << std::endl;
        return -1.0;
    } 
    else if(iRow < 1)
    {
        std::cerr << "The selected row has to be > 1" << std::endl;
        return -1.0;
    }

    TResultRow rSelectedRow;
    rSelectedRow = m_mResultMap[iRow-1];

    double dValue = atof(rSelectedRow[iField].c_str());

    return dValue;
}

double MySQLQuery::getDouble(const unsigned int &iRow, const std::string &strField)
{
    if(GetResultRowCount() < 1)
    {
        std::cerr << "The query didn't return any rows!" << std::endl;
        return -1.0;
    } 
    else if(GetResultRowCount() < iRow)
    {
        std::cerr << "There are only " << GetResultRowCount() << " rows available!" << std::endl;
        return -1.0;
    } 
    else if(iRow < 1)
    {
        std::cerr << "The selected row has to be > 1" << std::endl;
        return -1.0;
    }

    TResultRow rSelectedRow;
    rSelectedRow = m_mResultMap[iRow-1];

    int iFieldID = m_mFieldStringToIntMap[strField];
    double dValue = atof(rSelectedRow[iFieldID].c_str());

    return dValue;
}

time_t MySQLQuery::getTime(const unsigned int &row, const unsigned int &field)
{
    if(GetResultRowCount() < 1)
    {
        std::cerr << "The query didn't return any rows!" << std::endl;
        return -1;
    } 
    else if(GetResultRowCount() < row)
    {
        std::cerr << "There are only " << GetResultRowCount() << " rows available!" << std::endl;
        return -1;
    } 
    else if(row < 1)
    {
        std::cerr << "The selected row has to be > 1" << std::endl;
        return -1;
    }

    TResultRow rSelectedRow;
    rSelectedRow = m_mResultMap[row-1];

    time_t tValue = atoi(rSelectedRow[field].c_str());

    return tValue;
}

time_t MySQLQuery::getTime(const unsigned int &row, const std::string &field)
{
    if(GetResultRowCount() < 1)
    {
        std::cerr << "The query didn't return any rows!" << std::endl;
        return -1;
    } 
    else if(GetResultRowCount() < row)
    {
        std::cerr << "There are only " << GetResultRowCount() << " rows available!" << std::endl;
        return -1;
    } 
    else if(row < 1)
    {
        std::cerr << "The selected row has to be > 1" << std::endl;
        return -1;
    }

    TResultRow rSelectedRow;
    rSelectedRow = m_mResultMap[row-1];

    int iFieldID = m_mFieldStringToIntMap[field];
    time_t tValue = atoi(rSelectedRow[iFieldID].c_str());

    return tValue;
}


unsigned int MySQLQuery::GetResultRowCount()
{
    const int iRowCount = m_mResultMap.size();
    return iRowCount;
}

unsigned int MySQLQuery::GetFieldCount()
{
    const int iFieldCount = m_mFieldMap.size();
    return iFieldCount;
}

const std::string MySQLQuery::BuildQueryString()
{
    // replace each '?' with the corresponding value
    int iLastFoundPos = 0;
    std::string sPreparedStatement;
    sPreparedStatement = m_sStatement;
    for(unsigned int i = 1; i <= m_mArgMap.size(); i++)
    {
        iLastFoundPos = sPreparedStatement.find('?');
        sPreparedStatement.replace(iLastFoundPos, 1, "");
        sPreparedStatement.insert(iLastFoundPos, m_mArgMap[i]);
    }

    return sPreparedStatement;
}

bool MySQLQuery::ExecuteQuery()
{
    std::string sStatement = BuildQueryString();

    if(mysql_query(m_sqlConn->getConn(), sStatement.c_str()))
    {
        std::cerr << "MySQL Error: " << m_sqlConn->GetLastError() << std::endl;
        return false;
    }

    MYSQL_RES *pResult = mysql_store_result(m_sqlConn->getConn());

    if(pResult == NULL)
    {
        std::cerr << "MySQL Error: " << m_sqlConn->GetLastError() << std::endl;
        return false;
    }


    int iNumFields = mysql_num_fields(pResult);
    MYSQL_ROW row;
    MYSQL_FIELD *field;

    // Get field names and store it in the map
    int i = 0;
    while((field = mysql_fetch_field(pResult)))
    {
        m_mFieldMap.insert(std::pair<int, std::string>(i, field->name));
        m_mFieldStringToIntMap.insert(std::pair<std::string, int>(field->name, i));
        i++;
    }

    // Get Rows
    i = 0;
    while((row = mysql_fetch_row(pResult)))
    {
        TResultRow resRow;
        for(int n = 0; n < iNumFields; n++)
        {
            resRow.insert(std::pair<int, std::string>(n, row[n] ? row[n] : "NULL"));
        }

        m_mResultMap.insert(std::pair<int, TResultRow>(i, resRow));

        i++;
    }

    return true;
}

bool MySQLQuery::ExecuteUpdate()
{
    std::string sStatement = BuildQueryString();

    if(mysql_query(m_sqlConn->getConn(), sStatement.c_str()))
    {
        std::cerr << "MySQL Error: " << m_sqlConn->GetLastError() << std::endl;
        return false;
    }

    return true;
}

bool MySQLQuery::ExecuteInsert()
{
    std::string sStatement = BuildQueryString();

    if(mysql_query(m_sqlConn->getConn(), sStatement.c_str()))
    {
        std::cerr << "MySQL Error: " << m_sqlConn->GetLastError() << std::endl;
        return false;
    }

    return true;
}

bool MySQLQuery::ExecuteDelete()
{
	std::string sStatement = BuildQueryString();

	if (mysql_query(m_sqlConn->getConn(), sStatement.c_str()))
	{
		std::cerr << "MySQL Error: " << m_sqlConn->GetLastError() << std::endl;
		return false;
	}

	return true;
}

