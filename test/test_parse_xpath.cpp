#include "html/ParserDom.h"
#include "xpath/XPath.h"
#include <fstream>
#include <iostream>

/**
 * 读取html文件.
 */
std::string readHtml(const std::string& htmlFile)
{
	std::ifstream infile(htmlFile);
	if (!infile) {
		std::cout << "open finfile <" << htmlFile << "> filed" << std::endl;
		return {};
	}
	std::string buf;
	while (!infile.eof())
	{
		std::string line;
		if (!std::getline(infile, line)) {
			break;
		}
		buf.append(line);
	}
	return buf;
}

/**
 * 测试解析所有a标签.
 */
int test_parse(const std::string& html)
{
	htmlcxx::HTML::ParserDom parser;
	auto dom = parser.parseTree(html);

	for (auto& node : dom)
	{
		if (node.isTag())
		{
			if (node.tagName() == "a")
			{
				node.parseAttributes();
				auto pr = node.attribute("href");
				if (pr.first) {
					std::cout << pr.second << std::endl;
				}

			}
		}
	}

	return 0;
}

/**
 * 使用xpath解析.
 */
int test_xpath(const std::string& html)
{
	htmlcxx::XPath xpath(html);
	//完整xpath
	auto path =  R"(/html/body/div[1]/div[2]/div[4]/div[4]/div[2]/div/ul)";
	//xpaht
	//auto path =  R"(*[@id="j-catalogWrap"]/div[2]/div/ul)";

	//找到ul标签
	auto result = xpath.xpath(path);
	system("chcp 65001");
	//遍历ul标签的所有后代a标签
	for(auto it = result.begin();it!=result.end();it++) {
		for (auto& node : it) {
			if (!node.isTag())
				continue;

			if (node.tagName() == "a")
			{
				node.parseAttributes();
				auto pr = node.attribute("href");
				if (pr.first) {
					std::cout << pr.second << " : " << xpath.getNodeContent(node) << std::endl;
				}
			}
		}
	}

	return 0;
}


int main(int argc, char* argv[])
{
	auto html = readHtml("html/hongxiu.html");
	if (html.empty()) {
		std::cout << "html/hongxiu.html not found" << std::endl;
		return 0;
	}
	std::cout << "test_parse=====================================" << std::endl;
	test_parse(html);
	std::cout << "test_xpath=====================================" << std::endl;
	test_xpath(html);
	return 0;
}
