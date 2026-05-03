#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakoutBrick.generated.h"

class ABreakoutBall;
class UBoxComponent;
class UNiagaraSystem;
class UStaticMeshComponent;

UCLASS()
class NOTABOT_API ABreakoutBrick : public AActor
{
	GENERATED_BODY()

public:
	ABreakoutBrick();

	UFUNCTION(BlueprintCallable, Category="Breakout|Brick")
	void HandleBallHit(ABreakoutBall* Ball);

	UFUNCTION(BlueprintPure, Category="Breakout|Brick")
	FVector GetBrickSize() const;

protected:
	void PlayBreakEffect() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Breakout|Components")
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Breakout|Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Brick", meta=(ClampMin="1"))
	int32 HitPoints = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Effects")
	TObjectPtr<UNiagaraSystem> BreakEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Effects")
	FVector BreakEffectScale = FVector(1.0f);
};
