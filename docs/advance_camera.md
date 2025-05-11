# AdvanceCamera 使用文档

## 概述

`AdvanceCamera` 是一个灵活的相机系统，为3D应用程序提供多种相机控制模式和功能。它封装了基础的 `Camera` 类并添加了高级控制机制，使开发者能够轻松实现3D应用和游戏中常见的各种相机行为。

## 功能特性

- 多种相机模式（自由移动、固定位置、平移、轨道环绕）
- 相机位置和朝向的平滑过渡
- 自动调整视角以完整显示物体的缩放适配功能
- 可调节的裁剪平面，实现精细的深度控制
- 可配置的移动、旋转和动画速度
- 鼠标滚轮缩放功能（视场角或距离调整）

## 初始化

```cpp
// 使用窗口尺寸初始化（用于计算宽高比）
AdvanceCamera camera(windowWidth, windowHeight);
```

构造函数会自动配置内部透视相机，使用合理的默认值：
- 视场角：45度
- 近裁剪平面：0.1
- 远裁剪平面：10000.0
- 默认位置：(0, 0, 15)

## 相机模式

`AdvanceCamera` 支持四种不同的控制模式：

### FREE_ROAM（自由移动）模式
允许完全自由的移动和旋转。
```cpp
camera.setMode(CameraMode::FREE_ROAM);
```
- WASD键：前后左右移动
- QE键：上下移动
- 鼠标移动：环顾四周
- 鼠标滚轮：调整视场角（FOV），向上滚动缩小FOV（放大视图），向下滚动增大FOV（缩小视图）

### FIXED（固定位置）模式
相机位置固定，但可以旋转。
```cpp
camera.setMode(CameraMode::FIXED);
```
- 鼠标移动：环顾四周
- 鼠标滚轮：调整视场角（FOV），与FREE_ROAM模式相同

### PAN（平移）模式
相机只能平行于视图平面移动。
```cpp
camera.setMode(CameraMode::PAN);
```
- WASD键：沿视图平面移动相机
- 鼠标拖拽：平移相机
- 鼠标滚轮：调整视场角（FOV），与FREE_ROAM模式相同

### ORBIT（轨道环绕）模式
相机围绕目标点旋转。
```cpp
// 设置轨道目标点（可选，默认为原点）
camera.setOrbitTarget(glm::vec3(0.0f, 0.0f, 0.0f));
camera.setMode(CameraMode::ORBIT);
```
- 鼠标移动：围绕目标点旋转
- 鼠标滚轮：调整与目标的距离（而非FOV），向上滚动靠近目标，向下滚动远离目标
- W/S键：靠近/远离目标点

## 鼠标滚轮功能

鼠标滚轮在不同模式下有不同的行为：

```cpp
// 调整滚轮缩放速度（默认为2.0）
camera.setZoomSpeed(2.0f);
```

- **在ORBIT模式下**：滚轮控制相机与目标点的距离
  - 向上滚动：减小距离，靠近目标
  - 向下滚动：增加距离，远离目标
  
- **在其他模式下（FREE_ROAM、FIXED、PAN）**：滚轮控制视场角（FOV）
  - 向上滚动：缩小FOV，增加放大效果（视野变窄但细节变大）
  - 向下滚动：增大FOV，增加广角效果（视野变宽但细节变小）

- **FOV限制**：视场角自动限制在合理范围内（默认5°~120°）
  ```cpp
  // FOV限制在内部已设置（以弧度为单位）
  // _minFOV = glm::radians(5.0f);  // 最大放大（窄视野）
  // _maxFOV = glm::radians(120.0f); // 最大广角（宽视野）
  ```

## 输入处理

每帧都需要处理输入来更新相机：

```cpp
// 在主循环中：
camera.processInput(inputState);

// 如果使用动画功能：
float deltaTime = calculateDeltaTime();
if (camera.isAnimating()) {
    camera.updateAnimation(deltaTime);
}
```

## 裁剪平面

控制近裁剪平面和远裁剪平面：

```cpp
// 获取当前值
float nearPlane = camera.getNearPlane();
float farPlane = camera.getFarPlane();

// 设置新值
camera.setNearPlane(0.1f);    // 必须 > 0
camera.setFarPlane(1000.0f);  // 必须 > nearPlane
```

裁剪平面决定了可见深度范围：
- 近裁剪平面：最近可见物体的距离
- 远裁剪平面：最远可见物体的距离

## Zoom To Fit（缩放适配）

自动调整相机以完整显示特定对象或边界盒：

```cpp
// 获取对象的边界盒
BoundingBox bbox = model.getBoundingBox();
// 应用模型变换到边界盒
bbox.transform(model.transform.getLocalMatrix());

// 缩放适配，物体周围留20%的边距
camera.zoomToFit(bbox, 1.2f);
```

