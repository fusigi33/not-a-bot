#include "DominoMiniGame/DominoMiniGameWidget.h"

#include "Blueprint/WidgetTree.h"
#include "DominoMiniGame/DominoBlockActor.h"
#include "DominoMiniGame/DominoBoardActor.h"
#include "DominoMiniGame/DominoDragDropOperation.h"
#include "DominoMiniGame/DominoInventoryItemWidget.h"
#include "DominoMiniGame/DominoMiniGameManager.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Spacer.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"

DEFINE_LOG_CATEGORY_STATIC(LogDominoMiniGameWidget, Log, All);

void UDominoMiniGameWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Visible);

	if (WidgetTree && WidgetTree->RootWidget)
	{
		WidgetTree->RootWidget->SetVisibility(ESlateVisibility::Visible);
	}

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UDominoMiniGameWidget::HandleStartButtonClicked);
	}

	if (ResetButton)
	{
		ResetButton->OnClicked.AddDynamic(this, &UDominoMiniGameWidget::HandleResetButtonClicked);
	}

	if (BoardImage && RenderTargetMaterial)
	{
		BoardImage->SetBrushFromMaterial(RenderTargetMaterial);
	}

	if (BoardImage)
	{
		BoardImage->SetVisibility(ESlateVisibility::Visible);
	}
}

void UDominoMiniGameWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UDominoMiniGameWidget::InitializeDominoWidget(ADominoMiniGameManager* InManager)
{
	Manager = InManager;

	if (Manager)
	{
		Manager->BeginRound();
		CreateDominoInventoryIcons();
	}
}

void UDominoMiniGameWidget::CreateDominoInventoryIcons()
{
	if (!DominoInventoryBox || !Manager || !InventoryItemWidgetClass)
	{
		return;
	}

	DominoInventoryBox->ClearChildren();

	TArray<TObjectPtr<UDominoInventoryItemWidget>> ItemWidgets;
	ItemWidgets.Reserve(Manager->RoundData.AvailableDominoCount);

	for (int32 Index = 0; Index < Manager->RoundData.AvailableDominoCount; ++Index)
	{
		UDominoInventoryItemWidget* ItemWidget = CreateWidget<UDominoInventoryItemWidget>(GetWorld(), InventoryItemWidgetClass);
		if (!ItemWidget)
		{
			continue;
		}

		ItemWidget->DominoType = Manager->RoundData.DominoClass;
		ItemWidget->IconTexture = DominoIconTexture;
		ItemWidget->RefreshIconBrush();

		ItemWidgets.Add(ItemWidget);
	}

	if (ItemWidgets.IsEmpty())
	{
		return;
	}

	FSlateChildSize EdgeSpacerSize;
	EdgeSpacerSize.SizeRule = ESlateSizeRule::Fill;
	EdgeSpacerSize.Value = 1.0f;

	if (USpacer* LeftSpacer = WidgetTree->ConstructWidget<USpacer>())
	{
		UHorizontalBoxSlot* LeftSpacerSlot = DominoInventoryBox->AddChildToHorizontalBox(LeftSpacer);
		if (LeftSpacerSlot)
		{
			LeftSpacerSlot->SetSize(EdgeSpacerSize);
		}
	}

	constexpr float InventoryItemSpacing = 16.0f;

	for (int32 Index = 0; Index < ItemWidgets.Num(); ++Index)
	{
		if (Index > 0)
		{
			USpacer* GapSpacer = WidgetTree->ConstructWidget<USpacer>();
			if (GapSpacer)
			{
				GapSpacer->SetSize(FVector2D(InventoryItemSpacing, 1.0f));
				DominoInventoryBox->AddChildToHorizontalBox(GapSpacer);
			}
		}

		UDominoInventoryItemWidget* ItemWidget = ItemWidgets[Index];
		UHorizontalBoxSlot* ItemSlot = DominoInventoryBox->AddChildToHorizontalBox(ItemWidget);
		if (ItemSlot)
		{
			ItemSlot->SetPadding(FMargin(0.0f, 4.0f));
			ItemSlot->SetHorizontalAlignment(HAlign_Center);
			ItemSlot->SetVerticalAlignment(VAlign_Center);
		}
	}

	if (USpacer* RightSpacer = WidgetTree->ConstructWidget<USpacer>())
	{
		UHorizontalBoxSlot* RightSpacerSlot = DominoInventoryBox->AddChildToHorizontalBox(RightSpacer);
		if (RightSpacerSlot)
		{
			RightSpacerSlot->SetSize(EdgeSpacerSize);
		}
	}
}

