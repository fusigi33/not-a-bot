#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakoutBoundary.generated.h"

class ABreakoutBall;
class UBoxComponent;

UENUM(BlueprintType)
enum class EBreakoutBoundaryType : uint8
{
	Wall,
	KillZone
};

UCLASS()
class NOTABOT_API ABreakoutBoundary : public AActor
{
	GENERATED_BODY()

public:
	ABreakoutBoundary();

	UFUNCTION(BlueprintPure, Category="Breakout|Boundary")
	bool IsKillZone() const { return BoundaryType == EBreakoutBoundaryType::KillZone; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Breakout|Components")
	TObjectPtr<USceneComponent> RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Breakout|Components")
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Boundary")
	EBreakoutBoundaryType BoundaryType = EBreakoutBoundaryType::Wall;

private:
	UFUNCTION()
	void OnBoundaryBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void RefreshCollisionMode();
};
