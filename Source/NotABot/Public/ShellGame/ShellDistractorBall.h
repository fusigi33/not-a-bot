#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShellDistractorBall.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class NOTABOT_API AShellDistractorBall : public AActor
{
	GENERATED_BODY()

public:
	AShellDistractorBall();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Shell Game|Distractor")
	void Launch(const FVector& InitialVelocity);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game|Distractor")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game|Distractor")
	TObjectPtr<UStaticMeshComponent> BallMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shell Game|Distractor")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Distractor")
	float LifeSeconds = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Distractor")
	float BallRadius = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Distractor")
	float BounceFactor = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Distractor")
	float Friction = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shell Game|Distractor")
	float GravityScale = 1.35f;
};
