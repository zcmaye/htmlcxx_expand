# htmlcxx_expand

[English](README.md) | [中文](README_zh.md)

`htmlcxx_expand` is an HTML parser with XPath support, making it easy to extract data from HTML documents.

- The XPath parsing part is inspired by [charto/cxml](https://github.com/CodeCat-maker/cxml) and adapted for this project.
- The HTML parsing part is built on top of [htmlcxx](https://htmlcxx.sourceforge.net/).

## Features

- Parse HTML into a DOM tree.
- Query the DOM tree with a subset of XPath expressions.
- Extract element nodes, attribute values, and text content.
- Lightweight: no third-party runtime dependencies, C++17 only.

### Supported XPath subset

| Expression | Description |
| --- | --- |
| `//*` | Match all elements. |
| `//name` | Match all descendants named `name`. |
| `/name` | Match direct children named `name`. |
| `/name[n]` | Match the n-th direct child named `name` (1-based). |
| `/name[@attr]` | Match elements that have attribute `attr`. |
| `/name[@attr=value]` | Match elements whose `attr` equals `value`. |
| `/text()` | Return the text content of the current element. |
| `//text()` | Return the text content of the current element and all descendants. |
| `/@attr` | Return the value of attribute `attr` on the current element. |
| `/*` | Match any direct child. |

> Note: `..` (parent) and `.` (current) are advertised in the option table but are not actually wired up - see [CODE_ISSUES.md](CODE_ISSUES.md) §6.3.

## Build

1. Clone the source code:

```shell
git clone https://github.com/zcmaye/htmlcxx_expand.git
```

2. Enter the source directory:

```shell
cd htmlcxx_expand
```

3. Run CMake to configure:

```cmake
cmake -B build                                                    # Default install prefix (e.g. C:/Program Files (x86) on Windows)
cmake -B build -DCMAKE_INSTALL_PREFIX="C:/Users/Maye/Desktop"     # Custom install prefix
```

4. Build and install:

```cmake
cmake --build build --target INSTALL                       # Debug by default
cmake --build build --target INSTALL --config "Release"    # Release
```

> Note: The default install prefix on Windows is `C:/Program Files (x86)`, which requires administrator privileges. Run the command prompt as Administrator, or specify a different `CMAKE_INSTALL_PREFIX`.

Sample install output:

```
-- Install configuration: "Release"
-- Installing: C:/Program Files (x86)/htmlcxx_expand/include/html/Extensions.h
-- Installing: C:/Program Files (x86)/htmlcxx_expand/include/html/Node.h
-- Installing: C:/Program Files (x86)/htmlcxx_expand/include/html/ParserDom.h
-- Installing: C:/Program Files (x86)/htmlcxx_expand/include/html/ParserSax.h
-- Installing: C:/Program Files (x86)/htmlcxx_expand/include/html/Uri.h
```

### Build options

| Option | Default | Description |
| --- | --- | --- |
| `BUILD_TESTS` | `ON` | Build the test executables (`test_xpath`, `test_xpath_html`, `test_parserDom`). |

To disable tests:

```cmake
cmake -B build -DBUILD_TESTS=OFF
```

## Usage

A typical usage example (see [tests/test_xpath.cpp](tests/test_xpath.cpp) for the full version):

```cpp
#include "xpath/XPath.h"
#include <iostream>

using namespace xpath;

XPathExpression exprs[] = {
    R"(//*)",                              // all elements
    R"(//a)",                              // all <a> elements
    R"(//a[@href])",                       // <a> elements that have href
    R"(//*[@href])",                       // any element with href
    R"(/html/head/script[@*])",            // <script> under <html>/<head> with any attribute
    R"(/html/head/script[3])",             // the 3rd <script> under <html>/<head> (1-based)
    R"(/html/head/script[3]/@attr)",       // value of attr on the 3rd <script>
    R"(/html/head/script[3]/text())",      // text content of the 3rd <script>
    R"(/html/head/title/text())",          // text content of <title>
    R"(/html/head/div[@attr="attr"]/text())",     // text of <div attr="attr">
    R"(/html/head/div[@attr="attr"]//text())"     // text of <div attr="attr"> and all descendants
};

XPath xpath(htmlStr);
for (auto& exp : exprs) {
    std::cout << "[[ " << exp.string() << " ]]" << std::endl;
    auto result = xpath.get(exp);
    if (result) {
        std::cout << "----- nodes -----" << std::endl;
        for (auto& node : result) {
            node->parseAttributes();
            auto pr = node->attribute("href");
            std::cout << "tagName=" << node->tagName()
                << " content=" << node->content()
                << " href=" << (pr.first ? pr.second : std::string())
                << std::endl;
        }
        std::cout << "----- texts -----" << std::endl;
        for (auto& text : result.texts()) {
            std::cout << "  --- " << text << std::endl;
        }
    } else {
        std::cout << "ERROR: " << result.errorString() << std::endl;
    }
}
```

## Tests

The project ships three test executables (built when `BUILD_TESTS=ON`):

| Target | Source | Description |
| --- | --- | --- |
| `test_xpath` | [tests/test_xpath.cpp](tests/test_xpath.cpp) | XPath expression validation and end-to-end queries against an inline HTML string, with assertions. |
| `test_xpath_html` | [tests/test_xpath_html.cpp](tests/test_xpath_html.cpp) | Parse `tests/html/hongxiu.html` and run XPath queries on it. |
| `test_parserDom` | [tests/test_parserDom.cpp](tests/test_parserDom.cpp) | `ParserDom` tree traversal, attribute parsing, and tag-name comparison. |

Each test prints a summary at the end:

```
----------------------------------------
Assertions: 38 | Failures: 0
----------------------------------------
```

A non-zero exit code indicates that at least one assertion failed.

## Project layout

```
htmlcxx_expand/
├── src/
│   ├── html/        # HTML parser (Node, ParserDom, ParserSax, Uri, Extensions, tree)
│   └── xpath/       # XPath engine (XPath, XPathExpression, XPathOption, XPathResult)
├── tests/
│   ├── html/hongxiu.html
│   ├── test_common.hpp
│   ├── test_xpath.cpp
│   ├── test_xpath_html.cpp
│   └── test_parserDom.cpp
├── CMakeLists.txt
├── CODE_ISSUES.md   # Static review findings (see this file for known issues)
├── README.md        # English (default)
└── README_zh.md     # Chinese
```

## Known issues

A static code review is captured in [CODE_ISSUES.md](CODE_ISSUES.md). Highlights:

- `XPathResult::text()` returns a dangling reference when there are no texts.
- `XPathExpression::parse()` may read out of bounds for the input `"/"`.
- `Node` does not initialize scalar members in its default constructor.
- `maye_define.h` overrides standard library function names (`isspace`, `isalnum`, `isalpha`) via macros.
- `Extensions::check` dereferences potentially-null pointers.
- `readHtml` in `test_common.hpp` previously dropped the last character of every line.

See the file for the full list and recommended fixes.

## License

See [LICENSE](LICENSE).
