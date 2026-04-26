#include "BallShooter/BallShooterGameManager.h"

#include "BallShooter/BallShooterCaptureActor.h"
#include "BallShooter/BallShooterGoal.h"
#include "BallShooter/BallShooterHUDWidget.h"
#include "BallShooter/BallShooterMovingObstacle.h"
#include "BallShooter/BallShooterObstacleBase.h"
#include "BallShooter/BallShooterPawn.h"
#include "BallShooter/BallShooterRotatingObstacle.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/DataTable.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UnrealType.h"

namespace
{
	template <typename PropertyType>
	const PropertyType* FindPropertyByPrefix(const UStruct* Struct, const TCHAR* Prefix)
	{
		if (!Struct || !Prefix)
		{
			return nullptr;
		}

		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			const PropertyType* Property = CastField<PropertyType>(*It);
			if (Property && Property->GetName().StartsWith(Prefix))
			{
				return Property;
			}
		}

		return nullptr;
	}

	void LogStructProperties(const UStruct* Struct, const TCHAR* Label)
	{
		if (!Struct || !Label)
		{
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("[BallShooter] %s struct '%s' properties:"), Label, *Struct->GetName());
		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			const FProperty* Property = *It;
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[BallShooter]   - Name='%s', CPPType='%s', Class='%s'"),
				*Property->GetName(),
				*Property->GetCPPType(),
				*Property->GetClass()->GetName());
		}
	}
}

ABallShooterGameManager::ABallShooterGameManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABallShooterGameManager::BeginPlay()
{
	Super::BeginPlay();

	// Prepare gameplay references and hide the HUD until the round starts.
	ConfigureReferences();
	FindOrCreateHUDWidget();
	SetWidgetVisible(false);

	if (bAutoStartOnBeginPlay)
	{
		StartRound();
	}
}

void ABallShooterGameManager::StartRound()
{
	// Re-apply round setup so restart and first start share the same flow.
	ConfigureReferences();
	ApplyRoundData();
	EnsureShooterPawnPossessed();

	bRoundActive = true;
	bRoundEnded = false;
	SetWidgetVisible(true);
}

bool ABallShooterGameManager::SetRoundDataByRowName(FName InRowName)
{
	if (!RoundDataTable || InRowName.IsNone())
	{
		return false;
	}

	if (!RoundDataTable->FindRowUnchecked(InRowName))
	{
		return false;
	}

	RoundRowName = InRowName;
	RoundDifficultyName = NAME_None;
	ConfigureReferences();

	if (bRoundActive)
	{
		ApplyRoundData();
	}

	return true;
}

void ABallShooterGameManager::RestartRound()
{
	StartRound();
}

void ABallShooterGameManager::EndRound(EBallShooterRoundResult Result)
{
	if (bRoundEnded)
	{
		return;
	}

	// Lock player input in a terminal state before broadcasting the result.
	bRoundEnded = true;
	bRoundActive = false;

	if (ShooterPawn)
	{
		ShooterPawn->SetAimState(EBallShooterAimState::RoundEnded);
	}

	OnRoundEnded.Broadcast(ShooterPawn, Result);
	BP_OnRoundEnded(Result);
}

void ABallShooterGameManager::HandleBallFinished(ABallShooterPawn* FinishedPawn, EBallShooterRoundResult Result)
{
	if (!bRoundActive || FinishedPawn != ShooterPawn)
	{
		return;
	}

	EndRound(Result);
}

void ABallShooterGameManager::FindOrCreateHUDWidget()
{
	if (!HUDWidgetClass)
	{
		return;
	}

	if (!ActiveHUDWidget)
	{
		// Reuse an existing widget instance if one is already present in the viewport.
		TArray<UUserWidget*> FoundWidgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, FoundWidgets, HUDWidgetClass, false);

		for (UUserWidget* FoundWidget : FoundWidgets)
		{
			if (UBallShooterHUDWidget* ExistingWidget = Cast<UBallShooterHUDWidget>(FoundWidget))
			{
				ActiveHUDWidget = ExistingWidget;
				break;
			}
		}
	}

	if (!ActiveHUDWidget && bCreateWidgetIfMissing)
	{
		// Lazily create the HUD only when the manager is allowed to own it.
		if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
		{
			ActiveHUDWidget = CreateWidget<UBallShooterHUDWidget>(PlayerController, HUDWidgetClass);
			if (ActiveHUDWidget)
			{
				ActiveHUDWidget->AddToViewport(WidgetZOrder);
			}
		}
	}

	ConfigureWidgetMaterial();
}

