#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include "html/maye_define.h"
/**
 * 读取html文件，跳过空行能大幅度缩减保存的html数据.
 */
inline std::string readHtml(const std::string& htmlFile)
{
	std::ifstream infile(htmlFile,std::ios::binary);
	if (!infile) {
		std::cout << "open finfile <" << htmlFile << "> filed" << std::endl;
		return {};
	}
	std::string buf;
	while (!infile.eof()) {
		std::string line;
		if (!std::getline(infile, line)) {
			break;
		}
		//为空直接继续
		if (line.empty())
			continue;

		//跳过空行
		auto p = line.begin();
		while (p != line.end() && isspace(*p)) {
			p++; 
		}
		if (p == line.end()) {
			continue;
		}
		//std::cout << std::string(p,line.end()-1) <<"##" << std::endl;
		//-1删除结尾的\r或者\n
		buf.append(p,line.end()-1);
	}
	return buf;
}

