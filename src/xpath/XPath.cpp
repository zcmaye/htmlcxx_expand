#include "XPath.h"
#include "html/ParserDom.h"
#include "XPathOption.h"

#include <iostream>

using namespace htmlcxx;

#define CHECK(it, ret)\
    if ((it) == nullptr) return ret
#define CHECK_NODE_NAME(name,ret)\
    if(name.empty()) {\
        m_result.m_errorString = "The xpath path expression syntax is incorrect!";\
        return ret;\
    }
namespace xpath {

	/**
	 * XPath 工具函数.
	 */
	class XPathUtil
	{
	public:
		/**
		 * 获取带属性的路径表达式中的属性名
		 * @param  待解析的xpath，example : div[@id="wrap"]
		 * @return id
		 */
		static std::string get_attr_name(const std::string& name)
		{
			std::string s;
			int status = 0;
			for (auto it = name.cbegin(); it != name.cend(); it++)
			{
				if (*it == '@') {
					status = 1;     //属性名
					continue;
				}
				else if (*it == ']' || *it == '=') {
					break;          //解析完毕
				}
				if (status == 1) {
					s.push_back(*it);
				}
			}
			return s;
		}

		/**
		 * 获取带属性的路径表达式中的属性值
		 * @param  待解析的xpath，example : div[@id="wrap"]
		 * @return wrap
		 */
		static std::string get_attr_value(const std::string& name)
		{
			std::string s;
			int status = 0;
			for (auto it = name.cbegin(); it != name.cend(); it++)
			{
				if (*it == '=') {
					if (++it == name.cend()) {
						throw std::runtime_error("The xpath expression syntax is incorrect:" + name);
					}
					if (*it == '\"') {
						status = 1;     //开始解析属性值
						continue;       //跳过双引号
					}
				}
				else if (*it == ']' || *it == '\"') {
					break;          //解析完毕
				}
				if (status == 1) {
					s.push_back(*it);
				}
			}
			return s;
		}

		/**
		 * 获取带属性的路径表达式中的标签名
		 * @param  待解析的xpath，example : div[@id="wrap"]
		 * @return div
		 */
		static std::string get_node_name(const std::string& name)
		{
			std::string s;
			for (auto it = name.cbegin(); it != name.cend(); it++) {
				if (*it == '[') {
					break;          //解析完毕
				}
				s.push_back(*it);
			}
			return s;
		}
		/**
		 * 获取带索引的路径表达式中的索引值
		 * @param  待解析的xpath，example : div[23]
		 * @return 23
		 */
		static std::string get_array_index_string(const std::string& name)
		{
			int status = 0;
			std::string index;
			for (auto it = name.begin(); it != name.end(); it++) {
				if (*it == '[') {
					status = 1;
					continue;
				}
				if (*it == ']') {
					break;
				}
				if (status == 1) {
					index.push_back(*it);
				}
			}
			return index;
		}

		static int get_array_index(const std::string& name)
		{
			return std::stoi(get_array_index_string(name));
		}
	};

	/**
	 * XPath.
	 */
	XPath::XPath(const tree<HTML::Node>& tree, const std::string& html)
		: m_tree(tree), m_html(html)
	{
	}

	XPath::XPath(const std::string& html)
		: m_html(html)
	{
		htmlcxx::HTML::ParserDom dom;
		m_tree = dom.parseTree(html);
	}

	// 返回父节点
	iterator XPath::get_parent_node(iterator root)
	{
		CHECK(root, nullptr);
		return root.parent_;
	}

	// 返回当前节点
	iterator XPath::get_this_node(iterator root)
	{
		CHECK(root, nullptr);
		return root;
	}

	// 获取当前节点文本
	std::string XPath::get_text_from_this(iterator root)
	{
		CHECK(root, "");
		std::string str;
		for (auto& node : root) {
			if (!node.isTag() && !node.isComment()) {
				str.append(node.text());
				str.append(1, ' ');
			}
			//如果有br则添加换行
			if (!str.empty() && node.isTag() && node.tagName() == "br" && str.back() != '\n') {
				str.append(1, '\n');
			}
		}
		return str;
	}

	// 获取当前节点后节点的所有文本
	std::vector<std::string> XPath::get_texts_from_genera(iterator root)
	{
		CHECK(root, {});

		std::vector<std::string> texts;
		tree<HTML::Node>::pre_order_iterator begin(root);
		tree<HTML::Node>::pre_order_iterator end(++root);
		for (auto it = begin; it != end; it++) {
			if (!it->isTag() && !it->isComment()) {
				texts.push_back(it->text());
			}
		}
		return texts;
	}

