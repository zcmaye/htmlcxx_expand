#include "html/ParserDom.h"
#include "test_common.hpp"
#include <iostream>

/**
 * 遍历所有节点 / iterate over all nodes.
 */
void getAllNode(const tree<htmlcxx::HTML::Node>& tree)
{
	int tagCount = 0;
	for (auto& node : tree) {
		if (node.isTag()) {
			++tagCount;
			std::cout << "  " << node.tagName() << " " << node.content() << std::endl;
		}
	}
	std::cout << "  total tag nodes: " << tagCount << std::endl;
}

/**
 * 获取指定的节点 / get nodes by tag name.
 */
int getNodeByName(const tree<htmlcxx::HTML::Node>& tree, const std::string& tag)
{
	int count = 0;
	for (auto& node : tree) {
		if (node.isTag() && node.tagName() == tag) {
			++count;
			std::cout << "  " << node.tagName() << "  " << node.text() << " " << node.content() << std::endl;
		}
	}
	std::cout << "  <" << tag << "> count: " << count << std::endl;
	return count;
}

/**
 * 获取所有有用 data-rid 属性的 li 标签，下面的 a 标签的内容和超链接
 * Collect <li> nodes that have a data-rid attribute, then print their <a> children's
 * content and href.
 */
void getFiction(const tree<htmlcxx::HTML::Node>& mtree)
{
	int liCount = 0;
	for (auto it = mtree.begin(); it != mtree.end(); ++it) {
		if (!it->isTag())
			continue;
		if (it->tagName() == "li") {
			it->parseAttributes();
			auto pr = it->attribute("data-rid");
			if (!pr.first)
				continue;
			++liCount;
			std::cout << "  li text=" << it->text() << " content=" << it->content() << std::endl;
			for (auto& node : it) {  // iterate children of *it (tree iterator's begin/end)
				if (!node.isTag())
					continue;
				node.parseAttributes();
				auto href = node.attribute("href");
				std::cout << "    child: " << node.content()
					<< " href=" << (href.first ? href.second : std::string())
					<< std::endl;
			}
		}
	}
	std::cout << "  <li data-rid> count: " << liCount << std::endl;
}

/**
 * 测试 ParserDom 基本功能 / test basic ParserDom functionality.
 */
void test_parserdom_basic(const tree<htmlcxx::HTML::Node>& tree)
{
	TEST_SECTION("test_parserdom_basic");

	// 1) 树不为空 / tree should not be empty
	int total = 0;
	for (auto& node : tree) {
		(void)node;
		++total;
	}
	std::cout << "  total nodes: " << total << std::endl;
	TEST_TRUE(total > 0, "tree contains nodes");

	// 2) 至少包含一个 <html> 标签 / should contain at least one <html> tag
	int htmlCount = 0;
	for (auto& node : tree) {
		if (node.isTag() && node.tagName() == "html") {
			++htmlCount;
		}
	}
	TEST_TRUE(htmlCount >= 1, "tree contains at least one <html> tag");

	// 3) 至少包含一个 <body> 标签 / should contain at least one <body> tag
	int bodyCount = 0;
	for (auto& node : tree) {
		if (node.isTag() && node.tagName() == "body") {
			++bodyCount;
		}
	}
	TEST_TRUE(bodyCount >= 1, "tree contains at least one <body> tag");
}

/**
 * 测试 Node 的属性解析 / test Node attribute parsing.
 */
void test_node_attributes()
{
	TEST_SECTION("test_node_attributes");

	const char* tagText = R"(<a href="https://example.com" class="link" target="_blank">click</a>)";
	htmlcxx::HTML::Node node;
	node.text(tagText);
	node.tagName("a");
	node.isTag(true);

	node.parseAttributes();
	const auto& attrs = node.attributes();
	TEST_FALSE(attrs.empty(), "parsed attributes are non-empty");

	auto href = node.attribute("href");
	TEST_TRUE(href.first, "href attribute exists");
	TEST_EQ(href.second, std::string("https://example.com"), "href value matches");

	auto cls = node.attribute("class");
	TEST_TRUE(cls.first, "class attribute exists");
	TEST_EQ(cls.second, std::string("link"), "class value matches");

	auto missing = node.attribute("nonexistent");
	TEST_FALSE(missing.first, "nonexistent attribute returns first=false");

	// 大小写不敏感：标签名比较应不区分大小写 / tag name comparison should be case-insensitive
	htmlcxx::HTML::Node upperNode;
	upperNode.tagName("DIV");
	upperNode.isTag(true);
	htmlcxx::HTML::Node lowerNode;
	lowerNode.tagName("div");
	lowerNode.isTag(true);
	TEST_TRUE(upperNode == lowerNode, "tag name comparison is case-insensitive (DIV == div)");
}

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;
	setupConsoleUtf8();

	auto htmlStr = readHtml(PROJECT_PATH "/tests/html/hongxiu.html");
	TEST_SECTION("load html file");
	TEST_FALSE(htmlStr.empty(), "hongxiu.html was loaded");
	if (htmlStr.empty()) {
		std::cout << "  html/hongxiu.html not found, aborting." << std::endl;
		return TEST_REPORT();
	}

	htmlcxx::HTML::ParserDom parser;
	auto& tree = parser.parseTree(htmlStr);

	std::cout << "===================getAllNode==================" << std::endl;
	getAllNode(tree);

	std::cout << "===================getNodeByName(a)==================" << std::endl;
	int aCount = getNodeByName(tree, "a");
	TEST_TRUE(aCount > 0, "found at least one <a> tag");

	std::cout << "===================getNodeByName(li)==================" << std::endl;
	int liCount = getNodeByName(tree, "li");
	TEST_TRUE(liCount > 0, "found at least one <li> tag");

	std::cout << "===================getFiction==================" << std::endl;
	getFiction(tree);

	// 添加结构化断言 / add structured assertions
	test_parserdom_basic(tree);
	test_node_attributes();

	return TEST_REPORT();
}
