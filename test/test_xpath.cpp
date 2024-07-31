#include "xpath/XPath.h"
#include <iostream>

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


void test_xpath_expression()
{
	using namespace xpath;
	const char* exps[] =
	{
		R"(//)",
		R"(/html/body//a)",
		R"(//div/dd[21])",
		R"(//*[@id="wrap"]/dd)",
		R"(hello)",
	};
	for (int i = 0; i < sizeof(exps) / sizeof(exps[0]); i++)
	{
		std::cout << "===== " << i << " " << exps[i] << " =====" << std::endl;
		XPathExpression exp(exps[i]);
		if (!exp.isValid()) {
			std::cout << "exp invalid: " << exp.errorString() << std::endl;
		}
		else {
			for (auto& opt : exp.options()) {
				std::cout << " --- " << *opt.first << " " << opt.second << std::endl;
			}
		}
		std::cout << std::endl;
	}

}

void test_xpath()
{
	using namespace xpath;

	XPathExpression exprs[] = {
		R"(//*)",					/*!> 获取所有的元素 */
		R"(//a)",					/*!> 获取所有的a元素 */
		R"(//a[@href])",			/*!> 获取所有带href属性的a元素*/
		R"(//*[@href])",			/*!> 获取所有带href属性的所有元素*/
		R"(/html/head/script[@*])",	/*!> 获取/html/head下，所有带有属性的script元素*/
		R"(/html/head/script[3])",	/*!> 获取/html/head下第三个scritp元素(从1开始)*/
		R"(/html/head/script[3]/@attr)",	/*!> 获取/html/head下第三个scritp元素的attr属性值*/
		R"(/html/head/script[3]/text())",	/*!> 获取/html/head下第三个scritp元素的内容*/
		R"(/html/head/title/text())",		/*!> 获取/html/head/title的内容*/
		R"(/html/head/div[@attr="attr"]/text())",	/*!> 获取/html/head 子元素中attr属性为attr的div的内容*/
		R"(/html/head/div[@attr="attr"]//text())"	/*!> 获取/html/head 子元素中attr属性为attr的div的所有子元素的内容*/
	};

	XPath xpath(htmlStr);
	for (auto& exp : exprs)
	{
		std::cout << "[[[[[[[[[[[[[[[[[[[[[[[" + exp.string() + "]]]]]]]]]]]]]]]]]]]]]]]]]]]" << std::endl;
		auto resullt = xpath.get(exp);
		if (resullt) {

			std::cout << "==========node===========" << std::endl;
			for (auto& node : resullt) {
				node->parseAttributes();
				auto pr = node->attribute("href");
				std::cout << "tagName:"
					<< node->tagName() << " content:"
					<< node->content() << " href:"
					<< (pr.first ? pr.second : "") << std::endl;
			}
			std::cout << "==========text===========" << std::endl;
			for (auto& text : resullt.texts()) {
				std::cout << " --- " << text << std::endl;
			}
			std::cout << std::endl;
		}
		else {
			std::cout << "ERROR: " << resullt.errorString() << std::endl;
		}
	}
}
int main(int argc, char* argv[])
{
	std::cout << "\n############################## test_xpath_expression() ###############################" << std::endl;
	test_xpath_expression();
	std::cout << "\n############################## test_xpath() ###############################" << std::endl;
	test_xpath();
	return 0;
}
