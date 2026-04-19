#include "ShellGame/ShellCup.h"

#include "Components/BoxComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

namespace
{
const TArray<FName> HighlightColorParameterCandidates =
{
	TEXT("OutlineColor"),
	TEXT("HoverColor"),
	TEXT("TintColor"),
	TEXT("BaseColor"),
	TEXT("Color"),
	TEXT("GlowColor"),
	TEXT("EmissiveColor")
};

const TArray<FName> HighlightScalarParameterCandidates =
{
	TEXT("OutlineIntensity"),
	TEXT("HoverIntensity"),
	TEXT("HighlightIntensity"),
	TEXT("GlowIntensity"),
	TEXT("EmissiveStrength"),
	TEXT("EmissiveIntensity")
};
}

AShellCup::AShellCup()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	CupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CupMesh"));
	CupMesh->SetupAttachment(RootScene);
	CupMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	SelectionCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("SelectionCollision"));
	SelectionCollision->SetupAttachment(RootScene);
	SelectionCollision->SetBoxExtent(FVector(32.0f, 32.0f, 48.0f));
	SelectionCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SelectionCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	SelectionCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	SelectionCollision->SetGenerateOverlapEvents(false);
	SelectionCollision->SetHiddenInGame(true);
}

void AShellCup::BeginPlay()
{
	Super::BeginPlay();
	InitialClosedRotation = GetActorRotation();
	CacheBaseMaterials();
	ForceCloseImmediate();
	SetHoverHighlighted(false);
}

void AShellCup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bRevealAnimating)
	{
		return;
	}

	RevealElapsed += DeltaSeconds;
	const float Alpha = RevealDuration <= KINDA_SMALL_NUMBER ? 1.0f : FMath::Clamp(RevealElapsed / RevealDuration, 0.0f, 1.0f);
	ApplyRevealAlpha(Alpha);

	if (Alpha >= 1.0f)
	{
		bRevealAnimating = false;
	}
}

void AShellCup::SnapToWorldLocation(const FVector& WorldLocation)
{
	SetActorLocation(WorldLocation);
	ClosedLocation = WorldLocation;
	OpenLocation = ClosedLocation + FVector(0.0f, 0.0f, RevealLiftHeight);
}

void AShellCup::SetSelectionEnabled(bool bEnabled)
{
	if (SelectionCollision)
	{
		SelectionCollision->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}

	if (!bEnabled)
	{
		SetHoverHighlighted(false);
	}
}

void AShellCup::StartReveal(bool bOpen, float Duration)
{
	ClosedLocation = GetActorLocation();
	ClosedRotation = GetActorRotation();

	if (bRevealOpen && !bOpen)
	{
		ClosedLocation = OpenLocation;
		ClosedRotation = OpenRotation;
	}

	OpenLocation = ClosedLocation + FVector(0.0f, 0.0f, RevealLiftHeight);
	OpenRotation = ClosedRotation + FRotator(RevealPitch, 0.0f, 0.0f);
	RevealDuration = FMath::Max(0.0f, Duration);
	RevealElapsed = 0.0f;
	bRevealAnimating = RevealDuration > KINDA_SMALL_NUMBER;
	bRevealOpen = bOpen;

	if (!bRevealAnimating)
	{
		ApplyRevealAlpha(1.0f);
	}
}

void AShellCup::ForceCloseImmediate()
{
	bRevealAnimating = false;
	bRevealOpen = false;
	RevealDuration = 0.0f;
	RevealElapsed = 0.0f;
	ClosedLocation = GetActorLocation();
	ClosedRotation = InitialClosedRotation;
	OpenLocation = ClosedLocation + FVector(0.0f, 0.0f, RevealLiftHeight);
	OpenRotation = ClosedRotation + FRotator(RevealPitch, 0.0f, 0.0f);
	SetActorRotation(ClosedRotation);
	SetHoverHighlighted(false);
}

