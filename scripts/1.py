def parse_map_file(filename):
    """
    从文件中读取01字符串并转换为map_data格式
    支持多种格式：
    1. 每行一个字符串（如：01110101）
    2. 字符之间有空格（如：0 1 1 1 0 1 0 1）
    3. 混合格式
    """
    map_data = []
    
    try:
        with open(filename, 'r', encoding='utf-8') as f:
            lines = f.readlines()
        
        for line_num, line in enumerate(lines, 1):
            line = line.strip()
            if not line:  # 跳过空行
                continue
            
            row = []
            
            # 如果行包含空格，按空格分割
            if ' ' in line:
                parts = line.split()
                for part in parts:
                    if part in ['0', '1', '2']:
                        row.append(int(part))
                    else:
                        print(f"警告：第{line_num}行包含无效字符'{part}'，已跳过")
            else:
                # 没有空格，逐字符处理
                for char in line:
                    if char in ['0', '1', '2']:
                        row.append(int(char))
                    elif char not in [' ', '\t']:  # 忽略空格和制表符
                        print(f"警告：第{line_num}行包含无效字符'{char}'，已跳过")
            
            if row:  # 如果行不为空
                map_data.append(row)
        
        # 验证地图尺寸
        if map_data:
            expected_width = len(map_data[0])
            for i, row in enumerate(map_data):
                if len(row) != expected_width:
                    print(f"警告：第{i+1}行长度为{len(row)}，预期为{expected_width}")
        
        print(f"成功读取地图：{len(map_data)} x {len(map_data[0]) if map_data else 0}")
        return map_data
    
    except FileNotFoundError:
        print(f"错误：文件'{filename}'不存在")
        return None
    except Exception as e:
        print(f"读取文件时发生错误：{e}")
        return None

def validate_map_data(map_data):
    """验证地图数据的有效性"""
    if not map_data:
        return False
    
    height = len(map_data)
    width = len(map_data[0])
    
    # 检查是否为矩形
    for i, row in enumerate(map_data):
        if len(row) != width:
            print(f"错误：第{i+1}行长度不一致")
            return False
    
    # 检查是否只包含0、1、2
    invalid_cells = []
    for i, row in enumerate(map_data):
        for j, cell in enumerate(row):
            if cell not in [0, 1, 2]:
                invalid_cells.append((i+1, j+1, cell))
    
    if invalid_cells:
        print(f"错误：发现{len(invalid_cells)}个无效值（只允许0、1、2）:")
        for row, col, value in invalid_cells[:5]:
            print(f"  位置({row},{col}): {value}")
        if len(invalid_cells) > 5:
            print(f"  ... 还有{len(invalid_cells)-5}个")
        return False
    
    # 统计各种格子数量
    counts = {0: 0, 1: 0, 2: 0}
    for row in map_data:
        for cell in row:
            counts[cell] += 1
    
    print(f"地图验证通过：{height} x {width}")
    print(f"空地格子(0): {counts[0]}")
    print(f"墙壁格子(1): {counts[1]}")
    print(f"灯光格子(2): {counts[2]}")
    return True

def save_map_as_python(map_data, output_filename):
    """将map_data保存为Python格式的文件"""
    with open(output_filename, 'w', encoding='utf-8') as f:
        f.write("# Generated map_data\n")
        f.write("map_data = [\n")
        for row in map_data:
            f.write("    [" + ",".join(map(str, row)) + "],\n")
        f.write("]\n")
    print(f"已保存为Python格式：{output_filename}")

def create_sample_map_file():
    """创建一个示例地图文件"""
    sample_map = """01111111111111111111111111111111111111111111111110
00000000000000000000000000000000000000000000000000
01111111000111111111111000000000000011111110001110
01000001000100000000001000111111000010000010001010
01011101000101111110001000100001000010111010001010
01000001000100000010001000101101000010001010001010
01111101000111110110001000100001000011111010001010
01000001000000010000001000111111000000000010001010
11111101111111110111111000000000111111111110001011
00000000000000000100001111111111100001000000001000"""
    
    with open('sample_map.txt', 'w', encoding='utf-8') as f:
        f.write(sample_map)
    print("已创建示例地图文件：sample_map.txt")

