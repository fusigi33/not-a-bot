from __future__ import annotations

import argparse
from pathlib import Path
from PIL import Image

#################################################################################################################
# 사용법
# 사용 예시: python compress_image.py input.jpg output.jpg --quality 65
# 해상도까지 줄이려면: python compress_image.py input.jpg output.jpg --quality 65 --max-width 1280 --max-height 720
#################################################################################################################

def compress_image(
    input_path: Path,
    output_path: Path,
    quality: int = 75,
    max_width: int | None = None,
    max_height: int | None = None,
) -> None:
    with Image.open(input_path) as img:
        img_format = (img.format or input_path.suffix.replace(".", "")).upper()

        # RGBA/P 모드 이미지를 JPEG로 저장할 때 RGB 변환 필요
        if img_format in {"JPG", "JPEG"} and img.mode in ("RGBA", "P"):
            img = img.convert("RGB")

        # 해상도 축소
        if max_width or max_height:
            width, height = img.size
            target_width = max_width or width
            target_height = max_height or height
            img.thumbnail((target_width, target_height))

        save_kwargs = {}

        if output_path.suffix.lower() in [".jpg", ".jpeg"]:
            if img.mode in ("RGBA", "P"):
                img = img.convert("RGB")
            save_kwargs.update({
                "quality": quality,
                "optimize": True,
            })

        elif output_path.suffix.lower() == ".png":
            save_kwargs.update({
                "optimize": True,
            })

        elif output_path.suffix.lower() == ".webp":
            save_kwargs.update({
                "quality": quality,
                "method": 6,
            })

        else:
            raise ValueError("지원하는 출력 형식은 jpg, jpeg, png, webp 입니다.")

        img.save(output_path, **save_kwargs)


def main() -> None:
    parser = argparse.ArgumentParser(description="이미지 용량 압축 스크립트")
    parser.add_argument("input", type=str, help="원본 이미지 경로")
    parser.add_argument("output", type=str, help="저장할 이미지 경로")
    parser.add_argument(
        "--quality",
        type=int,
        default=75,
        help="압축 품질 (1~100, 기본값: 75, JPG/WEBP에 주로 적용)",
    )
    parser.add_argument(
        "--max-width",
        type=int,
        default=None,
        help="최대 너비 지정",
    )
    parser.add_argument(
        "--max-height",
        type=int,
        default=None,
        help="최대 높이 지정",
    )

    args = parser.parse_args()

    input_path = Path(args.input)
    output_path = Path(args.output)

    if not input_path.exists():
        raise FileNotFoundError(f"입력 파일을 찾을 수 없습니다: {input_path}")

    compress_image(
        input_path=input_path,
        output_path=output_path,
        quality=max(1, min(args.quality, 100)),
        max_width=args.max_width,
        max_height=args.max_height,
    )

    original_size = input_path.stat().st_size
    compressed_size = output_path.stat().st_size

    print(f"압축 완료: {output_path}")
    print(f"원본 크기: {original_size / 1024:.2f} KB")
    print(f"압축 크기: {compressed_size / 1024:.2f} KB")
    print(f"절감 비율: {(1 - compressed_size / original_size) * 100:.2f}%")


if __name__ == "__main__":
    main()