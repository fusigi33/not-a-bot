#include "BallShooter/BallShooterCaptureActor.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Components/SceneComponent.h"

ABallShooterCaptureActor::ABallShooterCaptureActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root keeps the capture camera easy to position in the level.
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	// Perspective capture renders only the actors explicitly assigned to the show-only list.
	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(RootScene);
	SceneCapture->ProjectionType = ECameraProjectionMode::Perspective;
	SceneCapture->FOVAngle = 40.0f;
	SceneCapture->bCaptureEveryFrame = true;
	SceneCapture->bCaptureOnMovement = false;
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	SceneCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
}

UTextureRenderTarget2D* ABallShooterCaptureActor::GetRenderTarget() const
{
	return SceneCapture ? SceneCapture->TextureTarget : nullptr;
}