void ABallShooterGameManager::SetWidgetVisible(bool bVisible)
{
	if (ActiveHUDWidget)
	{
		ActiveHUDWidget->SetVisibility(bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void ABallShooterGameManager::ConfigureReferences()
{
	// Push shared references so the shooter pawn owns both aiming and projectile state.
	if (ShooterPawn)
	{
		ShooterPawn->SetGameManager(this);
		ShooterPawn->SetBoardBounds(RoundData.BoardMinBounds, RoundData.BoardMaxBounds);
	}
}

bool ABallShooterGameManager::LoadRoundDataFromTable(
	FTransform& OutBallStartTransform,
	TArray<TTuple<TSubclassOf<AActor>, FTransform, float, bool>>& OutObstacles) const
{
	// 데이터테이블을 읽지 못할 때를 대비해 에디터에서 설정한 값을 기본값으로 유지합니다.
	OutBallStartTransform = RoundData.BallStartTransform;
	OutObstacles.Reset();

	if (!RoundDataTable || !RoundDataTable->GetRowStruct())
	{
		return false;
	}

	const UScriptStruct* RowStruct = RoundDataTable->GetRowStruct();
	const FStructProperty* BallStartTransformProperty = FindPropertyByPrefix<FStructProperty>(RowStruct, TEXT("BallStartTransform"));
	const FArrayProperty* ObstaclesProperty = FindPropertyByPrefix<FArrayProperty>(RowStruct, TEXT("Obstacles"));
	if (!BallStartTransformProperty || BallStartTransformProperty->Struct != TBaseStructure<FTransform>::Get() || !ObstaclesProperty)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BallShooter] Failed to resolve row properties. BallStartTransformProperty=%s, ObstaclesProperty=%s"),
			BallStartTransformProperty ? TEXT("Valid") : TEXT("Null"),
			ObstaclesProperty ? TEXT("Valid") : TEXT("Null"));
		LogStructProperties(RowStruct, TEXT("Row"));
		return false;
	}

	const FStructProperty* ObstacleEntryStructProperty = CastField<FStructProperty>(ObstaclesProperty->Inner);
	if (!ObstacleEntryStructProperty)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BallShooter] Obstacles array inner property is not a struct."));
		return false;
	}

	FName TargetRowName = RoundRowName;
	if (TargetRowName.IsNone() && !RoundDifficultyName.IsNone())
	{
		// 블루프린트 사용자 정의 Struct는 프로퍼티 이름 뒤에 GUID가 붙을 수 있으므로 prefix 기준으로 찾습니다.
		const FProperty* DifficultyProperty = FindPropertyByPrefix<FProperty>(RowStruct, TEXT("Difficulty"));
		const FByteProperty* DifficultyByteProperty = CastField<FByteProperty>(DifficultyProperty);
		const FEnumProperty* DifficultyEnumProperty = CastField<FEnumProperty>(DifficultyProperty);
		UEnum* DifficultyEnum = DifficultyEnumProperty
			? DifficultyEnumProperty->GetEnum()
			: (DifficultyByteProperty ? DifficultyByteProperty->Enum.Get() : nullptr);
		if (DifficultyEnum)
		{
			for (const FName& CandidateRowName : RoundDataTable->GetRowNames())
			{
				const uint8* CandidateRowData = RoundDataTable->FindRowUnchecked(CandidateRowName);
				if (!CandidateRowData)
				{
					continue;
				}

				const void* DifficultyValuePtr = DifficultyProperty->ContainerPtrToValuePtr<void>(CandidateRowData);
				const int64 DifficultyValue = DifficultyEnumProperty
					? DifficultyEnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(DifficultyValuePtr)
					: DifficultyByteProperty->GetSignedIntPropertyValue(DifficultyValuePtr);
				if (DifficultyEnum->GetAuthoredNameStringByValue(DifficultyValue) == RoundDifficultyName.ToString())
				{
					TargetRowName = CandidateRowName;
					break;
				}
			}
		}
	}

	if (TargetRowName.IsNone())
	{
		// 명시된 Row도 없고 Difficulty 일치 항목도 없을 때만 첫 번째 Row를 기본값으로 사용합니다.
		const TArray<FName> RowNames = RoundDataTable->GetRowNames();
		if (RowNames.Num() == 0)
		{
			return false;
		}

		TargetRowName = RowNames[0];
	}

 	const uint8* RowData = RoundDataTable->FindRowUnchecked(TargetRowName);
 	if (!RowData)
	{
		return false;
	}

	OutBallStartTransform = *BallStartTransformProperty->ContainerPtrToValuePtr<FTransform>(RowData);

	const void* ObstaclesArrayData = ObstaclesProperty->ContainerPtrToValuePtr<void>(RowData);
	FScriptArrayHelper ObstaclesArrayHelper(ObstaclesProperty, ObstaclesArrayData);

	// 장애물 엔트리 내부 필드도 동일한 이유로 prefix 기준으로 찾습니다.
	const FClassProperty* ObstacleClassProperty = FindPropertyByPrefix<FClassProperty>(ObstacleEntryStructProperty->Struct, TEXT("ObstacleClass"));
	const FStructProperty* ObstacleStartTransformProperty = FindPropertyByPrefix<FStructProperty>(ObstacleEntryStructProperty->Struct, TEXT("ObstacleStartTransform"));
	const FFloatProperty* ObstacleSpeedFloatProperty = FindPropertyByPrefix<FFloatProperty>(ObstacleEntryStructProperty->Struct, TEXT("ObstacleSpeed"));
	const FDoubleProperty* ObstacleSpeedDoubleProperty = FindPropertyByPrefix<FDoubleProperty>(ObstacleEntryStructProperty->Struct, TEXT("ObstacleSpeed"));
	const FBoolProperty* ClockwiseProperty = FindPropertyByPrefix<FBoolProperty>(ObstacleEntryStructProperty->Struct, TEXT("bClockwise"));
	if (!ObstacleClassProperty
		|| !ObstacleStartTransformProperty || ObstacleStartTransformProperty->Struct != TBaseStructure<FTransform>::Get()
		|| (!ObstacleSpeedFloatProperty && !ObstacleSpeedDoubleProperty)
		|| !ClockwiseProperty)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BallShooter] Failed to resolve obstacle entry properties. ObstacleClass=%s, ObstacleStartTransform=%s, ObstacleSpeed(float)=%s, ObstacleSpeed(double)=%s, bClockwise=%s"),
			ObstacleClassProperty ? TEXT("Valid") : TEXT("Null"),
			ObstacleStartTransformProperty ? TEXT("Valid") : TEXT("Null"),
			ObstacleSpeedFloatProperty ? TEXT("Valid") : TEXT("Null"),
			ObstacleSpeedDoubleProperty ? TEXT("Valid") : TEXT("Null"),
			ClockwiseProperty ? TEXT("Valid") : TEXT("Null"));
		LogStructProperties(ObstacleEntryStructProperty->Struct, TEXT("ObstacleEntry"));
		return false;
	}

	for (int32 Index = 0; Index < ObstaclesArrayHelper.Num(); ++Index)
	{
		// 각 장애물 엔트리를 런타임 스폰 로직에서 사용하는 튜플 형식으로 변환합니다.
		const uint8* ObstacleEntryData = ObstaclesArrayHelper.GetRawPtr(Index);
		if (!ObstacleEntryData)
		{
			continue;
		}

		const TSubclassOf<AActor>* ObstacleClassPtr = ObstacleClassProperty->ContainerPtrToValuePtr<TSubclassOf<AActor>>(ObstacleEntryData);
		if (!ObstacleClassPtr || !(*ObstacleClassPtr))
		{
			continue;
		}

		const TSubclassOf<AActor> ObstacleClass = *ObstacleClassPtr;
		if (!ObstacleClass)
		{
			continue;
		}

		const FTransform* ObstacleTransformPtr = ObstacleStartTransformProperty->ContainerPtrToValuePtr<FTransform>(ObstacleEntryData);
		const bool* ClockwisePtr = ClockwiseProperty->ContainerPtrToValuePtr<bool>(ObstacleEntryData);
		const float ObstacleSpeed = ObstacleSpeedFloatProperty
			? *ObstacleSpeedFloatProperty->ContainerPtrToValuePtr<float>(ObstacleEntryData)
			: static_cast<float>(*ObstacleSpeedDoubleProperty->ContainerPtrToValuePtr<double>(ObstacleEntryData));
		if (!ObstacleTransformPtr || !ClockwisePtr)
		{
			continue;
		}

		OutObstacles.Emplace(ObstacleClass, *ObstacleTransformPtr, ObstacleSpeed, *ClockwisePtr);
	}

	return true;
}

