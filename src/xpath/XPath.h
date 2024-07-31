#ifndef __XPATH__H_
#define __XPATH__H_

#include "XPathResult.h"
#include "html/Node.h"
#include "html/tree.h"
#include <queue>

namespace xpath {
	class XPath
	{
	public:
	public:
		XPath(const tree<htmlcxx::HTML::Node>& tree, const std::string& html);
		XPath(const std::string& html);

		XPathResult get(const XPathExpression& exp);
	private:
		tree<htmlcxx::HTML::Node> m_tree;
		std::string m_html;
		XPathResult m_result;
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
		std::vector<iterator> get_node_by_attr_and_name(const std::string& name, iterator root);
		// 通过数组选择与名字相同的节点
		iterator get_node_by_array_and_name(const std::string& name, iterator root);
		//选择当前元素子代元素中的name元素
		iterator get_node_from_child_by_name(const std::string& name, iterator root);
		//选择当前元素后代元素中的name元素
		std::vector<iterator> get_node_from_genera_by_name(const std::string& name, iterator root);

		std::string get_attr_from_this(const std::string& name, iterator root);
		//处理xpath操作
		//bool do_option(pre_order_iterator root);
		bool do_option(iterator root);
	};
}
#endif