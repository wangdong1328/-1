#include "ToolUtil.h"

ToolUtil::ToolUtil()
{
}

ToolUtil::~ToolUtil()
{
}

/***********************************************************************************
*   @Author     @Time         @Modify        @explain
*   wangdong    2020/12/26    ��һ���޸�      ��ȡָ��·�������е��ļ�
***********************************************************************************/
void ToolUtil::GetPathAllFile(string& _strFilePath, list<string>& _lstFilePath)
{
	//�ļ����
	intptr_t hFile = 0;
	//�ļ���Ϣ
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(_strFilePath).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//�����Ŀ¼,����֮
			//�������,�����б�
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				{
					GetPathAllFile(p.assign(_strFilePath).append("\\").append(fileinfo.name), _lstFilePath);
				}
					
			}
			else
			{
				_lstFilePath.push_back(p.assign(_strFilePath).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

string ToolUtil::GetFileContent(const string& _strFilePath, const bool& bIsBinary)
{
	string strContent = "";
	if (_strFilePath.empty())
	{
		return strContent;
	}
	ifstream inFile;

	if (bIsBinary)
	{
		inFile.open(_strFilePath, std::ios::in | std::ios::binary);
	}
	else
	{
		inFile.open(_strFilePath, std::ios::in);
	}

	if (!inFile.is_open())
	{
		
		return strContent;
	}

	stringstream streamBuffer;
	streamBuffer << inFile.rdbuf();
	inFile.close();

	strContent = streamBuffer.str();
	return strContent;

}


void ToolUtil::GetLineFileContent(const string& _strFilePath, list<string>& _lstFileLineContent)
{
	if (_strFilePath.empty())
	{
		return;
	}

	ifstream inFile;
	inFile.open(_strFilePath, std::ios::in);

	if (!inFile.is_open())
	{
		return;
	}
	string strLineContent = "";
	while (getline(inFile, strLineContent)) // line�в�����ÿ�еĻ��з�
	{
		_lstFileLineContent.push_back(strLineContent);
	}
}

