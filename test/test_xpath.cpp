#include "xpath/XPath.h"
#include "html/ParserDom.h"
#include "html/maye_define.h"
#include <fstream>
#include <iostream>

const char* htmlStr = R"(
<!DOCTYPE html>
<!-- 这是测试XPATH的HTML -->
<html>
	<head>
		<title>maye htmlcxx_expand 玩蛇蛇</title>
		<script>var _hmt1 = "script one"</script>
		<script>var _hmt2 = "script two"</script>
		<script>var _hmt3 = "script three"</script>
		<script>var _hmt4 = "script four"</script>
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
                          <dd><a href="/category/30055_f1_f1_f1_f1_f1_0_1"><em class="iconfont">&#xe904;</em>>轻小说</a></dd>
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

/**
 * 读取html文件，跳过空行能大幅度缩减保存的html数据.
 */
std::string readHtml(const std::string& htmlFile)
{
	std::ifstream infile(htmlFile, std::ios::binary);
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
		buf.append(p, line.end() - 1);
	}
	return buf;
}

/**
 * 获取所有a标签,并输出内容和href属性
 */
void getAllA(const std::string& htmlStr)
{
	htmlcxx::XPath xpath(htmlStr);

	auto resullt = xpath.xpath(R"(//a)");
	if (resullt) {
		for (auto& node: resullt) {
			node->parseAttributes();
			auto pr = node->attribute("href");
			std::cout <<"tagName:"
				<< node->tagName() << " content:"
				<< node->content() << " href:"
				<< (pr.first ? pr.second : "") << std::endl;
		}
	}
	else {
		std::cout << xpath.errorString() << std::endl;
	}
}

/**
 * 	获取标题
 */
void getHtmlTitle(const std::string& html)
{
	htmlcxx::XPath xpath(html);

	auto resullt = xpath.xpath(R"(/html/head/title)");
	if (resullt) {
		auto node_it = resullt.node();
		std::cout << node_it->tagName() << " : " << node_it->content() << std::endl;
	}
	else {
		std::cout << xpath.errorString() << std::endl;
	}

}

/**
 * 	获取head中第二个script节点
 */
void getScript2(const std::string& html)
{
	htmlcxx::XPath xpath(html);

	auto resullt = xpath.xpath(R"(/html/head/script[2])");
	if (resullt) {
		auto node_it = resullt.node();
		std::cout << node_it->tagName() << "   " << node_it->content() << std::endl;
	}
	else {
		std::cout << xpath.errorString() << std::endl;
	}

}

/**
 * 	获id为j-typeList，标签为div的所有a子节点
 */
void getNodeFromGeneraByName(const std::string& html)
{
	htmlcxx::XPath xpath(html);

	auto resullt = xpath.xpath(R"(//div[@id="j-typeList"]//a)");
	//auto resullt = xpath.xpath(R"(/html/body/div/ul/li/div/dl//a)");
	if (resullt) {
		for (auto& node: resullt) {
			node->parseAttributes();
			auto pr = node->attribute("href");
			std::cout <<"tagName:"
				<< node->tagName() << " content:"
				<< node->content() << " href:"
				<< (pr.first ? pr.second : "") << std::endl;
		}
	}
	else {
		std::cout << xpath.errorString() << std::endl;
	}
}
/**
 * 获取标签文本.
 */
void getAllATagTexts(const std::string& html)
{
	htmlcxx::XPath xpath(html);

	//auto resullt = xpath.xpath(R"(//a/text())");
	auto resullt = xpath.xpath(R"(/html/head/script/text())");
	if (resullt) {
		for (auto& text : resullt.texts()) {
			std::cout << text << std::endl;
		}
	}
	else {
		std::cout << xpath.errorString() << std::endl;
	}
}

int main(int argc, char* argv[])
{
	std::cout << "===================getAllA==================" << std::endl;
	getAllA(htmlStr);
	std::cout << "===================getHtmlTitle==================" << std::endl;
	getHtmlTitle(htmlStr);
	std::cout << "===================getScript2==================" << std::endl;
	getScript2(htmlStr);
	std::cout << "==================getNodeFromGeneraByName===================" << std::endl;
	getNodeFromGeneraByName(htmlStr);
	std::cout << "==================getAllATagTexts===================" << std::endl;
	getAllATagTexts(htmlStr);
	return 0;
}
