# pans

### 简介

[古法编程]现代C++实现高性能服务器框架

[视频链接(bilibili)](https://www.bilibili.com/video/BV1v9GK6CEVK)

根据视频要求, 该项目仅在学习解疑时使用AI查询所需知识并参考, 不会使用AI直接生成代码

更详细的文档参见`./docs`

### cmake常用命令速查

- `cmake -S <path1> -B <path2>`: 读取path1下的CMakeLists.txt, 构建并放在path2下. eg: `cmake -S . -B build` `-DXXX=yyy`可以给cmake提供标志, 例如切换构建方式: `-DCMAKE_BUILD_TYPE=Release`
- `cmake --build <path>`: 运行path下的构建文件, 就不用cd进去再make了

### 进度 

- 2026-10-15 进度至5p
- 2026-10-16 进度至6p, 学习线程互斥与同步的概念, 并在cpp中实践
- 2026-10-17 进度至8p, 学习并封装了两种自旋锁,并测试其性能, 将其与std::mutex对比; 搭建了日志系统的一部分, 内容很难理解 
- 2026-10-18 进度至10p, 大概理解了昨天的内容, 完善了自己的缓冲区实现, 并将其与std::stringstream对比测试, 初步搭建起日志框架.
