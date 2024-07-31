#pragma once

#include <iostream>
namespace xpath {

	struct XPathOption
	{
		const char* name;	/*!> 选项名*/
		const char* desc;	/*!> 选项描述*/
		friend std::ostream& operator<<(std::ostream& os, const XPathOption& option);
	};

	inline constexpr unsigned int OptionHash(const char* str, int h = 0) {
		return !str[h] ? 5381 : (OptionHash(str, h + 1) * 33) ^ str[h];
	}

	class XPathOptions
	{
	public:
		static const XPathOption* option(size_t index);
		static const XPathOption* options();
		static size_t optionsSize();
	private:
	};
}


