#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShellGame/ShellGameTypes.h"
#include "ShellObstacleMover.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class USplineComponent;

UCLASS()
class NOTABOT_API AShellObstacleMover : public AActor
{
	GENERATED_BODY()

public:
	AShellObstacleMover();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category="Shell Game")
	void ActivateObstacleMovement();

	UFUNCTION(BlueprintCallable, Category="Shell Game")
	void DeactivateObstacleMovement();

protected:
	float GetPointArrayPathLength() const;
	FVector GetPointArrayLocationAtDistance(float Distance) const;
	FVector GetLocationAtDistance(float Distance) const;
	FVector GetMovementDirectionAtDistance(float Distance) const;
	void SetObstacleVisible(bool bVisible);
	void UpdateMovement(float DeltaSeconds);
	void StartMovementNow();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	TObjectPtr<USceneComponent> RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	TObjectPtr<UStaticMeshComponent> ObstacleMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game")
	TObjectPtr<USplineComponent> PathSpline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game")
	FShellObstacleMovementSettings MovementSettings;

private:
	bool bMovementActive = false;
	float TravelDistance = 0.0f;
	float CachedPointArrayLength = 0.0f;
	FTimerHandle ActivationTimerHandle;
};
