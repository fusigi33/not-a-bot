#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DominoMiniGame/DominoTypes.h"
#include "DominoMiniGameManager.generated.h"

class ADominoBlockActor;
class ADominoBoardActor;
class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDominoMiniGameResultSignature);

/**
 * 도미노 미니게임의 전체 라운드 상태를 관리하는 매니저 액터입니다.
 *
 * 배치 단계, 프리뷰 도미노, 실제 도미노 스폰, 물리 시뮬레이션 시작,
 * 목표 도미노 성공/실패 판정을 담당합니다.
 */
UCLASS()
class NOTABOT_API ADominoMiniGameManager : public AActor
{
	GENERATED_BODY()

public:
	ADominoMiniGameManager();

	/** 새 라운드를 시작하고 배치 단계로 진입합니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino MiniGame")
	void BeginRound();

	/** 현재 라운드를 초기 상태로 되돌립니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino MiniGame")
	void ResetRound();

	/** DataTable의 지정 Row를 읽어 RoundData에 적용합니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino MiniGame")
	bool LoadRoundDataFromDataTable(UDataTable* DataTable, FName RowName);

	/** 월드 위치에 미리보기 도미노를 갱신합니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino MiniGame")
	bool RequestPreviewDominoAtWorldLocation(const FVector& WorldLocation);

	/** 현재 미리보기 도미노를 실제 배치 도미노로 확정합니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino MiniGame")
	bool ConfirmPlacePreviewDomino();

	/** 현재 미리보기 도미노를 제거합니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino MiniGame")
	void CancelPreviewDomino();

	/** 주어진 위치와 회전에 도미노를 배치할 수 있는지 검사합니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino MiniGame")
	bool CanPlaceDominoAt(const FVector& WorldLocation, FRotator Rotation) const;

	/** 배치된 모든 도미노의 물리를 켜고 시작 도미노를 쓰러뜨립니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino MiniGame")
	void StartSimulation();

	/** 목표 도미노가 넘어졌는지 검사합니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino MiniGame")
	void CheckGoalDominoFallen();

	/** 성공 상태로 전환하고 Blueprint 이벤트/델리게이트를 호출합니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino MiniGame")
	void HandleSuccess();

	/** 실패 상태로 전환하고 Blueprint 이벤트/델리게이트를 호출합니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino MiniGame")
	void HandleFailure();

	/** Blueprint에서 수동으로 성공 이벤트를 호출할 수 있는 래퍼입니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino MiniGame")
	void OnDominoMiniGameSucceeded();

	/** Blueprint에서 수동으로 실패 이벤트를 호출할 수 있는 래퍼입니다. */
	UFUNCTION(BlueprintCallable, Category = "Domino MiniGame")
	void OnDominoMiniGameFailed();

	/** 성공 시 Blueprint에서 연출을 구현할 수 있는 이벤트입니다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Domino MiniGame")
	void BP_OnMiniGameSucceeded();

	/** 실패 시 Blueprint에서 연출을 구현할 수 있는 이벤트입니다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Domino MiniGame")
	void BP_OnMiniGameFailed();

	/** 성공 시 호출되는 동적 멀티캐스트 델리게이트입니다. */
	UPROPERTY(BlueprintAssignable, Category = "Domino MiniGame")
	FOnDominoMiniGameResultSignature OnMiniGameSucceeded;

	/** 실패 시 호출되는 동적 멀티캐스트 델리게이트입니다. */
	UPROPERTY(BlueprintAssignable, Category = "Domino MiniGame")
	FOnDominoMiniGameResultSignature OnMiniGameFailed;

