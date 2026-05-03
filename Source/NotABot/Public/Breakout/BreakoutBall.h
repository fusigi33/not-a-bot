#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakoutBall.generated.h"

class ABreakoutBoundary;
class ABreakoutBrick;
class ABreakoutGameManager;
class ABreakoutPaddle;
class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class NOTABOT_API ABreakoutBall : public AActor
{
	GENERATED_BODY()

public:
	ABreakoutBall();
	
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category="Breakout|Ball")
	void LaunchBall();

	UFUNCTION(BlueprintCallable, Category="Breakout|Ball")
	void ResetBall();

	UFUNCTION(BlueprintCallable, Category="Breakout|Ball")
	void StopBall();

	UFUNCTION(BlueprintPure, Category="Breakout|Ball")
	FVector GetBallVelocity() const;

	UFUNCTION(BlueprintCallable, Category="Breakout|Ball")
	void HandleKillZoneOverlap();

	UFUNCTION(BlueprintCallable, Category="Breakout|Ball")
	void SetGameManager(ABreakoutGameManager* InGameManager) { GameManager = InGameManager; }

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Breakout|Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Breakout|Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Breakout|Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Breakout|References")
	TObjectPtr<ABreakoutPaddle> Paddle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Launch")
	FVector InitialDirection = FVector(0.0f, 0.35f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Speed", meta=(ClampMin="1.0"))
	float InitialSpeed = 950.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Speed", meta=(ClampMin="1.0"))
	float MaxSpeed = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Speed", meta=(ClampMin="0.0"))
	float SpeedIncreasePerPaddleHit = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Reflection", meta=(ClampMin="1.0", ClampMax="89.0"))
	float MinLaunchAngleDegrees = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Reflection", meta=(ClampMin="1.0", ClampMax="89.0"))
	float MaxLaunchAngleDegrees = 72.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Reflection", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MaxHorizontalRatio = 0.92f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Reflection", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MaxVerticalRatio = 0.96f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Reflection", meta=(ClampMin="0.0"))
	float SurfaceSeparationDistance = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Reflection", meta=(ClampMin="0.0"))
	float SameActorBounceCooldown = 0.03f;

private:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Breakout|References", meta=(AllowPrivateAccess="true"))
	TObjectPtr<ABreakoutGameManager> GameManager;

	FVector SpawnLocation = FVector::ZeroVector;
	FVector Velocity = FVector::ZeroVector;
	bool bBallActive = false;
	float TimeSinceLastHit = BIG_NUMBER;
	TWeakObjectPtr<AActor> LastHitActor;

	UFUNCTION()
	void HandleProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	void HandleBlockingHit(const FHitResult& Hit);
	void HandlePaddleBounce(ABreakoutPaddle* HitPaddle, const FHitResult& Hit);
	void MaintainMinimumSpeed();
	void ClampTravelDirection();
	FVector BuildPaddleBounceDirection(const ABreakoutPaddle* HitPaddle, const FHitResult& Hit) const;
	float GetCurrentSpeed() const;
	bool CanBounceOnActor(const AActor* HitActor) const;
};
