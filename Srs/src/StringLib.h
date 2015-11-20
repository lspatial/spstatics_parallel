/*********************************************************************************************
* 文件：StringLib
* 功能：基于的std::string实现的常用字符串操作，字符串分割，替换等
* 作者：张晓东
* 时间：2012-11-19
* 修改：2012-11-19完成初步版本，实现：字符串分割，字符串替换，提取文件路径，文件名字，文件扩展名
*********************************************************************************************/
#include <string>
#include <vector>
//using namespace std;

#ifndef   _StringLib_h
#define   _StringLib_h

#ifdef _cplusplus
extern "C" {
#endif
	//从字符串str中，使用pattern进行分割，并存储到strVec中
	bool StringSplit(std::string src, std::string pattern, std::vector<std::string>& strVec)
	{
		std::string::size_type pos;
		src +=pattern;//扩展字符串以方便操作
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
	//将字符串str中的所有target字符串替换为replacement
	bool StringReplace(std::string& src, std::string target, std::string replacement)
	{
		std::string::size_type startpos = 0;  
		while (startpos!= std::string::npos)  
		{  
			startpos = src.find(target);   //找到'.'的位置   
			if( startpos != std::string::npos ) //std::string::npos表示没有找到该字符   
			{  
				src.replace(startpos,1,replacement); //实施替换，注意后面一定要用""引起来，表示字符串   
			}  
		}  
		return true;
	}
	//提取路径中的文件名字（带路径，不带扩展名）
	//substr字符串中，第一个参数为截取的位置，第二个为截取的长度
	std::string StringGetFullFileName(std::string path)
	{
		return path.substr(0, path.rfind('.') == std::string::npos ? path.length() : path.rfind('.') );
	}
	//提取路径中的文件名字
	std::string StringGetFileName(std::string path)
	{
		StringReplace(path, "/", "\\");
		std::string::size_type startpos = path.rfind('\\') == std::string::npos ? path.length() : path.rfind('\\')+1;
		std::string::size_type endpos   = path.rfind('.') == std::string::npos ? path.length() : path.rfind('.');
		return path.substr(startpos, endpos-startpos);
	}
	//提取路径中文件名字（带扩展名）
	std::string StringGetFileNameWithExt(std::string path)
	{
		StringReplace(path, "/", "\\");
		std::string::size_type startpos = path.rfind('\\') == std::string::npos ? path.length() : path.rfind('\\')+1;
		return path.substr(startpos);
	}
	//提取路径中的文件路径
	std::string StringGetDirectory(std::string path)
	{
		StringReplace(path, "/", "\\");
		return path.substr(0, path.rfind('\\') == std::string::npos ? path.length() : path.rfind('\\') );
	}
	//提取路径中的文件类型
	std::string StringGetFileExt(std::string path)
	{
		StringReplace(path, "/", "\\");
		return path.substr(path.rfind('.') == std::string::npos ? path.length() : path.rfind('.')+1 );
	}
#ifdef _cplusplus
}
#endif
#endif
