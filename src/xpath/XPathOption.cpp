#include "XPathOption.h"
#include <assert.h>

namespace xpath {

	// child 子
	// genera 后
	static const XPathOption gOptions[] =
	{
		{"get_parent_node",                R"(/ ..选择父元素 ✅)"},
		{"get_this_node",                  R"(/. 选择当前元素 ✅)"},
		{"get_all_nodes",                  R"(/* 匹配任意元素)"},
		{"get_node_from_genera_by_name",   R"(//name 选择当前元素后代元素中的name元素 ✅)"},
		{"get_node_from_child_by_name",    R"(/name 选择当前元素子代元素中的name元素 ✅)"},
		{"get_node_by_array_and_name",     R"(/name[n] 选择当前元素下第n个name元素 ✅)"},
		{"get_node_by_attr_and_name",      R"(/name[@attr] 属性筛选，选择有attr属性的元素 ✅)"},
		{"get_node_by_attrValue_and_name", R"(/name[@attr=value] 属性筛选，选择attr属性的值为value的name元素(属性值不加分号) ✅)"},
		{"get_text_from_this",             R"(/text() 返回当前元素中的文本 ✅)"},
		{"get_texts_from_genera",          R"(//text() 返回当前元素以及它所有后代元素中的文本 ✅)"},
		{"get_attr_from_this",             R"(/@attr 返回当前元素attr属性的值 ✅)"},
		{"get_all_attr",                   R"(@* 匹配任意属性)" }
	};
	static const int OptionSize = sizeof(gOptions) / sizeof(gOptions[0]);


	const XPathOption* XPathOptions::option(size_t index)
	{
		assert(index >= 0 && index < OptionSize);
		return gOptions + index;
	}

	const XPathOption* XPathOptions::options()
	{

		return gOptions;
	}

	size_t XPathOptions::optionsSize()
	{
		return OptionSize;
	}


	std::ostream& operator<<(std::ostream& os, const XPathOption& option)
	{
		os << option.name << " : " << option.desc;
		return os;
	}
}
