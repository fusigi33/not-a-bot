#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DominoSimulationObjectInterface.generated.h"

UINTERFACE(BlueprintType)
class NOTABOT_API UDominoSimulationObjectInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 도미노 라운드의 배치 단계/시뮬레이션 단계 전환을 받는 상호작용 오브젝트 인터페이스입니다.
 */
class NOTABOT_API IDominoSimulationObjectInterface
{
	GENERATED_BODY()

public:
	/** 라운드 시뮬레이션이 시작/종료될 때 호출됩니다. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Domino MiniGame")
	void SetDominoSimulationEnabled(bool bEnabled);

	/** 라운드 리셋 시 오브젝트를 초기 배치 상태로 되돌립니다. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Domino MiniGame")
	void ResetDominoSimulationObject();
};
