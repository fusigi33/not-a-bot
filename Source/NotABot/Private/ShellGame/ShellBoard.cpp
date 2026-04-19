#include "ShellGame/ShellBoard.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "ShellGame/ShellBall.h"
#include "ShellGame/ShellCup.h"
#include "TimerManager.h"

AShellBoard::AShellBoard()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	BoardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoardMesh"));
	BoardMesh->SetupAttachment(RootScene);
	BoardMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	ChaosBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("ChaosBounds"));
	ChaosBounds->SetupAttachment(RootScene);
	ChaosBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ChaosBounds->SetBoxExtent(FVector(260.0f, 180.0f, 120.0f));

	SlotLocalOffsets =
	{
		FVector(-180.0f, -90.0f, 0.0f),
		FVector(-60.0f, 90.0f, 0.0f),
		FVector(60.0f, -90.0f, 0.0f),
		FVector(180.0f, 90.0f, 0.0f)
	};
}

void AShellBoard::BeginPlay()
{
	Super::BeginPlay();

	RandomStream.Initialize(static_cast<int32>(FDateTime::Now().GetTicks() & 0x7fffffff));
	InitializeBoard();
}

void AShellBoard::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	switch (AnimationMode)
	{
	case EBoardAnimationMode::IntroDrop:
		UpdateIntroDrop(DeltaSeconds);
		break;
	case EBoardAnimationMode::ShuffleMove:
		UpdateStepMove(DeltaSeconds);
		break;
	case EBoardAnimationMode::PinballChaos:
		UpdatePinballChaos(DeltaSeconds);
		break;
	case EBoardAnimationMode::PinballReturn:
		UpdatePinballReturn(DeltaSeconds);
		break;
	default:
		break;
	}
}

bool AShellBoard::InitializeBoard()
{
	if (Cups.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AShellBoard::InitializeBoard - Cups array is empty."));
		return false;
	}

	if (Cups.Num() != SlotLocalOffsets.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("AShellBoard::InitializeBoard - Cup count (%d) must match slot count (%d)."), Cups.Num(), SlotLocalOffsets.Num());
		return false;
	}

	CupStates.Reset();
	CupStates.Reserve(Cups.Num());

	for (int32 CupIndex = 0; CupIndex < Cups.Num(); ++CupIndex)
	{
		AShellCup* Cup = Cups[CupIndex];
		if (!ensure(Cup))
		{
			continue;
		}

		FShellCupState NewState;
		NewState.Cup = Cup;
		NewState.CurrentSlotIndex = CupIndex;
		CupStates.Add(NewState);

		Cup->SetCurrentSlotIndex(CupIndex);
		Cup->ForceCloseImmediate();
	}

	ResetBoardForNewRound();
	return true;
}

void AShellBoard::ResetBoardForNewRound()
{
	GetWorldTimerManager().ClearTimer(ShufflePauseTimerHandle);
	GetWorldTimerManager().ClearTimer(RevealSecondaryTimerHandle);
	GetWorldTimerManager().ClearTimer(RevealFinishedTimerHandle);

	AnimationMode = EBoardAnimationMode::None;
	AnimationElapsed = 0.0f;
	AnimationDuration = 0.0f;
	ShuffleSteps.Reset();
	ActiveShuffleStepIndex = INDEX_NONE;
	ChaosVelocities.Reset();
	ChaosReturnStartLocations.Reset();
	LastSelectionResult = FShellSelectionResult();

	for (int32 StateIndex = 0; StateIndex < CupStates.Num(); ++StateIndex)
	{
		FShellCupState& CupState = CupStates[StateIndex];
		CupState.CurrentSlotIndex = StateIndex;
		CupState.bContainsBall = false;
		CupState.bIsSelectable = false;

		if (AShellCup* Cup = CupState.Cup.Get())
		{
			Cup->SetCurrentSlotIndex(StateIndex);
			Cup->ForceCloseImmediate();
			Cup->SetSelectionEnabled(false);
			Cup->SnapToWorldLocation(GetSlotWorldLocation(StateIndex) + FVector(0.0f, 0.0f, IntroHoverHeight));
		}
	}

	const int32 WinningCupIndex = RandomStream.RandRange(0, CupStates.Num() - 1);
	CupStates[WinningCupIndex].bContainsBall = true;

	if (Ball)
	{
		Ball->DetachFromCup();
		Ball->SetActorLocation(GetSlotWorldLocation(CupStates[WinningCupIndex].CurrentSlotIndex) + BallStartWorldOffset);
		Ball->SetBallVisible(true);
	}
}