	// 根据属性 值 名称 来选择节点
	iterator XPath::get_node_by_attrValue_any(iterator root, const std::string& attrName, const std::string& attrValue)
	{
		CHECK(root, nullptr);
		CHECK_NODE_NAME(attrName, nullptr);

		tree<HTML::Node>::pre_order_iterator begin(root);
		//tree<HTML::Node>::pre_order_iterator end(root.parent_ ? root.range_last() : nullptr);
		tree<HTML::Node>::pre_order_iterator end(++root);
		for (auto it = begin; it != end; it++)
		{
			if (!it->isTag())
				continue;
			it->parseAttributes();
			auto pr = it->attribute(attrName);
			if (!pr.first)
				continue;
			//如果找到属性，并且属性值为空，直接返回(*[@data-rid]获取所有属性为data-rid的节点)
			if (attrValue.empty() || pr.second == attrValue) {
				return it;
			}
		}

		return nullptr;
	}

	iterator XPath::get_node_by_attrValue_name(iterator root, const std::string& attrName, const std::string& attrValue, const std::string& nodeName)
	{
		CHECK(root, nullptr);
		CHECK_NODE_NAME(nodeName, {});
		CHECK_NODE_NAME(attrName, nullptr);
		CHECK_NODE_NAME(attrValue, nullptr);

		for (auto it = root.begin(); it != root.end(); it++)
		{
			if (!it->isTag())
				continue;
			if (it->tagName() != nodeName)
				continue;
			it->parseAttributes();
			// 获取属性
			auto pr = it->attribute(attrName);
			// 没有查找的属性
			if (!pr.first) {
				continue;
			}
			//查找的属性为空，或者属性相等
			if (attrValue.empty() || pr.second == attrValue) {
				return it;
			}
		}
		return nullptr;
	}

	/**
	 * 根据属性获取节点
	 * @param name 带属性的路径表达式 example: *[@id="j-catalogWrap"]
	 */
	iterator XPath::get_node_by_attrValue(const std::string& name, iterator root)
	{
		CHECK(root, nullptr);
		CHECK_NODE_NAME(name, {});

		std::string attr_name = XPathUtil::get_attr_name(name);
		std::string value_name = XPathUtil::get_attr_value(name);
		std::string node_name = XPathUtil::get_node_name(name);
		// std::cout << attr_name << " " << node_name << " " << value_name << std::endl;
		if (node_name == "*")
			return get_node_by_attrValue_any(root, attr_name, value_name);
		return get_node_by_attrValue_name(root, attr_name, value_name, node_name);
	}

	// 通过属性名和名称获取节点
	std::vector<iterator> XPath::get_node_by_attr_and_name(const std::string& name, iterator root)
	{
		CHECK(root, {});
		CHECK_NODE_NAME(name, {});

		std::vector<iterator> items;
		std::string attr_name = XPathUtil::get_attr_name(name);
		std::string node_name = XPathUtil::get_node_name(name);
		for (auto it = root.begin(); it != root.end(); it++) {
			if (it->tagName() == node_name || node_name == "*") {
				it->parseAttributes();
				auto& attrs = it->attributes();
				if (attrs.empty()) {
					continue;
				}
				//获取带任意属性的元素
				if (attr_name == "*") {
					items.push_back(it);
				}
				else if (it->attribute(attr_name).first) {
					items.push_back(it);
					return items;
				}
			}
		}
		return items;
	}

	// 通过数组选择与名字相同的节点
	iterator XPath::get_node_by_array_and_name(const std::string& name, iterator root)
	{
		CHECK(root, nullptr);
		CHECK_NODE_NAME(name, {});

		std::string node_name = XPathUtil::get_node_name(name);
		int node_idx = XPathUtil::get_array_index(name);
		int curIndx = 1;
		// std::cout << tmp_name << " " << j << " " << std::endl;
		// it == child
		for (auto it = root.begin(); it != root.end(); it++) {
			if (it->tagName() == node_name) {
				if (curIndx++ == node_idx) {
					return it;
				}
			}
		}
		return nullptr;
	}

	// 选择当前元素子代元素中的name元素
	iterator XPath::get_node_from_child_by_name(const std::string& name, iterator root)
	{
		CHECK(root, nullptr);
		CHECK_NODE_NAME(name, {});

		for (auto it = root.begin(); it != root.end(); it++)
		{
			//std::cout<<it->tagName()<<":"<<it->tagName().size()<<" "<<name<<":"<<name.size()<<"  "<<(it->tagName() == name)<<std::endl;
			if (it->tagName() == name)
				return it;
		}
		return nullptr;
	}