	/** 라운드 설정 데이터입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino MiniGame")
	FDominoRoundData RoundData;

	/** 보드 영역 액터 참조입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino MiniGame")
	TObjectPtr<ADominoBoardActor> BoardActor;

	/** 레벨에 미리 배치된 시작 도미노입니다. 없으면 RoundData 위치에 자동 스폰합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino MiniGame")
	TObjectPtr<ADominoBlockActor> StartDomino;

	/** 레벨에 미리 배치된 목표 도미노입니다. 없으면 RoundData 위치에 자동 스폰합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino MiniGame")
	TObjectPtr<ADominoBlockActor> GoalDomino;

	/** 배치 검사에 사용할 도미노 절반 크기입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino MiniGame|Placement")
	FVector PlacementHalfExtent = FVector(12.0f, 4.0f, 42.0f);

	/** 보드 로컬 Y를 보드 두께 방향으로 보고, 도미노를 보드 앞쪽으로 보정합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino MiniGame|Placement")
	bool bPlaceDominoPivotAboveBoard = true;

	/** 도미노 피벗이 바닥에 있을 때, 메쉬 중심이 마우스 위치에 오도록 보정합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino MiniGame|Placement")
	bool bAlignDominoCenterToCursor = true;

	/** 배치 확정 시 도미노가 최종 위치보다 이 높이만큼 위에서 떨어지기 시작합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino MiniGame|Placement", meta = (ClampMin = "0.0"))
	float PlacementDropHeight = 300.0f;

	/** 보드 상단과 도미노 바닥 사이에 둘 여유 높이입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino MiniGame|Placement", meta = (ClampMin = "0.0"))
	float PlacementSurfaceClearance = 1.0f;

	/** 배치 단계에서 떨어진 도미노가 쉽게 쓰러지지 않도록 임시 무게중심 보정을 적용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino MiniGame|Placement")
	FVector PlacementCOMNudge = FVector(0.0f, 0.0f, -100.0f);

	/** 배치 단계 도미노의 임시 회전 댐핑입니다. 시뮬레이션 시작 시 0으로 복구합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino MiniGame|Placement", meta = (ClampMin = "0.0"))
	float PlacementAngularDamping = 3.0f;

	/** 배치 검사 시 사용할 쿼리 채널입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino MiniGame|Placement")
	TEnumAsByte<ECollisionChannel> PlacementTraceChannel = ECC_WorldDynamic;

	/** 현재 배치에 사용할 도미노 회전입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino MiniGame|Placement")
	FRotator CurrentPlacementRotation = FRotator::ZeroRotator;

	/** 목표 도미노 검사 주기입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domino MiniGame|Simulation", meta = (ClampMin = "0.01"))
	float GoalCheckInterval = 0.1f;

	/** 현재 라운드 상태입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domino MiniGame")
	EDominoMiniGameState CurrentState = EDominoMiniGameState::None;

	/** 배치된 실제 도미노 목록입니다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Domino MiniGame")
	TArray<TObjectPtr<ADominoBlockActor>> PlacedDominoes;

	/** 현재 미리보기 도미노입니다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Domino MiniGame")
	TObjectPtr<ADominoBlockActor> PreviewDomino;

	/** 라운드 데이터에서 스폰된 상호작용 오브젝트 목록입니다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Domino MiniGame")
	TArray<TObjectPtr<AActor>> SpawnedInteractiveObjects;

	/** 현재 미리보기 위치가 유효한지 여부입니다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Domino MiniGame")
	bool bPreviewPlacementValid = false;

protected:
	virtual void BeginPlay() override;

	/** 시작/목표 도미노를 준비합니다. */
	void EnsureEndpointDominoes();

	/** 라운드 데이터에 등록된 상호작용 오브젝트를 스폰합니다. */
	void SpawnInteractiveObjects();

	/** 인터페이스를 구현한 상호작용 오브젝트의 시뮬레이션 활성 상태를 변경합니다. */
	void SetInteractiveObjectsSimulationEnabled(bool bEnabled);

	/** 도미노 한 개를 스폰합니다. */
	ADominoBlockActor* SpawnDomino(const FVector& Location, const FRotator& Rotation, bool bPreview) const;

	/** 보드 표면 위치를 실제 도미노 액터 피벗 위치로 변환합니다. */
	FVector GetDominoActorLocationFromBoardLocation(const FVector& BoardLocation) const;

	/** 실제 도미노 액터 피벗 위치를 보드 표면 위치로 되돌립니다. */
	FVector GetBoardLocationFromDominoActorLocation(const FVector& ActorLocation) const;

	/** 실제 도미노 액터 피벗 위치에서 배치 검사 박스의 중심 위치를 계산합니다. */
	FVector GetPlacementQueryCenterFromDominoActorLocation(const FVector& ActorLocation, FRotator Rotation) const;

	/** 보드 표면 위치와 실제 도미노 액터 피벗 위치를 기준으로 배치 가능 여부를 검사합니다. */
	bool CanPlaceDominoAtBoardAndActorLocations(const FVector& BoardLocation, const FVector& ActorLocation, FRotator Rotation) const;

	/** 보드 액터 Bounds와 겹치면 도미노가 보드 앞쪽에 오도록 로컬 Y를 보정합니다. */
	FVector AdjustDominoLocationAboveBoard(const FVector& ActorLocation) const;

	/** 배치 단계용 안정화 물리값을 적용하거나 시뮬레이션용 기본값으로 복구합니다. */
	void ConfigurePlacedDominoPhysics(ADominoBlockActor* Domino, bool bUsePlacementStabilization) const;

	/** 시뮬레이션 타임아웃 처리입니다. */
	void HandleSimulationTimeout();

	/** 목표 검사 타이머입니다. */
	FTimerHandle GoalCheckTimerHandle;

	/** 제한 시간 타이머입니다. */
	FTimerHandle SimulationTimeoutTimerHandle;
};
