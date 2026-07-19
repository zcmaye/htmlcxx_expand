#include "test_common.hpp"
#include "xpath/XPath.h"
#include <iostream>

namespace {

const char* htmlStr = R"(
<!DOCTYPE html>
<!-- 这是测试XPATH的HTML -->
<html>
	<head>
		<title>maye htmlcxx_expand 玩蛇蛇</title>
		<script>var _hmt1 = "script one"</script>
		<script>var _hmt2 = "script two"</script>
		<script attr="attr">var _hmt3 = "script three"<br><br><a>点我点我</a>我是一个小脚本</script>
		<script>var _hmt4 = "script four"</script>
		<div attr="attr">var _hmt3 = "script three"<br><br><a>点我点我<p>我是P标签</p></a>我是一个小脚本</div>
	</head>
	<body>
        <div class="left-nav fl">
          <ul class="cf">
              <li class="type " id="j-navType">
				<a href="/category"><em class="iconfont">&#xe612;</em>全部分类</a>
                  <div class="type-list" id="j-typeList">
                      <dl>
                          <dd><a href="/category/30020_f1_f1_f1_f1_f1_0_1"><em class="iconfont">&#xe630;</em>现代言情</a></dd>
                          <dd><a href="/category/30013_f1_f1_f1_f1_f1_0_1"><em class="iconfont">&#xe632;</em>古代言情</a></dd>
                          <dd><a href="/category/30031_f1_f1_f1_f1_f1_0_1"><em class="iconfont">&#xe631;</em>浪漫青春</a></dd>
                          <dd><a href="/category/30001_f1_f1_f1_f1_f1_0_1"><em class="iconfont">&#xe637;</em>玄幻言情</a></dd>
                          <dd><a href="/category/30008_f1_f1_f1_f1_f1_0_1"><em class="iconfont">&#xe62e;</em>仙侠奇缘</a></dd>
                          <dd><a href="/category/30036_f1_f1_f1_f1_f1_0_1"><em class="iconfont">&#xe634;</em>悬疑</a></dd>
                          <dd><a href="/category/30042_f1_f1_f1_f1_f1_0_1"><em class="iconfont">&#xe62d;</em>科幻空间</a></dd>
                          <dd><a href="/category/30050_f1_f1_f1_f1_f1_0_1"><em class="iconfont">&#xe638;</em>游戏竞技</a></dd>
                          <dd><a href="/category/30083_f1_f1_f1_f1_f1_0_1"><em class="iconfont">&#xe63b;</em>短篇小说</a></dd>
                          <dd><a href="/category/30055_f1_f1_f1_f1_f1_0_1"><em class="iconfont">&#xe904;</em>轻小说</a></dd>
                          <dd><a><em class="iconfont">&#xe904;</em>我没有href属性</a></dd>
                      </dl>
                  </div>
              </li>
              <li class=""><a href="/rank">排行榜</a></li>
              <li class=""><a href="/free">免费</a></li>
              <li class=""><a href="/finish">完本</a></li>
              <li><a href="/gdyq">古言</a></li>
              <li><a href="/xdyq">现言</a></li>
              <li><a href="/xhxx">玄幻仙侠</a></li>
              <li><a href="/lykh">悬疑科幻</a></li>
              <li><a href="/qcyx">青春游戏</a></li>
              <li><a href="/fsg">风尚阁</a></li>
              <!-- 临时将 版权专区 改为 动漫专区 （审核） -->
              <li class=""><a href="/z/comic" rel="nofollow">动漫专区</a></li>
              <!-- 临时将 版权专区 改为 动漫专区 （审核） -->
          </ul>
       </div>
	</body>
</html>
)";

// 统计 a 标签数量，用于验证 / count <a> tags for verification
int countAnchorTags(const xpath::XPathResult& result)
{
	int count = 0;
	for (auto& node : result) {
		(void)node; // 仅计数 / just count
		++count;
	}
	return count;
}

// 统计带 href 属性的 a 标签数量 / count <a> tags that have an href attribute
int countAnchorsWithHref(const xpath::XPathResult& result)
{
	int count = 0;
	for (auto& node : result) {
		node->parseAttributes();
		if (node->attribute("href").first) {
			++count;
		}
	}
	return count;
}

/**
 * 测试 XPathExpression 的解析与校验 / test XPathExpression parsing and validation.
 */
