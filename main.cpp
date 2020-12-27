#include "ToolUtil.h"

#include "MySqlQuery.h"
#include <cassert>
using namespace std;

void MysqlTest();
int main()
{

	
	return 0;
}


void MysqlTest()
{
	MySQLConnection clsConn;
	if (!clsConn.Connect("127.0.0.1", 3306, "root", "123456", "wd"))
	{
		cerr << "Connection failed!!" << endl;
		return;
	}
	cout << "Connection Success" << endl;
	MySQLQuery clsQuery(&clsConn, "select * from tbl_student;");


	clsConn.Disconnect();
}