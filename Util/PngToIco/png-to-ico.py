from PIL import Image
from pathlib import Path

def png_to_ico(input_path: str, output_path: str = "Application.ico"):
    input_path = Path(input_path)
    output_path = Path(output_path)

    # PNG 열기
    img = Image.open(input_path).convert("RGBA")

    # 256x256으로 리사이즈
    img = img.resize((256, 256), Image.LANCZOS)

    # ICO로 저장
    img.save(output_path, format="ICO", sizes=[(256, 256)])

    print(f"Converted: {input_path} -> {output_path}")

if __name__ == "__main__":
    png_to_ico("notabotshortcut.png", "Application.ico")