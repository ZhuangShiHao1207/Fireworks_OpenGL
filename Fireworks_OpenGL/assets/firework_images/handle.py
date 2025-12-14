from PIL import Image
import sys
import os

def remove_black_background(input_path, output_path, threshold=50):
    """
    去除图片中接近黑色的背景，将其改为透明（硬边缘）
    
    参数:
        input_path: 输入图片路径
        output_path: 输出图片路径
        threshold: 黑色阈值(0-255)，像素RGB值之和小于此值将被视为黑色
    """
    try:
        # 打开图片
        img = Image.open(input_path)
        
        # 转换为RGBA模式（支持透明度）
        img = img.convert("RGBA")
        
        # 获取像素数据
        pixels = img.load()
        width, height = img.size
        
        # 处理每个像素
        processed_count = 0
        for y in range(height):
            for x in range(width):
                r, g, b, a = pixels[x, y]
                
                # 计算像素亮度（RGB之和）
                brightness = r + g + b
                
                # 如果接近黑色，将alpha设为0（完全透明）
                if brightness < threshold:
                    pixels[x, y] = (r, g, b, 0)
                    processed_count += 1
        
        # 保存处理后的图片
        img.save(output_path, "PNG")
        
        print(f"✓ 硬边缘处理完成！")
        print(f"  输入: {input_path}")
        print(f"  输出: {output_path}")
        print(f"  图片尺寸: {width}x{height}")
        print(f"  处理像素: {processed_count}/{width*height}")
        print(f"  黑色阈值: {threshold}")
        
    except Exception as e:
        print(f"✗ 错误: {e}")
        sys.exit(1)

def remove_black_gradient(input_path, output_path, min_threshold=20, max_threshold=150, color_threshold=30):
    """
    去除图片黑色背景，保留渐变透明效果（推荐用于有发光、过渡效果的图片）
    智能识别：只去除真正的黑色，保留有颜色的部分（如深蓝色）
    
    参数:
        input_path: 输入图片路径
        output_path: 输出图片路径
        min_threshold: 完全透明的阈值（亮度低于此值=完全透明），默认20
        max_threshold: 完全不透明的阈值（亮度高于此值=完全不透明），默认150
        color_threshold: 颜色差异阈值，默认30（RGB差异小于此值认为是黑色）
    """
    try:
        # 打开图片
        img = Image.open(input_path)
        
        # 转换为RGBA模式
        img = img.convert("RGBA")
        
        # 获取像素数据
        pixels = img.load()
        width, height = img.size
        
        # 处理每个像素
        transparent_count = 0
        semi_transparent_count = 0
        opaque_count = 0
        colored_preserved = 0  # 保留的有颜色的暗像素
        
        for y in range(height):
            for x in range(width):
                r, g, b, a = pixels[x, y]
                
                # 计算像素亮度（使用加权平均，更接近人眼感知）
                brightness = 0.299 * r + 0.587 * g + 0.114 * b
                
                # 判断是否接近黑色：RGB值的最大值和最小值差异小，说明是灰度/黑色
                # 如果差异大，说明有颜色（如深蓝色）
                rgb_max = max(r, g, b)
                rgb_min = min(r, g, b)
                color_difference = rgb_max - rgb_min
                
                # 同时判断亮度和黑色程度（色差）
                # 1. 亮度必须低于min_threshold
                # 2. 色差必须小于color_threshold（接近黑色）
                is_very_dark = brightness < min_threshold
                is_colorless = color_difference < color_threshold
                
                # 只有同时满足"亮度极低"和"无色彩"才完全透明
                if is_very_dark and is_colorless:
                    # 完全透明（真正的纯黑色）
                    new_alpha = 0
                    transparent_count += 1
                elif brightness > max_threshold:
                    # 完全不透明（明亮区域）
                    new_alpha = 255
                    opaque_count += 1
                else:
                    # 中间区域：根据亮度和色差综合判断
                    if color_difference > color_threshold:
                        # 有明显颜色（如深蓝），保留更高不透明度
                        # 使用RGB最大值计算渐变：20-100范围映射到51-255 alpha
                        if rgb_max < 20:
                            new_alpha = 0
                        elif rgb_max > 100:
                            new_alpha = 255
                        else:
                            # 线性映射：20->51 (20%), 100->255 (100%)
                            ratio = (rgb_max - 20) / (100 - 20)
                            new_alpha = int(51 + ratio * 204)
                        colored_preserved += 1
                    else:
                        # 灰色或接近黑色的过渡区域
                        # 使用亮度计算渐变：20-100范围映射到51-255 alpha
                        if brightness < 20:
                            new_alpha = 0
                        elif brightness > 100:
                            new_alpha = 255
                        else:
                            # 线性映射：20->51 (20%), 100->255 (100%)
                            ratio = (brightness - 20) / (100 - 20)
                            new_alpha = int(51 + ratio * 204)
                        semi_transparent_count += 1
                
                pixels[x, y] = (r, g, b, new_alpha)
        
        # 保存处理后的图片
        img.save(output_path, "PNG")
        
        total = width * height
        print(f"✓ 智能渐变透明处理完成！")
        print(f"  输入: {input_path}")
        print(f"  输出: {output_path}")
        print(f"  图片尺寸: {width}x{height}")
        print(f"  完全透明(黑色): {transparent_count} ({transparent_count/total*100:.1f}%)")
        print(f"  半透明(过渡): {semi_transparent_count} ({semi_transparent_count/total*100:.1f}%)")
        print(f"  保留有色暗部: {colored_preserved} ({colored_preserved/total*100:.1f}%)")
        print(f"  完全不透明: {opaque_count} ({opaque_count/total*100:.1f}%)")
        print(f"  参数: 亮度{min_threshold}~{max_threshold}, 色差阈值{color_threshold}")
        
    except Exception as e:
        print(f"✗ 错误: {e}")
        sys.exit(1)