void ABallShooterGameManager::RebuildRoundObstacles(const TArray<TTuple<TSubclassOf<AActor>, FTransform, float, bool>>& ObstacleEntries)
{
	ClearSpawnedObstacles();

	if (!GetWorld())
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const TTuple<TSubclassOf<AActor>, FTransform, float, bool>& ObstacleEntry : ObstacleEntries)
	{
		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ObstacleEntry.Get<0>(), ObstacleEntry.Get<1>(), SpawnParameters);
		ABallShooterObstacleBase* SpawnedObstacle = Cast<ABallShooterObstacleBase>(SpawnedActor);
		if (!SpawnedObstacle)
		{
			if (SpawnedActor)
			{
				SpawnedActor->Destroy();
			}
			continue;
		}

		if (ABallShooterMovingObstacle* MovingObstacle = Cast<ABallShooterMovingObstacle>(SpawnedObstacle))
		{
			MovingObstacle->SetMoveSpeed(ObstacleEntry.Get<2>());
		}

		if (ABallShooterRotatingObstacle* RotatingObstacle = Cast<ABallShooterRotatingObstacle>(SpawnedObstacle))
		{
			RotatingObstacle->SetRotationSpeed(ObstacleEntry.Get<2>(), ObstacleEntry.Get<3>());
		}

		SpawnedObstacles.Add(SpawnedObstacle);
	}
}

