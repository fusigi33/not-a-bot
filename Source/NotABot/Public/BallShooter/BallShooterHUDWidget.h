#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BallShooterHUDWidget.generated.h"

class UImage;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UTextureRenderTarget2D;

/**
 * BallShooter 보드 캡처 화면을 표시하는 HUD 위젯입니다.
 * 렌더 타깃을 머티리얼 인스턴스에 연결해 이미지 위젯에 출력합니다.
 */
UCLASS()
class NOTABOT_API UBallShooterHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 보드 캡처 화면을 표시할 머티리얼과 렌더 타깃을 초기화합니다.
	 * @param BaseMaterial 렌더 타깃을 표시할 기본 머티리얼입니다.
	 * @param RenderTarget SceneCapture가 출력하는 렌더 타깃입니다.
	 */
	UFUNCTION(BlueprintCallable, Category="BallShooter|UI")
	void InitializeBoardView(UMaterialInterface* BaseMaterial, UTextureRenderTarget2D* RenderTarget);

protected:
	/** 보드 전경을 표시하는 이미지 위젯입니다. */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_BoardView;

	/** 렌더 타깃을 머티리얼에 주입할 때 사용할 파라미터 이름입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|UI")
	FName RenderTargetParameterName = TEXT("CapturedTexture");

	/** 현재 보드 화면을 그리는 동적 머티리얼 인스턴스입니다. */
	UPROPERTY(BlueprintReadOnly, Category="BallShooter|UI")
	TObjectPtr<UMaterialInstanceDynamic> BoardViewMaterialInstance;
};
