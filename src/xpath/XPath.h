#ifndef __XPATH__H_
#define __XPATH__H_

#include "html/Node.h"
#include "html/tree.h"
#include <queue>

namespace htmlcxx 
{
	using iterator_base = tree<HTML::Node>::iterator_base;
	using iterator = tree<HTML::Node>::sibling_iterator;
	using pre_order_iterator = tree<HTML::Node>::pre_order_iterator;

	class NodeResult
	{
		friend class XPath;
	public:
		NodeResult(const iterator& node) :m_node(node) {}

		/**
		 * 遍历m_node的所有子节点.
		 */
		pre_order_iterator  begin()const { return m_node; }
		pre_order_iterator  end()const { return ++iterator(m_node); }

		/**
		 * 遍历父节点的所有子节点(也就是m_node的兄弟节点).
		 */
		pre_order_iterator   pbegin()const { return parent(); }
		pre_order_iterator   pend()const { return ++pre_order_iterator(parent()); }

		/**
		 * 获取父节点，用来遍历xpath结果的后续兄弟节点.
		 */
		pre_order_iterator  parent()const { return m_node.parent_; }

		bool empty()const {return m_its.empty() && m_texts.empty(); }
	   	operator bool()const { return !empty(); }

		const std::string& text()const 
		{
			if (m_texts.empty())
				return {};
			return m_texts.front(); 
		}
		const std::vector<std::string> texts()const { return m_texts; }
		const std::vector<iterator>& nodes()const { return m_its; }
		const iterator& node()const { return m_node; }
	private:
		std::vector<std::string> m_texts;
		std::vector<iterator> m_its;
		iterator m_node;
	};

	class XPath
	{
	public:
	public:
		XPath(const tree<HTML::Node>& tree,const std::string& html);
		XPath(const std::string& html);
		NodeResult xpath(const std::string& exp);

		/**
		 * 获取节点文本.
		 * -- <a>hello wrold</a> ==> hello wrold
		 * -- <div><p>你好</p></div> ==> <p>你好</p>
		 */
		std::pair<bool,std::string> getNodeContent(const htmlcxx::HTML::Node& node);

		const std::string& errorString()const { return m_errorString; }
	private:
		tree<HTML::Node> m_tree;
		std::string m_html;
		std::string m_errorString;

		typedef std::queue<std::pair<std::string, std::string> > qpss;
		qpss queue_option; //储存操作类型和名称
	private:
		std::pair<std::string, std::string> parse_option(const std::string& op);
		//返回父节点
		iterator get_parent_node(iterator root);
		//返回当前节点
		iterator get_this_node(iterator root);
		/**
		 * 获取当前节点文本,如果不是文本节点或者节点文本为空则返回空串.
		 */
		std::string get_text_from_this(iterator root);
		/**
		 * 获取当前节点所有后代节点的文本.
		 */
		std::vector<std::string> get_texts_from_genera(iterator root);
		//根据属性 值 名称 来选择节点
		iterator get_node_by_attrValue_any(iterator root, const std::string& attrName, const std::string& attrValue);
		iterator get_node_by_attrValue_name(iterator root, const std::string& attrName, const std::string& attrValue, const std::string& nodeName);
		iterator get_node_by_attrValue(const std::string& name, iterator root);
		//根据属性 in过程 选择节点
		iterator get_node_by_attr_and_name(const std::string& name, iterator root);
		// 通过数组选择与名字相同的节点
		iterator get_node_by_array_and_name(const std::string& name, iterator root);
		//选择当前元素子代元素中的name元素
		iterator get_node_from_child_by_name(const std::string& name, iterator root);
		//选择当前元素后代元素中的name元素
		std::vector<iterator> get_node_from_genera_by_name(const std::string& name, iterator root);
		std::string get_attr_from_this(const std::string& name, iterator root);
		//将操作入队 双指针算法入队
		bool get_option(const std::string& exp);
		//处理xpath操作
		bool do_option(iterator root, NodeResult& result);
	};
};
#endif