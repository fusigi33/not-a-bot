#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "BreakoutPaddle.generated.h"

class UBoxComponent;
class UFloatingPawnMovement;
class ABreakoutGameManager;
class UInputAction;
class UInputMappingContext;
class UStaticMeshComponent;

UCLASS()
class NOTABOT_API ABreakoutPaddle : public APawn
{
	GENERATED_BODY()

public:
	ABreakoutPaddle();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintPure, Category="Breakout|Paddle")
	float GetNormalizedHitOffset(const FVector& HitLocation) const;

	UFUNCTION(BlueprintPure, Category="Breakout|Paddle")
	FVector GetLaunchUpDirection() const;

	UFUNCTION(BlueprintPure, Category="Breakout|Paddle")
	FVector GetMoveAxis() const;

protected:
	void MoveInput(const FInputActionValue& Value);
	void ApplyMovement(float DeltaSeconds);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Breakout|Components")
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Breakout|Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Breakout|Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Breakout|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Movement", meta=(ClampMin="0.0"))
	float MoveSpeed = 1600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Movement", meta=(ClampMin="0.0"))
	float MaxTravelDistance = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Movement", meta=(ClampMin="0.0", ClampMax="1.0"))
	float InputSmoothing = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Reflection", meta=(ClampMin="1.0"))
	float EffectiveHalfWidth = 110.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Breakout|Reflection")
	FVector UpDirection = FVector::UpVector;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Breakout|References")
	TObjectPtr<ABreakoutGameManager> GameManager;

private:
	FVector InitialLocation = FVector::ZeroVector;
	float CurrentInput = 0.0f;
	float SmoothedInput = 0.0f;

	bool CanMove() const;
	void FindGameManagerIfMissing();

	UFUNCTION()
	void OnPaddleBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
