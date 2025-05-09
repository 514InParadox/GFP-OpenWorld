# 物理组件使用手册

## 简介

Physics 组件是一个轻量级的物理引擎模块，用于为 3D 应用程序中的模型提供基本物理行为，包括重力、碰撞检测和响应、弹性反弹等功能。通过将 Physics 组件附加到模型上，可以轻松实现物体的物理交互。

## 基本用法

### 1. 为模型添加物理组件

```cpp
// 假设已有一个 Model 对象
std::unique_ptr<Model> model = createBox(1.0f, 1.0f, 1.0f);

// 添加物理组件
model->addPhysics();

// 获取物理组件
Physics* physics = model->getPhysics();
```

### 2. 配置物理属性

```cpp
// 设置质量（千克）
physics->setMass(1.0f);

// 设置弹性系数（0.0-1.0，值越大反弹越强）
physics->setRestitution(0.85f);

// 设置摩擦系数（0.0-1.0，值越大摩擦力越大）
physics->setFriction(0.2f);

// 自定义重力加速度
physics->setGravity(glm::vec3(0.0f, -9.8f, 0.0f));

// 启用/禁用重力影响
physics->setGravityEnabled(true);  // 默认启用

// 设置为静态物体（不受力的影响）
physics->setStatic(false);  // 默认为动态物体
```

### 3. 更新物理状态

在主循环中，需要定期更新物理组件状态：

```cpp
// 在每帧更新物理状态
float deltaTime = calculateDeltaTime();  // 获取当前帧与上一帧的时间差

// 更新所有物理组件
physics->update(deltaTime);
```

### 4. 施加力和脉冲

```cpp
// 施加力（牛顿）- 会考虑质量，持续作用
physics->applyForce(glm::vec3(10.0f, 0.0f, 0.0f));  // 向右施加10牛顿的力

// 施加脉冲（瞬时力，直接改变速度）
physics->applyImpulse(glm::vec3(0.0f, 5.0f, 0.0f));  // 向上的瞬时冲量
```

### 5. 设置和获取速度

```cpp
// 直接设置线性速度
physics->setVelocity(glm::vec3(3.0f, 0.0f, 0.0f));  // 初始速度为向右3单位/秒

// 获取当前速度
glm::vec3 velocity = physics->getVelocity();

// 设置角速度（旋转）
physics->setAngularVelocity(glm::vec3(0.0f, 1.0f, 0.0f));  // 绕Y轴旋转
```

## 碰撞处理

Physics 组件提供了 `bounce()` 方法用于处理碰撞响应：

```cpp
// 当检测到碰撞时，调用bounce方法处理反弹
// normal: 碰撞表面的法向量（单位向量，指向物体外部）
// surfaceFriction: 表面摩擦系数（0-1）
// minVelocityThreshold: 最小速度阈值，低于此值速度将被设为0
physics->bounce(surfaceNormal, 0.3f, 0.1f);
```

### 平面碰撞检测示例

```cpp
// 平面方程: Ax + By + Cz + D = 0，其中 (A,B,C) 是平面法线
glm::vec3 planeNormal = glm::vec3(0.0f, 1.0f, 0.0f);  // 水平地面，法线向上
float D = -glm::dot(planeNormal, planePosition);

// 计算物体到平面的距离
glm::vec3 objectPosition = physics->getPosition();
float radius = 0.5f;  // 假设物体半径为0.5
glm::vec3 lowestPoint = objectPosition - glm::vec3(0.0f, radius, 0.0f);
float distanceToPlane = glm::dot(planeNormal, lowestPoint) + D;

// 如果距离为负或接近零，发生碰撞
if (distanceToPlane <= 0.01f) {
    // 调整位置，使物体位于平面稍上方
    objectPosition += planeNormal * (-distanceToPlane + 0.01f);
    physics->setPosition(objectPosition);
    
    // 处理反弹
    physics->bounce(planeNormal, 0.3f, 0.1f);
}
```

