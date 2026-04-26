#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakoutCaptureActor.generated.h"

class USceneCaptureComponent2D;
class USceneComponent;
class UTextureRenderTarget2D;

UCLASS()
class NOTABOT_API ABreakoutCaptureActor : public AActor
{
	GENERATED_BODY()

public:
	ABreakoutCaptureActor();

	UFUNCTION(BlueprintPure, Category="Breakout|Capture")
	UTextureRenderTarget2D* GetRenderTarget() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Breakout|Components")
	TObjectPtr<USceneComponent> RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Breakout|Components")
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;
};
