#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SlideMazePlayerActor.generated.h"

class USkeletalMeshComponent;

UCLASS()
class NOTABOT_API ASlideMazePlayerActor : public AActor
{
	GENERATED_BODY()

public:
	ASlideMazePlayerActor();

	UFUNCTION(BlueprintCallable, Category="SlideMaze|Player")
	void SetGridPosition(FIntPoint NewGrid, FVector NewWorldLocation);

	UFUNCTION(BlueprintCallable, Category="SlideMaze|Player")
	void SetVisualState(bool bIsActive);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|Player")
	TObjectPtr<USceneComponent> Root;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|Player")
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlideMaze|Player")
	FIntPoint CurrentGrid = FIntPoint::ZeroValue;
};
