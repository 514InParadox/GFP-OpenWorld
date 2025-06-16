## 游戏介绍 & 游玩方法

本游戏是一个后室的整体流程，玩家需要在恐怖的后室中逃离实体的追击，找到前来帮助的米塔，拿到米塔给的武器之后击杀实体，再逃出后室来到外界。

## 环境 & 运行

使用 Visual Studio 2022 开发。

要编译运行整个项目，需要在 Visual Studio 中正确包含对应的文件，以及设置解决方案配置（部分配置已在 vcxproj 中提供），将项目文件夹中的 `lib` 和 `dll` 文件夹加入链接目录中；

为了方便，此处提供了 Windows 下编译的静态/动态库。如果不能直接运行，请自己手动编译。

## 系统 & 架构

本次大作业参考课程给出的 CG-projects 实现的框架。整体游戏外部通过 Application 封装，通过 handleInput, renderFrame 等函数进行 IO 交互，在 `src/app/final/finalSceneApp.cpp` 中实现实际逻辑。

在 Application 对象内部维护 Model、Skybox、GameState 等实例，在 `src/app/final/finalSceneApp.hpp` 中。

文件夹结构如下：

```
.
├─ dll                             // 动态链接库
├─ docs                            // 文档
├─ include                         // 头文件
├─ lib                             // 静态链接库
├─ resource                        // 模型/材质等资源
│  ├─ model                        // 物体模型
│  ├─ text                         // 文字模型
│  └─ texture                      // 材质
│       └─ skybox
├─ screenshot                      // 截图
├─ scripts                         // 脚本
├─ shader                          // 顶点/片段着色器
│  ├─ fragment
│  └─ vertex
├─ src                             // 主要修改的源代码部分
│  ├─ animation                    // 刚体动画
│  ├─ app                          // 整体应用包装类和各种继承的不同功能子类 
│  │  └─ final                     // 最终运行应用的内容，包括地图、实体等最终应用中物体的具体逻辑
│  ├─ model
│  │  └─ model.hpp/cpp             // 基本模型类，其中 model.cpp 是自行实现了 .obj 模型导入的，advancedModel.cpp 用以导入复杂模型
│  └─ utils                        // 功能文件，基本使用 CG-Projects 中的内容
└─ main.cpp                        // 入口文件
```

## 操作方法

+ 移动：WASD

+ 疾跑：按住 Shift

+ 射击：鼠标左键

+ 截图：P，会输出到本地 `screenshot` 文件夹下

## 实现要求

### 基本要求

+ [√] 基本体素建模表达能力：有模型。

+ [√] 具有三维网格导入导出功能：在 `src/model/model.cpp` 中实现了手动 .obj 导入

+ [√] 具有基本材质、纹理的显示和编辑能力：模型有材质。

+ [√] 具有基本几何变换功能：包括摄像机的移动、视角变换（`src/app/final/player.cpp` 和 `handleInput` 函数中）；米塔模型的跟随玩家位置（`src/app/final/finalSceneApp.cpp renderSceneToFrameBuffer()` 中 `draw mita`）等。

+ [√] 具有基本的光源照明功能，并实现基本的光源编辑：对 NPC 进行动态光源渲染；map 使用静态渲染烘培。

+ [√] 能对建模后场景进行漫游等观察功能：玩家可以移动。

+ [√] 能够提供动画播放功能：米塔和实体使用骨骼动画。具体实现在 `src/animation/` 中。

### EX

+ 构建了基于此引擎的三维游戏，具有可玩性

    + [√] 要求提出核⼼玩法并加以实现，对于⾮核⼼玩法的部分可从简：有整体游玩流程

    + [×] 实现场景编辑器，可以在场景中添加、删除、复制、编辑物体，可以保存和加载场景，以实现的正确性、易⽤性、功能性为评价标准

+ [?] 实现实时碰撞检测：只实现了射击时的包围盒检测（`src/app/final/finalSceneApp.cpp` 的 `handleInput()` 中进行鼠标左键检查）

+ [√] ⾼级光照和阴影效果：静态渲染有阴影绘制、光线追踪等；动态渲染使用 Blinn-Phong 模型、辉光。

+ [√] 实现三维场景中的中⽂⽂字渲染：米塔的对话使用三维文字渲染。

+ [×] 具有⼀定的对象表达能⼒

+ [√] ⾼级动画效果：骨骼动画

+ [×] 基于 GPU 的加速