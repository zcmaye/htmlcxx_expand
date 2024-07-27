# htmlcxx_expand
htmlcxx_expand是一个HTML解析器，支持xpath，可以很方便的解析HTML。

+ xpath解析部分参考了[charto/cxml](https://github.com/charto/cxml)，并进行了修改
+ html解析部分使用了[htmlcxx](https://htmlcxx.sourceforge.net/)

# 构建

1. 从github下载源码

```shell
https://github.com/zcmaye/htmlcxx_expand.git
```

2. 进入源码目录

```shell
cd HtmlCxx_expand
```

3. 执行cmake指令

```cmake
cmake -B build												 	#默认在C:/Program Files (x86)目录中
cmake -B build -DCMAKE_INSTALL_PREFIX="C:\Users\Maye\Desktop"	#可以自己指定安装目录
```

4. 接着执行

```cmake
cmake --build build --target INSTALL						# 默认debug
cmake --build build --target INSTALL --config "Release"		# Release
```

注意：默认会安装到到C盘（C盘还必须要有权限，也就是必须用管理员打开cmd），可以自己剪切到自己专门放库的目录去,

```shell
  -- Install configuration: "Release"
  -- Installing: C:/Program Files (x86)/CppHtml_expand/include/html/Extensions.h
  -- Installing: C:/Program Files (x86)/CppHtml_expand/include/html/Node.h
  -- Installing: C:/Program Files (x86)/CppHtml_expand/include/html/ParserDom.h
  -- Installing: C:/Program Files (x86)/CppHtml_expand/include/html/ParserSax.h
  -- Installing: C:/Program Files (x86)/CppHtml_expand/include/html/Uri.h
```

