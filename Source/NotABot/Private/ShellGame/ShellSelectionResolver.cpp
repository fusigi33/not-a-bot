#include "ShellGame/ShellSelectionResolver.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "ShellGame/ShellCup.h"

AShellCup* UShellSelectionResolver::ResolveCupFromCaptureUV(
	UObject* WorldContextObject,
	USceneCaptureComponent2D* SceneCapture,
	UTextureRenderTarget2D* RenderTarget,
	const FVector2D& UV,
	float TraceDistance,
	TEnumAsByte<ECollisionChannel> TraceChannel
) const
{
	return TraceCupFromCaptureUV(WorldContextObject, SceneCapture, RenderTarget, UV, TraceDistance, TraceChannel);
}

FShellSelectionResult UShellSelectionResolver::ResolveSelectionFromCaptureUV(
	UObject* WorldContextObject,
	USceneCaptureComponent2D* SceneCapture,
	UTextureRenderTarget2D* RenderTarget,
	const FVector2D& UV,
	float TraceDistance,
	TEnumAsByte<ECollisionChannel> TraceChannel
) const
{
	FShellSelectionResult Result;
	AShellCup* HitCup = TraceCupFromCaptureUV(WorldContextObject, SceneCapture, RenderTarget, UV, TraceDistance, TraceChannel);
	if (!HitCup)
	{
		return Result;
	}

	Result.bValidSelection = true;
	Result.SelectedCup = HitCup;
	Result.SelectedSlotIndex = HitCup->GetCurrentSlotIndex();
	return Result;
}

AShellCup* UShellSelectionResolver::TraceCupFromCaptureUV(
	UObject* WorldContextObject,
	USceneCaptureComponent2D* SceneCapture,
	UTextureRenderTarget2D* RenderTarget,
	const FVector2D& UV,
	float TraceDistance,
	TEnumAsByte<ECollisionChannel> TraceChannel
) const
{
	if (!WorldContextObject || !SceneCapture || !RenderTarget)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FVector RayOrigin = FVector::ZeroVector;
	FVector RayDirection = FVector::ForwardVector;
	BuildWorldRayFromUV(SceneCapture, RenderTarget, UV, RayOrigin, RayDirection);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ShellSelectionTrace), false);
	QueryParams.bReturnPhysicalMaterial = false;

	const bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		RayOrigin,
		RayOrigin + (RayDirection * TraceDistance),
		TraceChannel,
		QueryParams
	);

	if (!bHit)
	{
		return nullptr;
	}

	AShellCup* HitCup = Cast<AShellCup>(HitResult.GetActor());
	if (!HitCup && HitResult.GetComponent())
	{
		HitCup = Cast<AShellCup>(HitResult.GetComponent()->GetOwner());
	}

	return HitCup;
}

void UShellSelectionResolver::BuildWorldRayFromUV(
	USceneCaptureComponent2D* SceneCapture,
	UTextureRenderTarget2D* RenderTarget,
	const FVector2D& UV,
	FVector& OutOrigin,
	FVector& OutDirection
) const
{
	const FVector2D ClampedUV(FMath::Clamp(UV.X, 0.0f, 1.0f), FMath::Clamp(UV.Y, 0.0f, 1.0f));
	const float AspectRatio = RenderTarget->SizeY > 0 ? static_cast<float>(RenderTarget->SizeX) / static_cast<float>(RenderTarget->SizeY) : 1.0f;
	const float HalfHorizontalFovRad = FMath::DegreesToRadians(SceneCapture->FOVAngle * 0.5f);
	const float HalfVerticalFovRad = FMath::Atan(FMath::Tan(HalfHorizontalFovRad) / FMath::Max(AspectRatio, KINDA_SMALL_NUMBER));

	const float ScreenX = (ClampedUV.X * 2.0f) - 1.0f;
	const float ScreenY = 1.0f - (ClampedUV.Y * 2.0f);
	const FVector CameraSpaceDirection(
		1.0f,
		ScreenX * FMath::Tan(HalfHorizontalFovRad),
		ScreenY * FMath::Tan(HalfVerticalFovRad)
	);

	const FTransform CaptureTransform = SceneCapture->GetComponentTransform();
	OutOrigin = CaptureTransform.GetLocation();
	OutDirection = CaptureTransform.TransformVectorNoScale(CameraSpaceDirection).GetSafeNormal();
}
