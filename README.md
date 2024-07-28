# htmlcxx_expand
htmlcxx_expand是一个HTML解析器，支持xpath，可以很方便的解析HTML。

+ xpath解析部分参考了[charto/cxml](https://github.com/CodeCat-maker/cxml)，并进行了修改
+ html解析部分使用了[htmlcxx](https://htmlcxx.sourceforge.net/)

# 构建

1. 从github下载源码

```shell
https://github.com/zcmaye/htmlcxx_expand.git
```

2. 进入源码目录

```shell
cd htmlcxx_expand
```

3. 执行cmake指令

```cmake
cmake -B build												 	#默认在C:/Program Files (x86)目录中
cmake -B build -DCMAKE_INSTALL_PREFIX="C:\Users\Maye\Desktop"	#可以自己指定安装目录
```

4. 接着执行

```cmake
cmake --build build --target INSTALL						# 默认debug
cmake --build build --target INSTALL --config "Release"		# Release
```

注意：默认会安装到到C盘（C盘还必须要有权限，也就是必须用管理员打开cmd），可以自己剪切到自己专门放库的目录去,

```shell
  -- Install configuration: "Release"
  -- Installing: C:/Program Files (x86)/htmlcxx_expand/include/html/Extensions.h
  -- Installing: C:/Program Files (x86)/htmlcxx_expand/include/html/Node.h
  -- Installing: C:/Program Files (x86)/htmlcxx_expand/include/html/ParserDom.h
  -- Installing: C:/Program Files (x86)/htmlcxx_expand/include/html/ParserSax.h
  -- Installing: C:/Program Files (x86)/htmlcxx_expand/include/html/Uri.h
```

## 使用

使用非常简单，看下面代码：

```cpp
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
```

