/*********************************************************************************************
* �ļ���StringLib
* ���ܣ����ڵ�std::stringʵ�ֵĳ����ַ����������ַ����ָ�滻��
* ���ߣ�������
* ʱ�䣺2012-11-19
* �޸ģ�2012-11-19��ɳ����汾��ʵ�֣��ַ����ָ�ַ����滻����ȡ�ļ�·�����ļ����֣��ļ���չ��
*********************************************************************************************/
#include <string>
#include <vector>
//using namespace std;

#ifndef   _StringLib_h
#define   _StringLib_h

#ifdef _cplusplus
extern "C" {
#endif
	//���ַ���str�У�ʹ��pattern���зָ���洢��strVec��
	bool StringSplit(std::string src, std::string pattern, std::vector<std::string>& strVec)
	{
		std::string::size_type pos;
		src +=pattern;//��չ�ַ����Է������
		int size=src.size();

		for(int i=0; i<size; i++)
		{
			pos = src.find(pattern,i);
			if(pos<size)
			{
				std::string s=src.substr(i,pos-i);
				strVec.push_back(s);
				i=pos+pattern.size()-1;
			}
		}
		return true;
	}
	//���ַ���str�е�����target�ַ����滻Ϊreplacement
	bool StringReplace(std::string& src, std::string target, std::string replacement)
	{
		std::string::size_type startpos = 0;  
		while (startpos!= std::string::npos)  
		{  
			startpos = src.find(target);   //�ҵ�'.'��λ��   
			if( startpos != std::string::npos ) //std::string::npos��ʾû���ҵ����ַ�   
			{  
				src.replace(startpos,1,replacement); //ʵʩ�滻��ע�����һ��Ҫ��""����������ʾ�ַ���   
			}  
		}  
		return true;
	}
	//��ȡ·���е��ļ����֣���·����������չ����
	//substr�ַ����У���һ������Ϊ��ȡ��λ�ã��ڶ���Ϊ��ȡ�ĳ���
	std::string StringGetFullFileName(std::string path)
	{
		return path.substr(0, path.rfind('.') == std::string::npos ? path.length() : path.rfind('.') );
	}
	//��ȡ·���е��ļ�����
	std::string StringGetFileName(std::string path)
	{
		StringReplace(path, "/", "\\");
		std::string::size_type startpos = path.rfind('\\') == std::string::npos ? path.length() : path.rfind('\\')+1;
		std::string::size_type endpos   = path.rfind('.') == std::string::npos ? path.length() : path.rfind('.');
		return path.substr(startpos, endpos-startpos);
	}
	//��ȡ·�����ļ����֣�����չ����
	std::string StringGetFileNameWithExt(std::string path)
	{
		StringReplace(path, "/", "\\");
		std::string::size_type startpos = path.rfind('\\') == std::string::npos ? path.length() : path.rfind('\\')+1;
		return path.substr(startpos);
	}
	//��ȡ·���е��ļ�·��
	std::string StringGetDirectory(std::string path)
	{
		StringReplace(path, "/", "\\");
		return path.substr(0, path.rfind('\\') == std::string::npos ? path.length() : path.rfind('\\') );
	}
	//��ȡ·���е��ļ�����
	std::string StringGetFileExt(std::string path)
	{
		StringReplace(path, "/", "\\");
		return path.substr(path.rfind('.') == std::string::npos ? path.length() : path.rfind('.')+1 );
	}
#ifdef _cplusplus
}
#endif
#endif
