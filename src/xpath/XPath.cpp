#include "XPath.h"
#include "html/ParserDom.h"

#include <queue>
#include <list>
#include <iostream>

#define Node(it) (it.node->data)

//child 子
//genera 后
const std::string options[] = {
    "get_parent_node",                // /.. 选择父元素 ✅
    "get_this_node",                  // /. 选择当前元素 ✅
    "get_all_nodes",                  // /* 匹配任意元素
    "get_node_from_genera_by_name",   // //name 选择当前元素后代元素中的name元素 ✅
    "get_node_from_child_by_name",    // /name 选择当前元素子代元素中的name元素 ✅
    "get_node_by_array_and_name",     // /name[n] 选择当前元素下第n个name元素 ✅
    "get_node_by_attr_and_name",      // /name[@attr] 属性筛选，选择有attr属性的元素 ✅
    "get_node_by_attrValue_and_name", // /name[@attr=value] 属性筛选，选择attr属性的值为value的name元素(属性值不加分号) ✅
    "get_text_from_this",             // /text() 返回当前元素中的文本 ✅
    "get_texts_from_genera",          // //text() 返回当前元素以及它所有后代元素中的文本 ✅
    "get_attr_from_this",             // /@attr 返回当前元素attr属性的值 ✅
    "get_all_attr"                    // @* 匹配任意属性};
};