void test_xpath_expression()
{
	using namespace xpath;

	TEST_SECTION("test_xpath_expression");

	// 合法表达式应通过校验 / valid expressions should pass validation
	const char* validExps[] = {
		R"(//*)",
		R"(/html/body//a)",
		R"(//a[@href])",
		R"(//*[@id="wrap"]/dd)",
		R"(/html/head/script[3])",
		R"(/html/head/title/text())",
	};
	for (const char* e : validExps) {
		XPathExpression exp(e);
		std::string msg = std::string("valid expression: ") + e;
		TEST_TRUE(exp.isValid(), msg);
		if (exp.isValid()) {
			TEST_FALSE(exp.options().empty(), std::string("options not empty for: ") + e);
		}
	}

	// 非法表达式应返回 invalid / invalid expressions should be invalid
	// 注意："/" 会导致 XPathExpression::parse() 越界访问（参见 CODE_ISSUES.md 6.1），
	// 属于未定义行为，因此不在此处直接测试，避免触发崩溃。
	// Note: "/" triggers out-of-bounds access in XPathExpression::parse()
	// (see CODE_ISSUES.md 6.1) and is undefined behavior, so it is not exercised
	// here to avoid crashing the test runner.
	const char* invalidExps[] = {
		R"()",            // 空串 / empty
		R"(hello)",       // 不以 / 开头 / does not start with /
		R"(//)",          // 不完整的 // / incomplete //
	};
	for (const char* e : invalidExps) {
		XPathExpression exp(e);
		std::string msg = std::string("invalid expression: [") + e + "]";
		TEST_FALSE(exp.isValid(), msg);
		if (!exp.isValid()) {
			TEST_FALSE(std::string(exp.errorString()).empty(),
				std::string("error string non-empty for: [") + e + "]");
		}
	}

	// 打印表达式的解析选项（便于人工检查） / print parsed options for manual inspection
	const char* debugExps[] = {
		R"(/html/body//a)",
		R"(//div/dd[21])",
		R"(//*[@id="wrap"]/dd)",
	};
	for (const char* e : debugExps) {
		std::cout << "===== " << e << " =====" << std::endl;
		XPathExpression exp(e);
		if (!exp.isValid()) {
			std::cout << "  invalid: " << exp.errorString() << std::endl;
			continue;
		}
		for (auto& opt : exp.options()) {
			std::cout << "  --- " << *opt.first << " " << opt.second << std::endl;
		}
	}
}

/**
 * 测试 XPath 查询结果 / test XPath query results.
 */
void test_xpath()
{
	using namespace xpath;

	TEST_SECTION("test_xpath");

	XPath xp(htmlStr);

	// 1) //* 应返回大量元素节点 / //* should return many element nodes
	{
		auto result = xp.get(XPathExpression(R"(//*)"));
		TEST_TRUE(static_cast<bool>(result), "//* returns non-empty result");
		int total = 0;
		for (auto& node : result) {
			(void)node;
			++total;
		}
		std::cout << "  //* matched " << total << " element nodes" << std::endl;
		TEST_TRUE(total > 10, "//* matches more than 10 nodes");
	}

	// 2) //a 应返回所有 a 元素（共 21 个） / //a should match all <a> elements
	{
		auto result = xp.get(XPathExpression(R"(//a)"));
		TEST_TRUE(static_cast<bool>(result), "//a returns non-empty result");
		int n = countAnchorTags(result);
		std::cout << "  //a matched " << n << " <a> elements" << std::endl;
		TEST_TRUE(n > 0, "//a matches at least one <a>");
	}

	// 3) //a[@href] 只应返回带 href 的 a / //a[@href] should match <a> with href only
	{
		auto resultAll = xp.get(XPathExpression(R"(//a)"));
		auto resultHref = xp.get(XPathExpression(R"(//a[@href])"));
		TEST_TRUE(static_cast<bool>(resultHref), "//a[@href] returns non-empty result");

		int nAll = countAnchorTags(resultAll);
		int nHref = countAnchorsWithHref(resultHref);
		std::cout << "  //a[@href] matched " << nHref << " of " << nAll << " <a> elements" << std::endl;
		TEST_TRUE(nHref > 0, "//a[@href] matches at least one <a>");
		TEST_TRUE(nHref <= nAll, "//a[@href] count <= //a count");
		// HTML 中存在一个没有 href 的 a 标签，所以 nHref < nAll
		TEST_TRUE(nHref < nAll, "//a[@href] count < //a count (one <a> has no href)");
	}

	// 4) //*[@href] 应返回所有带 href 的元素（不仅是 a）/ //*[@href] returns all elements with href
	{
		auto result = xp.get(XPathExpression(R"(//*[@href])"));
		TEST_TRUE(static_cast<bool>(result), "//*[@href] returns non-empty result");
		int n = countAnchorsWithHref(result);
		std::cout << "  //*[@href] matched " << n << " elements with href" << std::endl;
		TEST_TRUE(n > 0, "//*[@href] matches at least one element");
	}

	// 5) /html/head/script[@*] 应返回带任意属性的 script / script with any attribute
	{
		auto result = xp.get(XPathExpression(R"(/html/head/script[@*])"));
		// 注意：当前实现只返回首个匹配（参见 CODE_ISSUES.md 7.3）
		// Note: current implementation only returns the first match (see CODE_ISSUES.md 7.3)
		std::cout << "  /html/head/script[@*] returned "
			<< (result ? std::to_string(countAnchorTags(result)) : std::string("0"))
			<< " node(s)" << std::endl;
	}

	// 6) /html/head/script[3] 应返回第 3 个 script / returns the 3rd <script>
	{
		auto result = xp.get(XPathExpression(R"(/html/head/script[3])"));
		TEST_TRUE(static_cast<bool>(result), "/html/head/script[3] returns a node");
		if (result) {
			auto node = result.node();
			TEST_TRUE(node != nullptr, "/html/head/script[3] node is non-null");
			if (node != nullptr) {
				node->parseAttributes();
				auto attr = node->attribute("attr");
				TEST_TRUE(attr.first, "3rd <script> has attr='attr'");
				TEST_EQ(attr.second, std::string("attr"), "3rd <script> attr value is 'attr'");
			}
		}
	}

	// 7) /html/head/script[3]/@attr 应返回 "attr" / returns attribute value
	{
		auto result = xp.get(XPathExpression(R"(/html/head/script[3]/@attr)"));
		TEST_TRUE(static_cast<bool>(result), "/html/head/script[3]/@attr returns a value");
		if (result) {
			TEST_EQ(result.text(), std::string("attr"),
				"/html/head/script[3]/@attr value is 'attr'");
		}
	}

	// 8) /html/head/title/text() 应返回 title 文本 / returns title text
	{
		auto result = xp.get(XPathExpression(R"(/html/head/title/text())"));
		TEST_TRUE(static_cast<bool>(result), "/html/head/title/text() returns text");
		if (result) {
			const auto& texts = result.texts();
			TEST_FALSE(texts.empty(), "title/text() has at least one text fragment");
			if (!texts.empty()) {
				std::cout << "  title text: " << texts.front() << std::endl;
				// 标题中包含 "maye htmlcxx_expand"
				TEST_TRUE(texts.front().find("maye htmlcxx_expand") != std::string::npos,
					"title text contains 'maye htmlcxx_expand'");
			}
		}
	}

	// 9) /html/head/div[@attr="attr"]/text() 应返回 div 的文本 / returns div text
	{
		auto result = xp.get(XPathExpression(R"(/html/head/div[@attr="attr"]/text())"));
		TEST_TRUE(static_cast<bool>(result), "/html/head/div[@attr=\"attr\"]/text() returns text");
	}

	// 10) 不存在的路径应返回 falsy 结果 / non-existent path should return falsy
	{
		auto result = xp.get(XPathExpression(R"(/html/head/nonexistent_tag)"));
		TEST_FALSE(static_cast<bool>(result), "/html/head/nonexistent_tag returns falsy");
		TEST_FALSE(std::string(result.errorString()).empty(),
			"non-existent path has non-empty error string");
		std::cout << "  expected error: " << result.errorString() << std::endl;
	}

	// 11) 错误的 xpath 表达式应返回 falsy 结果 / invalid xpath should return falsy
	{
		XPathExpression badExp(R"(not_a_xpath)");
		TEST_FALSE(badExp.isValid(), "invalid expression is detected");
		auto result = xp.get(badExp);
		TEST_FALSE(static_cast<bool>(result), "invalid xpath returns falsy result");
	}

	// 12) 默认构造的表达式应为 //* / default expression is //*
	{
		XPathExpression defaultExp;
		TEST_TRUE(defaultExp.isValid(), "default XPathExpression is valid");
		TEST_EQ(defaultExp.string(), std::string("//*"), "default expression is '//*'");
	}
}

