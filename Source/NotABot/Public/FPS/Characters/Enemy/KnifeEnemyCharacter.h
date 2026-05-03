#pragma once

#include "CoreMinimal.h"
#include "EnemyShooterCharacter.h"
#include "KnifeEnemyCharacter.generated.h"

UCLASS()
class NOTABOT_API AKnifeEnemyCharacter : public AEnemyShooterCharacter
{
	GENERATED_BODY()

public:
	AKnifeEnemyCharacter();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category="AI|Target")
	AActor* GetRecognizedTarget() const { return RecognizedTarget.Get(); }

	UFUNCTION(BlueprintPure, Category="AI|Knife")
	bool IsCachedTargetInKnifeRange() const { return bCachedTargetInKnifeRange; }

	UFUNCTION(BlueprintPure, Category="AI|Knife")
	float GetCachedKnifeTargetDistance() const { return CachedKnifeTargetDistance; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI|Knife")
	bool bCachedTargetInKnifeRange = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI|Knife")
	float CachedKnifeTargetDistance = 0.f;
};