`zoomToFit` 函数会：
1. 临时切换到ORBIT模式
2. 将轨道目标设置为边界盒的中心
3. 计算最佳距离和视场角以查看整个对象
4. 平滑地将相机动画过渡到新视角
5. 动画完成后可选择恢复到之前的相机模式

## 动画系统

控制相机动画速度：

```cpp
// 设置动画速度（默认为2.0）
camera.setAnimationSpeed(1.0f); // 更慢
camera.setAnimationSpeed(4.0f); // 更快

// 检查动画是否正在进行
if (camera.isAnimating()) {
    // 跳过用户输入处理
}
```

## 相机参数

调整相机行为：

```cpp
// 获取/设置视场角（单位为弧度）
float fov = camera.getFOV();
camera.setFOV(glm::radians(60.0f)); // 设置视场角为60度

// 视场角会自动限制在 _minFOV 和 _maxFOV 范围内
// 默认范围为 5° 到 120°

// 设置移动速度
camera.setMoveSpeed(0.2f);     // 移动速度（默认：0.1）
camera.setRotateSpeed(0.003f); // 旋转速度（默认：0.002）
camera.setOrbitSpeed(0.004f);  // 轨道旋转速度（默认：0.002）
camera.setZoomSpeed(3.0f);     // 缩放速度（默认：2.0）
```

## 访问相机属性

获取底层相机用于渲染：

```cpp
// 获取基础相机指针以获取视图和投影矩阵
Camera* baseCamera = camera.getCamera();
glm::mat4 view = baseCamera->getViewMatrix();
glm::mat4 projection = baseCamera->getProjectionMatrix();
```

## 完整使用示例

```cpp
// 初始化
AdvanceCamera camera(windowWidth, windowHeight);

// 设置
camera.setMoveSpeed(0.15f);
camera.setMode(CameraMode::FREE_ROAM);

// 主循环
while (!glfwWindowShouldClose(window)) {
    float deltaTime = calculateDeltaTime();
    
    // 更新相机
    if (camera.isAnimating()) {
        camera.updateAnimation(deltaTime);
    } else {
        camera.processInput(input);
        
        // 示例：按F键触发Zoom To Fit
        if (input.keyboard.keyStates[GLFW_KEY_F] == GLFW_PRESS) {
            camera.zoomToFit(objectBoundingBox, 1.5f);
        }
    }
    
    // 渲染
    Camera* baseCamera = camera.getCamera();
    glm::mat4 view = baseCamera->getViewMatrix();
    glm::mat4 projection = baseCamera->getProjectionMatrix();
    
    // 使用视图和投影矩阵进行渲染
    // ...
}
```

## 使用技巧和最佳实践

1. **相机过渡**：在处理用户输入前，使用 `isAnimating()` 检查是否有过渡动画正在进行。

2. **裁剪平面**：设置适当的近/远裁剪平面以避免z-fighting并确保所有对象可见：
   - 过小的近裁剪平面（< 0.01）可能导致精度问题
   - 远/近裁剪平面比值过大会导致深度缓冲精度问题

3. **轨道模式**：在切换到轨道模式前，总是设置一个合适的轨道目标点。

4. **缩放适配**：为获得最佳效果，确保边界盒准确代表对象的尺寸。

5. **性能**：相机系统设计为高效运行，但动画更新需要每帧调用 `updateAnimation()`。

6. **边界情况**：在实现相机的UI控制时，处理这些边界情况：
   - 防止轨道计算中的除零错误
   - 将值限制在合理范围内
   - 平滑处理差异较大的相机位置之间的过渡

## 类参考

### 公共方法

- `AdvanceCamera(int windowWidth, int windowHeight)`
- `void processInput(const Input& input)`
- `Camera* getCamera() const`
- `void setMode(CameraMode mode)`
- `CameraMode getMode() const`
- `std::string getModeString() const`
- `void setMoveSpeed(float speed)`
- `void setRotateSpeed(float speed)`
- `void setOrbitSpeed(float speed)`
- `void setZoomSpeed(float speed)`
- `void setAnimationSpeed(float speed)`
- `float getFOV() const`
- `void setFOV(float fov)`
- `void setOrbitTarget(const glm::vec3& target)`
- `glm::vec3 getOrbitTarget() const`
- `void updateOrbitPosition()`
- `void zoomToFit(const BoundingBox& targetBBox, float padding = 1.2f)`
- `void updateAnimation(float deltaTime)`
- `bool isAnimating() const`
- `float getNearPlane() const`
- `void setNearPlane(float nearPlane)`
- `float getFarPlane() const` 
- `void setFarPlane(float farPlane)`

# CameraTestApp 测试应用

## 概述

`CameraTestApp` 是一个专门用于测试和展示 `AdvanceCamera` 功能的示例应用程序。它提供了一个交互式环境，让用户可以体验所有相机控制模式，并测试近裁剪平面和远裁剪平面的效果。

## 主要功能

