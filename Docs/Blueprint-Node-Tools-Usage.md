# 蓝图节点工具使用指南

## 📋 工具概述

蓝图节点工具专门用于在蓝图的视觉脚本系统中创建和管理节点。通过这个工具，您可以添加事件节点、函数调用节点、变量节点，并建立节点之间的连接关系，构建完整的游戏逻辑流程。

## 🎯 主要功能

### 1. 事件节点管理
- 添加标准事件节点（BeginPlay、Tick、EndPlay 等）
- 创建自定义事件节点
- 添加输入动作事件节点

### 2. 函数节点创建
- 调用引擎内置函数
- 调用自定义蓝图函数
- 添加数学运算节点

### 3. 节点连接
- 建立执行流程连接
- 连接数据引脚
- 批量连接多个节点

### 4. 节点查找
- 按类型查找节点
- 按事件类型查找
- 在指定图表中搜索

## 🚀 使用示例

### 创建基础事件流程

**自然语言命令：**
```
"在 PlayerCharacter 蓝图中添加 BeginPlay 事件节点，然后添加一个 Print String 节点连接到 BeginPlay，设置打印内容为 'Player Spawned'"
```

**AI 执行过程：**
1. 在 `BP_PlayerCharacter` 的事件图表中添加 `BeginPlay` 事件节点
2. 添加 `Print String` 函数节点
3. 将 `BeginPlay` 的执行引脚连接到 `Print String` 的执行引脚
4. 设置 `Print String` 的文本参数为 "Player Spawned"

### 创建输入响应系统

**自然语言命令：**
```
"在 PlayerCharacter 蓝图中添加 Jump 输入动作事件，连接到 Launch Character 函数，设置跳跃力度为 500"
```

**AI 执行过程：**
1. 添加 `Jump` 输入动作事件节点
2. 添加 `Launch Character` 函数节点
3. 连接执行流程
4. 设置 `Launch Character` 的 `Launch Velocity` 参数为 (0, 0, 500)

### 创建变量操作流程

**自然语言命令：**
```
"在 PlayerCharacter 蓝图中添加一个 Tick 事件，连接到 Get Health 变量节点，然后连接到 Print String 显示当前血量"
```

**AI 执行过程：**
1. 添加 `Tick` 事件节点
2. 添加 `Get Health` 变量节点
3. 添加 `Print String` 节点
4. 连接执行流程
5. 将 `Health` 变量连接到 `Print String` 的文本参数

## 📝 详细命令参考

### 事件节点命令

| 事件类型 | 示例 | 说明 |
|---------|------|------|
| BeginPlay | "添加 BeginPlay 事件" | 游戏开始时触发 |
| Tick | "添加 Tick 事件" | 每帧触发 |
| EndPlay | "添加 EndPlay 事件" | 游戏结束时触发 |
| 自定义事件 | "添加自定义事件 OnPlayerDied" | 创建自定义事件 |

### 函数节点命令

| 函数类型 | 示例 | 说明 |
|---------|------|------|
| 打印函数 | "添加 Print String 节点" | 输出调试信息 |
| 数学运算 | "添加 Add 节点" | 数值计算 |
| 字符串操作 | "添加 Append 节点" | 字符串拼接 |
| 数组操作 | "添加 Add 节点到数组" | 数组操作 |

### 变量节点命令

| 变量操作 | 示例 | 说明 |
|---------|------|------|
| 获取变量 | "添加 Get Health 节点" | 读取变量值 |
| 设置变量 | "添加 Set Health 节点" | 修改变量值 |
| 创建变量 | "添加 Create Health 变量" | 创建新变量 |

### 节点连接命令

| 连接类型 | 示例 | 说明 |
|---------|------|------|
| 执行连接 | "连接 BeginPlay 到 Print String" | 建立执行流程 |
| 数据连接 | "连接 Health 到 Print String 文本" | 传递数据值 |
| 批量连接 | "连接所有节点形成循环" | 一次性连接多个节点 |

## 🎮 实际应用场景

### 1. 玩家控制系统
```
"在 PlayerController 蓝图中添加 Move Forward 输入事件，连接到 Add Movement Input 函数，设置移动方向为前向"
```

