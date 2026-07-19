# Code Issues Report / 代码问题报告

> This file documents issues found during a static review of the `htmlcxx_expand` source tree.
> 本文件记录在对 `htmlcxx_expand` 源码进行静态审查时发现的问题。

Severity levels / 严重级别:
- **[Critical]** May cause crashes, undefined behavior, or incorrect results. / 可能导致崩溃、未定义行为或错误结果。
- **[High]** Likely bug or strong design risk. / 大概率 Bug 或较强设计风险。
- **[Medium]** Code smell, minor bug, or maintainability issue. / 代码异味、轻微 Bug 或可维护性问题。
- **[Low]** Style / documentation / portability. / 风格、文档、可移植性。

---

## 1. `src/html/maye_define.h`

### 1.1 [Critical] Macro override of standard library functions / 宏覆盖标准库函数

```cpp
#define isspace _isspace
#define isalnum _isalnum
#define isalpha _isalpha
```

`isspace`, `isalnum`, `isalpha` are standard library functions. Redefining them as macros in a header pollutes every translation unit that includes it (directly or transitively), and can silently change behavior of unrelated code (e.g. `Node.cc` and `ci_string.h` both rely on `isspace`/`isalnum`/`tolower` semantics). If the header is included before `<cctype>` is consulted, the standard functions may even be replaced inside `<cctype>` itself on some implementations.

这三个宏覆盖了标准库函数。任何直接或间接包含此头文件的翻译单元都会被污染，可能改变无关代码的行为。某些实现下若此头文件先于 `<cctype>` 被包含，`<cctype>` 中的函数也可能被替换。

**Recommendation / 建议:** Provide the helpers as `inline` functions in a namespace (e.g. `htmlcxx::util::is_space`) and call them explicitly; do not shadow standard names.

## 2. `src/html/Node.h` & `src/html/Node.cc`

### 2.1 [Critical] Uninitialized members in default constructor / 默认构造函数未初始化成员

```cpp
Node() {}
```

`mIsHtmlTag`, `mComment`, `mOffset`, `mLength` are left uninitialized. Reading them (e.g. via `isTag()`, `isComment()`, `offset()`, `length()`) before assignment is undefined behavior. `Node` is value-constructed in many places (e.g. inside `ParserDom::beginParsing`), so this is reachable in normal usage.

`mIsHtmlTag`、`mComment`、`mOffset`、`mLength` 在默认构造时未初始化。通过 `isTag()` 等读取它们属于未定义行为。`Node` 在多处被值构造（例如 `ParserDom::beginParsing`），因此正常运行时即可触发。

**Recommendation / 建议:**
```cpp
Node() : mIsHtmlTag(false), mComment(false), mOffset(0), mLength(0) {}
```

### 2.2 [Critical] `parseAttributes` uses `::tolower` with `char` / `parseAttributes` 中对 `char` 直接使用 `::tolower`

```cpp
transform(ptr, end, key.begin(), ::tolower);
```

`tolower` expects an `int` in the range of `unsigned char` or `EOF`. Feeding a signed `char` with a negative value (any byte >= 0x80) is undefined behavior. The same problem exists in `ci_string.h` (`tolower(c1)`).

`tolower` 期望参数为 `unsigned char` 范围内的 `int` 或 `EOF`。传入有符号 `char` 的负值（任何 >= 0x80 的字节）属于未定义行为。`ci_string.h` 中的 `tolower(c1)` 同样存在此问题。

**Recommendation / 建议:**
```cpp
transform(ptr, end, key.begin(), [](char c){ return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });
```

### 2.3 [High] Shadowed `end` variable in `parseAttributes` / `parseAttributes` 中变量 `end` 被遮蔽

```cpp
const char *end;
...
if (*ptr == '"' || *ptr == '\'') {
    char quote = *ptr;
    const char *end = strchr(ptr + 1, quote);   // shadows outer `end`
    ...
}
```

The inner `end` shadows the outer one, which is confusing and error-prone during maintenance.

内层 `end` 遮蔽外层 `end`，维护时容易混淆出错。

**Recommendation / 建议:** Rename the inner variable to `quote_end` (or remove the outer unused declaration).

### 2.4 [Medium] Implicit conversion to `std::string` / 隐式转换为 `std::string`