	// 选择当前元素后代元素中的name元素
	//-- //title[@lang='eng'] 选取所有 title 元素，且这些元素拥有值为 eng 的 lang 属性。
	std::vector<iterator> XPath::get_node_from_genera_by_name(const std::string& name, iterator root)
	{
		CHECK(root, {});
		CHECK_NODE_NAME(name, {});

		std::vector<iterator> its;
		std::string attr_name = XPathUtil::get_attr_name(name);
		std::string attr_value = XPathUtil::get_attr_value(name);
		std::string node_name = XPathUtil::get_node_name(name);

		// 遍历树
		tree<HTML::Node>::pre_order_iterator begin(root);
		tree<HTML::Node>::pre_order_iterator end(++root);
		for (auto it = begin; it != end; it++)
		{
			if (!it->isTag())
				continue;

			if (node_name == "*" || node_name == it->tagName())
			{
				//属性名不为空，查找所有属性为attr_name的节点
				if (!attr_name.empty()) {
					it->parseAttributes();
					auto pr = it->attribute(attr_name);
					if (!pr.first)
						continue;
					if (attr_value.empty() || pr.second == attr_value) {
						its.push_back(it);
					}
				}
				//属性名为空，所有节点都保存起来
				else {
					its.push_back(it);
				}

			}
		}
		return its;
	}

	std::string XPath::get_attr_from_this(const std::string& name, iterator root)
	{
		CHECK(root, {});
		CHECK_NODE_NAME(name, {});

		std::string attr_name = XPathUtil::get_attr_name(name);
		root->parseAttributes();
		auto pr = root->attribute(attr_name);
		if (pr.first) {
			return pr.second;
		}
		return {};
	}

	// 处理xpath操作
	bool XPath::do_option(iterator root)
	{
		CHECK(root, false);
		auto node = root;
		std::string ret_text;
		std::vector<std::string> texts;
		auto& expr = m_result.expression();
		for (auto& opt : expr.options())
		{
			auto& name = opt.first->name;
			auto& path = opt.second;
			// std::cout << option << " " << name << std::endl;
			switch (OptionHash(name))
			{
			case OptionHash("get_node_from_genera_by_name"):
				m_result.m_its = get_node_from_genera_by_name(path, node);
				node = m_result.node();
				break;
			case OptionHash("get_node_from_child_by_name"):
				node = get_node_from_child_by_name(path, node);
				m_result.m_its = { node };
				break;
			case OptionHash("get_node_by_array_and_name"):
				node = get_node_by_array_and_name(path, node);
				m_result.m_its = { node };
				break;
			case OptionHash("get_node_by_attr_and_name"):
				m_result.m_its = get_node_by_attr_and_name(path, node);
				node = m_result.node();
				break;
			case OptionHash("get_node_by_attrValue_and_name"):
				node = get_node_by_attrValue(path, node);
				m_result.m_its = { node };
				break;
			case OptionHash("get_text_from_this"):
				for (auto it = m_result.begin(); it != m_result.end(); it++) {
					ret_text = get_text_from_this(*it);
					m_result.m_texts.push_back(std::move(ret_text));
				}
				//ret_text = get_text_from_this(node);
				//m_result.m_texts.push_back(ret_text);
				m_result.m_its.clear();
				node = nullptr;
				return true;
			case OptionHash("get_texts_from_genera"):
				texts = get_texts_from_genera(node);
				m_result.m_texts = texts;
				//m_result.m_its = { node};
				m_result.m_its.clear();
				return true;
			case OptionHash("get_this_node"):
				node = get_this_node(node);
				m_result.m_its = { node };
				break;
			case OptionHash("get_parent_node"):
				node = get_parent_node(node);
				m_result.m_its = { node };
				break;
			case OptionHash("get_attr_from_this"):
				ret_text = get_attr_from_this(path, node);
				m_result.m_texts.push_back(ret_text);
				m_result.m_its = { node };
				return true;
			default:
				throw std::runtime_error("option invlid");
			}
			if (node == nullptr) {
				m_result.m_its.clear();
				throw std::runtime_error("xpath(" + expr.string() + "): Not Found<" + opt.second + ">");
			}
		}
		if (m_result.empty() && node != nullptr)
			m_result.m_its.push_back(node);
		return true;
	}

	// 返回xpath解析内容
	XPathResult XPath::get(const XPathExpression& exp)
	{

		XPathResult result;
		result.m_expression = exp;
		m_result.swap(result);

		if (!exp.isValid()) {
			m_result.m_errorString = m_result.expression().errorString();
			return std::move(m_result);
		}

		auto& expr = m_result.expression();
		try
		{
			do_option(m_tree.begin());
		}
		catch (const std::exception& e)
		{
			m_result.m_errorString = e.what() + (!expr.isValid() ? expr.errorString() : "");
		}

		return std::move(m_result);
	}
}



