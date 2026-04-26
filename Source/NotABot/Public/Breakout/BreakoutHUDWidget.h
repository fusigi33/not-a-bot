#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BreakoutHUDWidget.generated.h"

class UImage;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UTextureRenderTarget2D;

UCLASS()
class NOTABOT_API UBreakoutHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Breakout|UI")
	void InitializeCaptureMaterial(UMaterialInterface* BaseMaterial, UTextureRenderTarget2D* RenderTarget);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_BreakoutView;

	UPROPERTY(BlueprintReadOnly, Category="Breakout|UI")
	TObjectPtr<UMaterialInstanceDynamic> CaptureMaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|UI")
	FName RenderTargetParameterName = TEXT("CapturedTexture");
};