bool UDominoMiniGameWidget::ConvertBoardImageScreenPositionToWorld(const FGeometry& BoardGeometry, const FVector2D& ScreenPosition, FVector& OutWorldLocation) const
{
	if (!SceneCaptureComponent)
	{
		UE_LOG(LogDominoMiniGameWidget, Warning, TEXT("Board position conversion failed: SceneCaptureComponent is not set."));
		return false;
	}

	const FVector2D LocalPosition = BoardGeometry.AbsoluteToLocal(ScreenPosition);
	const FVector2D LocalSize = BoardGeometry.GetLocalSize();

	if (LocalSize.X <= KINDA_SMALL_NUMBER || LocalSize.Y <= KINDA_SMALL_NUMBER)
	{
		UE_LOG(LogDominoMiniGameWidget, Warning, TEXT("Board position conversion failed: BoardImage local size is invalid. LocalSize=%s"),
			*LocalSize.ToString());
		return false;
	}

	const FVector2D UV(LocalPosition.X / LocalSize.X, LocalPosition.Y / LocalSize.Y);
	if (UV.X < 0.0f || UV.X > 1.0f || UV.Y < 0.0f || UV.Y > 1.0f)
	{
		UE_LOG(LogDominoMiniGameWidget, Warning, TEXT("Board position conversion failed: drop is outside BoardImage. ScreenPosition=%s, LocalPosition=%s, LocalSize=%s, UV=%s"),
			*ScreenPosition.ToString(),
			*LocalPosition.ToString(),
			*LocalSize.ToString(),
			*UV.ToString());
		return false;
	}

	const float OrthoWidth = SceneCaptureComponent->OrthoWidth;
	if (OrthoWidth <= KINDA_SMALL_NUMBER)
	{
		UE_LOG(LogDominoMiniGameWidget, Warning, TEXT("Board position conversion failed: SceneCaptureComponent OrthoWidth is invalid. OrthoWidth=%f"),
			OrthoWidth);
		return false;
	}

	// 전제: SceneCapture는 Orthographic입니다.
	// UV를 캡처 카메라의 로컬 Right/Up 축 오프셋으로 변환한 뒤, 카메라 Forward 방향 Ray를 보드 평면 Z와 교차시킵니다.
	float AspectRatio = LocalSize.Y / LocalSize.X;
	if (SceneCaptureComponent->TextureTarget && SceneCaptureComponent->TextureTarget->SizeX > 0)
	{
		AspectRatio = static_cast<float>(SceneCaptureComponent->TextureTarget->SizeY) / static_cast<float>(SceneCaptureComponent->TextureTarget->SizeX);
	}

	const float OrthoHeight = OrthoWidth * AspectRatio;
	const float LocalCaptureX = (UV.X - 0.5f) * OrthoWidth;
	const float LocalCaptureY = (0.5f - UV.Y) * OrthoHeight;

	const FTransform CaptureTransform = SceneCaptureComponent->GetComponentTransform();
	const FVector RayOrigin =
		CaptureTransform.GetLocation()
		+ CaptureTransform.GetUnitAxis(EAxis::Y) * LocalCaptureX
		+ CaptureTransform.GetUnitAxis(EAxis::Z) * LocalCaptureY;

	const FVector RayDirection = CaptureTransform.GetUnitAxis(EAxis::X);
	if (Manager && Manager->BoardActor)
	{
		const FTransform BoardTransform = Manager->BoardActor->GetActorTransform();
		const FVector PlanePoint = BoardTransform.GetLocation();
		const FVector PlaneNormal = BoardTransform.GetUnitAxis(EAxis::Y);
		const float Denominator = FVector::DotProduct(RayDirection, PlaneNormal);

		if (FMath::Abs(Denominator) <= KINDA_SMALL_NUMBER)
		{
			UE_LOG(LogDominoMiniGameWidget, Warning, TEXT("Board position conversion failed: SceneCapture ray is parallel to board XZ plane."));
			return false;
		}

		const float T = FVector::DotProduct(PlanePoint - RayOrigin, PlaneNormal) / Denominator;
		OutWorldLocation = RayOrigin + RayDirection * T;
		return true;
	}

	const float TargetPlaneZ = Manager && Manager->BoardActor ? Manager->BoardActor->BoardPlaneZ : BoardPlaneZ;

	if (FMath::Abs(RayDirection.Z) > KINDA_SMALL_NUMBER)
	{
		const float T = (TargetPlaneZ - RayOrigin.Z) / RayDirection.Z;
		OutWorldLocation = RayOrigin + RayDirection * T;
	}
	else
	{
		// 카메라가 보드 평면과 거의 평행하면 정확한 교차 계산이 불가능합니다.
		// 이 경우 캡처 평면상의 X/Y를 유지하고 Z만 보드 평면으로 고정합니다.
		OutWorldLocation = RayOrigin;
		OutWorldLocation.Z = TargetPlaneZ;
	}

	return true;
}