```cpp
operator std::string() const;
```

Implicit conversion operators cause surprising behavior (e.g. `Node` accidentally binding to `const std::string&` overloads). Mark it `explicit`.

隐式转换运算符会导致意外行为，建议改为 `explicit`。

### 2.5 [Low] `parseAttributes` early-returns silently on malformed tags / `parseAttributes` 对畸形标签静默返回

Multiple `if (...) return;` paths (e.g. `if (end == 0) return;`) leave `mAttributes` partially populated. Callers cannot tell whether parsing succeeded.

多个 `if (...) return;` 路径会让 `mAttributes` 处于半填充状态，调用方无法判断是否解析成功。

## 3. `src/html/ParserDom.cc`

### 3.1 [Low] Unused `static int i = 0;` / 未使用的 `static int i = 0;`

```cpp
void ParserDom::foundText(Node node) {
    ...
    static int i = 0;        // never used meaningfully
    auto counter = std::count_if(...);
    if (counter == text.size()) {
        //printf("content: empty %d %lld %llu\n", i++,counter,text.size());
        return;
    }
    ...
}
```

Leftover debugging artifact. Should be removed.

遗留调试代码，应删除。

### 3.2 [Medium] Text content is stored twice / 文本内容被存储两次

```cpp
mCurrentState->appendContent(node.text());
mHtmlTree.append_child(mCurrentState, node);
```

The same text is appended to the parent's `mContents` and also added as a separate child node. This is redundant storage and means downstream code must be careful not to double-count text. If this is intentional, document it; otherwise pick one location.

文本同时被追加到父节点的 `mContents` 并作为独立子节点加入树中。这造成冗余存储，下游代码需小心避免重复统计。若是有意为之，需注释说明；否则只保留一处。

### 3.3 [Medium] `foundText` whitespace check allocates a counter / `foundText` 空白检查方式低效

```cpp
auto counter = std::count_if(text.begin(), text.end(), [](char c) { return !!isspace(c); });
if (counter == text.size()) return;
```

This works but mixes the macro-overridden `isspace` (see 1.1). Prefer `text.find_first_not_of(" \t\r\n\v\f") == npos`.

可工作但依赖被宏覆盖的 `isspace`（见 1.1）。建议改用 `text.find_first_not_of(" \t\r\n\v\f") == npos`。

### 3.4 [Medium] `assert(i->tagName().length())` may fire on synthetic tags / `assert(i->tagName().length())` 可能对合成标签触发

In `foundTag`, the loop walks up to `mHtmlTree.begin()`. The synthetic root node created in `beginParsing` has an empty tag name, so if a closing tag walks all the way up, the `assert` will abort in debug builds and the behavior in release builds is undefined.

在 `foundTag` 中循环向上遍历到 `mHtmlTree.begin()`。`beginParsing` 中创建的合成根节点标签名为空，因此若闭合标签一路向上找到根，调试版会 `assert` 失败，Release 版为未定义行为。

## 4. `src/html/Extensions.cc`

### 4.1 [High] Pointer comparison with NULL is UB / NULL 指针比较属于未定义行为

```cpp
slash = strrchr(url.c_str(), '/');
dot   = strrchr(url.c_str(), '.');

if (slash >= dot) return false;       // UB if either is NULL

ci_string ext(dot);                   // UB if dot is NULL
```

If `dot` is NULL, both the comparison and the `ci_string` construction are undefined behavior.

若 `dot` 为 NULL，比较与构造 `ci_string` 均为未定义行为。

**Recommendation / 建议:**
```cpp
if (!dot) return false;
if (slash && slash >= dot) return false;
```

## 5. `src/xpath/XPathResult.h` & `src/xpath/XPathResult.cpp`

### 5.1 [Critical] `text()` returns dangling reference / `text()` 返回悬空引用

```cpp
const std::string& text() const {
    if (m_texts.empty())
        return {};           // returns reference to a temporary!
    return m_texts.front();
}
```

`return {}` constructs a temporary `std::string` and returns a reference to it - the reference dangles immediately. This is undefined behavior.

`return {}` 构造一个临时 `std::string` 并返回它的引用，引用立即悬空，属于未定义行为。

**Recommendation / 建议:** Return by value, or keep a static empty string member and return a reference to it.

