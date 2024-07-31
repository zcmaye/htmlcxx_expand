#include "XPathExpression.h"
#include <stdexcept>
#include <iostream>

namespace xpath {

	XPathExpression::XPathExpression()
		:XPathExpression("//*")
	{
	}

	XPathExpression::XPathExpression(const std::string& exp)
		: m_exp(exp)
		, m_valid(false)
	{
		m_valid = parse();
	}

	XPathExpression::XPathExpression(const char* exp)
		:XPathExpression(std::string(exp))
	{

	}

	void XPathExpression::swap(XPathExpression& other)
	{
		m_exp.swap(other.m_exp);
		m_errorString.swap(other.m_errorString);
		std::swap(m_valid, other.m_valid);
		m_options.swap(other.m_options);

	}

	bool XPathExpression::parse()
	{
		if (m_exp.empty()) {
			m_errorString = "xpath express is empty";
			return  false;
		}
		std::string::const_iterator l(m_exp.cbegin());
		std::string::const_iterator r(m_exp.cbegin());

		if (*l != '/') {
			m_errorString = "xpath express must start /";
			return  false;
		}

		while (r != m_exp.cend()) {
			if (*l == '/') {
				if (*(l + 1) == '/') {
					r = l + 2;
				}
				else {
					r = l + 1;
				}
				while (r != m_exp.cend() && *r != '/') {
					r++;
				}
				std::string path(l, r);
				//std::cout << path  <<"     ";
				try
				{
					m_options.push_back(parseOption(path));
				}
				catch (const std::exception& e)
				{
					m_errorString = e.what();
					return false;
				}
			}
			l = r;
		}
		return  true;
	}

	std::pair<const XPathOption*, std::string> XPathExpression::parseOption(const std::string& path)
	{
		std::pair<const XPathOption*, std::string> option;

		// 取后代
		if (path.find("//") != std::string::npos)
		{
			if (path.find("text()") != std::string::npos)
			{
				option = { XPathOptions::option(9), path.substr(2) };
			}
			else
			{
				option = { XPathOptions::option(3), path.substr(2) };
			}
		}
		else
		{
			// 取子代
			if (path.find("..") != std::string::npos)
			{
				option = { XPathOptions::option(0), "" };
			}
			else if (path.find(".") != std::string::npos)
			{
				option = { XPathOptions::option(1), "" };
			}
			else if (path.find("[") != std::string::npos)
			{
				if (path.find("@") != std::string::npos)
				{
					if (path.find("=") != std::string::npos)
					{
						if (path[0] == '*')
							option = { XPathOptions::option(7), path };
						else
							option = { XPathOptions::option(7), path.substr(1) };
					}
					else
					{
						option = { XPathOptions::option(6), path.substr(1) };
					}
				}
				else
				{
					option = { XPathOptions::option(5), path.substr(1) };
				}
			}
			else if (path.find("@") != std::string::npos)
			{
				option = { XPathOptions::option(10), path.substr(1) };
			}
			else if (path.find("text()") != std::string::npos)
			{
				option = { XPathOptions::option(8), path.substr(1) };
			}
			else
			{
				option = { XPathOptions::option(4), path.substr(1) };
			}
		}
		if (option.second.empty()) {
			throw std::runtime_error("The xpath path expression syntax is incorrect,missing path!");
		}
		//std::cout << *option.first << "     " << option.second << std::endl;
		return option;
	}

	std::ostream& operator<<(std::ostream& os, const XPathExpression& exp)
	{
		os << exp.m_exp;
		return os;
	}
}