void UDominoMiniGameWidget::HandleStartButtonClicked()
{
	if (Manager)
	{
		Manager->StartSimulation();
	}
}

void UDominoMiniGameWidget::HandleResetButtonClicked()
{
	if (Manager)
	{
		Manager->BeginRound();
		CreateDominoInventoryIcons();
	}
}

bool UDominoMiniGameWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);

	if (!Manager || !BoardImage || !Cast<UDominoDragDropOperation>(InOperation))
	{
		return false;
	}

	FVector WorldLocation;
	if (ConvertBoardImageScreenPositionToWorld(BoardImage->GetCachedGeometry(), InDragDropEvent.GetScreenSpacePosition(), WorldLocation))
	{
		Manager->RequestPreviewDominoAtWorldLocation(WorldLocation);
		return true;
	}

	Manager->CancelPreviewDomino();
	return false;
}

bool UDominoMiniGameWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UE_LOG(LogDominoMiniGameWidget, Warning, TEXT("Domino drop event received."));

	UDominoDragDropOperation* DominoOperation = Cast<UDominoDragDropOperation>(InOperation);
	if (!Manager || !BoardImage || !DominoOperation)
	{
		UE_LOG(LogDominoMiniGameWidget, Warning, TEXT("Domino drop failed: Manager=%s, BoardImage=%s, DominoOperation=%s"),
			Manager ? TEXT("Valid") : TEXT("None"),
			BoardImage ? TEXT("Valid") : TEXT("None"),
			DominoOperation ? TEXT("Valid") : TEXT("None"));
		return false;
	}

	FVector WorldLocation;
	const bool bOnBoard = ConvertBoardImageScreenPositionToWorld(BoardImage->GetCachedGeometry(), InDragDropEvent.GetScreenSpacePosition(), WorldLocation);
	bool bPlaced = false;

	if (bOnBoard)
	{
		Manager->RequestPreviewDominoAtWorldLocation(WorldLocation);
		bPlaced = Manager->ConfirmPlacePreviewDomino();
	}
	else
	{
		Manager->CancelPreviewDomino();
	}

	if (!bPlaced && DominoOperation->SourceItemWidget)
	{
		DominoOperation->SourceItemWidget->SetItemHiddenDuringDrag(false);
	}

	if (!bPlaced)
	{
		UE_LOG(LogDominoMiniGameWidget, Warning, TEXT("Domino drop did not place. bOnBoard=%s, WorldLocation=%s, ManagerState=%d, DominoClass=%s"),
			bOnBoard ? TEXT("true") : TEXT("false"),
			*WorldLocation.ToString(),
			Manager ? static_cast<int32>(Manager->CurrentState) : -1,
			Manager && Manager->RoundData.DominoClass ? *Manager->RoundData.DominoClass->GetName() : TEXT("None"));
	}

	return true;
}

void UDominoMiniGameWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	if (Manager && Cast<UDominoDragDropOperation>(InOperation))
	{
		Manager->CancelPreviewDomino();
	}
}
