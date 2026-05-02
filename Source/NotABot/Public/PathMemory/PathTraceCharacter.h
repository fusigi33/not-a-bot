#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PathTraceCharacter.generated.h"

class USpringArmComponent;
class USceneCaptureComponent2D;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UEnhancedInputLocalPlayerSubsystem;

UCLASS()
class NOTABOT_API APathTraceCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APathTraceCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCameraComponent> FollowCamera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneCaptureComponent2D> PlayerCapture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PathTrace|Record")
	float RecordMinDistance = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="PathTrace|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="PathTrace|Input")
	TObjectPtr<UInputAction> MoveAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="PathTrace|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="PathTrace|Input")
	TObjectPtr<UInputAction> CursorAction;

private:
	bool bCanPlayerMove = false;
	TArray<FVector> RecordedPath;
	FVector LastRecordedLocation = FVector::ZeroVector;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void ShowMouseCursorWhilePlayerTurn();
	void HideMouseCursor();
	void ForceHideMouseCursor();
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;
	void RegisterMappingContext();

public:
	void SetCanPlayerMove(bool bEnable);
	void ResetRecordedPath();

	const TArray<FVector>& GetRecordedPath() const { return RecordedPath; }
};
