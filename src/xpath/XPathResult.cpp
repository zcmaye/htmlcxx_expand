#include "XPathResult.h"

namespace xpath {

	XPathResult::XPathResult(const iterator& node)
		:m_errorString("No Error")
	{
	}

	XPathResult::XPathResult(XPathResult&& other) noexcept
	{
		swap(other);
	}

	XPathResult& XPathResult::operator=(XPathResult&& other)noexcept
	{
		swap(other);
		return *this;
	}

	void XPathResult::swap(XPathResult& other)
	{
		m_expression = std::move(other.m_expression);
		m_its.swap(other.m_its);
		m_texts.swap(other.m_texts);
		m_errorString.swap(other.m_errorString);
	}

	void XPathResult::clear()
	{
		XPathExpression exp;
		m_expression.swap(exp);
		m_texts.clear();
		m_its.clear();
		m_errorString = "No Error";
	}
}