### 斜面碰撞示例

对于斜面，只需要提供正确的平面法线：

```cpp
// 30度斜面，法线方向
glm::vec3 slopeNormal = glm::vec3(-0.5f, 0.866f, 0.0f);  // 法线指向左上方
```

## 高级用法

### 1. 创建边界墙

```cpp
// 左墙碰撞检测（X轴负方向边界）
if (position.x - radius <= leftWallX) {
    position.x = leftWallX + radius + 0.001f;  // 稍微调整位置避免卡住
    glm::vec3 leftWallNormal(1.0f, 0.0f, 0.0f);  // 向右的法线
    physics->bounce(leftWallNormal, friction, minVelocityThreshold);
    physics->setPosition(position);
}
```

### 2. 检测物体静止

```cpp
// 检查物体是否几乎静止
const float MIN_VELOCITY_THRESHOLD = 0.1f;
if (glm::length(physics->getVelocity()) < MIN_VELOCITY_THRESHOLD) {
    // 物体已停止运动
    physics->setVelocity(glm::vec3(0.0f));
}
```

### 3. 重置模拟

```cpp
// 重置物体到初始位置
physics->setPosition(initialPosition);
physics->setVelocity(glm::vec3(0.0f));
physics->setAngularVelocity(glm::vec3(0.0f));
```

## 性能考虑

1. 对于静态物体（如地面、墙壁），应设置 `setStatic(true)` 以避免不必要的计算
2. 当物体速度低于阈值时，可以将其设置为零以减少抖动和计算
3. 对于大型场景，可以实现空间分区以减少碰撞检测的计算量

## 限制和注意事项

1. 当前物理引擎不支持复杂形状的精确碰撞检测，主要适用于简单的几何形状
2. 不支持物体之间的碰撞，只支持物体与环境（墙壁、地面等）的碰撞
3. 旋转计算是简化的，不考虑惯性张量
4. 物理计算的准确性取决于每帧的时间步长；时间步长过大可能导致穿透和不稳定性

## 示例项目

### 1. TestPhysicsApp

`TestPhysicsApp` 是一个基础物理测试应用，用于测试基本的重力和碰撞响应。

**使用方法：**
- 按空格键：开始模拟（物体会从初始位置以水平速度射出）
- 按R键：重置模拟
- 按ESC键：退出应用

**测试内容：**
- 重力对物体的影响
- 物体与水平地面的碰撞和反弹
- 物体与四面墙壁的碰撞和反弹
- 摩擦力对水平速度的影响

### 2. TestPhysicsTwoApp

`TestPhysicsTwoApp` 是一个进阶物理测试应用，展示了如何使用 Physics 组件实现斜面上物体的运动。

**使用方法：**
- 按空格键：开始模拟（物体会从初始位置以水平速度射出）
- 按R键：重置模拟
- 按ESC键：退出应用

**测试内容：**
- 斜面对物体运动的影响
- 使用平面方程进行非水平表面的碰撞检测
- 基于表面法线的反弹计算
- 模拟不同角度的斜面碰撞

### 3. TestPhysicsThreeApp

`TestPhysicsThreeApp` 是一个简单的角速度测试应用，用于演示物体围绕中心点的旋转运动。

**使用方法：**
- 按1键：增加角速度
- 按2键：减少角速度
- 按ESC键：退出应用

**测试内容：**
- 角速度和线速度的转换
- 圆周运动的模拟
- 角速度调整对圆周运动的影响
- 无重力环境下的纯角速度测试

## 如何选择示例项目

- 如果需要测试基本的重力和碰撞，使用 `TestPhysicsApp`
- 如果需要测试非水平表面的碰撞响应，使用 `TestPhysicsTwoApp`
- 如果需要测试角速度和围绕中心点的旋转，使用 `TestPhysicsThreeApp`

各个示例项目的源代码可以作为开发自己的物理功能的起点。