- 展示了四种相机模式的操作方式和效果
- 提供了多个模型进行测试，可以观察不同视角下的效果
- 支持调整近裁剪平面和远裁剪平面
- 实现了 Zoom To Fit 功能，可以自动调整视角以显示所有模型
- 在窗口标题中显示当前相机模式和操作提示

## 启动应用

```cpp
// 创建应用实例
CameraTestApp app(options);

// 启动应用
app.run();
```

## 界面操作指南

### 基本控制

- **ESC键**：退出应用程序
- **数字键1-4**：切换相机模式
  - **1**：FREE_ROAM 模式（自由移动）
  - **2**：FIXED 模式（固定位置）
  - **3**：PAN 模式（平移）
  - **4**：ORBIT 模式（环绕）
- **F键**：触发 Zoom To Fit 功能，自动调整视角以显示所有模型

### 相机移动控制

- **WASD键**：基本移动控制（根据相机模式有不同效果）
  - FREE_ROAM 模式：前后左右移动
  - PAN 模式：沿视图平面移动
  - ORBIT 模式：W/S键调整与目标的距离
- **QE键**：在 FREE_ROAM 模式下上下移动
- **鼠标移动**：控制相机旋转或环绕
- **鼠标滚轮**：缩放控制（调整FOV或距离，取决于模式）

### 裁剪平面调整

- **Z/X键**：调整近裁剪平面
  - **Z**：减小近裁剪平面距离（向相机方向移动）
  - **X**：增加近裁剪平面距离（远离相机方向移动）
- **C/V键**：调整远裁剪平面
  - **C**：减小远裁剪平面距离（向相机方向移动）
  - **V**：增加远裁剪平面距离（远离相机方向移动）
- **N/M键**：调整视场角（FOV）
  - **N**：减小视场角（放大效果）
  - **M**：增加视场角（缩小效果）
- **R键**：重置裁剪平面和FOV到默认值（近平面=1.0，远平面=15.0，FOV=45°）

## 测试场景

CameraTestApp 创建了一个包含多个模型的测试场景：

- 七个猴头模型排成一行，大小逐渐增加
- 中心位置为原点 (0, 0, 0)
- 模型间距为3个单位
- 天空盒提供背景参考

这种布局设计用于测试以下功能：
- 相机模式切换后的控制差异
- 裁剪平面调整效果（近处或远处的模型可能被裁剪）
- Zoom To Fit 功能的准确性

## 裁剪平面测试

调整裁剪平面是 CameraTestApp 的重要功能，它可以帮助您理解裁剪平面对渲染的影响：

1. **近裁剪平面测试**：
   - 按 Z/X 键调整近裁剪平面
   - 当增加近裁剪平面距离时，靠近相机的模型会被裁剪掉
   - 观察近裁剪平面穿过模型时的效果

2. **远裁剪平面测试**：
   - 按 C/V 键调整远裁剪平面
   - 当减小远裁剪平面距离时，远离相机的模型会被裁剪掉
   - 适当的远裁剪平面设置可以提高深度精度

3. **裁剪平面信息显示**：
   - 调整裁剪平面时，控制台会输出当前的裁剪平面值
   - 窗口标题栏显示相机模式和操作提示

## 使用技巧

1. **初始探索**：
   - 先使用 F 键触发 Zoom To Fit，获得整体视图
   - 然后尝试不同的相机模式，感受控制差异

2. **裁剪平面实验**：
   - 使用 ORBIT 模式并调整到合适位置
   - 然后使用 Z/X/C/V 键微调裁剪平面
   - 观察不同裁剪平面设置对场景的影响

3. **性能测试**：
   - 通过调整远裁剪平面观察对性能的影响
   - 远裁剪平面设置较小值可能提高渲染性能

## 源代码参考

CameraTestApp 的实现提供了 AdvanceCamera 使用的完整示例，包括：

```cpp
// 初始化相机
_advCamera.reset(new AdvanceCamera(_windowWidth, _windowHeight));

// 设置和处理输入
void CameraTestApp::handleInput() {
    // 更新帧时间
    updateFrameTime();
    
    // 检查相机动画
    if (_advCamera->isAnimating()) {
        _advCamera->updateAnimation(_deltaTime);
        // 动画过程中跳过其他输入处理
        _input.forwardState();
        return;
    }
    
    // 处理键盘和鼠标输入
    // ...
    
    // 处理相机输入
    _advCamera->processInput(_input);
}

// 渲染场景
void CameraTestApp::renderFrame() {
    // 获取相机矩阵
    Camera* camera = _advCamera->getCamera();
    glm::mat4 projection = camera->getProjectionMatrix();
    glm::mat4 view = camera->getViewMatrix();
    
    // 渲染模型
    // ...
}
```

完整的源代码可在项目的 `src/app/cameraTestApp.cpp` 和 `src/app/cameraTestApp.hpp` 文件中找到。
