#pragma once

#include "XPathExpression.h"
#include "html/Node.h"
#include "tree.h"

namespace xpath
{
	using iterator = tree<htmlcxx::HTML::Node>::sibling_iterator;
	class XPathResult
	{
	public:
		friend class XPath;
	public:
		XPathResult(const iterator& node = nullptr);
		XPathResult(XPathResult&& other)noexcept;
		XPathResult& operator=(XPathResult&& other)noexcept;

		XPathResult(const XPathResult& node) = delete;
		XPathResult& operator=(const XPathResult&) = delete;

		/**
		 * 遍历m_node的所有子节点.
		 */
		auto begin()const { return m_its.begin(); }
		auto end()const { return m_its.end(); }

		bool empty()const { return m_its.empty() && m_texts.empty(); }

		operator bool()const { return !empty(); }

		const std::string& text()const
		{
			if (m_texts.empty())
				return {};
			return m_texts.front();
		}
		const std::vector<std::string> texts()const { return m_texts; }
		const std::vector<iterator>& nodes()const { return m_its; }
		iterator node()const { return m_its.empty() ? nullptr : m_its.front(); }

		const std::string& errorString()const { return m_errorString; }
		const XPathExpression& expression()const { return m_expression; }
		void swap(XPathResult& other);
		void clear();
	private:
		XPathExpression m_expression;
		std::vector<std::string> m_texts;
		std::vector<iterator> m_its;
		std::string m_errorString;
	};
}