void AShellBoard::StartIntroDrop()
{
	AnimationMode = EBoardAnimationMode::IntroDrop;
	AnimationElapsed = 0.0f;
	AnimationDuration = 0.6f;
}

void AShellBoard::StartShuffleSequence()
{
	SetCupSelectionEnabled(false);
	ShuffleSteps.Reset();
	ActiveShuffleStepIndex = INDEX_NONE;

	switch (ShuffleType)
	{
	case EShuffleType::TwoCupSwap:
		BuildTwoCupSwapSteps();
		break;
	case EShuffleType::AllCupShuffle:
		BuildAllCupShuffleSteps();
		break;
	case EShuffleType::PinballChaos:
		BeginPinballChaos();
		return;
	default:
		break;
	}

	if (ShuffleSteps.Num() == 0)
	{
		OnShuffleFinished.Broadcast();
		return;
	}

	ActiveShuffleStepIndex = 0;
	StartStepMove(ShuffleSteps[ActiveShuffleStepIndex]);
}

FShellSelectionResult AShellBoard::StartRevealForSelection(AShellCup* SelectedCup)
{
	LastSelectionResult = FShellSelectionResult();

	const int32 SelectedStateIndex = FindCupStateIndex(SelectedCup);
	const AShellCup* WinningCup = GetWinningCup();
	const int32 WinningStateIndex = FindCupStateIndex(WinningCup);

	if (SelectedStateIndex == INDEX_NONE || WinningStateIndex == INDEX_NONE)
	{
		OnRevealFinished.Broadcast(LastSelectionResult);
		return LastSelectionResult;
	}

	LastSelectionResult.bValidSelection = true;
	LastSelectionResult.SelectedCup = SelectedCup;
	LastSelectionResult.SelectedSlotIndex = CupStates[SelectedStateIndex].CurrentSlotIndex;
	LastSelectionResult.WinningCup = CupStates[WinningStateIndex].Cup;
	LastSelectionResult.WinningSlotIndex = CupStates[WinningStateIndex].CurrentSlotIndex;
	LastSelectionResult.bIsCorrect = (SelectedStateIndex == WinningStateIndex);

	SetCupSelectionEnabled(false);

	ResolveWinningCupVisibility(SelectedCup);
	SelectedCup->StartReveal(true, RevealDuration);

	if (!LastSelectionResult.bIsCorrect && LastSelectionResult.WinningCup)
	{
		FTimerDelegate SecondaryRevealDelegate;
		SecondaryRevealDelegate.BindLambda([this]()
		{
			if (LastSelectionResult.WinningCup)
			{
				ResolveWinningCupVisibility(LastSelectionResult.WinningCup);
				LastSelectionResult.WinningCup->StartReveal(true, RevealDuration);
			}
		});
		GetWorldTimerManager().SetTimer(RevealSecondaryTimerHandle, SecondaryRevealDelegate, WrongCupRevealDelay, false);
	}

	const float FinishDelay = LastSelectionResult.bIsCorrect ? RevealDuration : WrongCupRevealDelay + RevealDuration;
	FTimerDelegate FinishDelegate;
	FinishDelegate.BindLambda([this]()
	{
		OnRevealFinished.Broadcast(LastSelectionResult);
	});
	GetWorldTimerManager().SetTimer(RevealFinishedTimerHandle, FinishDelegate, FinishDelay, false);

	return LastSelectionResult;
}

void AShellBoard::SetCupSelectionEnabled(bool bEnabled)
{
	for (FShellCupState& CupState : CupStates)
	{
		CupState.bIsSelectable = bEnabled;
		if (AShellCup* Cup = CupState.Cup.Get())
		{
			Cup->SetSelectionEnabled(bEnabled);
		}
	}
}

FVector AShellBoard::GetSlotWorldLocation(int32 SlotIndex) const
{
	return SlotLocalOffsets.IsValidIndex(SlotIndex)
		? GetActorTransform().TransformPosition(SlotLocalOffsets[SlotIndex])
		: GetActorLocation();
}

AShellCup* AShellBoard::GetWinningCup() const
{
	for (const FShellCupState& CupState : CupStates)
	{
		if (CupState.bContainsBall)
		{
			return CupState.Cup;
		}
	}

	return nullptr;
}

int32 AShellBoard::GetWinningSlotIndex() const
{
	for (const FShellCupState& CupState : CupStates)
	{
		if (CupState.bContainsBall)
		{
			return CupState.CurrentSlotIndex;
		}
	}

	return INDEX_NONE;
}

