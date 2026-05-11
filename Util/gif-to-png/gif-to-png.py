from PIL import Image
import os

def remove_white_background(gif_path, output_folder='extracted_frames', threshold=240):
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    with Image.open(gif_path) as im:
        for i in range(im.n_frames):
            im.seek(i)
            
            # 1. 일단 RGBA로 변환
            frame = im.convert("RGBA")
            datas = frame.getdata()

            # 2. 픽셀별로 검사하여 흰색 계열을 투명하게 처리
            new_data = []
            for item in datas:
                # item[0, 1, 2]는 각각 R, G, B 값 (0~255)
                # 세 값이 모두 threshold보다 높으면 흰색으로 판단하고 투명화
                if item[0] > threshold and item[1] > threshold and item[2] > threshold:
                    new_data.append((255, 255, 255, 0))  # 투명으로 변경
                else:
                    new_data.append(item)  # 원래 색상 유지

            # 3. 데이터 업데이트 및 저장
            frame.putdata(new_data)
            
            frame_name = f"frame_{i:03d}.png"
            save_path = os.path.join(output_folder, frame_name)
            frame.save(save_path, "PNG")
            print(f"저장 완료: {save_path} (흰색 제거 적용)")

# threshold를 255로 하면 완전한 흰색만, 낮출수록 약간 밝은 회색까지 투명해집니다.
remove_white_background('C:\\Users\\최찬우\\Downloads\\gif-to-png\\icons8-로딩-서클.gif', threshold=250)