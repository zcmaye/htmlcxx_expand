#ifndef __HTML_PARSER_NODE_H
#define __HTML_PARSER_NODE_H

#include <map>
#include <vector>
#include <string>
//#include <utility>

namespace htmlcxx {
	namespace HTML
	{
		class Node {

			public:
				Node() {}
				//Node(const Node &rhs); //uses default
				~Node() {}

				/** 标签内容 */
				inline const std::string& content() const { return mContents.empty() ? "" : this->mContents.front(); }
				inline void content(const std::string& content) { this->mContents.push_back(content); }

				inline const std::vector<std::string>& contents() const { return this->mContents; }
				inline void appendContent(const std::string& content) {
					mContents.push_back(content); 
				}

				/** 开始标签完整文本(<a href="xxxxx">) */
				inline const std::string& text() const { return this->mText; }
				inline void text(const std::string& text) { this->mText = text; }

				/** 结尾标签 */
				inline const std::string& closingText() const { return mClosingText; }
				inline void closingText(const std::string &text) { this->mClosingText = text; }

				/** 标签在整个文档中的偏移量(首字符<) */
				inline unsigned int offset() const { return this->mOffset; }
				inline void offset(unsigned int offset) { this->mOffset = offset; }

				/** 开始标签长度 如:<html class="oaded"> 长度为21  */
				inline unsigned int length() const { return this->mLength; }
				inline void length(unsigned int length) { this->mLength = length; }

				/** 标签名 */
				inline const std::string& tagName() const { return this->mTagName; }
				inline void tagName(const std::string& tagname) { this->mTagName = tagname; }

				/** 判断是不是标签节点(还有注释) */
				void isTag(bool is_html_tag){ this->mIsHtmlTag = is_html_tag; }
				bool isTag() const { return this->mIsHtmlTag; }

				/** 判断是不是注释节点(还有标签) */
				void isComment(bool comment){ this->mComment = comment; }
				bool isComment() const { return this->mComment; }

				/** 判断是不是文本节点(文本节点也有可能是标签节点，包含文本的节点就是文本节点) */
				bool hasContent() const { return !mContents.empty(); }

				std::pair<bool, std::string> attribute(const std::string &attr) const
				{ 
					std::map<std::string, std::string>::const_iterator i = this->mAttributes.find(attr);
					if (i != this->mAttributes.end()) {
						return make_pair(true, i->second);
					} else {
						return make_pair(false, std::string());
					}
				}

				/** 如果是标签节点则返回标签名，否则返回标签文本 */
				operator std::string() const;
				std::ostream &operator<<(std::ostream &stream) const;

				/** 获取标签所有属性，必须先调用parAttributes解析属性 */
				const std::map<std::string, std::string>& attributes() const { return this->mAttributes; }
				/** 解析标签属性 */
				void parseAttributes();

				bool operator==(const Node &rhs) const;

			protected:
				std::vector<std::string> mContents;						/*!> 节点内容*/
				std::string mText;										/*!> 标签完整文本(<a href="xxxxx">)*/
				std::string mClosingText;								/*!> 结尾标签文本(</a>)*/
				unsigned int mOffset;									/*!> 标签在文档中的偏移量*/
				unsigned int mLength;									/*!> 标签完整长度(mText.size())*/
				std::string mTagName;									/*!> 标签名(a、p、div等)*/
				std::map<std::string, std::string> mAttributes;			/*!> 标签所有属性*/
				bool mIsHtmlTag;										/*!> 是否是标签节点*/
				bool mComment;											/*!> 是否是注释节点*/
		};
	}
}

#endif
