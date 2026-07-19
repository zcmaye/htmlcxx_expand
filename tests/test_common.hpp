#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "html/maye_define.h"

/**
 * 读取html文件，跳过空行能大幅度缩减保存的html数据.
 * Read an HTML file. Skipping blank lines greatly shrinks the stored HTML.
 */
inline std::string readHtml(const std::string& htmlFile)
{
	std::ifstream infile(htmlFile, std::ios::binary);
	if (!infile) {
		std::cout << "open file <" << htmlFile << "> failed" << std::endl;
		return {};
	}
	std::string buf;
	while (!infile.eof()) {
		std::string line;
		if (!std::getline(infile, line)) {
			break;
		}
		//为空直接继续 / skip empty lines
		if (line.empty())
			continue;

		//跳过前导空白 / skip leading whitespace
		auto p = line.begin();
		while (p != line.end() && isspace(*p)) {
			++p;
		}
		if (p == line.end()) {
			continue;
		}

		// std::getline 已经去掉了末尾的 '\n'，这里只需处理 Windows 下的 '\r'
		// std::getline already strips the trailing '\n'; only strip '\r' on Windows.
		auto e = line.end();
		if (e != p && *(e - 1) == '\r') {
			--e;
		}
		buf.append(p, e);
	}
	return buf;
}

/**
 * 在 Windows 控制台下设置 UTF-8 代码页，避免中文乱码。
 * Set the Windows console code page to UTF-8 so Chinese characters render correctly.
 * 在非 Windows 平台上是空操作。No-op on non-Windows platforms.
 */
inline void setupConsoleUtf8()
{
#ifdef _WIN32
	system("chcp 65001 > nul");
#endif
}

// =========================================================================
// 极简测试框架 / Minimal test framework
// 用法 / Usage:
//   TEST_CHECK(cond, "message");
//   TEST_EQ(actual, expected, "message");
//   TEST_TRUE(cond, "message");
//   TEST_FALSE(cond, "message");
// =========================================================================

namespace test_util {

	struct TestStats {
		int assertions = 0;
		int failures = 0;
	};

	inline TestStats& stats() {
		static TestStats s;
		return s;
	}

	inline void record(bool ok, const std::string& msg) {
		++stats().assertions;
		if (!ok) {
			++stats().failures;
			std::cout << "  [FAIL] " << msg << std::endl;
		}
	}

	inline int report() {
		const auto& s = stats();
		std::cout << "----------------------------------------" << std::endl;
		std::cout << "Assertions: " << s.assertions
			<< " | Failures: " << s.failures << std::endl;
		std::cout << "----------------------------------------" << std::endl;
		return s.failures == 0 ? 0 : 1;
	}
}

// 检查条件为真 / assert that condition is true
#define TEST_CHECK(cond, msg) \
	do { \
		bool _ok = static_cast<bool>(cond); \
		test_util::record(_ok, std::string(msg) + "  (cond: " #cond ")"); \
	} while (0)

// 检查条件为真（语义化）/ assert that condition is true (semantic alias)
#define TEST_TRUE(cond, msg)  TEST_CHECK(cond, msg)

// 检查条件为假 / assert that condition is false
#define TEST_FALSE(cond, msg) TEST_CHECK(!(cond), msg)

// 检查两个值相等 / assert that two values are equal
#define TEST_EQ(actual, expected, msg) \
	do { \
		auto _a = (actual); \
		auto _b = (expected); \
		bool _ok = (_a == _b); \
		std::ostringstream _oss; \
		_oss << msg << "  (actual=[" << _a << "] expected=[" << _b << "])"; \
		test_util::record(_ok, _oss.str()); \
	} while (0)

// 检查两个值不相等 / assert that two values are not equal
#define TEST_NE(actual, expected, msg) \
	do { \
		auto _a = (actual); \
		auto _b = (expected); \
		bool _ok = (_a != _b); \
		std::ostringstream _oss; \
		_oss << msg << "  (actual=[" << _a << "] expected!=[" << _b << "])"; \
		test_util::record(_ok, _oss.str()); \
	} while (0)

// 输出测试分节标题 / print a section banner
#define TEST_SECTION(name) \
	do { std::cout << "\n==================== " << name << " ====================" << std::endl; } while (0)

// 报告并返回退出码 / report results and return an exit code
#define TEST_REPORT() test_util::report()
