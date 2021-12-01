# learn-cef
学习cef

# cef介绍
CEF 是一个 BSD 许可的开源项目，由 Marshall Greenblatt 于 2008 年创立，基于Google Chromium项目。与 Chromium 项目本身主要侧重于 Google Chrome 应用程序开发不同，CEF 侧重于促进第三方应用程序中的嵌入式浏览器用例。CEF 通过提供生产质量稳定的 API、跟踪特定 Chromium 版本的发布分支和二进制分发，将用户与底层 Chromium 和 Blink 代码复杂性隔离开来。CEF 中的大多数功能都有默认实现，可提供丰富的功能，同时几乎不需要用户进行集成工作。目前，全球有超过 1 亿个已安装的 CEF 实例嵌入到来自各种公司和行业的产品中。CEF 维基百科页面上提供了部分使用 CEF 的公司和产品列表。CEF 的一些用例包括：

在现有的本机应用程序中嵌入符合 HTML5 的 Web 浏览器控件。
创建一个轻量级的本机“外壳”应用程序，该应用程序承载主要使用 Web 技术开发的用户界面。
在具有自己的自定义绘图框架的应用程序中“离屏”呈现 Web 内容。
充当对现有 Web 属性和应用程序进行自动化测试的主机。
CEF 支持多种编程语言和操作系统，并且可以轻松集成到新的和现有的应用程序中。它的设计从头开始就兼顾了性能和易用性。基础框架包括通过本机库公开的 C 和 C++ 编程接口，这些接口将主机应用程序与 Chromium 和 Blink 实现细节隔离开来。它提供浏览器和主机应用程序之间的紧密集成，包括对自定义插件、协议、JavaScript 对象和 JavaScript 扩展的支持。主机应用程序可以选择控制资源加载、导航、上下文菜单、打印等，同时利用 Google Chrome Web 浏览器中可用的相同性能和 HTML5 技术。

# 编译
- [在此](https://cef-builds.spotifycdn.com/index.html)根据自己的平台下载cef开发包（选择Standard Distribution）
- 解压cef开发包后重命名文件夹为cef，并移动到learn-cef/third_party目录
- 在learn-cef根目录使用以下命令即可编译：（具体不同平台的编译环境要求和编译步骤（包括不同平台项目生成）可以参考[这里](https://github.com/chromiumembedded/cef/blob/f6cf7f9ec7de6f868f04f8d680faeb6c53d4d813/CMakeLists.txt.in#L35)）
    ```
    mkdir build && cd build
    cmake ..
    cmake --build . -j8
    ```

# cef目录介绍
```
cef
    - cmake 辅助构建项目的cmake脚本
    - include cef头文件
    - libcef cef核心实现
    - libcef_dll cef的cpp封装
    - tests 示例代码
    - tools 工具脚本
```

# 参考文档
- [cef github源码](https://github.com/chromiumembedded/cef)
- [cef 官方文档](https://bitbucket.org/chromiumembedded/cef/wiki/Home)
- [cef 开发包下载地址](https://cef-builds.spotifycdn.com/index.html)

- [初学必读：如何使用cef创建简单应用（cefsimple详细教程）](https://bitbucket.org/chromiumembedded/cef/wiki/Tutorial.md)
- [初学必读：cef一般用法（cefclient详细教程）](https://bitbucket.org/chromiumembedded/cef/wiki/GeneralUsage.md)
- [初学必读：官方js集成教程](https://bitbucket.org/chromiumembedded/cef/wiki/JavaScriptIntegration.md)
- [一起学libcef](https://blog.csdn.net/wangshubo1989/category_6004479.html)
- [Qt浏览器开发：关于CEF开发知识点以及QCef开发原理与使用](https://blog.csdn.net/qq_36651243/article/details/115350668)

- [QtCefDemoCmake：cef嵌入Qt的简单Demo（基于窗口模式）(由于没有处理cef消息循环，所以显示不出网页，应该使用CefDoMessageLoopWork异步处理)](https://github.com/w4ngzhen/QtCefDemoCmake)
- [QtCef：cef嵌入Qt的完整点的Demo（基于窗口模式）](https://github.com/3dproger/QtCef)
- [QCefWidget：cef嵌入QWidget（支持窗口模式和离屏渲染模式）](https://github.com/winsoft666/QCefWidget)
- [CefViewCore：方便嵌入cef到其他UI框架的cef封装](https://github.com/CefView/CefViewCore)
- [QCefBrowser：基于CEF封装的Qt浏览器](https://github.com/KikyoShaw/QCefBrowser)
- [CEF3SimpleSample: 参考js集成相关代码](https://github.com/acristoffers/CEF3SimpleSample)