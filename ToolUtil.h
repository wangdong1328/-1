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
	 //2020/11/21 wangdong: ��ȡ�ļ����������ļ���
	void GetPathAllFile(string& _strFilePath, list<string>& _lstAllFilePath);
	
	 //2020/12/26 wangdong: ��ȡָ��·��������
	string GetFileContent(const string& _strFilePath, const bool& bIsBinary);

	 //2020/12/26 wangdong: ���л�ȡ�ļ�����
	void GetLineFileContent(const string& _strFilePath, list<string>& _lstAllFilePath);

private:
	

};

