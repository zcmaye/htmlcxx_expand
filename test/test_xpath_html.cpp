#include "test_common.hpp"
#include "xpath/XPath.h"

int main(int argc, char* argv[])
{
	system("chcp 65001");
	auto htmlStr = readHtml(PROJECT_PATH"/test/html/hongxiu.html");
	if (htmlStr.empty()) {
		std::cout << "html/hongxiu.html not found" << std::endl;
		return 0;
	}

	xpath::XPath xpath(htmlStr);
	if (auto& result = xpath.get(R"(//*[@id="j-catalogWrap"]/div[2]/div/ul[1]//a)"))
	{
		for (auto& node : result)
		{
			node->parseAttributes();
			std::cout << node->text() << " " << node->content() << " " << node->attribute("href").second << std::endl;
		}
	}
	else {
		std::cout << "ERROR:" << result.errorString() << std::endl;
	}
	return 0;
}
