# GFP-OpenWorld

Graphic Final Project - Open World

# 开发

## 环境

使用 Visual Studio 2022 开发，请手动导入库文件并修改 VS 配置。

新建/删除文件时注意将其包括进项目/从项目移除。相关问题，例如编译不存在的文件，目录中文件未纳入编译等，通过 `GFP-OpenWorld.vcxproj` 排查。

模型等可以使用相对路径，当前位置即 `main.cpp` 所在位置；
include 文件夹有 `~/include` 和 `~/src` 两个，前者为外部库，后者为此次开发内容。

## 文件结构

```
.
├─ include                         // 头文件
├─ lib                             // 库文件
├─ resource                        // 模型/材质等资源
│  ├─ model
│  ├─ texture 
│       └─ skybox
├─ shader                          // 着色器
│  ├─ fragment
│  └─ vertex
├─ src                             // 主要修改的源代码部分
│  ├─ animation                    // 刚体动画
│  ├─ app                          // 最终会使用类 Application 运行应用。可以在这里添加不同的 app 以进行单元测试
│  │  ├─ application.hpp/cpp       // 基类
│  ├─ model
│  │  ├─ model.hpp/cpp             // 基本模型类（其他类别可以添加物体特性/物理等（？））
│  ├─ utils                        // TODO: 目前基本采用 CG-Project 的文件
│  ├─ test                         // 神秘 test，大家随意发挥
```

## 开发流程

先完成基本脚手架的搭建，完成基本模型导入、渲染、OpenGL 框架等，形成固定工作流。

然后，整体的作业完成流程应该就是不断写新的 `model-shader-application`，例如相机、光源编辑、碰撞检测、文字渲染、物体交互等，最后会把所有要用到的 `model-shader` 合并到一个最终的 `application` 中。

可以参考 `./src/app/triangleApp` 和 `./src/app/initSceneApp`

但是需要注意写的同时需要合并测试，或者预留充足的时间做最后的合并调试。

### 单元开发

在 `src/model` 文件夹中创建一个模型类别（此处类别指物体、角色、场景、半透明物体等以行为区分的类别），在 `shader` 文件夹中创建相应着色器。可以在 `src/app` 中创建一个新的类别对此物体的行为进行测试。