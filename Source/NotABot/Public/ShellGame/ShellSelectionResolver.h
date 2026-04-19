#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ShellGame/ShellGameTypes.h"
#include "ShellSelectionResolver.generated.h"

class AShellCup;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

UCLASS(BlueprintType)
class NOTABOT_API UShellSelectionResolver : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Shell Game")
	AShellCup* ResolveCupFromCaptureUV(
		UObject* WorldContextObject,
		USceneCaptureComponent2D* SceneCapture,
		UTextureRenderTarget2D* RenderTarget,
		const FVector2D& UV,
		float TraceDistance,
		TEnumAsByte<ECollisionChannel> TraceChannel
	) const;

	UFUNCTION(BlueprintCallable, Category="Shell Game")
	FShellSelectionResult ResolveSelectionFromCaptureUV(
		UObject* WorldContextObject,
		USceneCaptureComponent2D* SceneCapture,
		UTextureRenderTarget2D* RenderTarget,
		const FVector2D& UV,
		float TraceDistance,
		TEnumAsByte<ECollisionChannel> TraceChannel
	) const;

protected:
	AShellCup* TraceCupFromCaptureUV(
		UObject* WorldContextObject,
		USceneCaptureComponent2D* SceneCapture,
		UTextureRenderTarget2D* RenderTarget,
		const FVector2D& UV,
		float TraceDistance,
		TEnumAsByte<ECollisionChannel> TraceChannel
	) const;

	void BuildWorldRayFromUV(
		USceneCaptureComponent2D* SceneCapture,
		UTextureRenderTarget2D* RenderTarget,
		const FVector2D& UV,
		FVector& OutOrigin,
		FVector& OutDirection
	) const;
};
