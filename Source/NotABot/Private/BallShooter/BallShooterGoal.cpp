#include "BallShooter/BallShooterGoal.h"

#include "BallShooter/BallShooterPawn.h"
#include "DrawDebugHelpers.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

ABallShooterGoal::ABallShooterGoal()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	// The trigger box is the authoritative success volume.
	GoalBoxTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("GoalBoxTrigger"));
	GoalBoxTrigger->SetupAttachment(SceneRoot);
	GoalBoxTrigger->SetBoxExtent(FVector(77.0f, 37.0f, 38.0f));
	GoalBoxTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GoalBoxTrigger->SetCollisionObjectType(ECC_Pawn);
	GoalBoxTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	GoalBoxTrigger->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	GoalBoxTrigger->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GoalBoxTrigger->SetGenerateOverlapEvents(true);

	GoalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GoalMesh"));
	GoalMesh->SetupAttachment(SceneRoot);
	GoalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABallShooterGoal::BeginPlay()
{
	Super::BeginPlay();
	// Listen for ball overlaps once runtime begins.
	GoalBoxTrigger->OnComponentBeginOverlap.AddDynamic(this, &ABallShooterGoal::HandleGoalOverlap);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[BallShooterGoal] BeginPlay Goal=%s ActorCollision=%s Trigger=%s CollisionEnabled=%d QueryEnabled=%s GenerateOverlap=%s ObjectType=%d ResponseToPhysicsBody=%d ResponseToPawn=%d ResponseToVisibility=%d"),
		*GetName(),
		GetActorEnableCollision() ? TEXT("true") : TEXT("false"),
		GoalBoxTrigger ? *GoalBoxTrigger->GetName() : TEXT("None"),
		GoalBoxTrigger ? static_cast<int32>(GoalBoxTrigger->GetCollisionEnabled()) : -1,
		GoalBoxTrigger && GoalBoxTrigger->IsQueryCollisionEnabled() ? TEXT("true") : TEXT("false"),
		GoalBoxTrigger && GoalBoxTrigger->GetGenerateOverlapEvents() ? TEXT("true") : TEXT("false"),
		GoalBoxTrigger ? static_cast<int32>(GoalBoxTrigger->GetCollisionObjectType()) : -1,
		GoalBoxTrigger ? static_cast<int32>(GoalBoxTrigger->GetCollisionResponseToChannel(ECC_PhysicsBody)) : -1,
		GoalBoxTrigger ? static_cast<int32>(GoalBoxTrigger->GetCollisionResponseToChannel(ECC_Pawn)) : -1,
		GoalBoxTrigger ? static_cast<int32>(GoalBoxTrigger->GetCollisionResponseToChannel(ECC_Visibility)) : -1);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[BallShooterGoal] MeshState Goal=%s ActorHiddenInGame=%s Mesh=%s Visible=%s HiddenInGame=%s StaticMesh=%s"),
		*GetName(),
		IsHidden() ? TEXT("true") : TEXT("false"),
		GoalMesh ? *GoalMesh->GetName() : TEXT("None"),
		GoalMesh && GoalMesh->IsVisible() ? TEXT("true") : TEXT("false"),
		GoalMesh && GoalMesh->bHiddenInGame ? TEXT("true") : TEXT("false"),
		GoalMesh && GoalMesh->GetStaticMesh() ? *GoalMesh->GetStaticMesh()->GetName() : TEXT("None"));

	if (GetWorld() && GoalBoxTrigger)
	{
		DrawDebugBox(
			GetWorld(),
			GoalBoxTrigger->GetComponentLocation(),
			GoalBoxTrigger->GetScaledBoxExtent(),
			GoalBoxTrigger->GetComponentQuat(),
			FColor::Green,
			true,
			30.0f,
			0,
			2.0f);

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BallShooterGoal] DebugBox Center=%s Extent=%s Rotation=%s"),
			*GoalBoxTrigger->GetComponentLocation().ToCompactString(),
			*GoalBoxTrigger->GetScaledBoxExtent().ToCompactString(),
			*GoalBoxTrigger->GetComponentRotation().ToCompactString());
	}
}

void ABallShooterGoal::HandleGoalOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[BallShooterGoal] HandleGoalOverlap Goal=%s OverlappedComponent=%s OtherActor=%s OtherClass=%s OtherComp=%s OtherCompGenerateOverlap=%s OtherCompCollisionEnabled=%d OtherCompObjectType=%d bFromSweep=%s"),
		*GetName(),
		OverlappedComponent ? *OverlappedComponent->GetName() : TEXT("None"),
		OtherActor ? *OtherActor->GetName() : TEXT("None"),
		OtherActor ? *OtherActor->GetClass()->GetName() : TEXT("None"),
		OtherComp ? *OtherComp->GetName() : TEXT("None"),
		OtherComp && OtherComp->GetGenerateOverlapEvents() ? TEXT("true") : TEXT("false"),
		OtherComp ? static_cast<int32>(OtherComp->GetCollisionEnabled()) : -1,
		OtherComp ? static_cast<int32>(OtherComp->GetCollisionObjectType()) : -1,
		bFromSweep ? TEXT("true") : TEXT("false"));

	if (ABallShooterPawn* Pawn = Cast<ABallShooterPawn>(OtherActor))
	{
		// Notify listeners first, then let the pawn finalize itself as a success.
		OnGoalReached.Broadcast(this, Pawn);
		Pawn->NotifyReachedGoal();
	}
}