void ABallShooterGameManager::ClearSpawnedObstacles()
{
	for (ABallShooterObstacleBase* SpawnedObstacle : SpawnedObstacles)
	{
		if (SpawnedObstacle && IsValid(SpawnedObstacle))
		{
			SpawnedObstacle->Destroy();
		}
	}

	SpawnedObstacles.Reset();
}

void ABallShooterGameManager::ApplyRoundData()
{
	FTransform BallStartTransform = RoundData.BallStartTransform;
	TArray<TTuple<TSubclassOf<AActor>, FTransform, float, bool>> ObstacleEntries;
	if (LoadRoundDataFromTable(BallStartTransform, ObstacleEntries))
	{
		RebuildRoundObstacles(ObstacleEntries);
	}
	else
	{
		ClearSpawnedObstacles();
	}

	// Reset all placed actors to the transforms defined for the active round.
	if (Goal)
	{
		Goal->SetActorTransform(RoundData.GoalTransform);
	}

	if (ShooterPawn)
	{
		ShooterPawn->ResetAiming(BallStartTransform);
	}
}

UTextureRenderTarget2D* ABallShooterGameManager::ResolveRenderTarget() const
{
	return CaptureActor ? CaptureActor->GetRenderTarget() : nullptr;
}

void ABallShooterGameManager::ConfigureWidgetMaterial() const
{
	if (ActiveHUDWidget)
	{
		// Feed the latest capture material and render target into the HUD.
		ActiveHUDWidget->InitializeBoardView(CaptureUIMaterial, ResolveRenderTarget());
	}
}

void ABallShooterGameManager::EnsureShooterPawnPossessed() const
{
	if (!ShooterPawn)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController || PlayerController->GetPawn() == ShooterPawn)
	{
		return;
	}

	PlayerController->Possess(ShooterPawn);
}