### 2. 游戏状态管理
```
"在 GameMode 蓝图中添加 BeginPlay 事件，连接到 Set Game State 函数，设置游戏状态为 Playing"
```

### 3. UI 交互系统
```
"在 MainMenu 蓝图中添加 OnClicked 事件到开始游戏按钮，连接到 Open Level 函数，设置目标关卡为 GameLevel"
```

### 4. 伤害系统
```
"在 Enemy 蓝图中添加 OnTakeDamage 事件，连接到 Subtract 函数减少血量，然后连接到 Branch 判断是否死亡"
```

## 🔧 节点查找功能

### 查找特定类型节点
```
"在 PlayerCharacter 蓝图中查找所有 Print String 节点"
```

### 查找事件节点
```
"在 PlayerCharacter 蓝图中查找所有 BeginPlay 事件"
```

### 查找变量节点
```
"在 PlayerCharacter 蓝图中查找所有 Health 相关节点"
```

### 在指定图表中查找
```
"在 PlayerCharacter 的 EventGraph 中查找所有 Tick 事件"
```

## ⚠️ 注意事项

### 1. 节点命名规范
- 事件节点通常以动词开头（如 BeginPlay、OnClicked）
- 函数节点使用驼峰命名法（如 PrintString、AddMovementInput）
- 变量节点使用 Get/Set 前缀（如 GetHealth、SetHealth）

### 2. 连接规则
- 执行引脚只能连接到执行引脚
- 数据引脚只能连接到相同类型的数据引脚
- 一个执行引脚可以连接到多个目标
- 数据引脚通常是一对一连接

### 3. 性能考虑
- 避免在 Tick 事件中添加过多复杂逻辑
- 合理使用 Branch 节点减少不必要的计算
- 及时清理未使用的节点和连接

## 🔧 故障排除

### 常见问题

1. **节点创建失败**
   - 检查蓝图是否已编译
   - 确认函数名称是否正确
   - 验证蓝图类型是否支持该节点

2. **连接失败**
   - 检查引脚类型是否匹配
   - 确认节点是否在同一个图表中
   - 验证连接是否违反规则

3. **节点查找失败**
   - 确认节点名称拼写正确
   - 检查是否在正确的图表中查找
   - 验证节点类型是否存在

### 调试技巧

1. **查看节点结构**
   - 在蓝图编辑器中检查节点布局
   - 验证连接关系是否正确
   - 确认节点参数设置

2. **测试节点功能**
   - 使用 Print String 节点输出调试信息
   - 检查变量值是否正确传递
   - 验证执行流程是否符合预期

## 📚 进阶用法

### 1. 复杂逻辑流程
```
"在 PlayerCharacter 中创建以下流程：BeginPlay -> Set Health = 100 -> Print 'Player Ready' -> Branch (Is Health > 0) -> True: Print 'Alive' -> False: Print 'Dead'"
```

### 2. 循环系统
```
"在 Enemy 蓝图中创建 Tick 事件，连接到 Branch 判断是否看到玩家，True 分支连接到 Move To 函数，False 分支连接到 Wait 节点延迟1秒"
```

### 3. 事件链系统
```
"在 Weapon 蓝图中创建 OnFire 事件，连接到 Spawn Actor 生成子弹，然后连接到 Play Sound 播放射击音效，最后连接到 Add Recoil 添加后坐力"
```

### 4. 状态机系统
```
"在 PlayerCharacter 中创建状态机：Idle -> Walk -> Run -> Jump -> Idle，每个状态转换都有相应的条件判断"
```

## 🎯 最佳实践

### 1. 模块化设计
- 将复杂逻辑拆分为多个函数
- 使用自定义事件组织代码结构
- 保持节点图的清晰和可读性

### 2. 错误处理
- 在关键节点添加 Branch 判断
- 使用 Print String 输出调试信息
- 设置合理的默认值和边界条件

### 3. 性能优化
- 避免在 Tick 中进行复杂计算
- 合理使用 Delay 和 Wait 节点
- 及时清理未使用的节点和变量

---

通过蓝图节点工具，您可以构建复杂的游戏逻辑系统，让 AI 助手帮助您快速实现各种游戏功能，从简单的玩家控制到复杂的 AI 行为系统。
