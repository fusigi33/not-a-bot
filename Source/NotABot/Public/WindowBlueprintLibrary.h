#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WindowBlueprintLibrary.generated.h"

UCLASS()
class NOTABOT_API UWindowBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Window", meta=(WorldContext="WorldContextObject"))
	static void MinimizeGameWindow(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="Window", meta=(WorldContext="WorldContextObject"))
	static void MaximizeGameWindow(const UObject* WorldContextObject);

private:
	static TSharedPtr<class SWindow> GetTargetWindow(const UObject* WorldContextObject);
};
