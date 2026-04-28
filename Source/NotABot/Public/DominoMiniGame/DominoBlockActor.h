#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DominoMiniGame/DominoTypes.h"
#include "DominoBlockActor.generated.h"

class UBoxComponent;

/**
 * 월드에 배치되는 물리 도미노 블록 액터입니다.
 *
 * 이 클래스는 도미노의 메시, 충돌, 물리 활성/비활성, 미리보기 색상,
 * 시작 충격 적용, 쓰러짐 판정만 담당합니다.
 */
UCLASS()
class NOTABOT_API ADominoBlockActor : public AActor
{
	GENERATED_BODY()

public:
	ADominoBlockActor();

	/** 도미노를 배치 단계 상태로 전환합니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino")
	void SetPlacementMode(bool bInPlacementMode);

	/** 도미노 물리 시뮬레이션을 켜거나 끕니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino")
	void SetPhysicsEnabled(bool bEnabled);

	/** 시작 도미노에 지정 방향의 초기 충격을 가합니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino")
	void ApplyInitialFallImpulse(EDominoFallDirection Direction);

	/** 도미노가 지정 임계값 이상 넘어졌는지 반환합니다. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Domino")
	bool IsFallen() const;

	/** 미리보기 도미노가 현재 유효한 위치인지 색상으로 표시합니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino")
	void SetPreviewValid(bool bIsValid);

	/** 이 도미노를 미리보기 액터로 사용할지 설정합니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino")
	void SetAsPreview(bool bPreview);

	/** 라운드 시작 시 저장한 위치와 회전으로 되돌립니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino")
	void ResetDominoTransform();

	/** 도미노 메시 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** 배치 검사에 사용할 기본 박스 크기입니다. 메시 Bounds가 없을 때도 사용됩니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino|Placement")
	FVector PlacementHalfExtent = FVector(10.0f, 3.0f, 40.0f);

	/** 이전 선형 시작 충격 값입니다. 현재 시작 동작에는 사용하지 않고 에셋 호환용으로 유지합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino|Physics")
	float InitialImpulseStrength = 800.0f;

	/** 이전 시작 토크 값입니다. 현재 시작 동작에는 사용하지 않고 에셋 호환용으로 유지합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino|Physics")
	float InitialTorqueStrength = 250000.0f;

	/** 시작 도미노를 제자리에서 살짝 기울이는 각 임펄스 세기입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino|Physics", meta = (ClampMin = "0.0"))
	float InitialAngularImpulseStrength = 4.0f;

	/** Up Vector와 월드 Up Vector의 내적이 이 값 이하이면 쓰러졌다고 판단합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino|Physics", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float FallenDotThreshold = 0.5f;

	/** 일반 배치 완료 도미노에 사용할 머티리얼입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino|Material")
	TObjectPtr<UMaterialInterface> NormalMaterial;

	/** 미리보기 유효 상태 머티리얼입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino|Material")
	TObjectPtr<UMaterialInterface> PreviewValidMaterial;

	/** 미리보기 불가 상태 머티리얼입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino|Material")
	TObjectPtr<UMaterialInterface> PreviewInvalidMaterial;

	/** 현재 미리보기 액터인지 여부입니다. */
	UPROPERTY(BlueprintReadOnly, Category = "Domino")
	bool bIsPreview = false;

protected:
	virtual void BeginPlay() override;

	/** 최초 배치 트랜스폼입니다. Reset에서 사용합니다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Domino")
	FTransform InitialTransform;

	/** 미리보기/일반 상태에 맞는 머티리얼을 적용합니다. */
	void ApplyMaterial(UMaterialInterface* Material);
};
