# htmlcxx_expand

[中文](README_zh.md) | [English](README.md)

`htmlcxx_expand` 是一个支持 XPath 的 HTML 解析器，可以方便地从 HTML 文档中提取数据。

- XPath 解析部分参考了 [charto/cxml](https://github.com/CodeCat-maker/cxml) 并进行了修改。
- HTML 解析部分使用了 [htmlcxx](https://htmlcxx.sourceforge.net/)。

## 功能特性

- 将 HTML 解析为 DOM 树。
- 通过 XPath 子集对 DOM 树进行查询。
- 提取元素节点、属性值和文本内容。
- 轻量级：无第三方运行时依赖，仅需 C++17。

### 支持的 XPath 子集

| 表达式 | 说明 |
| --- | --- |
| `//*` | 匹配所有元素。 |
| `//name` | 匹配名为 `name` 的所有后代元素。 |
| `/name` | 匹配名为 `name` 的直接子元素。 |
| `/name[n]` | 匹配第 n 个名为 `name` 的直接子元素（从 1 开始）。 |
| `/name[@attr]` | 匹配带有 `attr` 属性的元素。 |
| `/name[@attr=value]` | 匹配 `attr` 属性值为 `value` 的元素。 |
| `/text()` | 返回当前元素的文本内容。 |
| `//text()` | 返回当前元素及其所有后代的文本内容。 |
| `/@attr` | 返回当前元素的 `attr` 属性值。 |
| `/*` | 匹配任意直接子元素。 |

> 注意：选项表中虽然标注支持 `..`（父节点）和 `.`（当前节点），但实际未接通，详见 [CODE_ISSUES.md](CODE_ISSUES.md) §6.3。

## 构建

1. 从 GitHub 下载源码：

```shell
git clone https://github.com/zcmaye/htmlcxx_expand.git
```

2. 进入源码目录：

```shell
cd htmlcxx_expand
```

3. 执行 CMake 配置：

```cmake
cmake -B build                                                    # 默认安装目录（Windows 下为 C:/Program Files (x86)）
cmake -B build -DCMAKE_INSTALL_PREFIX="C:\Users\Maye\Desktop"     # 自定义安装目录
```

4. 构建并安装：

```cmake
cmake --build build --target INSTALL                       # 默认 Debug
cmake --build build --target INSTALL --config "Release"    # Release
```

> 注意：Windows 下默认会安装到 `C:/Program Files (x86)`，需要管理员权限。请以管理员身份打开命令提示符，或指定不同的 `CMAKE_INSTALL_PREFIX`。

安装输出示例：

```
-- Install configuration: "Release"
-- Installing: C:/Program Files (x86)/htmlcxx_expand/include/html/Extensions.h
-- Installing: C:/Program Files (x86)/htmlcxx_expand/include/html/Node.h
-- Installing: C:/Program Files (x86)/htmlcxx_expand/include/html/ParserDom.h
-- Installing: C:/Program Files (x86)/htmlcxx_expand/include/html/ParserSax.h
-- Installing: C:/Program Files (x86)/htmlcxx_expand/include/html/Uri.h
```

### 构建选项

| 选项 | 默认值 | 说明 |
| --- | --- | --- |
| `BUILD_TESTS` | `ON` | 是否构建测试可执行文件（`test_xpath`、`test_xpath_html`、`test_parserDom`）。 |

关闭测试构建：

```cmake
cmake -B build -DBUILD_TESTS=OFF
```

## 使用方法

典型用法（完整代码见 [tests/test_xpath.cpp](tests/test_xpath.cpp)）：

```cpp
#include "xpath/XPath.h"
#include <iostream>

using namespace xpath;

XPathExpression exprs[] = {
    R"(//*)",                              // 获取所有的元素
    R"(//a)",                              // 获取所有的 a 元素
    R"(//a[@href])",                       // 获取所有带 href 属性的 a 元素
    R"(//*[@href])",                       // 获取所有带 href 属性的元素
    R"(/html/head/script[@*])",            // 获取 /html/head 下所有带属性的 script 元素
    R"(/html/head/script[3])",             // 获取 /html/head 下第 3 个 script 元素（从 1 开始）
    R"(/html/head/script[3]/@attr)",       // 获取 /html/head 下第 3 个 script 元素的 attr 属性值
    R"(/html/head/script[3]/text())",      // 获取 /html/head 下第 3 个 script 元素的文本内容
    R"(/html/head/title/text())",          // 获取 /html/head/title 的文本内容
    R"(/html/head/div[@attr="attr"]/text())",     // 获取 /html/head 下 attr="attr" 的 div 的文本
    R"(/html/head/div[@attr="attr"]//text())"     // 获取 /html/head 下 attr="attr" 的 div 及其后代的文本
};

XPath xpath(htmlStr);
for (auto& exp : exprs) {
    std::cout << "[[ " << exp.string() << " ]]" << std::endl;
    auto result = xpath.get(exp);
    if (result) {
        std::cout << "----- 节点 -----" << std::endl;
        for (auto& node : result) {
            node->parseAttributes();
            auto pr = node->attribute("href");
            std::cout << "tagName=" << node->tagName()
                << " content=" << node->content()
                << " href=" << (pr.first ? pr.second : std::string())
                << std::endl;
        }
        std::cout << "----- 文本 -----" << std::endl;
        for (auto& text : result.texts()) {
            std::cout << "  --- " << text << std::endl;
        }
    } else {
        std::cout << "ERROR: " << result.errorString() << std::endl;
    }
}
```

## 测试

项目提供三个测试可执行文件（`BUILD_TESTS=ON` 时构建）：

| 目标 | 源文件 | 说明 |
| --- | --- | --- |
| `test_xpath` | [tests/test_xpath.cpp](tests/test_xpath.cpp) | 校验 XPath 表达式合法性，并对内嵌 HTML 字符串进行端到端查询，带断言。 |
| `test_xpath_html` | [tests/test_xpath_html.cpp](tests/test_xpath_html.cpp) | 解析 `tests/html/hongxiu.html` 并执行 XPath 查询。 |
| `test_parserDom` | [tests/test_parserDom.cpp](tests/test_parserDom.cpp) | `ParserDom` 树遍历、属性解析、标签名比较。 |

每个测试在结尾会输出汇总信息：

```
----------------------------------------
Assertions: 38 | Failures: 0
----------------------------------------
```

退出码非 0 表示至少有一条断言失败。

## 项目结构

```
htmlcxx_expand/
├── src/
│   ├── html/        # HTML 解析器（Node、ParserDom、ParserSax、Uri、Extensions、tree）
│   └── xpath/       # XPath 引擎（XPath、XPathExpression、XPathOption、XPathResult）
├── tests/
│   ├── html/hongxiu.html
│   ├── test_common.hpp
│   ├── test_xpath.cpp
│   ├── test_xpath_html.cpp
│   └── test_parserDom.cpp
├── CMakeLists.txt
├── CODE_ISSUES.md   # 静态代码审查记录（已知问题详见此文件）
├── README.md        # 英文（默认）
└── README_zh.md     # 中文
```

## 已知问题

静态代码审查结果记录在 [CODE_ISSUES.md](CODE_ISSUES.md) 中，要点如下：

- `XPathResult::text()` 在没有文本时返回悬空引用。
- `XPathExpression::parse()` 对输入 `"/"` 可能越界访问。
- `Node` 默认构造函数未初始化标量成员。
- `maye_define.h` 通过宏覆盖了标准库函数名（`isspace`、`isalnum`、`isalpha`）。
- `Extensions::check` 解引用可能为空的指针。
- `test_common.hpp` 中的 `readHtml` 之前会丢掉每行的最后一个字符。

完整列表与修复建议见该文件。

## 许可证

详见 [LICENSE](LICENSE)。
