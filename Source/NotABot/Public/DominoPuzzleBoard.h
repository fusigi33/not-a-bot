#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DominoPuzzleBoard.generated.h"

class UDominoPuzzleWidget;
class UBoxComponent;
class USceneCaptureComponent2D;
class UStaticMeshComponent;
class ADominoBlock;

UCLASS()
class NOTABOT_API ADominoPuzzleBoard : public AActor
{
    GENERATED_BODY()

public:
    ADominoPuzzleBoard();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USceneComponent> Root;

    // 도미노들이 놓일 기준 바닥
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> BoardMesh;

    // 도미노를 내려놓을 수 있는 허용 구역
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UBoxComponent> PlacementArea;

    // 카메라
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
    TObjectPtr<USceneCaptureComponent2D> SceneCapture;

    // 처음 넘어뜨릴 도미노 블록
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Puzzle")
    TObjectPtr<ADominoBlock> StartDomino;

    // 넘어뜨려야 하는 목표 도미노 블록
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Puzzle")
    TObjectPtr<ADominoBlock> EndDomino;

    // 배치 규칙
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle|Placement")
    float GridSnap = 50.0f;

    // 주어지는 도미노 블록 개수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle|Placement")
    int32 MaxPlacedDominoes = 5;

    // Side View 전용:
    // 화면 가로 = X, 화면 세로 = Z, Y는 고정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle|Placement")
    float FixedDepthY = 0.0f;

    // 보드 면에서 약간 앞으로 띄워서 Z-fighting/겹침 방지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle|Placement")
    float SurfaceOffsetY = 4.0f;

    // RenderTarget 종횡비
    // RT가 정사각형이면 1.0
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle|Capture")
    float CaptureAspectRatio = 0.5f;

    // 시작 시 밀어줄 힘
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle|Simulation")
    float PushImpulse = 250.0f;

    // 첫 도미노를 밀고 몇 초 뒤에 결과를 판정할지 결정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle|Simulation")
    float ResultCheckDelay = 2.5f;

    UPROPERTY(BlueprintReadOnly, Category="Puzzle")
    bool bPuzzleRunning = false;

    UPROPERTY(BlueprintReadOnly, Category="Puzzle")
    TArray<TObjectPtr<ADominoBlock>> PlacedDominoes;
    
    UPROPERTY()
    TObjectPtr<UDominoPuzzleWidget> DominoPuzzleWidget;

public:
    /**
     * 2D 화면(UI) 상의 좌표(UV)를 3D 게임 월드 내 보드 위의 실제 좌표로 변환하는 함수입니다.
     * 
     * @param UV 0.0 ~ 1.0 사이의 정규화된 2D 좌표                                                                   
     * @param OutWorld 계산이 완료된 3D 월드 좌표계의 결과값
     * @return : 변환이 성공적으로 이루어졌으면 true, 유효하지 않은 값이면 false
     */
    UFUNCTION(BlueprintCallable, Category="Puzzle|Placement")
    bool ConvertUVToBoardWorld(FVector2D UV, FVector& OutWorld) const;

    /**
     * 사용자가 화면(UV)을 클릭하거나 드래그 앤 드롭을 마쳤을 때, 관련 데이터를 트랜스폼으로 만듭니다.
     * 
     * @param UV 마우스가 위치한 2D 화면 좌표
     * @param YawDegrees 도미노가 바라볼 방향 (회전값)
     * @param OutTransform 도미노가 최종적으로 배치될 3D 월드의 트랜스폼
     * @return 이 위치에 도미노를 놓기 위한 좌표 계산이 유효한지 여부
     */
    UFUNCTION(BlueprintCallable, Category="Puzzle|Placement")
    bool GetPlacementTransformFromUV(FVector2D UV, float YawDegrees, FTransform& OutTransform) const;

    /**
     * 지정된 월드 트랜스폼(위치 및 회전)에 도미노 블록을 배치할 수 있는지 논리적, 물리적 유효성을 검사합니다.
     * 
     * @param TestTransform 도미노가 배치될 목표 월드 트랜스폼 (위치 및 회전)
     * @param DominoClass 스폰하려는 도미노 블록의 클래스 (가상 바운딩 박스의 크기를 동적으로 계산하는 데 사용)
     * @return 배치가 허용되면 true, 규칙을 위반하거나 예외 처리되지 않은 다른 물체와 물리적으로 겹친다면 false
     */
    UFUNCTION(BlueprintCallable, Category="Puzzle|Placement")
    bool CanPlaceAtTransform(const FTransform& TestTransform, TSubclassOf<ADominoBlock> DominoClass) const;

    /**
     * 2D 화면의 UV 좌표를 기준으로 게임 월드에 도미노 블록을 스폰하고 배치합니다.
     * 
     * @param UV 도미노를 배치할 2D 화면(UI)상의 정규화된 좌표 (X, Y 모두 0.0 ~ 1.0)
     * @param DominoClass 스폰할 도미노 블록의 클래스 정보 (CDO)
     * @param YawDegrees 배치될 도미노 블록에 적용할 좌우 회전각 (Yaw)
     */
    UFUNCTION(BlueprintCallable, Category="Puzzle|Placement")
    ADominoBlock* TryPlaceDominoFromUV(FVector2D UV, TSubclassOf<ADominoBlock> DominoClass, float YawDegrees);

    /**
     * 플레이어가 UI 화면(UV)에서 특정 위치를 클릭했을 때, 해당 위치에 있는 도미노를 찾아서 월드에서 지우고(Destroy) 관리 목록에서 삭제합니다.
     * @param UV 도미노를 제거하기 위해 클릭/선택한 화면의 정규화된 2D 좌표 (0.0 ~ 1.0)
     * @return 유효한 범위 내에서 도미노를 성공적으로 찾아 제거했다면 true, 아무것도 제거하지 못했거나 퍼즐 진행 중이라면 false
     */
    UFUNCTION(BlueprintCallable, Category="Puzzle|Placement")
    ADominoBlock* TryRemoveDominoFromUV(FVector2D UV);

    UFUNCTION(BlueprintCallable, Category="Puzzle|Simulation")
    void StartSimulation();

    UFUNCTION(BlueprintCallable, Category="Puzzle|Simulation")
    void ResetPuzzle();

protected:
    /**
     * 플레이어가 배치한 도미노가 PlacementArea를 벗어났는지 검사합니다.
     * 
     * @param LocalPos 보드 컴포넌트의 중심(0, 0, 0)을 기준으로 변환된 타겟의 3D 로컬 좌표
     * @return 위치가 배치 영역(X, Z) 내부에 속하면 true, 범위를 벗어났거나 PlacementArea 컴포넌트가 유효하지 않으면 false
     */
    bool IsInsidePlacementBoundsLocal(const FVector& LocalPos) const;

    /**
     * 플레이어가 도미노 배치를 모두 마치고 시작(Play) 버튼을 눌렀을 때 호출되어 연쇄 반응을 일으키는 트리거입니다.
     */
    void PushStartDomino();
    
    void CheckPuzzleResult();
};