void AShellBoard::StartStepMove(const FShellShuffleStep& Step)
{
	ActiveStep = Step;
	AnimationMode = EBoardAnimationMode::ShuffleMove;
	AnimationElapsed = 0.0f;
	AnimationDuration = FMath::Max(0.01f, Step.MoveDuration);

	ActiveMoveStartLocations.Reset();
	ActiveMoveStartRotations.Reset();

	for (const int32 CupStateIndex : Step.CupIndices)
	{
		const AShellCup* Cup = CupStates.IsValidIndex(CupStateIndex) ? CupStates[CupStateIndex].Cup.Get() : nullptr;
		ActiveMoveStartLocations.Add(Cup ? Cup->GetActorLocation() : FVector::ZeroVector);
		ActiveMoveStartRotations.Add(Cup ? Cup->GetActorRotation() : FRotator::ZeroRotator);
	}
}

void AShellBoard::FinishCurrentShuffleStep()
{
	for (int32 ArrayIndex = 0; ArrayIndex < ActiveStep.CupIndices.Num(); ++ArrayIndex)
	{
		const int32 CupStateIndex = ActiveStep.CupIndices[ArrayIndex];
		const int32 NewSlotIndex = ActiveStep.ToSlotIndices.IsValidIndex(ArrayIndex) ? ActiveStep.ToSlotIndices[ArrayIndex] : INDEX_NONE;
		if (!CupStates.IsValidIndex(CupStateIndex) || !CupStates[CupStateIndex].Cup || NewSlotIndex == INDEX_NONE)
		{
			continue;
		}

		CupStates[CupStateIndex].CurrentSlotIndex = NewSlotIndex;
		CupStates[CupStateIndex].Cup->SetCurrentSlotIndex(NewSlotIndex);
		CupStates[CupStateIndex].Cup->SnapToWorldLocation(GetSlotWorldLocation(NewSlotIndex));
	}

	if (ActiveShuffleStepIndex + 1 < ShuffleSteps.Num())
	{
		AnimationMode = EBoardAnimationMode::ShufflePause;
		const float PauseDuration = FMath::Max(0.0f, ActiveStep.PauseDuration);

		FTimerDelegate PauseDelegate;
		PauseDelegate.BindLambda([this]()
		{
			++ActiveShuffleStepIndex;
			StartStepMove(ShuffleSteps[ActiveShuffleStepIndex]);
		});

		GetWorldTimerManager().SetTimer(ShufflePauseTimerHandle, PauseDelegate, PauseDuration, false);
		return;
	}

	AnimationMode = EBoardAnimationMode::None;
	OnShuffleFinished.Broadcast();
}

void AShellBoard::BuildTwoCupSwapSteps()
{
	TArray<int32> WorkingSlots;
	WorkingSlots.Reserve(CupStates.Num());
	for (const FShellCupState& CupState : CupStates)
	{
		WorkingSlots.Add(CupState.CurrentSlotIndex);
	}

	const int32 TotalSwaps = FMath::Max(1, TwoCupSwapCount);
	for (int32 StepIndex = 0; StepIndex < TotalSwaps; ++StepIndex)
	{
		const int32 FirstIndex = RandomStream.RandRange(0, CupStates.Num() - 1);
		int32 SecondIndex = RandomStream.RandRange(0, CupStates.Num() - 1);
		while (SecondIndex == FirstIndex)
		{
			SecondIndex = RandomStream.RandRange(0, CupStates.Num() - 1);
		}

		FShellShuffleStep Step;
		Step.ShuffleType = EShuffleType::TwoCupSwap;
		Step.CupIndices = { FirstIndex, SecondIndex };
		Step.FromSlotIndices = { WorkingSlots[FirstIndex], WorkingSlots[SecondIndex] };
		Step.ToSlotIndices = { WorkingSlots[SecondIndex], WorkingSlots[FirstIndex] };
		Step.MoveDuration = TwoCupMoveDuration;
		Step.PauseDuration = TwoCupPauseDuration;
		ShuffleSteps.Add(Step);

		WorkingSlots.Swap(FirstIndex, SecondIndex);
	}
}

