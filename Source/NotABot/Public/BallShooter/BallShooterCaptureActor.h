#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BallShooterCaptureActor.generated.h"

class USceneCaptureComponent2D;
class USceneComponent;
class UTextureRenderTarget2D;

/**
 * 보드 전경을 렌더 타깃으로 캡처하는 보조 액터입니다.
 * HUD 위젯에서 사용하는 SceneCaptureComponent2D를 소유합니다.
 */
UCLASS()
class NOTABOT_API ABallShooterCaptureActor : public AActor
{
	GENERATED_BODY()

public:
	/** 캡처용 루트와 SceneCapture 컴포넌트를 생성합니다. */
	ABallShooterCaptureActor();

	/** 장면 캡처 컴포넌트를 반환합니다. */
	UFUNCTION(BlueprintPure, Category="BallShooter|Capture")
	USceneCaptureComponent2D* GetSceneCapture() const { return SceneCapture; }

	/** 현재 SceneCapture에 연결된 렌더 타깃을 반환합니다. */
	UFUNCTION(BlueprintPure, Category="BallShooter|Capture")
	UTextureRenderTarget2D* GetRenderTarget() const;

protected:
	/** 캡처 액터의 루트 씬 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BallShooter|Components")
	TObjectPtr<USceneComponent> RootScene;

	/** 보드 전경을 렌더 타깃에 기록하는 캡처 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BallShooter|Components")
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;
};
