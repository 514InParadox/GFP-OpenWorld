# 骨骼动画系统使用说明

## 概述
这是一个完整的骨骼动画系统实现，支持使用 Assimp 库加载和播放骨骼动画模型。

## 功能特性

### 核心功能
- ✅ 骨骼动画加载和播放
- ✅ 多个动画切换
- ✅ 动画播放控制（播放/暂停/调速）
- ✅ 交互式骨骼控制
- ✅ 实时骨骼变换（旋转、位移、缩放）
- ✅ 光照系统集成
- ✅ ImGui 用户界面

### 技术特点
- 支持最多 100 个骨骼
- 每个顶点最多受 4 个骨骼影响
- 使用四元数进行旋转插值
- 支持位置、旋转、缩放关键帧动画
- 骨骼层次结构遍历
- GPU 骨骼变换计算

## 文件结构

### 核心类
- `AnimatedVertex` - 包含骨骼权重信息的顶点结构
- `Bone` - 单个骨骼的关键帧数据和插值
- `Animation` - 动画数据加载和管理
- `Animator` - 动画播放和骨骼变换计算
- `AnimatedModel` - 骨骼动画模型
- `AnimatedMesh` - 包含骨骼权重的网格

### 着色器
- `animated_model.vert` - 顶点着色器（骨骼变换）
- `animated_model.frag` - 片段着色器（光照计算）

### 应用程序
- `SkeletalAnimationApp` - 完整的测试应用程序

## 使用方法

### 1. 准备模型文件
支持的格式：`.dae`, `.fbx`, `.gltf`, `.blend` 等 Assimp 支持的格式

推荐使用包含骨骼动画的模型文件，例如：
- Mixamo 导出的角色动画
- Blender 导出的 .dae 或 .fbx 文件
- glTF 格式的动画模型

### 2. 放置模型文件
将模型文件放在 `resource/model/` 目录下，例如：
```
resource/model/vampire/dancing_vampire.dae
resource/model/character/character.fbx
```

### 3. 修改配置
在 `skeletalAnimationApp.cpp` 中修改模型路径：
```cpp
const std::string modelPath = "resource/model/your_model/model.dae";
const std::string animationPath = "resource/model/your_model/model.dae";
```

### 4. 编译运行
确保项目包含所有新添加的文件，然后编译运行。

## 控制说明

### 相机控制
- `WASD` - 移动相机
- `QE` - 上下移动
- `鼠标 + 左键` - 环视

### 动画控制
- `空格键` - 播放/暂停动画
- GUI 面板中的速度滑块 - 调整播放速度
- GUI 面板中的动画选择 - 切换不同动画

### 交互式骨骼控制
1. 在 GUI 中勾选 "Show Bone Controls"
2. 从下拉菜单选择要控制的骨骼
3. 使用滑块调整骨骼的旋转、位移、缩放
4. 点击 "Reset Bone" 重置骨骼变换

### 光照控制
- 调整光源位置、颜色和强度
- 支持方向光和点光源

## 故障排除

### 常见问题

1. **模型加载失败**
   - 检查模型文件路径是否正确
   - 确保模型文件包含骨骼和动画数据
   - 查看控制台错误信息

2. **着色器编译失败**
   - 检查着色器文件路径
   - 确保 OpenGL 版本支持（需要 3.3+）

3. **骨骼不显示**
   - 确保模型包含骨骼权重信息
   - 检查 MAX_BONES 常量是否足够大

4. **动画不播放**
   - 确保模型文件包含动画数据
   - 检查动画时长和帧率设置

### 性能优化
- 减少骨骼数量（当前限制：100个）
- 降低模型顶点数
- 使用 LOD（细节层次）系统
- 优化纹理分辨率

## 扩展功能

### 可以添加的功能
1. **动画混合** - 多个动画之间的平滑过渡
2. **逆运动学（IK）** - 基于目标位置计算骨骼姿态
3. **动画状态机** - 复杂的动画逻辑控制
4. **骨骼约束** - 限制骨骼运动范围
5. **动画事件** - 在动画特定时间点触发事件
6. **骨骼附件** - 在骨骼上挂载物体（武器、装备等）

### 集成建议
- 与物理系统结合实现布娃娃效果
- 与AI系统结合实现智能角色动画
- 与音频系统结合实现音频驱动的动画

## 技术细节

### 骨骼变换计算
```glsl
// 顶点着色器中的骨骼变换
vec4 totalPosition = vec4(0.0f);
for(int i = 0; i < MAX_BONE_INFLUENCE; i++) {
    if(aBoneIds[i] == -1) continue;
    vec4 localPosition = finalBonesMatrices[aBoneIds[i]] * vec4(aPos, 1.0f);
    totalPosition += localPosition * aWeights[i];
}
```

### 动画插值
```cpp
// 位置插值
glm::vec3 finalPosition = glm::mix(pos0, pos1, scaleFactor);
// 旋转插值（四元数）
glm::quat finalRotation = glm::slerp(rot0, rot1, scaleFactor);
```

## 依赖库
- OpenGL 3.3+
- GLFW
- GLAD
- GLM
- Assimp
- ImGui
- stb_image

## 许可证
此代码遵循项目原有许可证。