void AShellBoard::BuildAllCupShuffleSteps()
{
	TArray<int32> WorkingSlots;
	WorkingSlots.Reserve(CupStates.Num());
	for (const FShellCupState& CupState : CupStates)
	{
		WorkingSlots.Add(CupState.CurrentSlotIndex);
	}

	const int32 TotalShuffles = FMath::Max(1, AllCupShuffleCount);
	for (int32 StepIndex = 0; StepIndex < TotalShuffles; ++StepIndex)
	{
		TArray<int32> NewSlots = WorkingSlots;
		do
		{
			for (int32 ShuffleIndex = NewSlots.Num() - 1; ShuffleIndex > 0; --ShuffleIndex)
			{
				const int32 SwapIndex = RandomStream.RandRange(0, ShuffleIndex);
				NewSlots.Swap(ShuffleIndex, SwapIndex);
			}
		}
		while (NewSlots == WorkingSlots);

		FShellShuffleStep Step;
		Step.ShuffleType = EShuffleType::AllCupShuffle;
		Step.MoveDuration = AllCupMoveDuration;
		Step.PauseDuration = AllCupPauseDuration;

		for (int32 CupIndex = 0; CupIndex < CupStates.Num(); ++CupIndex)
		{
			Step.CupIndices.Add(CupIndex);
			Step.FromSlotIndices.Add(WorkingSlots[CupIndex]);
			Step.ToSlotIndices.Add(NewSlots[CupIndex]);
		}

		ShuffleSteps.Add(Step);
		WorkingSlots = NewSlots;
	}
}

void AShellBoard::BeginPinballChaos()
{
	AnimationMode = EBoardAnimationMode::PinballChaos;
	AnimationElapsed = 0.0f;
	AnimationDuration = FMath::Max(0.25f, PinballDuration);
	ChaosVelocities.Reset();

	for (int32 StateIndex = 0; StateIndex < CupStates.Num(); ++StateIndex)
	{
		const float Speed = RandomStream.FRandRange(PinballSpeedMin, PinballSpeedMax);
		FVector2D Direction(RandomStream.FRandRange(-1.0f, 1.0f), RandomStream.FRandRange(-1.0f, 1.0f));
		Direction = Direction.GetSafeNormal();
		ChaosVelocities.Add(FVector(Direction.X * Speed, Direction.Y * Speed, 0.0f));
	}
}

void AShellBoard::FinishPinballChaos()
{
	CacheReturnTargets();
	AnimationMode = EBoardAnimationMode::PinballReturn;
	AnimationElapsed = 0.0f;
	AnimationDuration = FMath::Max(0.05f, PinballReturnDuration);
}

void AShellBoard::UpdateIntroDrop(float DeltaSeconds)
{
	AnimationElapsed += DeltaSeconds;
	const float Alpha = FMath::Clamp(AnimationElapsed / AnimationDuration, 0.0f, 1.0f);

	for (int32 StateIndex = 0; StateIndex < CupStates.Num(); ++StateIndex)
	{
		if (AShellCup* Cup = CupStates[StateIndex].Cup.Get())
		{
			const FVector StartLocation = GetSlotWorldLocation(StateIndex) + FVector(0.0f, 0.0f, IntroHoverHeight);
			const FVector EndLocation = GetSlotWorldLocation(StateIndex);
			Cup->SnapToWorldLocation(FMath::Lerp(StartLocation, EndLocation, Alpha));
		}
	}

	if (Alpha >= 1.0f)
	{
		AnimationMode = EBoardAnimationMode::None;
		PrepareBallAttachment();
		OnIntroFinished.Broadcast();
	}
}

void AShellBoard::UpdateStepMove(float DeltaSeconds)
{
	AnimationElapsed += DeltaSeconds;
	const float Alpha = FMath::Clamp(AnimationElapsed / AnimationDuration, 0.0f, 1.0f);
	const float ArcAlpha = FMath::Sin(Alpha * PI);

	for (int32 ArrayIndex = 0; ArrayIndex < ActiveStep.CupIndices.Num(); ++ArrayIndex)
	{
		const int32 CupStateIndex = ActiveStep.CupIndices[ArrayIndex];
		AShellCup* Cup = CupStates.IsValidIndex(CupStateIndex) ? CupStates[CupStateIndex].Cup.Get() : nullptr;
		if (!Cup)
		{
			continue;
		}

		const FVector StartLocation = ActiveMoveStartLocations.IsValidIndex(ArrayIndex) ? ActiveMoveStartLocations[ArrayIndex] : Cup->GetActorLocation();
		const FVector EndLocation = GetSlotWorldLocation(ActiveStep.ToSlotIndices[ArrayIndex]);
		FVector NewLocation = FMath::Lerp(StartLocation, EndLocation, Alpha);

		if (ActiveStep.ShuffleType == EShuffleType::TwoCupSwap)
		{
			NewLocation.Z += ArcAlpha * TwoCupArcHeight;
		}

		Cup->SnapToWorldLocation(NewLocation);
	}

	if (Alpha >= 1.0f)
	{
		FinishCurrentShuffleStep();
	}
}