def main():
    # 检查是否使用渐变模式
    use_gradient = False
    arg_offset = 0
    
    if sys.argv[1] == "-g":
        use_gradient = True
        arg_offset = 1
        if len(sys.argv) < 3:
            print("✗ 错误: -g 模式需要指定输入图片")
            sys.exit(1)
    
    # 获取参数
    input_path = sys.argv[1 + arg_offset]
    
    # 检查输入文件是否存在
    if not os.path.exists(input_path):
        print(f"✗ 错误: 输入文件不存在: {input_path}")
        sys.exit(1)
    
    # 输出路径
    if len(sys.argv) >= 3 + arg_offset:
        output_path = sys.argv[2 + arg_offset]
    else:
        # 默认输出路径
        name, ext = os.path.splitext(input_path)
        output_path = f"{name}_transparent.png"
    
    if use_gradient:
        # 渐变透明模式
        min_threshold = 39
        max_threshold = 95
        color_threshold = 50
        
        if len(sys.argv) >= 4 + arg_offset:
            try:
                min_threshold = int(sys.argv[3 + arg_offset])
            except ValueError:
                print(f"✗ 警告: 最小阈值必须是整数，使用默认值20")
        
        if len(sys.argv) >= 5 + arg_offset:
            try:
                max_threshold = int(sys.argv[4 + arg_offset])
            except ValueError:
                print(f"✗ 警告: 最大阈值必须是整数，使用默认值150")
        
        if len(sys.argv) >= 6 + arg_offset:
            try:
                color_threshold = int(sys.argv[5 + arg_offset])
            except ValueError:
                print(f"✗ 警告: 色差阈值必须是整数，使用默认值30")
        
        # 参数验证
        if min_threshold >= max_threshold:
            print(f"✗ 错误: 最小阈值({min_threshold})必须小于最大阈值({max_threshold})")
            sys.exit(1)
        
        remove_black_gradient(input_path, output_path, min_threshold, max_threshold, color_threshold)
    else:
        # 硬边缘模式
        threshold = 50
        if len(sys.argv) >= 4 + arg_offset:
            try:
                threshold = int(sys.argv[3 + arg_offset])
                if threshold < 0 or threshold > 765:
                    print(f"✗ 警告: 阈值应在0-765之间，使用默认值50")
                    threshold = 50
            except ValueError:
                print(f"✗ 警告: 阈值必须是整数，使用默认值50")
        
        remove_black_background(input_path, output_path, threshold)

if __name__ == "__main__":
    main()
