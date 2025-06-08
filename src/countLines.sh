#!/bin/bash

# 统计当前目录下所有.hpp和.cpp文件的总行数

# 初始化计数器
total_lines=0

# 使用find命令查找所有.hpp和.cpp文件
while IFS= read -r file; do
    # 使用wc -l计算每个文件的行数，并累加到total_lines
    lines=$(wc -l < "$file")
    total_lines=$((total_lines + lines))
done < <(find . -type f \( -name "*.hpp" -o -name "*.cpp" \))

# 输出结果
echo "Total lines in .hpp and .cpp files: $total_lines"