### 5.2 [Medium] `texts()` returns by value (copy) inconsistently / `texts()` 与 `nodes()` 返回方式不一致

```cpp
const std::vector<std::string> texts() const { return m_texts; }           // copy
const std::vector<iterator>&   nodes() const { return m_its; }            // reference
```

`texts()` always copies; `nodes()` returns a reference. Make `texts()` return `const std::vector<std::string>&` for consistency and performance.

`texts()` 总是拷贝，`nodes()` 返回引用。建议 `texts()` 也返回 `const std::vector<std::string>&`。

### 5.3 [Low] Constructor parameter is ignored / 构造函数参数被忽略

```cpp
XPathResult(const iterator& node = nullptr) : m_errorString("No Error") {}
```

The `node` parameter is unused. Either remove it or store it; the current signature is misleading.

参数 `node` 未使用，签名具有误导性，建议移除或实际使用。

## 6. `src/xpath/XPathExpression.cpp`

### 6.1 [Critical] Out-of-bounds access in `parse()` / `parse()` 中越界访问

```cpp
if (*l != '/') { ... }
...
while (r != m_exp.cend()) {
    if (*l == '/') {
        if (*(l + 1) == '/') {        // UB if l+1 == cend()
            r = l + 2;
        } else {
            r = l + 1;
        }
```

If the expression is exactly `"/"`, then `l == r == cbegin()`, `*l == '/'`, and `l + 1 == cend()`. Dereferencing `l + 1` is undefined behavior. Even when not UB, the resulting `r = l + 2` is past the end; the subsequent `while (r != ... && *r != '/')` would also be UB.

若表达式恰好为 `"/"`，则 `l+1 == cend()`，解引用 `l+1` 为未定义行为。即便不是 UB，`r = l + 2` 也越过末尾，后续解引用同样 UB。

**Recommendation / 建议:** Check `l + 1 != m_exp.cend()` before dereferencing, and ensure `r` never exceeds `cend()`.

### 6.2 [High] `parseOption` uses fragile `path[0]` checks / `parseOption` 使用脆弱的 `path[0]` 判断

```cpp
if (path[0] == '*')
    option = { XPathOptions::option(7), path };
else
    option = { XPathOptions::option(7), path.substr(1) };
```

The decision to keep or strip the leading `*` depends on the first character only, and the resulting `option.second` semantics differ across branches. This makes downstream parsing (e.g. `XPathUtil::get_node_name`) inconsistent for `*[@id="x"]` vs `name[@id="x"]`. A more explicit representation (e.g. always strip the leading tag token, store wildcard flag separately) would be safer.

是否保留前导 `*` 仅根据首字符决定，导致 `option.second` 在不同分支语义不一致，使 `XPathUtil::get_node_name` 在处理 `*[@id="x"]` 与 `name[@id="x"]` 时行为不统一。建议显式表示（例如始终剥离前导标签 token，单独保存通配标志）。

### 6.3 [Medium] `parseOption` for `..` and `.` ignores the rest of the path / `..` 与 `.` 分支忽略后续路径

```cpp
if (path.find("..") != std::string::npos) {
    option = { XPathOptions::option(0), "" };
}
```

The path `..` is detected by `find("..")`, but then `option.second` is empty, which triggers the `if (option.second.empty()) throw` below. So `..` and `.` always throw. They are advertised as supported in `XPathOption.cpp` (`✅` markers) but are not actually usable.

`..` 通过 `find("..")` 识别，但 `option.second` 为空，会触发下方的 `throw`。也就是说 `..` 与 `.` 实际上无法使用，但 `XPathOption.cpp` 中却标记为支持（`✅`）。

**Recommendation / 建议:** Either implement `..` / `.` properly (pass `path` as the option payload) or remove them from the supported list.

### 6.4 [Medium] `parseOption` cannot distinguish `text()` from a tag named `text()` / 无法区分 `text()` 与名为 `text()` 的标签

Detection is purely `path.find("text()")`. A real tag literally named `<text()>` (unusual but syntactically possible in some HTML) would be misclassified. Low impact in practice but worth noting.

仅通过 `path.find("text()")` 判断。若存在名为 `<text()>` 的标签会被误判。实践中影响较小，但值得记录。

