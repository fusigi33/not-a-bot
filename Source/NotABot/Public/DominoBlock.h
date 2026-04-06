#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DominoBlock.generated.h"

UCLASS()
class NOTABOT_API ADominoBlock : public AActor
{
	GENERATED_BODY()

public:
	ADominoBlock();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Domino")
	bool bIsStartDomino = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Domino")
	bool bIsEndDomino = false;

	UPROPERTY(BlueprintReadWrite, Category="Domino")
	FTransform InitialTransform;

	UFUNCTION(BlueprintCallable)
	void SetPhysicsEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable)
	void ResetDomino();

	UFUNCTION(BlueprintCallable)
	bool IsKnockedDown(float DotThreshold = 0.5f) const;
};