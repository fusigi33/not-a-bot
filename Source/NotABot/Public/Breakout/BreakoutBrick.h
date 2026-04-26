#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakoutBrick.generated.h"

class ABreakoutBall;
class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class NOTABOT_API ABreakoutBrick : public AActor
{
	GENERATED_BODY()

public:
	ABreakoutBrick();

	UFUNCTION(BlueprintCallable, Category="Breakout|Brick")
	void HandleBallHit(ABreakoutBall* Ball);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Breakout|Components")
	TObjectPtr<USceneComponent> RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Breakout|Components")
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Breakout|Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Brick", meta=(ClampMin="1"))
	int32 HitPoints = 1;
};