## 7. `src/xpath/XPath.cpp`

### 7.1 [High] `get_node_by_attrValue_name` rejects empty `attrValue` / `get_node_by_attrValue_name` 拒绝空 `attrValue`

```cpp
CHECK_NODE_NAME(attrValue, nullptr);
```

`CHECK_NODE_NAME` returns `nullptr` when the string is empty. But the function comment and the call site in `get_node_by_attrValue` state that an empty `attrValue` should mean "match any value" (e.g. `*[@id]`). So `*[@id]` is unreachable through this code path; only `get_node_by_attrValue_any` handles it. Either path is fine, but the check is misleading and silently disables the documented wildcard behavior.

`CHECK_NODE_NAME` 在字符串为空时返回 `nullptr`。但函数注释与 `get_node_by_attrValue` 的调用点都说明：空 `attrValue` 应表示"匹配任意值"（如 `*[@id]`）。因此 `*[@id]` 经此路径不可达，只能由 `get_node_by_attrValue_any` 处理。任一方案都可，但此处检查具有误导性，悄悄禁用了文档化的通配行为。

### 7.2 [High] `get_node_by_attr_and_name` lacks `isTag()` guard / `get_node_by_attr_and_name` 缺少 `isTag()` 判断

```cpp
for (auto it = root.begin(); it != root.end(); it++) {
    if (it->tagName() == node_name || node_name == "*") {
        it->parseAttributes();
        ...
    }
}
```

`tagName()` of a non-tag node (text/comment) is empty. When `node_name == "*"`, the condition is true for non-tag nodes too, and `parseAttributes()` is then called on a non-tag node (which no-ops because of the `isTag()` check inside, but the node still gets pushed into `items`). The result list may contain text/comment nodes.

非标签节点的 `tagName()` 为空。当 `node_name == "*"` 时条件对非标签节点也成立，虽然 `parseAttributes()` 内部因 `isTag()` 检查而空操作，但节点仍会被加入 `items`，结果列表可能包含文本/注释节点。

**Recommendation / 建议:** Add `if (!it->isTag()) continue;` at the top of the loop body.

### 7.3 [High] `get_node_by_attr_and_name` returns early only for the specific-attribute case / `get_node_by_attr_and_name` 仅在特定属性情况下提前返回

```cpp
if (attr_name == "*") {
    items.push_back(it);
}
else if (it->attribute(attr_name).first) {
    items.push_back(it);
    return items;          // early return only here
}
```

When `attr_name == "*"` (e.g. `/script[@*]`), the function collects all children with any attribute across all matching siblings. For a specific `attr_name`, it returns after the first match. The asymmetry means `/a[@*]` returns all matches but `/a[@href]` returns only the first one. This contradicts the usual XPath semantics where `/a[@href]` should also return all matching siblings.

`attr_name == "*"`（如 `/script[@*]`）时遍历所有兄弟节点收集任意属性的元素；而指定 `attr_name` 时仅返回首个匹配。这种不对称与 XPath 通常语义不符（`/a[@href]` 也应返回所有匹配兄弟）。

**Recommendation / 建议:** Remove the `return items;` early return so all matching siblings are collected.

### 7.4 [Medium] Iterators computed from `++root` modify the parameter / 通过 `++root` 计算迭代器会修改参数

```cpp
tree<HTML::Node>::pre_order_iterator begin(root);
tree<HTML::Node>::pre_order_iterator end(++root);     // mutates root
```

This pattern appears in `get_texts_from_genera`, `get_node_from_genera_by_name`, and `get_node_by_attrValue_any`. After the call, the caller's `iterator root` is advanced, which is surprising and can break downstream logic in `do_option` (where `node` is the same variable reused across options).

此模式出现在 `get_texts_from_genera`、`get_node_from_genera_by_name`、`get_node_by_attrValue_any`。调用后调用方的 `iterator root` 被推进，可能破坏 `do_option` 中跨选项复用 `node` 的逻辑。

**Recommendation / 建议:** Compute `end` from a local copy: `iterator end = root; ++end;` (or use `tree.begin(root)`/`tree.end(root)`-style helpers).

### 7.5 [Medium] Typo "option invlid" / 拼写错误 "option invlid"

