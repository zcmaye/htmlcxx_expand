#include "html/ParserDom.h"
#include "test_common.hpp"

/**
 * 遍历所有节点.
 */
void getAllNode(const tree<htmlcxx::HTML::Node>& tree)
{
	for (auto& node : tree) {
		if (node.isTag())
			std::cout << node.tagName() << " " << node.content() << std::endl;
	}
}

/**
 * 获取指定的节点.
 */
void getNodeByName(const tree<htmlcxx::HTML::Node>& tree,const std::string& tag)
{
	for (auto& node : tree) {
		if (node.isTag())
		{
			if(node.tagName() == tag)
				std::cout << node.tagName() << "  " << node.text() << " " << node.content() << std::endl;
		}
	}
}

/**
 * 获取所有有用data-rid属性的li标签，下面的a标签的内容和超链接
 */
void getFiction(const tree<htmlcxx::HTML::Node>& mtree)
{
	auto fit = mtree.begin();
	for(auto it = mtree.begin();it!=mtree.end();it++)
	{
		if (!it->isTag())
			continue;
		if (it->tagName() == "li") {
			it->parseAttributes();
			auto pr = it->attribute("data-rid");
			if (!pr.first)
				continue;
			std::cout << it->text() << " " << it->content() << " ";
			for (auto& node: it)
			{
				node.parseAttributes();
				auto pr = node.attribute("href");
				std::cout << node.content()<<" "<<pr.second << std::endl;
			}
		}
	}
}

int main(int argc, char* argv[])
{
	system("chcp 65001");
	auto htmlStr = readHtml(PROJECT_PATH"/test/html/hongxiu.html");
	if (htmlStr.empty()) {
		std::cout << "html/hongxiu.html not found" << std::endl;
		return 0;
	}

	htmlcxx::HTML::ParserDom parser;
	auto& tree = parser.parseTree(htmlStr);
	std::cout << "===================getAllNode==================" << std::endl;
	getAllNode(tree);
	std::cout << "===================getNodeByName(a)==================" << std::endl;
	getNodeByName(tree, "a");
	std::cout << "===================getNodeByName(li)==================" << std::endl;
	getNodeByName(tree, "li");
	std::cout << "===================getFiction==================" << std::endl;
	getFiction(tree);
	return 0;
}