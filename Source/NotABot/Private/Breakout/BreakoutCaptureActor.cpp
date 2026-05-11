#include "Breakout/BreakoutCaptureActor.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Components/SceneComponent.h"

ABreakoutCaptureActor::ABreakoutCaptureActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(RootScene);
	SceneCapture->ProjectionType = ECameraProjectionMode::Perspective;
	SceneCapture->FOVAngle = 40.0f;
	SceneCapture->bAutoActivate = false;
	SceneCapture->bCaptureEveryFrame = false;
	SceneCapture->bCaptureOnMovement = false;
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	SceneCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
}

UTextureRenderTarget2D* ABreakoutCaptureActor::GetRenderTarget() const
{
	return SceneCapture ? SceneCapture->TextureTarget : nullptr;
}

void ABreakoutCaptureActor::SetSceneCaptureEnabled(bool bEnabled)
{
	if (!SceneCapture)
	{
		return;
	}

	SceneCapture->bCaptureEveryFrame = bEnabled;
	SceneCapture->SetComponentTickEnabled(bEnabled);
	SceneCapture->SetActive(bEnabled, true);
}
