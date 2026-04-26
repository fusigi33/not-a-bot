#include "BallShooter/BallShooterHUDWidget.h"

#include "Components/Image.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"

void UBallShooterHUDWidget::InitializeBoardView(UMaterialInterface* BaseMaterial, UTextureRenderTarget2D* RenderTarget)
{
	if (!Image_BoardView || !BaseMaterial || !RenderTarget)
	{
		return;
	}

	// Create a per-widget material instance so the captured texture can be swapped safely.
	BoardViewMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	if (!BoardViewMaterialInstance)
	{
		return;
	}

	// Bind the render target and present it through the image brush.
	BoardViewMaterialInstance->SetTextureParameterValue(RenderTargetParameterName, RenderTarget);
	Image_BoardView->SetBrushFromMaterial(BoardViewMaterialInstance);
}
