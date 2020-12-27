#pragma once
#include <iostream>
#include <string>
#include <list>
#include <io.h>
#include <fstream>
#include <sstream>
using namespace std;


class ToolUtil
{
public:
	ToolUtil();
	~ToolUtil();

public:
	 //2020/11/21 wangdong: 获取文件夹下所有文件名
	void GetPathAllFile(string& _strFilePath, list<string>& _lstAllFilePath);
	
	 //2020/12/26 wangdong: 获取指定路径的内容
	string GetFileContent(const string& _strFilePath, const bool& bIsBinary);

	 //2020/12/26 wangdong: 按行获取文件内容
	void GetLineFileContent(const string& _strFilePath, list<string>& _lstAllFilePath);

private:
	

};