```cpp
throw std::runtime_error("option invlid");
```

Should be `"option invalid"`.

### 7.6 [Medium] `get_node_by_array_and_name` does not handle wildcard `*` / `get_node_by_array_and_name` 不支持通配符 `*`

```cpp
if (it->tagName() == node_name) { ... }
```

So `/*[1]` cannot work. If wildcard indexing is desired, special-case `node_name == "*"`.

`/*[1]` 无法工作。若希望支持通配索引，需特判 `node_name == "*"`。

### 7.7 [Low] `get` uses `auto& expr = m_result.expression();` but `expr` may be invalidated / `get` 中引用可能在异常后失效

The reference is fine in practice because `m_result` is not swapped again inside `do_option`, but the lifetime coupling is fragile. A local copy or restructuring would be safer.

实际不会出问题（因为 `do_option` 内未再次 swap `m_result`），但耦合脆弱，使用局部拷贝或重构更安全。

### 7.8 [Low] `CHECK_NODE_NAME(name, {})` returns `{}` for `iterator` returns / `CHECK_NODE_NAME(name, {})` 对 `iterator` 返回类型用 `{}`

```cpp
iterator get_node_by_attrValue(const std::string& name, iterator root) {
    CHECK(root, nullptr);
    CHECK_NODE_NAME(name, {});    // returns {} for an iterator?
```

`{}` for an `iterator` type relies on `sibling_iterator` being constructible from `nullptr`/empty. This works only because of implicit conversions; explicit `nullptr` is clearer.

`iterator` 类型上的 `{}` 依赖 `sibling_iterator` 可由 `nullptr`/空构造，仅因隐式转换才成立。显式写 `nullptr` 更清晰。

## 8. `src/xpath/XPathOption.cpp`

### 8.1 [Low] `assert(index >= 0 && index < OptionSize)` is wrong type / `assert` 类型不正确

```cpp
static const int OptionSize = ...;
const XPathOption* XPathOptions::option(size_t index) {
    assert(index >= 0 && index < OptionSize);
```

`index` is `size_t` (unsigned), so `index >= 0` is always true and may trigger a compiler warning. Drop the lower-bound check.

`index` 为 `size_t`（无符号），`index >= 0` 恒为真并可能触发编译警告，删除下界检查即可。

### 8.2 [Low] Index-based dispatch in `XPathExpression::parseOption` is fragile / 基于索引的派发脆弱

`parseOption` returns `XPathOptions::option(N)` with hardcoded indices. Adding/reordering options in `gOptions` silently breaks parsing. Prefer lookup by name.

`parseOption` 用硬编码索引 `XPathOptions::option(N)` 返回选项。`gOptions` 中增删或重排都会悄悄破坏解析，建议按名称查找。

## 9. `test/test_common.hpp`

### 9.1 [High] `readHtml` drops the last character of every line / `readHtml` 每行都会丢失最后一个字符

```cpp
buf.append(p, line.end() - 1);
```

`std::getline` already strips the trailing `\n`, so `line` typically ends with `\r` on Windows or the real last character on other platforms. Subtracting 1 unconditionally drops a real character when the line does not end with `\r`. This corrupts HTML content (e.g. a `<` at end of line becomes nothing, shifting the rest).

`std::getline` 已剥离 `\n`，`line` 在 Windows 下以 `\r` 结尾，在其他平台则是真实末字符。无条件 `-1` 在不以 `\r` 结尾时会丢掉真实字符，破坏 HTML 内容。

**Recommendation / 建议:**
```cpp
auto e = line.end();
if (!line.empty() && line.back() == '\r') --e;
buf.append(p, e);
```

### 9.2 [Medium] `readHtml` ignores lines whose non-space content starts beyond offset 0 / `readHtml` 删除每行前导空白

The function trims leading whitespace from every line. For HTML inside `<pre>` blocks or where indentation is significant (rare but possible), this loses information. Document the behavior or make it optional.

该函数会去掉每行前导空白。对 `<pre>` 块或对缩进敏感的内容会丢失信息。建议注释说明或提供开关。

### 9.3 [Low] `infile` is not closed explicitly / `infile` 未显式关闭

RAII handles it, but the early `return {}` on open failure is fine; just noting.

RAII 会处理，仅作记录。

