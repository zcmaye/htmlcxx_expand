#pragma once

#include "XPathOption.h"
#include <string>
#include <vector>

namespace xpath {

	class XPathExpression
	{
		class XPath;
	public:
		XPathExpression();
		XPathExpression(const std::string& exp);
		XPathExpression(const char* exp);

		bool isValid()const { return m_valid; }
		const std::string& errorString()const { return m_errorString; }
		const std::string& string()const { return m_exp; }

		const std::vector<std::pair<const XPathOption*, std::string>>& options()const { return m_options; }
		void swap(XPathExpression& other);
	protected:
		bool parse();
		std::pair<const XPathOption*, std::string> parseOption(const std::string& path);
		friend std::ostream& operator<<(std::ostream& os, const XPathExpression& exp);
	private:
		std::string m_exp;
		std::string m_errorString;
		bool m_valid;
		std::vector<std::pair<const XPathOption*, std::string>> m_options;
	};

}
