# cpp代码格式规范

## 1. 命名规范

- 常量: 全大写(UPPER_SNAKE_CASE)
- 类名: 首字母大写, 驼峰(UpperCamelCase)
- 非静态成员函数: 首字母小写, 驼峰(lowerCamelCase)
- 静态成员函数: 首字母大写, 驼峰(UpperCamelCase)
- 非静态成员变量: 以m_开头, 首字母小写, 驼峰(m_ prefix lowerCamelCase)
- 静态成员变量: s_开头, 首字母小写, 驼峰(s_ prefix lowerCamelCase)
- 全局函数: 首字母大写, 驼峰(UpperCamelCase)
- 函数参数: 小写, 下划线分隔(snake_case)
- 局部变量: 小写, 下划线分隔(snake_case)

根据以上要求, 可以写出一套命名规范, 放在项目根目录的`.clang-tidy`下, 就可以让clang-format程序自动检查你的代码命名, 在clion IDE下开发体验更佳.

## 2. 格式规范

- 缩进: 4个空格
- 左花括号放在上一行末尾(Attach风格)

根据以上要求写出格式规范, 并放在项目根目录的`.clang-format`里