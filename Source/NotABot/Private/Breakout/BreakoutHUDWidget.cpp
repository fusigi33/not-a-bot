#include "Breakout/BreakoutHUDWidget.h"

#include "Components/Image.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"

void UBreakoutHUDWidget::InitializeCaptureMaterial(UMaterialInterface* BaseMaterial, UTextureRenderTarget2D* RenderTarget)
{
	if (!Image_BreakoutView || !BaseMaterial || !RenderTarget)
	{
		return;
	}

	CaptureMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	if (!CaptureMaterialInstance)
	{
		return;
	}

	CaptureMaterialInstance->SetTextureParameterValue(RenderTargetParameterName, RenderTarget);
	Image_BreakoutView->SetBrushFromMaterial(CaptureMaterialInstance);
}
