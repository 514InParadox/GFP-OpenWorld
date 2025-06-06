# 辉光效果使用指南

## 概述

本文档介绍如何在GFP-OpenWorld项目中使用辉光（Bloom/Glow）效果。辉光效果已经实现完成，开发者只需要按照本指南进行配置和使用。

## 快速开始

### 1. 启用辉光测试应用

在 `main.cpp` 中启用 `GlowTestApp`：

```cpp
// 注释掉其他应用，启用GlowTestApp
GlowTestApp app(options);
app.run();
```

### 2. 运行程序

编译并运行程序，你将看到：
- 一个棕色的猴头模型
- 一个发光的白色立方体（代表光源）
- 黑色背景以突出辉光效果

## 交互控制

### 基本控制
- **G键**：开关辉光效果
- **ESC键**：退出程序

### 光源控制
- **+键**：增加光源亮度
- **-键**：减少光源亮度
- **方向键**：移动光源位置（前后左右）
- **Page Up/Down**：移动光源高度

### 辉光参数调节
- **[键**：减少辉光强度
- **]键**：增加辉光强度

### 相机控制
- **WASD键**：移动相机
- **鼠标移动**：旋转视角

## 在你的应用中集成辉光效果

### 1. 复制必要文件

确保你的项目包含以下着色器文件：
```
shader/fragment/extract_bright.frag
shader/fragment/gaussian_blur.frag  
shader/fragment/glow_combine.frag
shader/fragment/emissive.frag
shader/vertex/screen_quad.vert
```

### 2. 继承或参考GlowTestApp

```cpp
#include "app/glowTestApp.hpp"

// 方法1：直接使用
GlowTestApp app(options);

// 方法2：继承并扩展
class MyApp : public GlowTestApp {
    // 添加你的自定义功能
};
```

### 3. 创建发光物体

要让物体产生辉光效果，需要：

```cpp
// 设置发光材质
material->materialColor = glm::vec3(3.0f, 3.0f, 3.0f); // 亮度 > 1.0

// 或者使用发光着色器
_emissiveShader->use();
_emissiveShader->setUniformVec3("material.color", glm::vec3(1.0f, 1.0f, 1.0f));
_emissiveShader->setUniformFloat("material.intensity", 2.0f); // 强度 > 1.0
```

## 参数配置

### 亮度阈值调节

在 `extract_bright.frag` 中修改阈值：
```glsl
if (luminance > 1.0) brightColor = sceneColor; // 修改1.0为其他值
```
- **0.8**：更多物体会发光
- **1.5**：只有很亮的物体才发光

### 模糊强度调节

在 `gaussian_blur.frag` 中：
- 修改权重数组大小（当前为7）
- 调整循环次数

在 `glowTestApp.cpp` 的 `renderBlur()` 中：
```cpp
int amount = 10; // 修改模糊次数，范围6-15
```

### 辉光强度

在运行时通过代码调节：
```cpp
_glowIntensity = 1.5f; // 建议范围0.5-3.0
```

## 性能优化

### 降低分辨率
```cpp
// 在initFramebuffers()中使用较小的分辨率
int blurWidth = _windowWidth / 2;
int blurHeight = _windowHeight / 2;
```

### 减少模糊次数
```cpp
int amount = 6; // 从10减少到6
```

### 动态开关
```cpp
if (distanceToCamera > 50.0f) {
    // 远距离物体不使用辉光
    _enableGlow = false;
}
```

## 常用配置预设

### 柔和辉光
```cpp
_glowIntensity = 0.8f;
// gaussian_blur.frag: amount = 8
// extract_bright.frag: luminance > 1.2
```

### 强烈辉光
```cpp
_glowIntensity = 2.5f;
// gaussian_blur.frag: amount = 12
// extract_bright.frag: luminance > 0.8
```

### 性能优先
```cpp
_glowIntensity = 1.0f;
// gaussian_blur.frag: amount = 6
// 使用一半分辨率的模糊缓冲
```

## 注意事项

1. **HDR支持**：确保使用RGBA16F格式的帧缓冲
2. **亮度设置**：发光物体的材质颜色或强度必须 > 1.0
3. **背景颜色**：建议使用深色背景以突出辉光效果
4. **性能监控**：辉光效果会增加GPU负担，注意帧率变化

---

*参考实现：GlowTestApp* 