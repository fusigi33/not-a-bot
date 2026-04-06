#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PathTraceWidget.generated.h"

class UImage;
class UTextBlock;
class UMaterialInstanceDynamic;
class UTextureRenderTarget2D;

UCLASS()
class NOTABOT_API UPathTraceWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_BoardView;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PathTrace")
	TObjectPtr<UMaterialInterface> BoardViewMaterial;

private:
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BoardMID;
	
	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> PendingBoardRT;

public:
	UFUNCTION(BlueprintCallable)
	void SetBoardRenderTarget(UTextureRenderTarget2D* InRT);

	UFUNCTION(BlueprintCallable)
	void ShowResult(bool bSuccess, float AccuracyPercent);
};