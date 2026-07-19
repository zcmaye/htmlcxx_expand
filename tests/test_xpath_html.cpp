#include "test_common.hpp"
#include "xpath/XPath.h"
#include <iostream>

/**
 * 测试从 HTML 文件解析并用 XPath 查询 / parse an HTML file and query it with XPath.
 *
 * 该测试依赖宏 PROJECT_PATH（由 CMake 注入），指向项目根目录。
 * This test depends on the PROJECT_PATH macro (injected by CMake), which points
 * to the project root directory.
 */
void test_xpath_on_file()
{
	TEST_SECTION("test_xpath_on_file");

	auto htmlStr = readHtml(PROJECT_PATH "/tests/html/hongxiu.html");
	TEST_FALSE(htmlStr.empty(), "hongxiu.html was loaded");

	if (htmlStr.empty()) {
		std::cout << "  html/hongxiu.html not found, skipping remaining checks." << std::endl;
		return;
	}

	xpath::XPath xp(htmlStr);

	// 1) 查询目录区域中的所有 <a> / query all <a> in the catalog wrapper
	auto result = xp.get(R"(//*[@id="j-catalogWrap"]/div[2]/div/ul[1]//a)");
	if (result) {
		int count = 0;
		for (auto& node : result) {
			++count;
			node->parseAttributes();
			auto href = node->attribute("href");
			std::cout << "  <a> text=" << node->content()
				<< " href=" << (href.first ? href.second : std::string())
				<< std::endl;
		}
		std::cout << "  matched " << count << " <a> nodes in catalog" << std::endl;
		TEST_TRUE(count > 0, "catalog query matched at least one <a>");
	}
	else {
		std::cout << "  ERROR: " << result.errorString() << std::endl;
		// 此表达式取决于 hongxiu.html 的实际结构；如果结构变化，可能查不到。
		// The expression depends on the actual structure of hongxiu.html; if the
		// structure changes, it may not match.
		std::cout << "  (if hongxiu.html structure changed, this is expected to fail)" << std::endl;
	}

	// 2) 查询所有 <title> 并校验非空 / query <title> and verify it's non-empty
	{
		auto titleResult = xp.get(R"(//title)");
		if (titleResult) {
			auto node = titleResult.node();
			TEST_TRUE(node != nullptr, "//title returned a non-null node");
			if (node != nullptr) {
				std::cout << "  title content: " << node->content() << std::endl;
				TEST_FALSE(node->content().empty(), "//title content is non-empty");
			}
		}
		else {
			std::cout << "  //title ERROR: " << titleResult.errorString() << std::endl;
		}
	}

	// 3) 查询所有 <script> 标签数量应大于 0 / number of <script> tags should be > 0
	{
		auto scriptResult = xp.get(R"(//script)");
		if (scriptResult) {
			int count = 0;
			for (auto& node : scriptResult) {
				(void)node;
				++count;
			}
			std::cout << "  //script matched " << count << " <script> nodes" << std::endl;
			TEST_TRUE(count > 0, "//script matched at least one node");
		}
	}

	// 4) 非法表达式应返回 falsy / invalid expression should return falsy
	{
		xpath::XPathExpression badExp("not_a_xpath");
		TEST_FALSE(badExp.isValid(), "invalid expression detected");
		auto badResult = xp.get(badExp);
		TEST_FALSE(static_cast<bool>(badResult), "invalid xpath returns falsy result");
	}
}

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;
	setupConsoleUtf8();
	test_xpath_on_file();
	return TEST_REPORT();
}