# 完整的转换脚本
def convert_map_file_to_obj(input_filename, obj_filename="converted_map.obj"):
    """完整的转换流程：从文件读取 -> 转换 -> 生成OBJ"""
    
    # 读取地图数据
    map_data = parse_map_file(input_filename)
    if not map_data:
        return False
    
    # 验证数据
    if not validate_map_data(map_data):
        return False
    
    # 生成OBJ文件
    generate_obj_file(map_data, obj_filename)
    return True

def generate_obj_file(map_data, filename="horror_map.obj"):
    """生成OBJ文件的函数（使用共享顶点优化）"""
    height = len(map_data)
    width = len(map_data[0]) if height > 0 else 0
    
    def rotate_x_90(x, y, z):
        """绕x轴旋转90度的变换: (x, y, z) -> (x, -z, y)"""
        return (x, -z, y)
    
    # 预先计算所有可能需要的顶点坐标
    wall_height = 3.0
    vertex_map = {}  # 坐标到顶点索引的映射
    vertices = []    # 所有顶点列表
    vertex_count = 0
    
    def add_vertex(x, y, z):
        """添加顶点，如果已存在则返回现有索引"""
        nonlocal vertex_count
        coord = (x, y, z)
        if coord not in vertex_map:
            vertices.append(rotate_x_90(x, y, z))
            vertex_map[coord] = vertex_count
            vertex_count += 1
        return vertex_map[coord]
    
    # 1. 添加地板顶点
    floor_indices = []
    floor_coords = [(0, 0, 0), (width, 0, 0), (width, height, 0), (0, height, 0)]
    for coord in floor_coords:
        floor_indices.append(add_vertex(*coord))
    
    # 2. 为所有墙壁和天花板预先生成顶点
    wall_blocks = []
    ceiling_tiles = []
    light_tiles = []
    
    for y in range(height):
        for x in range(width):
            if map_data[y][x] == 1:  # 墙壁
                # 墙壁立方体的8个顶点
                wall_vertices = [
                    add_vertex(x, y, 0),           # 底面左下
                    add_vertex(x+1, y, 0),         # 底面右下
                    add_vertex(x+1, y+1, 0),       # 底面右上
                    add_vertex(x, y+1, 0),         # 底面左上
                    add_vertex(x, y, wall_height), # 顶面左下
                    add_vertex(x+1, y, wall_height), # 顶面右下
                    add_vertex(x+1, y+1, wall_height), # 顶面右上
                    add_vertex(x, y+1, wall_height)  # 顶面左上
                ]
                wall_blocks.append(wall_vertices)
            
            elif map_data[y][x] == 0:  # 普通空地天花板
                ceiling_vertices = [
                    add_vertex(x, y, wall_height),     # 左下
                    add_vertex(x+1, y, wall_height),   # 右下
                    add_vertex(x+1, y+1, wall_height), # 右上
                    add_vertex(x, y+1, wall_height)    # 左上
                ]
                ceiling_tiles.append(ceiling_vertices)
            
            elif map_data[y][x] == 2:  # 灯光天花板
                light_vertices = [
                    add_vertex(x, y, wall_height),     # 左下
                    add_vertex(x+1, y, wall_height),   # 右下
                    add_vertex(x+1, y+1, wall_height), # 右上
                    add_vertex(x, y+1, wall_height)    # 左上
                ]
                light_tiles.append(light_vertices)
    
    # 写入OBJ文件
    with open(filename, 'w') as f:
        f.write("# Horror Map OBJ File (Optimized with Shared Vertices)\n")
        f.write(f"# Generated from {height}x{width} map data\n")
        f.write(f"# Total vertices: {len(vertices)} (optimized)\n")
        f.write("# Includes floor, walls, individual ceiling tiles, and light fixtures\n")
        f.write("# Model rotated 90° around X-axis for horizontal orientation\n")
        f.write("# Values: 0=empty, 1=wall, 2=light\n\n")
        
        # 添加材质库文件引用
        f.write("mtllib horror_map.mtl\n\n")
        
        # 写入所有顶点
        f.write("# All vertices (shared and optimized)\n")
        for i, vertex in enumerate(vertices):
            f.write(f"v {vertex[0]} {vertex[1]} {vertex[2]}\n")
        f.write("\n")
        
        # 写入地板面
        f.write("# Floor face\n")
        f.write("usemtl floor_material\n")
        # OBJ索引从1开始，所以需要+1
        floor_face = [idx + 1 for idx in floor_indices]
        f.write(f"f {floor_face[0]} {floor_face[1]} {floor_face[2]} {floor_face[3]}\n\n")
        
        # 写入墙壁面
        f.write("# Wall faces\n")
        f.write("usemtl wall_material\n")
        for wall_vertices in wall_blocks:
            # 转换为OBJ索引（+1）
            v = [idx + 1 for idx in wall_vertices]
            
            # 立方体的6个面
            faces = [
                [v[0], v[1], v[2], v[3]],      # 底面
                [v[4], v[7], v[6], v[5]],      # 顶面
                [v[0], v[4], v[5], v[1]],      # 前面
                [v[2], v[6], v[7], v[3]],      # 后面
                [v[0], v[3], v[7], v[4]],      # 左面
                [v[1], v[5], v[6], v[2]]       # 右面
            ]
            
            for face in faces:
                f.write(f"f {face[0]} {face[1]} {face[2]} {face[3]}\n")
        f.write("\n")
        
        # 写入普通天花板面
        if ceiling_tiles:
            f.write("# Ceiling tiles\n")
            f.write("usemtl ceiling_material\n")
            for ceiling_vertices in ceiling_tiles:
                # 转换为OBJ索引（+1）
                v = [idx + 1 for idx in ceiling_vertices]
                # 天花板面朝下
                f.write(f"f {v[0]} {v[3]} {v[2]} {v[1]}\n")
            f.write("\n")
        
        # 写入灯光天花板面
        if light_tiles:
            f.write("# Light ceiling tiles\n")
            f.write("usemtl light_material\n")
            for light_vertices in light_tiles:
                # 转换为OBJ索引（+1）
                v = [idx + 1 for idx in light_vertices]
                # 天花板面朝下
                f.write(f"f {v[0]} {v[3]} {v[2]} {v[1]}\n")
            f.write("\n")
        
        print(f"OBJ file '{filename}' generated successfully!")
        print(f"Generated {len(wall_blocks)} walls, {len(ceiling_tiles)} ceiling tiles, and {len(light_tiles)} light ceiling tiles")
        print(f"Total vertices: {len(vertices)} (optimized with shared vertices)")
        print(f"Vertex reduction: {((len(wall_blocks)*8 + len(ceiling_tiles)*4 + len(light_tiles)*4 + 4) - len(vertices))} vertices saved")
        print("Model rotated 90° around X-axis for horizontal orientation")

# 使用示例
if __name__ == "__main__":
    print("地图文件转换工具")
    print("=" * 40)
    
    # 创建示例文件（可选）
    # create_sample_map_file()
    
    # 方法1：直接转换为OBJ
    input_file = input("请输入地图文件名（直接回车使用sample_map.txt）：").strip()
    if not input_file:
        input_file = "sample_map.txt"
    
    if convert_map_file_to_obj(input_file):
        print("转换成功！")
    else:
        print("转换失败！")
    
    # 方法2：只解析并保存为Python格式
    # save_python = input("是否保存为Python格式？(y/n)：").strip().lower()
    # if save_python == 'y':
    #     map_data = parse_map_file(input_file)
    #     if map_data:
    #         save_map_as_python(map_data, "map_data.py")