#include "ParserSax.h"


/**
 * 计算UTF8BOM头.
 */
size_t utf8BomOffset(const std::string& html)
{
	if (html.compare(0, 3, "\xEF\xBB\xBF") == 0) {
		return 3;
	}
	return 0;
}

void htmlcxx::HTML::ParserSax::parse(const std::string &html)
{
//	std::cerr << "Parsing string" << std::endl;
	parse(html.data() + utf8BomOffset(html), html.data() + html.length());
	//kparse(html.data(), html.data() + html.length());
}