void AShellBoard::UpdatePinballChaos(float DeltaSeconds)
{
	AnimationElapsed += DeltaSeconds;
	const FVector BoundsCenter = ChaosBounds->GetComponentLocation();
	const FVector BoundsExtent = ChaosBounds->GetScaledBoxExtent();

	for (int32 StateIndex = 0; StateIndex < CupStates.Num(); ++StateIndex)
	{
		AShellCup* Cup = CupStates.IsValidIndex(StateIndex) ? CupStates[StateIndex].Cup.Get() : nullptr;
		if (!Cup || !ChaosVelocities.IsValidIndex(StateIndex))
		{
			continue;
		}

		FVector NewLocation = Cup->GetActorLocation() + ChaosVelocities[StateIndex] * DeltaSeconds;
		FVector CenterOffset = NewLocation - BoundsCenter;

		if (FMath::Abs(CenterOffset.X) > BoundsExtent.X)
		{
			ChaosVelocities[StateIndex].X *= -1.0f;
			CenterOffset.X = FMath::Clamp(CenterOffset.X, -BoundsExtent.X, BoundsExtent.X);
		}

		if (FMath::Abs(CenterOffset.Y) > BoundsExtent.Y)
		{
			ChaosVelocities[StateIndex].Y *= -1.0f;
			CenterOffset.Y = FMath::Clamp(CenterOffset.Y, -BoundsExtent.Y, BoundsExtent.Y);
		}

		ChaosVelocities[StateIndex] *= (1.0f - FMath::Clamp(PinballDamping * DeltaSeconds, 0.0f, 0.95f));

		NewLocation = BoundsCenter + CenterOffset;
		NewLocation.Z = GetSlotWorldLocation(CupStates[StateIndex].CurrentSlotIndex).Z + FMath::Sin(AnimationElapsed * 8.0f + static_cast<float>(StateIndex)) * 8.0f;
		Cup->SnapToWorldLocation(NewLocation);
	}

	if (AnimationElapsed >= AnimationDuration)
	{
		FinishPinballChaos();
	}
}

void AShellBoard::UpdatePinballReturn(float DeltaSeconds)
{
	AnimationElapsed += DeltaSeconds;
	const float Alpha = FMath::Clamp(AnimationElapsed / AnimationDuration, 0.0f, 1.0f);

	for (int32 StateIndex = 0; StateIndex < CupStates.Num(); ++StateIndex)
	{
		AShellCup* Cup = CupStates.IsValidIndex(StateIndex) ? CupStates[StateIndex].Cup.Get() : nullptr;
		if (!Cup || !ChaosReturnStartLocations.IsValidIndex(StateIndex))
		{
			continue;
		}

		const FVector EndLocation = GetSlotWorldLocation(CupStates[StateIndex].CurrentSlotIndex);
		Cup->SnapToWorldLocation(FMath::Lerp(ChaosReturnStartLocations[StateIndex], EndLocation, Alpha));
	}

	if (Alpha >= 1.0f)
	{
		AnimationMode = EBoardAnimationMode::None;
		OnShuffleFinished.Broadcast();
	}
}

void AShellBoard::PrepareBallAttachment()
{
	if (!Ball)
	{
		return;
	}

	if (AShellCup* WinningCup = GetWinningCup())
	{
		Ball->AttachToCup(WinningCup, BallLocalOffset);
		Ball->SetBallVisible(false);
	}
}

void AShellBoard::ResolveWinningCupVisibility(AShellCup* CupToOpen)
{
	if (!Ball || !CupToOpen)
	{
		return;
	}

	const int32 StateIndex = FindCupStateIndex(CupToOpen);
	if (StateIndex == INDEX_NONE)
	{
		return;
	}

	const bool bContainsBall = CupStates[StateIndex].bContainsBall;
	Ball->SetBallVisible(bContainsBall);
}

int32 AShellBoard::FindCupStateIndex(const AShellCup* Cup) const
{
	for (int32 StateIndex = 0; StateIndex < CupStates.Num(); ++StateIndex)
	{
		if (CupStates[StateIndex].Cup == Cup)
		{
			return StateIndex;
		}
	}

	return INDEX_NONE;
}

void AShellBoard::CacheReturnTargets()
{
	ChaosReturnStartLocations.Reset();
	ChaosReturnStartLocations.Reserve(CupStates.Num());

	for (const FShellCupState& CupState : CupStates)
	{
		ChaosReturnStartLocations.Add(CupState.Cup ? CupState.Cup->GetActorLocation() : FVector::ZeroVector);
	}
}