## 10. `test/test_xpath.cpp` & `test/test_xpath_html.cpp` & `test/test_parserDom.cpp`

### 10.1 [Medium] Tests have no assertions / 测试缺少断言

The tests print to stdout only. A regression that changes output silently cannot be detected automatically. Add assertions (a tiny custom macro is enough; no external framework needed).

测试仅向 stdout 打印输出，回归无法被自动检测。建议加入断言（用一个小型自定义宏即可，无需引入外部框架）。

### 10.2 [Medium] Tests are Windows-specific / 测试为 Windows 专用

```cpp
system("chcp 65001");
```

`chcp` exists only on Windows. Guard with `#ifdef _WIN32` so the tests can also run on Linux/macOS.

`chcp` 仅 Windows 存在，建议用 `#ifdef _WIN32` 包裹，便于在 Linux/macOS 上运行。

### 10.3 [Low] `test_xpath.cpp` does not call `chcp 65001` / `test_xpath.cpp` 未调用 `chcp 65001`

Inconsistent with the other two test files; Chinese characters will render incorrectly in the Windows console.

与其他两个测试文件不一致，Windows 控制台下中文显示会乱码。

### 10.4 [Low] `test_xpath_html.cpp` dereferences `attribute("href").second` without checking `first` / `test_xpath_html.cpp` 未检查 `.first` 即使用 `.second`

```cpp
std::cout << ... << node->attribute("href").second << std::endl;
```

When `href` is missing, `.second` is an empty string (safe), but the test then prints misleading output. Prefer an explicit check.

`href` 缺失时 `.second` 为空字符串（安全），但输出会具有误导性，建议显式检查。

## 11. `CMakeLists.txt`

### 11.1 [Medium] `PROJECT_PATH` embedded as a string literal may contain backslashes / `PROJECT_PATH` 作为字符串字面量内嵌可能含反斜杠

```cmake
add_compile_definitions(PROJECT_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
```

On Windows, `CMAKE_CURRENT_SOURCE_DIR` uses backslashes. CMake escapes them correctly for the compiler, but the resulting `PROJECT_PATH"/test/..."` adjacent-string concatenation is fragile. Prefer forwarding the path via `target_compile_definitions` with a forward-slashed path:

Windows 下 `CMAKE_CURRENT_SOURCE_DIR` 使用反斜杠。CMake 会正确转义，但相邻字符串拼接 `PROJECT_PATH"/test/..."` 较脆弱。建议改用 `target_compile_definitions` 并将路径中的反斜杠替换为正斜杠：

```cmake
file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" PROJECT_PATH_SRC)
target_compile_definitions(test_xpath_html PRIVATE PROJECT_PATH="${PROJECT_PATH_SRC}")
```

### 11.2 [Low] `BUILD_TESTS` ON by default may surprise packagers / `BUILD_TESTS` 默认开启可能让打包者意外

Default `ON` is fine for development, but a packaging build typically wants `OFF`. Consider defaulting to `OFF` when the project is built as a subproject (`CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR`).

默认 `ON` 利于开发，但打包构建通常希望 `OFF`。建议在作为子项目构建时默认 `OFF`。

### 11.3 [Low] Test executables share a single `add_compile_definitions` / 测试可执行文件共享全局 `add_compile_definitions`

`PROJECT_PATH` is added globally, which means it is also defined for the library target. Move it to the test targets only.

`PROJECT_PATH` 是全局定义，意味着库目标也被定义了该宏。建议仅对测试目标定义。

---

## Summary / 总结

| Severity | Count |
| --- | --- |
| Critical | 5 |
| High | 5 |
| Medium | 11 |
| Low | 10 |

Top priorities / 优先处理:
1. Fix dangling reference in `XPathResult::text()` (5.1).
2. Fix out-of-bounds access in `XPathExpression::parse()` (6.1).
3. Initialize `Node` members (2.1) and fix `tolower` UB (2.2).
4. Replace macro overrides in `maye_define.h` (1.1).
5. Fix `Extensions::check` NULL pointer UB (4.1).
6. Fix `readHtml` dropping the last character of every line (9.1).
7. Add `isTag()` guard and fix asymmetric early-return in `get_node_by_attr_and_name` (7.2, 7.3).
