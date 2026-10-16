# pans

### 简介

[古法编程]现代C++实现高性能服务器框架

[视频链接(bilibili)](https://www.bilibili.com/video/BV1v9GK6CEVK)

根据视频要求, 该项目仅在学习解疑时使用AI查询所需知识并参考, 不会使用AI直接生成代码

更详细的文档参见`./docs`

### cmake常用命令速查

- `cmake -S <path1> -B <path2>`: 读取path1下的CMakeLists.txt, 构建并放在path2下. eg: `cmake -S . -B build`
- `cmake --build <path>`: 运行path下的构建文件, 就不用cd进去再make了

### 进度 

- 2026-10-15 进度至5p
- 2026-10-16 进度至6p, 学习线程互斥与同步的概念, 并在cpp中实践