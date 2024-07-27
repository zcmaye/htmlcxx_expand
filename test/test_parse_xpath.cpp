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
	//auto path = R"(//*[@id="j-catalogWrap"]/div[2]/div/ul/li[1]/a)";

	//找到ul标签
	auto result = xpath.xpath(path);
	if (result)
	{
		std::cout << "=====1=====" << std::endl;
		std::cout << result.nodes().front()->text() << std::endl;
		std::cout << "=====2=====" << std::endl;
		for (auto& it: result.nodes()) {
			std::cout << xpath.getNodeContent(*it).second <<"  " << it->text() << std::endl;
		}
	}
	else
	{
		std::cout << xpath.errorString() << std::endl;
	}

	return 0;
}

/**
 * xpath查找文本.
 */
int test_xpath_text(const std::string& html)
{
	htmlcxx::XPath xpath(html);
	//完整xpath
	//auto path =  R"(/html/body/div[1]/div[2]/div[4]/div[4]/div[2]/div/ul/li[1]/a/text())";
	//xpaht
	//auto path =  R"(//*[@id="j-catalogWrap"]/div[2]/div/ul/li[1]/a/text())";
	//auto path =  R"(//*[@id="j-catalogWrap"]/div[2]/div/ul/li[1]//text())";
	//auto path =  R"(//*[@id="j-catalogWrap"]/div[2]/div/ul//text())";
	auto path =  R"(//*[@id="j-catalogWrap"]//text())";
	//auto path =  R"(//text())";
	//auto path =  R"(/text())";

	//xpath
	auto result = xpath.xpath(path);
	if (result) {
		//遍历结果
		int i = 0;
		for (auto& s : result.texts()) {
			std::cout << i++<<" : "<<s.size()<<" "<< s << std::endl;
		}
	}
	else {
		std::cout << xpath.errorString() << std::endl;
	}


	return 0;
}

int test_xpath_attr(const std::string& html)
{
	htmlcxx::XPath xpath(html);
	//完整xpath
	//auto path =  R"(/html/body/div[1]/div[2]/div[4]/div[4]/div[2]/div/ul/li[1]/a)";
	//auto path =  R"(/html/body/div[1]/div[2]/div[4]/div[4]/div[2]/div/ul)";
	//xpath
	//auto path =  R"(//*[@data-rid])";
	//auto path =  R"(//li[@data-rid="3"])";
	auto path =  R"(//a)";
	//auto path = R"(//*[@id="j-catalogWrap"]/div[2]/div/ul/li[1])";
	//找到a标签
	auto result = xpath.xpath(path);
	if (result) {
		int i = 0;
		for (auto& s : result.nodes()) {
			std::cout << i++<<" : "<< s->text() << std::endl;
		}
		//std::cout << result->text() << std::endl;
	}
	else
	{
		std::cout << xpath.errorString() << std::endl;
	}


	return 0;
}

/**
 * 获取小说所有章节以及链接.
 */
int test_xpath_fiction(const std::string& html)
{
	htmlcxx::XPath xpath(html);
	std::cout << "=================================1===================================" << std::endl;
	//完整xpath
	//auto path =  R"(/html/body/div[1]/div[2]/div[4]/div[4]/div[2]/div/ul//a)";
	//xpath
	auto path = R"(//*[@id="j-catalogWrap"]/div[2]/div/ul//a)";
	auto result = xpath.xpath(path);
	if (result) {
		int i = 0;
		for (auto& it : result.nodes()) {
			it->parseAttributes();
			std::cout << i++<<" : "<< xpath.getNodeContent(*it).second 
				<<" url: " << it->attribute("href").second
				<< std::endl;
		}
	}
	else
	{
		std::cout << xpath.errorString() << std::endl;
	}

	std::cout << "=================================2===================================" << std::endl;
	path = R"(//*[@id="j-catalogWrap"]/div[2]/div/ul)";
	result = xpath.xpath(path);
	if (result) {
		int i = 0;
		for (auto& node : result) {
			if (!node.isTag())
				continue;
			if (node.tagName() != "a")
				continue;
			node.parseAttributes();
			std::cout << i++<<" "<<node.tagName() << " : " << xpath.getNodeContent(node).second
				<<" url: " << node.attribute("href").second
				<< std::endl;
		}
	}
	else
	{
		std::cout << xpath.errorString() << std::endl;
	}
	return 0;
}
int main(int argc, char* argv[])
{
	system("chcp 65001");
	auto html = readHtml(PROJECT_PATH"/test/html/hongxiu.html");
	if (html.empty()) {
		std::cout << "html/hongxiu.html not found" << std::endl;
		return 0;
	}
	std::cout << "test_parse=====================================" << std::endl;
	test_parse(html);
	std::cout << "test_xpath=====================================" << std::endl;
	test_xpath(html);
	std::cout << "test_xpath_text=====================================" << std::endl;
	test_xpath_text(html);
	std::cout << "test_xpath_attr=====================================" << std::endl;
	test_xpath_attr(html);
	std::cout << "test_xpath_fiction=====================================" << std::endl;
	test_xpath_fiction(html);
	return 0;
}