void AShellCup::SetHoverHighlighted(bool bHighlighted)
{
	if (!CupMesh)
	{
		return;
	}

	if (!bHighlighted)
	{
		CacheBaseMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < CachedBaseMaterials.Num(); ++MaterialIndex)
		{
			CupMesh->SetMaterial(MaterialIndex, CachedBaseMaterials[MaterialIndex]);
		}
		return;
	}
	
	// ---- [수정된 부분] HoverOverlayMaterial 적용을 최우선으로 처리합니다 ----
	if (HoverOverlayMaterial)
	{
		if (!HoverOverlayMaterialInstance)
		{
			HoverOverlayMaterialInstance = UMaterialInstanceDynamic::Create(HoverOverlayMaterial, this);
		}

		if (HoverOverlayMaterialInstance)
		{
			if (HoverColorParameterName != NAME_None)
			{
				HoverOverlayMaterialInstance->SetVectorParameterValue(HoverColorParameterName, HoverHighlightColor);
			}

			if (HoverIntensityParameterName != NAME_None)
			{
				HoverOverlayMaterialInstance->SetScalarParameterValue(HoverIntensityParameterName, HoverHighlightIntensity);
			}

			CacheBaseMaterials();
			for (int32 MaterialIndex = 0; MaterialIndex < CupMesh->GetNumMaterials(); ++MaterialIndex)
			{
				CupMesh->SetMaterial(MaterialIndex, HoverOverlayMaterialInstance);
			}
			return; // 덮어씌우기에 성공하면 함수 종료
		}
	}
	// 별도의 오버레이 머티리얼이 없거나 실패했다면 기존 베이스 머티리얼 파라미터 조정을 시도합니다.
	ApplyHighlightToBaseMaterials();
	
	// if (ApplyHighlightToBaseMaterials())
	// {
	// 	return;
	// }
	//
	// if (!HoverOverlayMaterial)
	// {
	// 	return;
	// }
	//
	// if (!HoverOverlayMaterialInstance)
	// {
	// 	HoverOverlayMaterialInstance = UMaterialInstanceDynamic::Create(HoverOverlayMaterial, this);
	// }
	//
	// if (!HoverOverlayMaterialInstance)
	// {
	// 	ApplyHighlightToBaseMaterials();
	// 	return;
	// }
	//
	// if (HoverColorParameterName != NAME_None)
	// {
	// 	HoverOverlayMaterialInstance->SetVectorParameterValue(HoverColorParameterName, HoverHighlightColor);
	// }
	//
	// if (HoverIntensityParameterName != NAME_None)
	// {
	// 	HoverOverlayMaterialInstance->SetScalarParameterValue(HoverIntensityParameterName, HoverHighlightIntensity);
	// }
	//
	// CacheBaseMaterials();
	// for (int32 MaterialIndex = 0; MaterialIndex < CupMesh->GetNumMaterials(); ++MaterialIndex)
	// {
	// 	CupMesh->SetMaterial(MaterialIndex, HoverOverlayMaterialInstance);
	// }
}

void AShellCup::CacheBaseMaterials()
{
	if (!CupMesh)
	{
		return;
	}

	const int32 MaterialCount = CupMesh->GetNumMaterials();
	if (MaterialCount <= 0)
	{
		CachedBaseMaterials.Reset();
		return;
	}

	if (CachedBaseMaterials.Num() == MaterialCount)
	{
		return;
	}

	CachedBaseMaterials.Reset();
	CachedBaseMaterials.Reserve(MaterialCount);

	for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
	{
		CachedBaseMaterials.Add(CupMesh->GetMaterial(MaterialIndex));
	}
}

bool AShellCup::ApplyHighlightToBaseMaterials()
{
	if (!CupMesh)
	{
		return false;
	}

	CacheBaseMaterials();

	if (CachedBaseMaterials.Num() == 0)
	{
		return false;
	}

	if (HoverBaseMaterialInstances.Num() != CachedBaseMaterials.Num())
	{
		HoverBaseMaterialInstances.Reset();
		HoverBaseMaterialInstances.Reserve(CachedBaseMaterials.Num());

		for (UMaterialInterface* BaseMaterial : CachedBaseMaterials)
		{
			HoverBaseMaterialInstances.Add(BaseMaterial ? UMaterialInstanceDynamic::Create(BaseMaterial, this) : nullptr);
		}
	}

	bool bAppliedAnyMaterial = false;

	for (int32 MaterialIndex = 0; MaterialIndex < HoverBaseMaterialInstances.Num(); ++MaterialIndex)
	{
		UMaterialInstanceDynamic* DynamicMaterial = HoverBaseMaterialInstances[MaterialIndex];
		if (!DynamicMaterial)
		{
			continue;
		}

		for (const FName& ParameterName : HighlightColorParameterCandidates)
		{
			DynamicMaterial->SetVectorParameterValue(ParameterName, HoverHighlightColor);
		}

		for (const FName& ParameterName : HighlightScalarParameterCandidates)
		{
			DynamicMaterial->SetScalarParameterValue(ParameterName, HoverHighlightIntensity);
		}

		CupMesh->SetMaterial(MaterialIndex, DynamicMaterial);
		bAppliedAnyMaterial = true;
	}

	return bAppliedAnyMaterial;
}

void AShellCup::ApplyRevealAlpha(float Alpha)
{
	const float FinalAlpha = bRevealOpen ? Alpha : 1.0f - Alpha;
	SetActorLocationAndRotation(
		FMath::Lerp(ClosedLocation, OpenLocation, FinalAlpha),
		FMath::Lerp(ClosedRotation, OpenRotation, FinalAlpha)
	);
}
