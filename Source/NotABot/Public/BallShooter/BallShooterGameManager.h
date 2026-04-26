#pragma once

#include "CoreMinimal.h"
#include "BallShooter/BallShooterTypes.h"
#include "GameFramework/Actor.h"
#include "BallShooterGameManager.generated.h"

class ABallShooterCaptureActor;
class ABallShooterGoal;
class ABallShooterObstacleBase;
class ABallShooterPawn;
class UBallShooterHUDWidget;
class UDataTable;
class UMaterialInterface;
class UTextureRenderTarget2D;

/** 라운드 종료 시 공과 결과를 전달하는 델리게이트입니다. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBallShooterRoundEndedSignature, ABallShooterPawn*, Pawn, EBallShooterRoundResult, Result);

/**
 * BallShooter 미니게임의 라운드 흐름과 UI를 총괄하는 매니저입니다.
 * 참조 액터 초기화, 라운드 시작/종료, HUD 구성까지 담당합니다.
 */
UCLASS()
class NOTABOT_API ABallShooterGameManager : public AActor
{
	GENERATED_BODY()

public:
	/** 매니저 기본 상태를 초기화합니다. */
	ABallShooterGameManager();

	/** 참조와 UI를 구성하고 필요하면 라운드를 자동 시작합니다. */
	virtual void BeginPlay() override;

	/** 현재 설정된 라운드 데이터를 기준으로 라운드를 시작합니다. */
	UFUNCTION(BlueprintCallable, Category="BallShooter|Round")
	void StartRound();

	/**
	 * 런타임에 사용할 데이터테이블 Row를 교체합니다.
	 * @param InRowName 새로 적용할 Row 이름입니다.
	 * @return Row를 정상적으로 찾고 적용 대상으로 설정했으면 true입니다.
	 */
	UFUNCTION(BlueprintCallable, Category="BallShooter|Round")
	bool SetRoundDataByRowName(FName InRowName);

	/** 현재 라운드를 처음 상태로 다시 시작합니다. */
	UFUNCTION(BlueprintCallable, Category="BallShooter|Round")
	void RestartRound();

	/**
	 * 라운드를 종료하고 결과 이벤트를 전파합니다.
	 * @param Result 라운드의 최종 결과입니다.
	 */
	UFUNCTION(BlueprintCallable, Category="BallShooter|Round")
	void EndRound(EBallShooterRoundResult Result);

	/**
	 * 폰의 결과 통지를 받아 현재 라운드 종료 여부를 결정합니다.
	 * @param Pawn 종료를 보고한 폰입니다.
	 * @param Result 폰의 최종 결과입니다.
	 */
	UFUNCTION(BlueprintCallable, Category="BallShooter|Round")
	void HandleBallFinished(ABallShooterPawn* Pawn, EBallShooterRoundResult Result);

	/** 기존 HUD를 찾거나 필요 시 새로 생성합니다. */
	UFUNCTION(BlueprintCallable, Category="BallShooter|UI")
	void FindOrCreateHUDWidget();

	/**
	 * HUD 표시 여부를 전환합니다.
	 * @param bVisible true면 표시하고 false면 숨깁니다.
	 */
	UFUNCTION(BlueprintCallable, Category="BallShooter|UI")
	void SetWidgetVisible(bool bVisible);

	/** 라운드가 종료되었을 때 브로드캐스트됩니다. */
	UPROPERTY(BlueprintAssignable, Category="BallShooter|Events")
	FOnBallShooterRoundEndedSignature OnRoundEnded;

	/**
	 * 블루프린트에서 라운드 종료 연출을 구현할 때 호출됩니다.
	 * @param Result 라운드의 최종 결과입니다.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="BallShooter|Events")
	void BP_OnRoundEnded(EBallShooterRoundResult Result);

protected:
	/** 플레이어 조작과 조준을 담당하는 폰입니다. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="BallShooter|References")
	TObjectPtr<ABallShooterPawn> ShooterPawn;

	/** 공이 도달해야 하는 목표 액터입니다. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="BallShooter|References")
	TObjectPtr<ABallShooterGoal> Goal;

	/** 보드 화면을 UI에 제공하는 캡처 액터입니다. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="BallShooter|References")
	TObjectPtr<ABallShooterCaptureActor> CaptureActor;

	/** 현재 라운드 배치와 규칙 데이터를 정의합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Round")
	FBallShooterRoundData RoundData;

	/** 블루프린트 데이터테이블에서 BallShooter 라운드 배치를 읽어옵니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Round Data")
	TObjectPtr<UDataTable> RoundDataTable;

	/** 우선적으로 사용할 데이터테이블 Row 이름입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Round Data")
	FName RoundRowName;

	/** Row 이름이 비어 있으면 Difficulty 필드에서 찾을 enum 엔트리 이름입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Round Data")
	FName RoundDifficultyName;

	/** BeginPlay 직후 라운드를 자동으로 시작할지 여부입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|Round")
	bool bAutoStartOnBeginPlay = false;

	/** 화면에 띄울 HUD 위젯 클래스입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|UI")
	TSubclassOf<UBallShooterHUDWidget> HUDWidgetClass;

	/** 렌더 타깃을 UI에 표시할 때 사용할 기본 머티리얼입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|UI")
	TObjectPtr<UMaterialInterface> CaptureUIMaterial;

	/** HUD를 찾지 못했을 때 새로 생성할지 여부입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|UI")
	bool bCreateWidgetIfMissing = true;

	/** HUD를 뷰포트에 추가할 때 사용할 Z 오더입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BallShooter|UI")
	int32 WidgetZOrder = 50;

	/** 현재 활성화된 HUD 위젯 인스턴스입니다. */
	UPROPERTY(Transient, BlueprintReadOnly, Category="BallShooter|UI")
	TObjectPtr<UBallShooterHUDWidget> ActiveHUDWidget;

	/** 현재 라운드가 진행 중인지 여부입니다. */
	UPROPERTY(BlueprintReadOnly, Category="BallShooter|Round")
	bool bRoundActive = false;

private:
	/** 라운드 종료 로직이 이미 실행되었는지 여부입니다. */
	bool bRoundEnded = false;

	/** 현재 라운드에서 생성해 관리 중인 장애물들입니다. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<ABallShooterObstacleBase>> SpawnedObstacles;

	/** 연결된 참조 액터들에 상호 참조를 설정합니다. */
	void ConfigureReferences();
	/** 데이터테이블에서 현재 라운드 배치 데이터를 읽습니다. */
	bool LoadRoundDataFromTable(FTransform& OutBallStartTransform, TArray<TTuple<TSubclassOf<AActor>, FTransform, float, bool>>& OutObstacles) const;
	/** 데이터테이블에서 읽은 장애물들을 재생성합니다. */
	void RebuildRoundObstacles(const TArray<TTuple<TSubclassOf<AActor>, FTransform, float, bool>>& ObstacleEntries);
	/** 현재 매니저가 생성한 장애물들을 정리합니다. */
	void ClearSpawnedObstacles();
	/** RoundData를 각 액터에 적용하여 라운드를 준비합니다. */
	void ApplyRoundData();
	/** 캡처 액터에서 UI에 사용할 렌더 타깃을 조회합니다. */
	UTextureRenderTarget2D* ResolveRenderTarget() const;
	/** HUD가 사용할 머티리얼과 텍스처를 구성합니다. */
	void ConfigureWidgetMaterial() const;
	/** StartRound 시 플레이어 컨트롤러가 슈터 폰을 빙의하도록 보장합니다. */
	void EnsureShooterPawnPossessed() const;
};
