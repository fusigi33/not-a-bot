#include "ShellGame/ShellCaptureRig.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Components/SceneComponent.h"

AShellCaptureRig::AShellCaptureRig()
{
	PrimaryActorTick.bCanEverTick = false;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(RootScene);
	SceneCapture->ProjectionType = ECameraProjectionMode::Perspective;
	SceneCapture->FOVAngle = 50.0f;
	SceneCapture->bCaptureEveryFrame = true;
	SceneCapture->bCaptureOnMovement = true;
}

void AShellCaptureRig::BeginPlay()
{
	Super::BeginPlay();
}

UTextureRenderTarget2D* AShellCaptureRig::GetRenderTarget() const
{
	return SceneCapture ? SceneCapture->TextureTarget : nullptr;
}
