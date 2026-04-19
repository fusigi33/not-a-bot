#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShellBall.generated.h"

class AShellCup;
class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class NOTABOT_API AShellBall : public AActor
{
	GENERATED_BODY()

public:
	AShellBall();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Shell Game")
	void AttachToCup(AShellCup* InCup, const FVector& InLocalOffset);

	UFUNCTION(BlueprintCallable, Category="Shell Game")
	void DetachFromCup();

	UFUNCTION(BlueprintCallable, Category="Shell Game")
	void SetBallVisible(bool bVisible);

	UFUNCTION(BlueprintPure, Category="Shell Game")
	AShellCup* GetOwningCup() const { return OwningCup.Get(); }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	TObjectPtr<USceneComponent> RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	TObjectPtr<UStaticMeshComponent> BallMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	FVector LocalOffset = FVector(0.0f, 0.0f, -24.0f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	TObjectPtr<AShellCup> OwningCup;
};
