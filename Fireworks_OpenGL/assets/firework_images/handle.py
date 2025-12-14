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

def remove_black_gradient(input_path, output_path, min_threshold=20, max_threshold=150):
    """
    去除图片黑色背景，保留渐变透明效果（推荐用于有发光、过渡效果的图片）
    
    参数:
        input_path: 输入图片路径
        output_path: 输出图片路径
        min_threshold: 完全透明的阈值（亮度低于此值=完全透明），默认20
        max_threshold: 完全不透明的阈值（亮度高于此值=完全不透明），默认150
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
        
        for y in range(height):
            for x in range(width):
                r, g, b, a = pixels[x, y]
                
                # 计算像素亮度（使用加权平均，更接近人眼感知）
                brightness = 0.299 * r + 0.587 * g + 0.114 * b
                
                # 根据亮度计算透明度
                if brightness < min_threshold:
                    # 完全透明
                    new_alpha = 0
                    transparent_count += 1
                elif brightness > max_threshold:
                    # 完全不透明
                    new_alpha = 255
                    opaque_count += 1
                else:
                    # 渐变透明（线性插值）
                    ratio = (brightness - min_threshold) / (max_threshold - min_threshold)
                    new_alpha = int(ratio * 255)
                    semi_transparent_count += 1
                
                pixels[x, y] = (r, g, b, new_alpha)
        
        # 保存处理后的图片
        img.save(output_path, "PNG")
        
        total = width * height
        print(f"✓ 渐变透明处理完成！")
        print(f"  输入: {input_path}")
        print(f"  输出: {output_path}")
        print(f"  图片尺寸: {width}x{height}")
        print(f"  完全透明: {transparent_count} ({transparent_count/total*100:.1f}%)")
        print(f"  半透明: {semi_transparent_count} ({semi_transparent_count/total*100:.1f}%)")
        print(f"  不透明: {opaque_count} ({opaque_count/total*100:.1f}%)")
        print(f"  亮度范围: {min_threshold} (透明) ~ {max_threshold} (不透明)")
        
    except Exception as e:
        print(f"✗ 错误: {e}")
        sys.exit(1)

def main():
    # 使用说明
    if len(sys.argv) < 2:
        print("用法:")
        print("  python handle.py <输入图片> [输出图片] [阈值]")
        print()
        print("参数:")
        print("  输入图片   - 必需，要处理的图片路径")
        print("  输出图片   - 可选，默认为 输入图片_transparent.png")
        print("  阈值       - 可选，黑色判定阈值(0-765)，默认50")
        print()
        print("示例:")
        print("  python handle.py fish.png")
        print("  python handle.py fish.png fish_output.png")
        print("  python handle.py fish.png fish_output.png 80")
        sys.exit(1)
    
    # 获取参数
    input_path = sys.argv[1]
    
    # 检查输入文件是否存在
    if not os.path.exists(input_path):
        print(f"✗ 错误: 输入文件不存在: {input_path}")
        sys.exit(1)
    
    # 输出路径
    if len(sys.argv) >= 3:
        output_path = sys.argv[2]
    else:
        # 默认输出路径：在原文件名后加 _transparent
        name, ext = os.path.splitext(input_path)
        output_path = f"{name}_transparent.png"
    
    # 阈值
    threshold = 50
    if len(sys.argv) >= 4:
        try:
            threshold = int(sys.argv[3])
            if threshold < 0 or threshold > 765:
                print(f"✗ 警告: 阈值应在0-765之间，使用默认值50")
                threshold = 50
        except ValueError:
            print(f"✗ 警告: 阈值必须是整数，使用默认值50")
    
    # 处理图片
    remove_black_background(input_path, output_path, threshold)

if __name__ == "__main__":
    main()
