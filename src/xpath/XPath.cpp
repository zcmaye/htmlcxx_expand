#include "XPath.h"
#include "html/ParserDom.h"

#include <queue>
#include <list>
#include <iostream>

#define CHECK(it, ret)\
    if ((it) == nullptr) return ret
#define CHECK_NODE_NAME(name,ret)\
    if(name.empty()) {\
        m_errorString = "The xpath path expression syntax is incorrect!";\
        return ret;\
    }

// child 子
// genera 后
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
    std::pair<std::string, std::string> XPath::parse_option(const std::string &op)
    {
        std::pair<std::string, std::string> ret;
        // 取后代
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
            // 取子代
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
                        if (op[0] == '*')
                            ret = {options[7], op};
                        else
                            ret = {options[7], op.substr(1)};
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
        // std::cout << ret.first << " " << ret.second << std::endl;
        return ret;
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
        //当前标签内容开始位置
        auto off = root->offset() + root->text().size();
        //结尾标签的位置
        auto closePos = root->offset() + root->length() - root->closingText().size();

        //跳过BOM头
        if (off == 0 && m_html.compare(0,3,"\xEF\xBB\xBF") == 0) {
            off += 3;
        }
        //跳过空格
        while (m_html[off] == ' ') { ++off; }
        //while (m_html[closePos] == ' ') { --closePos; }
        //内容长度
        auto count = closePos - off;

        //不是文本节点
        if (m_html[off] == '<') { 
            return {};
        }
        return m_html.substr(off, count);
    }
    // 获取当前节点后节点的所有文本
    std::vector<std::string> XPath::get_texts_from_genera(iterator root)
    {
        CHECK(root, {});
        
        std::vector<std::string> ret;
        tree<HTML::Node>::pre_order_iterator begin(root);
		tree<HTML::Node>::pre_order_iterator end(++root);
        for (auto it = begin; it != end; it++) {
            if (it->isTag()) {
                auto text = get_text_from_this(it);
                if(!text.empty())
					ret.push_back(text);
				//std::cout <<  " 【" << get_text_from_this(it)<<"】" << std::endl;
            }
        }
        return ret;
    }

    // 根据属性 值 名称 来选择节点
    iterator XPath::get_node_by_attrValue_any(iterator root, const std::string &attrName, const std::string &attrValue)
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

    iterator XPath::get_node_by_attrValue_name(iterator root, const std::string &attrName, const std::string &attrValue, const std::string &nodeName)
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

    std::string get_attr_name(const std::string& name);
    std::string get_attr_value(const std::string& name);
    std::string get_node_name(const std::string& name);


    std::string get_attr_name(const std::string& name)
    {
        std::string s;
        int status = 0;
        for (auto it = name.begin(); it != name.end(); it++)
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
    std::string get_attr_value(const std::string& name)
    {
        std::string s;
        int status = 0;
        for (auto it = name.begin(); it != name.end(); it++)
        {
            if (*it == '=') {
                if (++it == name.end()) {
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
    std::string get_node_name(const std::string& name)
    {
		std::string s;
		for (auto it = name.begin(); it != name.end(); it++) {
			if (*it == '[') {
				break;          //解析完毕
            }
			s.push_back(*it);
        }
        return s;
    }

    iterator XPath::get_node_by_attrValue(const std::string &name, iterator root)
    {
        CHECK(root, nullptr);
        CHECK_NODE_NAME(name, {});
        //*[@data-rid]
        //*[@id="j-catalogWrap"]
        //std::string attr_name = name.substr(name.find("@") + 1, name.find("=") - name.find("@") - 1);
        //std::string value_name = name.substr(name.find("=") + 1, name.find("]") - name.find("=") - 1);
        //std::string node_name = name.substr(0, name.find("["));
        std::string attr_name = get_attr_name(name);
        std::string value_name = get_attr_value(name); 
        std::string node_name =  get_node_name(name);
        // std::cout << attr_name << " " << node_name << " " << value_name << std::endl;
        if (node_name == "*")
            return get_node_by_attrValue_any(root, attr_name, value_name);
        return get_node_by_attrValue_name(root, attr_name, value_name, node_name);
    }
    // 根据属性 in过程 选择节点
    iterator XPath::get_node_by_attr_and_name(const std::string &name, iterator root)
    {
        CHECK(root, nullptr);
        CHECK_NODE_NAME(name, {});
        std::string attr_name = name.substr(name.find("@") + 1, name.find("]") - name.find("@") - 1);
        std::string node_name = name.substr(0, name.find("["));
        for (auto it = root.begin(); it != root.end(); it++)
        {
            it->parseAttributes();
            if (it->tagName() == node_name)
            {
                for (auto &item : it->attributes())
                {
                    if (item.first == attr_name)
                        return it;
                }
            }
        }
        return nullptr;
    }
    // 通过数组选择与名字相同的节点
    iterator XPath::get_node_by_array_and_name(const std::string &name, iterator root)
    {
        CHECK(root, nullptr);
        CHECK_NODE_NAME(name, {});

        int j = *(name.substr(name.find("[") + 1, name.find("]") - name.find("[")).c_str()) - '0';
        std::string tmp_name = name.substr(0, name.find("["));
        // std::cout << tmp_name << " " << j << " " << std::endl;
        int i = 0;

        // it == child
        for (auto it = root.begin(); it != root.end(); it++)
        {
            if (it->tagName() == tmp_name)
            {
                i++;
                if (i == j)
                    return it;
            }
        }
        return nullptr;
    }
    // 选择当前元素子代元素中的name元素
    iterator XPath::get_node_from_child_by_name(const std::string &name, iterator root)
    {
        CHECK(root, nullptr);
        CHECK_NODE_NAME(name, {});

        for (auto it = root.begin(); it != root.end(); it++)
        {
            if (it->tagName() == name)
                return it;
        }
        return nullptr;
    }

    // 选择当前元素后代元素中的name元素
    //-- //title[@lang='eng'] 选取所有 title 元素，且这些元素拥有值为 eng 的 lang 属性。
    /*iterator XPath::get_node_from_genera_by_name(const std::string& name, iterator root)
    {
        CHECK(root, nullptr);
        CHECK_NODE_NAME(name, {});

        std::string attr_name = get_attr_name(name);
        std::string value_name = get_attr_value(name); 
        std::string node_name =  get_node_name(name);


        // 有属性的情况
        //-- //*[@id="chapter-50733113105954003"];
        if (name.find('@') != std::string::npos)
        {
            return get_node_by_attrValue(name, root);
        }
        // 没有属性的情况
        // 遍历树
        tree<HTML::Node>::pre_order_iterator begin(root);
        //tree<HTML::Node>::pre_order_iterator end = m_tree.end(); // (nullptr);
        tree<HTML::Node>::pre_order_iterator end(++root);
        for (auto it = begin; it != end; it++)
        {
            if (it->isTag())
            {
                if (it->tagName() == name)
                {
                    // std::cout << it->tagName() << "  " << getNodeContent(*it) << std::endl;
                    return it;
                }
            }
        }
        return nullptr;
    }*/
	std::vector<iterator> XPath::get_node_from_genera_by_name(const std::string& name, iterator root)
    {
        CHECK(root, {});
        CHECK_NODE_NAME(name, {});

        std::vector<iterator> its;
        std::string attr_name = get_attr_name(name);
        std::string attr_value = get_attr_value(name); 
        std::string node_name =  get_node_name(name);

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
        CHECK(root, nullptr);
        CHECK_NODE_NAME(name, {})

        std::string attr_name = name.substr(name.find("@") + 1);
        root->parseAttributes();
        for (auto &m : root->attributes())
        {
            if (m.first == attr_name)
                return m.second;
        }
        return {};
    }
    // 将操作入队 双指针算法入队
    bool XPath::get_option(const std::string &exp)
    {
        int l(0), r(0);
        int len = 0;

        while (len <= exp.length())
        {
            if (exp[0] != '/' && exp[0] != '*')
            {
                m_errorString = "Path expression error";
                return false;
            }
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
                // std::cout << tmp_option << " ";
                queue_option.push(parse_option(tmp_option));
            }
            len = r;
            l = r;
        }
        // std::string name = exp.substr(0, len);
        return true;
    }

    // 处理xpath操作
    bool XPath::do_option(iterator root, NodeResult&result)
    {
        CHECK(root, false);
        result.m_node = root;
        std::string ret_text;
        std::vector<std::string> texts;
        while (!queue_option.empty())
        {
            std::pair<std::string, std::string> op = queue_option.front();
            queue_option.pop();
            std::string option = op.first;
            std::string name = op.second;
            // std::cout << option << " " << name << std::endl;
            switch (str2int(option.data()))
            {
            case str2int("get_node_from_genera_by_name"):
				result.m_its = get_node_from_genera_by_name(name, result.m_node);
				result.m_node = result.m_its.empty() ? nullptr : result.m_its.front();
                break;
            case str2int("get_node_from_child_by_name"):
                result.m_node = get_node_from_child_by_name(name, result.m_node);
				result.m_its = { result.m_node };
                break;
            case str2int("get_node_by_array_and_name"):
                result.m_node = get_node_by_array_and_name(name, result.m_node);
                break;
            case str2int("get_node_by_attr_and_name"):
                result.m_node = get_node_by_attr_and_name(name, result.m_node);
                break;
            case str2int("get_node_by_attrValue_and_name"):
                result.m_node = get_node_by_attrValue(name, result.m_node);
                break;
            case str2int("get_text_from_this"):
                ret_text = get_text_from_this(result.m_node);
                result.m_texts.push_back(ret_text);
				//result.m_its = { result.m_node};
                result.m_its.clear();
                result.m_node = nullptr;
                return true;
            case str2int("get_texts_from_genera"):
                texts = get_texts_from_genera(result.m_node);
                result.m_texts = texts;
				//result.m_its = { result.m_node};
                result.m_its.clear();
                result.m_node = nullptr;
                return true;
            case str2int("get_this_node"):
                result.m_node = get_this_node(result.m_node);
                break;
            case str2int("get_parent_node"):
                result.m_node = get_parent_node(result.m_node);
                break;
            case str2int("get_attr_from_this"):
                ret_text = get_attr_from_this(name, result.m_node);
                result.m_texts.push_back(ret_text);
				result.m_its = { result.m_node };
                return true;
            default:
                return false;
            }
            if (result.m_node == nullptr) {
                result.m_its.clear();
                return false;
            }
        }
        if (result.empty() && result.m_node != nullptr)
            result.m_its.push_back(result.m_node);
        return true;
    }

    // 返回xpath解析内容
    NodeResult XPath::xpath(const std::string &exp)
    {
        m_errorString = "No Error";
        NodeResult result(nullptr);
        const char *ptr = exp.c_str();
        try
        {
            get_option(exp);
            if (!do_option(m_tree.begin(), result))
            {
				m_errorString = "Not Found:<" + exp + ">";
                return NodeResult(nullptr);
            }
        }
        catch (const std::exception & e)
        {
            m_errorString = e.what();
            return NodeResult(nullptr);
        }
        return result;
    }

    std::pair<bool,std::string> XPath::getNodeContent(const htmlcxx::HTML::Node &node)
    {
        //当前标签内容开始位置
        auto off = node.offset() + node.text().size();
        //结尾标签的位置
        auto closePos = node.offset() + node.length() - node.closingText().size();

        //跳过空格
        while (m_html[off] == ' ') { ++off; }

        //内容长度
        auto count = closePos - off;

        //不是文本节点
        if (m_html[off] == '<') { 
            return { false,"Not text node" };
        }
        return { true,m_html.substr(off, count) };
    }

    XPath::XPath(const tree<HTML::Node> &tree, const std::string &html)
        : m_tree(tree), m_html(html)
        , m_errorString("No Error")
    {
    }
    XPath::XPath(const std::string &html)
        : m_html(html)
        , m_errorString("No Error")
    {
        htmlcxx::HTML::ParserDom dom;
        m_tree = dom.parseTree(html);
    }

};
