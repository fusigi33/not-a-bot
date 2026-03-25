from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import random

# =========================
# 사용자 설정
# =========================

# 폰트명
FONT_NAME = "AzeretMono"

# 출력 폴더
OUTPUT_DIR = Path(f"captcha_chars\\{FONT_NAME}")

# 사용할 문자 집합
CHARSET = "abcdefghijklmnopqrstuvwxyz0123456789"

# 글자별 몇 벌 생성할지
VARIANTS_PER_CHAR = 3

# 캔버스 크기
CANVAS_SIZE = (128, 128)

# 폰트 경로
# 예:
# Windows: r"C:\Windows\Fonts\arial.ttf"
# macOS:   "/System/Library/Fonts/Supplemental/Arial.ttf"
# 직접 받은 폰트: r"D:\Fonts\MyHandwritingFont.ttf"
FONT_PATH = r"C:\Users\최찬우\Downloads\Azeret_Mono,IBM_Plex_Mono,Special_Elite\Azeret_Mono\AzeretMono-VariableFont_wght.ttf"

# 기본 폰트 크기
BASE_FONT_SIZE = 72

# 글자 색상 (검정)
TEXT_COLOR = (0, 0, 0, 255)

# 랜덤 범위
# ROTATION_RANGE = (-12, 12)     # 회전 각도
# OFFSET_X_RANGE = (-6, 6)       # 좌우 오프셋
# OFFSET_Y_RANGE = (-8, 8)       # 상하 오프셋
ROTATION_RANGE = (-18, 18)
OFFSET_X_RANGE = (-8, 8)
OFFSET_Y_RANGE = (-10, 10)
FONT_SIZE_JITTER = (-6, 6)     # 폰트 크기 변동

# 랜덤 시드 고정 여부 (재현 가능한 결과가 필요하면 숫자로 지정)
RANDOM_SEED = None
# RANDOM_SEED = 42

# 파일명 prefix
FILE_PREFIX = "T_Captcha"

# 이미지 여백 보정
PADDING = 12


# =========================
# 유틸
# =========================

def ensure_output_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def get_font(font_path: str, font_size: int) -> ImageFont.FreeTypeFont:
    return ImageFont.truetype(font_path, font_size)


def measure_text_bbox(char: str, font: ImageFont.FreeTypeFont):
    """
    글자의 실제 bounding box를 얻는다.
    """
    dummy_img = Image.new("RGBA", (512, 512), (0, 0, 0, 0))
    draw = ImageDraw.Draw(dummy_img)
    return draw.textbbox((0, 0), char, font=font)


def render_character_to_layer(
    char: str,
    font_path: str,
    canvas_size: tuple[int, int],
    base_font_size: int,
) -> Image.Image:
    """
    문자 하나를 투명 레이어에 그린 뒤 약간 회전/오프셋을 준 이미지를 반환.
    """
    width, height = canvas_size

    font_size = max(8, base_font_size + random.randint(*FONT_SIZE_JITTER))
    font = get_font(font_path, font_size)

    bbox = measure_text_bbox(char, font)
    text_w = bbox[2] - bbox[0]
    text_h = bbox[3] - bbox[1]

    # 글자용 임시 레이어
    layer = Image.new("RGBA", canvas_size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)

    # bbox의 left/top 보정 포함해서 중앙 정렬
    center_x = width // 2
    center_y = height // 2

    x = center_x - text_w // 2 - bbox[0]
    y = center_y - text_h // 2 - bbox[1]

    # 랜덤 오프셋
    x += random.randint(*OFFSET_X_RANGE)
    y += random.randint(*OFFSET_Y_RANGE)

    draw.text((x, y), char, font=font, fill=TEXT_COLOR)

    # 회전
    angle = random.uniform(*ROTATION_RANGE)
    rotated = layer.rotate(
        angle,
        resample=Image.Resampling.BICUBIC,
        expand=False,
        center=(width / 2, height / 2),
    )
    return rotated


def crop_to_content_with_padding(img: Image.Image, padding: int) -> Image.Image:
    """
    투명 영역을 제외한 실제 글자 영역만 crop하되, 회전 대비 여백을 조금 남긴다.
    """
    bbox = img.getbbox()
    if bbox is None:
        return img

    left, top, right, bottom = bbox
    left = max(0, left - padding)
    top = max(0, top - padding)
    right = min(img.width, right + padding)
    bottom = min(img.height, bottom + padding)

    return img.crop((left, top, right, bottom))


def save_character_image(img: Image.Image, output_path: Path) -> None:
    img.save(output_path, format="PNG")


def build_filename(char: str, variant_index: int) -> str:
    return f"{FILE_PREFIX}_{char}_{variant_index:02d}.png"


# =========================
# 메인
# =========================

def main() -> None:
    if RANDOM_SEED is not None:
        random.seed(RANDOM_SEED)

    font_path = Path(FONT_PATH)
    if not font_path.exists():
        raise FileNotFoundError(
            f"FONT_PATH가 올바르지 않음: {font_path}\n"
            f"스크립트 상단의 FONT_PATH를 실제 .ttf 또는 .otf 경로로 바꿔주세요."
        )

    ensure_output_dir(OUTPUT_DIR)

    total = 0
    for char in CHARSET:
        for variant in range(1, VARIANTS_PER_CHAR + 1):
            img = render_character_to_layer(
                char=char,
                font_path=str(font_path),
                canvas_size=CANVAS_SIZE,
                base_font_size=BASE_FONT_SIZE,
            )

            cropped = crop_to_content_with_padding(img, PADDING)

            filename = build_filename(char, variant)
            output_path = OUTPUT_DIR / filename
            save_character_image(cropped, output_path)

            total += 1
            print(f"Saved: {output_path}")

    print(f"\n완료: {total}개 생성됨")
    print(f"출력 폴더: {OUTPUT_DIR.resolve()}")


if __name__ == "__main__":
    main()