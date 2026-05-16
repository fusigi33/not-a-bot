#include "WindowBlueprintLibrary.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"

void UWindowBlueprintLibrary::MinimizeGameWindow(const UObject* WorldContextObject)
{
	if (TSharedPtr<SWindow> Window = GetTargetWindow(WorldContextObject))
	{
		Window->Minimize();
	}
}

void UWindowBlueprintLibrary::MaximizeGameWindow(const UObject* WorldContextObject)
{
	if (TSharedPtr<SWindow> Window = GetTargetWindow(WorldContextObject))
	{
		Window->Maximize();
	}
}

TSharedPtr<SWindow> UWindowBlueprintLibrary::GetTargetWindow(const UObject* WorldContextObject)
{
	if (GEngine)
	{
		if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull))
		{
			if (UGameViewportClient* GameViewport = World->GetGameViewport())
			{
				if (TSharedPtr<SWindow> Window = GameViewport->GetWindow())
				{
					return Window;
				}
			}
		}

		if (GEngine->GameViewport)
		{
			if (TSharedPtr<SWindow> Window = GEngine->GameViewport->GetWindow())
			{
				return Window;
			}
		}
	}

	if (FSlateApplication::IsInitialized())
	{
		return FSlateApplication::Get().GetActiveTopLevelWindow();
	}

	return nullptr;
}
