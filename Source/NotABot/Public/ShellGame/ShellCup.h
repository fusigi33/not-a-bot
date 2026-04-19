#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShellCup.generated.h"

class UBoxComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class NOTABOT_API AShellCup : public AActor
{
	GENERATED_BODY()

public:
	AShellCup();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category="Shell Game")
	void SnapToWorldLocation(const FVector& WorldLocation);

	UFUNCTION(BlueprintCallable, Category="Shell Game")
	void SetSelectionEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category="Shell Game")
	void StartReveal(bool bOpen, float Duration);

	UFUNCTION(BlueprintCallable, Category="Shell Game")
	void ForceCloseImmediate();

	UFUNCTION(BlueprintCallable, Category="Shell Game|Highlight")
	void SetHoverHighlighted(bool bHighlighted);

	UFUNCTION(BlueprintPure, Category="Shell Game")
	bool IsRevealOpen() const { return bRevealOpen; }

	UFUNCTION(BlueprintPure, Category="Shell Game")
	bool IsRevealAnimating() const { return bRevealAnimating; }

	UFUNCTION(BlueprintCallable, Category="Shell Game")
	void SetCurrentSlotIndex(int32 InSlotIndex) { CurrentSlotIndex = InSlotIndex; }

	UFUNCTION(BlueprintPure, Category="Shell Game")
	int32 GetCurrentSlotIndex() const { return CurrentSlotIndex; }

	UFUNCTION(BlueprintPure, Category="Shell Game")
	UBoxComponent* GetSelectionCollision() const { return SelectionCollision; }

protected:
	void ApplyRevealAlpha(float Alpha);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	TObjectPtr<USceneComponent> RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	TObjectPtr<UStaticMeshComponent> CupMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	TObjectPtr<UBoxComponent> SelectionCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Reveal")
	float RevealLiftHeight = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Reveal")
	float RevealPitch = -18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Highlight")
	TObjectPtr<UMaterialInterface> HoverOverlayMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Highlight")
	FLinearColor HoverHighlightColor = FLinearColor(1.0f, 0.42f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Highlight")
	FName HoverColorParameterName = TEXT("OutlineColor");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Highlight")
	FName HoverIntensityParameterName = TEXT("OutlineIntensity");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Highlight", meta=(ClampMin="0.0"))
	float HoverHighlightIntensity = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	int32 CurrentSlotIndex = INDEX_NONE;

private:
	void CacheBaseMaterials();
	bool ApplyHighlightToBaseMaterials();

	bool bRevealAnimating = false;
	bool bRevealOpen = false;
	float RevealDuration = 0.0f;
	float RevealElapsed = 0.0f;
	FRotator InitialClosedRotation = FRotator::ZeroRotator;
	FVector ClosedLocation = FVector::ZeroVector;
	FVector OpenLocation = FVector::ZeroVector;
	FRotator ClosedRotation = FRotator::ZeroRotator;
	FRotator OpenRotation = FRotator::ZeroRotator;
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> HoverOverlayMaterialInstance;
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInterface>> CachedBaseMaterials;
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> HoverBaseMaterialInstances;
};