namespace htmlcxx 
{
size_t maxLength = 10000;
constexpr unsigned int str2int(const char *str, int h = 0)
{
    return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}
std::pair<std::string, std::string> XPath::parse_option(const std::string& op)
{
    std::pair<std::string, std::string> ret;
    //取后代
    if (op.find("//") < maxLength)
    {

        if (op.find("text()") < maxLength)
        {
            ret = {options[9], op.substr(2)};
        }
        else
        {
            ret = {options[3], op.substr(2)};
        }
    }
    else
    {
        //取子代
        if (op.find("..") < maxLength)
        {
            ret = {options[0], ""};
        }
        else if (op.find(".") < maxLength)
        {
            ret = {options[1], ""};
        }
        else if (op.find("[") < maxLength)
        {
            if (op.find("@") < maxLength)
            {
                if (op.find("=") < maxLength)
                {
                    if(op[0] == '*')
						ret = { options[7], op };
                    else
						ret = { options[7], op.substr(1) };
                }
                else
                {
                    ret = {options[6], op.substr(1)};
                }
            }
            else
            {
                ret = {options[5], op.substr(1)};
            }
        }
        else if (op.find("@") < maxLength)
        {
            ret = {options[10], op.substr(1)};
        }
        else if (op.find("text()") < maxLength)
        {
            ret = {options[8], op.substr(1)};
        }
        else
        {
            ret = {options[4], op.substr(1)};
        }
    }
    //std::cout << ret.first << " " << ret.second << std::endl;
    return ret;
}
//返回父节点
iterator XPath::get_parent_node(iterator root)
{
    return root.parent_;
}
//返回当前节点
iterator XPath::get_this_node(iterator root)
{
    return root;
}
//获取当前节点文本
std::string XPath::get_text_from_this(iterator root)
{
	//获取文本内容
	auto off = Node(root).offset() + Node(root).text().size();
	auto count = Node(root).length() - Node(root).text().size() - Node(root).closingText().size();
	return m_html.substr(off, count);
}
//获取当前节点后节点的所有文本
std::string XPath::get_texts_from_genera(iterator root)
{
    std::pair<iterator, bool> d;
    std::queue<iterator> q;
    q.push(root);
    std::string ret;
    while (!q.empty())
    {
        auto p = q.front();
        q.pop();
        for(auto it = p.begin();it!=p.end();it++)
        {
			ret.append(it->text());
            q.push(it);
        }
    }
    return ret;
}


//根据属性 值 名称 来选择节点
iterator XPath::get_node_by_attrValue_any(iterator root,const std::string& attrName,const std::string&attrValue,const std::string&nodeName)
{
    auto noQuoteAttrValue =  attrValue.substr(1, attrValue.size() - 2);

    tree<HTML::Node>::pre_order_iterator begin(root);
    tree<HTML::Node>::pre_order_iterator end = m_tree.end(); //(nullptr);
	for (auto it = begin; it != end; it++) {
        if (it == nullptr) {
            std::cout << " == nullptr" << std::endl;
        }
		if (it->isTag()) {
			it->parseAttributes();
			auto pr = it->attribute(attrName);
			if (!pr.first)
				continue;
            if (pr.second.find("catalog.html") != std::string::npos) {
                std::cout << "found it？" << std::endl;
            }
			//std::cout << it->tagName() << "  " << pr.second << std::endl;
			if (pr.second == attrValue) {
				return it;
			}
			if (pr.second == noQuoteAttrValue) {
				return it;
			}
        }
	}

    return nullptr;
    //for (auto& node : m_tree) 
	for (auto it = m_tree.begin(); it != m_tree.end(); it++)
    {
        if (!it->isTag())
            continue;
        it->parseAttributes();
        auto pr = it->attribute(attrName);
        if (!pr.first)
            continue;
        if (pr.second == attrValue) {
            return it;
        }
        if (pr.second == noQuoteAttrValue) {
            return it;
        }
    }
    return nullptr;
}

iterator XPath::get_node_by_attrValue_name(iterator root,const std::string& attrName,const std::string&attrValue,const std::string&nodeName)
{
	for (auto it = root.begin(); it != root.end(); it++) {
        if (!it->isTag())
            continue;
        it->parseAttributes();
        //获取属性
		auto pr = it->attribute(attrName);
        //没有查找的属性
        if (pr.first) {
            continue;
        }
        //判断属性是否相等
		if (pr.second == attrName) {
            //如果标签名为* 或者 等于指定的标签，则直接返回
            if (it->tagName() == nodeName) {
                return it;
            }
		}
	}
    return nullptr;
}

iterator XPath::get_node_by_attrValue(const std::string& name, iterator root)
{    
    if (root == nullptr)
        return nullptr;

    std::string attr_name = name.substr(name.find("@") + 1, name.find("=") - name.find("@") - 1);
    std::string value_name = name.substr(name.find("=") + 1, name.find("]") - name.find("=") - 1);
    std::string node_name = name.substr(0, name.find("["));
    //std::cout << attr_name << " " << node_name << " " << value_name << std::endl;
    if (node_name == "*")
       return get_node_by_attrValue_any(root,attr_name, value_name, node_name);
	return get_node_by_attrValue_name(root, attr_name, value_name, node_name);
}
//根据属性 in过程 选择节点
iterator XPath::get_node_by_attr_and_name(const std::string& name, iterator root)
{
	std::string attr_name = name.substr(name.find("@") + 1, name.find("]") - name.find("@") - 1);
	std::string node_name = name.substr(0, name.find("["));
	for (auto it = root.begin(); it != root.end(); it++)
	{
		it->parseAttributes();
		if (it->tagName() == node_name) {
			for (auto& item : it->attributes()) {
				if (item.first == attr_name)
					return it;
			}
		}
	}
	return nullptr;
}
// 通过数组选择与名字相同的节点
iterator XPath::get_node_by_array_and_name(const std::string& name, iterator root)
{
	int j = *(name.substr(name.find("[") + 1, name.find("]") - name.find("[")).c_str()) - '0';
	std::string tmp_name = name.substr(0, name.find("["));
	//std::cout << tmp_name << " " << j << " " << std::endl;
	int i = 0;

	//it == child
	for (auto it = root.begin(); it != root.end(); it++)
		//for (auto child : root->children)
	{
		//std::cout << child->name << std::endl;
		if (it->tagName() == tmp_name) {
			i++;
			if (i == j)
				return it;
		}
	}
	return nullptr;
}
//选择当前元素子代元素中的name元素
iterator XPath::get_node_from_child_by_name(const std::string& name, iterator root)
{
	if (root.begin() == root.end())
		return nullptr;

	for (auto it = root.begin(); it != root.end(); it++)
	{
		if (it->tagName() == name)
			return it;
	}
	return nullptr;
}

//选择当前元素后代元素中的name元素
iterator XPath::get_node_from_genera_by_name(const std::string& name, iterator root)
{
    if (root == nullptr)
        return nullptr;

    //有属性的情况
	//-- //*[@id="chapter-50733113105954003"];
	if (name.find('@') != std::string::npos) {
		return get_node_by_attrValue(name, root);
    }
    //没有属性的情况

    //遍历树
    tree<HTML::Node>::pre_order_iterator begin(root);
    tree<HTML::Node>::pre_order_iterator end = m_tree.end();// (nullptr);
	for (auto it = begin; it != end; it++) {
        if (it->isTag()) {
            if (it->tagName() == name)
            {
				//std::cout << it->tagName() << "  " << getNodeContent(*it) << std::endl;
                return it;
            }
        }
	}
	return nullptr;
}

std::string XPath::get_attr_from_this(const std::string& name, iterator root)
{
	std::string attr_name = name.substr(name.find("@") + 1);
	root->parseAttributes();
	for (auto& m : root->attributes())
	{
		if (m.first == attr_name)
			return m.second;
	}
	return {};
}
//将操作入队 双指针算法入队
bool XPath::get_option(const std::string& exp)
{
	int l(0), r(0);
	int len = 0;

	while (len <= exp.length())
	{

		if (exp[len] == '/' || (exp[len] == '*' && len == 0))
		{
			if (exp[len + 1] == '/')
				r = l + 2;
			else
				r = l + 1;
			while (r <= exp.length())
			{
				if (exp[r] == '/')
					break;
				r++;
			}
			std::string tmp_option = exp.substr(l, r - l);
			//std::cout << tmp_option << " ";
			queue_option.push(parse_option(tmp_option));
		}
		len = r;
		l = r;
	}
	//std::string name = exp.substr(0, len);
	return true;
}

//处理xpath操作
bool XPath::do_option(iterator root, iterator& result)
{
	iterator node = root;
	std::string ret_text;
	while (queue_option.empty() == false)
	{
		std::pair<std::string, std::string> op = queue_option.front();
		queue_option.pop();
		std::string option = op.first;
		std::string name = op.second;
		//std::cout << option << " " << name << std::endl;
		switch (str2int(option.data()))
		{
		case str2int("get_node_from_genera_by_name"):
			node = get_node_from_genera_by_name(name, node);
			result = node;
			break;
		case str2int("get_node_from_child_by_name"):
			node = get_node_from_child_by_name(name, node);
			result = node;
			break;
		case str2int("get_node_by_array_and_name"):
			node = get_node_by_array_and_name(name, node);
			result = node;
			break;
		case str2int("get_node_by_attr_and_name"):
			node = get_node_by_attr_and_name(name, node);
			result = node;
			break;
		case str2int("get_node_by_attrValue_and_name"):
			node = get_node_by_attrValue(name, node);
            result = node;
            break;
        case str2int("get_text_from_this"):
            ret_text = get_text_from_this(node);
            result = node;
            return true;
        case str2int("get_texts_from_genera"):
            ret_text = get_texts_from_genera(node);
            result = node;
            return true;
        case str2int("get_this_node"):
            node = get_this_node(node);
            result = node;
            break;
        case str2int("get_parent_node"):
            node = get_parent_node(node);
            result = node;
            break;
        case str2int("get_attr_from_this"):
            ret_text = get_attr_from_this(name, node);
            result = node;
            return true;
        default:
            return false;
        }
        if (result == nullptr) {
			m_errorString = "node not found <" + name + ">";
            return false;
        }
    }
    return true;
}

//返回xpath解析内容
NodeResult XPath::xpath(const std::string& exp)
{
    iterator result;
    const char *ptr = exp.c_str();
    try
    {
        get_option(exp);
        if (!do_option(m_tree.begin(), result)) {
			return NodeResult(nullptr);
        }
    }
    catch (const std::exception &)
    {
        return NodeResult(nullptr);
    }
    return result;
}

std::string XPath::getNodeContent(const htmlcxx::HTML::Node& node)
{
	auto off = node.offset() + node.text().size();
	auto count = node.length() - node.text().size() - node.closingText().size();

    auto pos = m_html.find('<', off);
    if (pos != std::string::npos) {
        return m_html.substr(off, pos - off);
    }
    return m_html.substr(off, count);
}

XPath::XPath(const tree<HTML::Node>& tree,const std::string& html)
    :m_tree(tree)
    ,m_html(html)
{
}
XPath::XPath(const std::string& html)
    : m_html(html)
{
    htmlcxx::HTML::ParserDom dom;
    m_tree = dom.parseTree(html);
}

};