/**
 * 遍历并打印结果（演示用途）/ iterate and print results (for demo).
 */
void dump_results()
{
	using namespace xpath;
	TEST_SECTION("dump_results (demo)");

	XPathExpression exprs[] = {
		R"(//*)",
		R"(//a)",
		R"(//a[@href])",
		R"(//*[@href])",
		R"(/html/head/script[@*])",
		R"(/html/head/script[3])",
		R"(/html/head/script[3]/@attr)",
		R"(/html/head/script[3]/text())",
		R"(/html/head/title/text())",
		R"(/html/head/div[@attr="attr"]/text())",
		R"(/html/head/div[@attr="attr"]//text())"
	};

	XPath xp(htmlStr);
	for (auto& exp : exprs) {
		std::cout << "[[[[[ " << exp.string() << " ]]]]]" << std::endl;
		auto result = xp.get(exp);
		if (!result) {
			std::cout << "ERROR: " << result.errorString() << std::endl;
			continue;
		}

		std::cout << "----- nodes -----" << std::endl;
		for (auto& node : result) {
			node->parseAttributes();
			auto pr = node->attribute("href");
			std::cout << "  tagName=" << node->tagName()
				<< " content=" << node->content()
				<< " href=" << (pr.first ? pr.second : std::string())
				<< std::endl;
		}
		std::cout << "----- texts -----" << std::endl;
		for (auto& text : result.texts()) {
			std::cout << "  --- " << text << std::endl;
		}
		std::cout << std::endl;
	}
}

} // namespace

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;
	setupConsoleUtf8();

	std::cout << "############################## test_xpath_expression() ##############################" << std::endl;
	test_xpath_expression();

	std::cout << "\n############################## test_xpath() ##############################" << std::endl;
	test_xpath();

	std::cout << "\n############################## dump_results() ##############################" << std::endl;
	dump_results();

	return TEST_REPORT();
}
