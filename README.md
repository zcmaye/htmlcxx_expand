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

使用非常简单，请查看`test/test_xapth.cpp`，部分代码如下

```cpp
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
```

