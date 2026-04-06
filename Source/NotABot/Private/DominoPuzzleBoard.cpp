#include "DominoPuzzleBoard.h"
#include "DominoBlock.h"

#include "Components/BoxComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADominoPuzzleBoard::ADominoPuzzleBoard()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    BoardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoardMesh"));
    BoardMesh->SetupAttachment(Root);
    BoardMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    BoardMesh->SetCollisionProfileName(TEXT("BlockAll"));

    PlacementArea = CreateDefaultSubobject<UBoxComponent>(TEXT("PlacementArea"));
    PlacementArea->SetupAttachment(Root);
    PlacementArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // X 범위, Y 두께, Z 범위
    PlacementArea->SetBoxExtent(FVector(300.0f, 50.0f, 200.0f));

    SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
    SceneCapture->SetupAttachment(Root);
    SceneCapture->ProjectionType = ECameraProjectionMode::Orthographic;
    SceneCapture->OrthoWidth = 1024.0f;
    SceneCapture->bCaptureEveryFrame = true;
    SceneCapture->bCaptureOnMovement = true;
}

void ADominoPuzzleBoard::BeginPlay()
{
    Super::BeginPlay();
    
    if (StartDomino)
    {
        StartDomino->SetPhysicsEnabled(false);
    }
    
    if (EndDomino)
    {
        EndDomino->SetPhysicsEnabled(false);
    }
}

bool ADominoPuzzleBoard::ConvertUVToBoardWorld(FVector2D UV, FVector& OutWorld) const
{
    if (!SceneCapture || !BoardMesh)
    {
        return false;
    }

    // UV 유효 범위 체크
    if (UV.X < 0.0f || UV.X > 1.0f || UV.Y < 0.0f || UV.Y > 1.0f)
    {
        return false;
    }

    // Orthographic width는 가로 폭
    const float OrthoWidth = SceneCapture->OrthoWidth;

    // 세로 폭은 RT 종횡비 기준으로 계산
    // Aspect = Width / Height 이므로 Height은 OrthoWidth / SafeAspect
    const float SafeAspect = FMath::Max(0.01f, CaptureAspectRatio);
    const float OrthoHeight = OrthoWidth / SafeAspect;

    // 화면 중심 기준 -0.5 ~ +0.5
    const float UCentered = UV.X - 0.5f;
    const float VCentered = UV.Y - 0.5f;

    // Side View 기준:
    // 화면 가로 -> 월드 X
    // 화면 세로 -> 월드 Z
    //
    // UMG는 아래로 갈수록 Y가 증가하니까, 월드 Z(위쪽+)와 맞추려면 세로는 부호 반전
    const float LocalX = UCentered * OrthoWidth;
    const float LocalZ = -VCentered * OrthoHeight;
    const float LocalY = FixedDepthY;

    const FVector LocalBoardPos(LocalX, LocalY, LocalZ);
    OutWorld = PlacementArea->GetComponentTransform().TransformPosition(LocalBoardPos);
    return true;
}

bool ADominoPuzzleBoard::IsInsidePlacementBoundsLocal(const FVector& LocalPos) const
{
    if (!PlacementArea)
    {
        return false;
    }

    // LocalPos는 월드 좌표에서 보드의 트랜스폼 영향을 모두 역산하여 벗겨낸 순수한 로컬 좌표이므로
    // 검사하는 기준선이 되는 박스의 영역(Extent) 역시 스케일이 들어가지 않은 순수한 원본 크기를 가져와야함
    const FVector Extent = PlacementArea->GetUnscaledBoxExtent();

    // Side View 배치:
    // X, Z 범위만 체크
    return FMath::Abs(LocalPos.X) <= Extent.X &&
           FMath::Abs(LocalPos.Z) <= Extent.Z;
}

bool ADominoPuzzleBoard::GetPlacementTransformFromUV(FVector2D UV, float YawDegrees, FTransform& OutTransform) const
{
    // 2D 마우스 좌표를 도미노 보드판 위의 3D 월드 좌표로 변환
    FVector WorldPos;
    if (!ConvertUVToBoardWorld(UV, WorldPos))
    {
        return false;
    }

    const FTransform TargetTransform = PlacementArea->GetComponentTransform();
    FVector LocalPos = TargetTransform.InverseTransformPosition(WorldPos);

    // 도미노가 아무 곳에나 마구잡이로 놓이지 않고 체스판처럼 정해진 칸(Grid)에 예쁘게 정렬되도록 함
    // FVector SnappedLocal = LocalPos;
    // SnappedLocal.X = FMath::GridSnap(SnappedLocal.X, GridSnap);
    // SnappedLocal.Z = FMath::GridSnap(SnappedLocal.Z, GridSnap);
    // SnappedLocal.Y = FixedDepthY;
    
    FVector SnappedLocal = LocalPos;
    SnappedLocal.Y = FixedDepthY;

    // 보드 범위를 벗어났는지 검사
    if (!IsInsidePlacementBoundsLocal(SnappedLocal))
    {
        return false;
    }
    
    FVector FinalWorld = TargetTransform.TransformPosition(SnappedLocal);

    // 도미노와 보드판의 3D 메시가 완전히 겹쳐서 텍스처가 깜빡거리는 현상(Z-fighting)을 막고,
    // 물리적인 충돌 판정을 위해 보드 표면에서 아주 살짝 앞(Y축 방향)으로 띄워줌
    FinalWorld.Y += SurfaceOffsetY;
    
    // 계산된 최종 위치(FinalWorld)와, 입력으로 받은 회전값(YawDegrees)을 결합하여 최종 트랜스폼 조립
    const FRotator FinalRot(0.0f, YawDegrees, 0.0f);
    OutTransform = FTransform(FinalRot, FinalWorld, FVector(1.0f));
    return true;
}

bool ADominoPuzzleBoard::CanPlaceAtTransform(const FTransform& TestTransform, TSubclassOf<ADominoBlock> DominoClass) const
{
    // 물리적인 계산을 하기 전에, 게임의 논리적인 규칙을 어겼는지 확인
    if (!DominoClass || bPuzzleRunning)
    {
        return false;
    }

    if (PlacedDominoes.Num() >= MaxPlacedDominoes)
    {
        return false;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // 혹시 모를 상황을 대비한 기본(Fallback) 크기
    FVector HalfSize(6.0f, 4.0f, 10.0f);
    
    // CDO(Class Default Object)를 활용한 동적 바운딩 박스 계산
    ADominoBlock* DefaultDomino = DominoClass->GetDefaultObject<ADominoBlock>();
    if (DefaultDomino && DefaultDomino->Mesh && DefaultDomino->Mesh->GetStaticMesh())
    {
        FBoxSphereBounds MeshBounds = DefaultDomino->Mesh->GetStaticMesh()->GetBounds();
        FVector Scale = DefaultDomino->Mesh->GetRelativeScale3D();
        HalfSize = MeshBounds.BoxExtent * Scale;
    }
    
    const FQuat Rot = TestTransform.GetRotation();
    const FCollisionShape Shape = FCollisionShape::MakeBox(HalfSize);

    FCollisionQueryParams Params(SCENE_QUERY_STAT(DominoPlaceOverlap), false);
    Params.AddIgnoredActor(this);

    // 목표 위치(GetLocation)에, 목표한 회전 각도(Rot)로, 방금 만든 가상 상자(Shape)를 들이밀었을 때, 
    // ECC_WorldDynamic 채널에 속하는 어떤 물체와 부딪히거나 겹치는지 검사.
    // 보드판에 놓여있는 다른 도미노들이 바로 이 ECC_WorldDynamic 채널에 속해 있기 때문에
    // 목표 자리에 이미 다른 도미노가 알박기를 하고 있다면 이 검사망에 걸리게 됨
    TArray<FOverlapResult> Overlaps;
    const bool bAnyOverlap = World->OverlapMultiByChannel(
        Overlaps,
        TestTransform.GetLocation(),
        Rot,
        ECC_WorldDynamic,
        Shape,
        Params
    );

    // 최종 결과 로직
    if (!bAnyOverlap)
    {
        return true;
    }

    for (const FOverlapResult& Result : Overlaps)
    {
        const AActor* HitActor = Result.GetActor();
        if (!HitActor)
        {
            continue;
        }

        // [방법 A] 태그(Tag)를 사용하는 방식
        /*
        if (HitActor->ActorHasTag(TEXT("Obstacle")))
        {
            continue;
        }
        */

        // [방법 B] 클래스 타입을 검사하는 방식
        /*
        if (HitActor->IsA<ADominoBlock>())
        {
            return false;
        }
        */
        return false;
    }

    return true;
}

ADominoBlock* ADominoPuzzleBoard::TryPlaceDominoFromUV(FVector2D UV, TSubclassOf<ADominoBlock> DominoClass, float YawDegrees)
{
    if (!DominoClass || bPuzzleRunning)
    {
        return nullptr;
    }

    FTransform SpawnTransform;
    if (!GetPlacementTransformFromUV(UV, YawDegrees, SpawnTransform))
    {
        return nullptr;
    }

    if (!CanPlaceAtTransform(SpawnTransform, DominoClass))
    {
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ADominoBlock* NewDomino = World->SpawnActor<ADominoBlock>(DominoClass, SpawnTransform, Params);
    if (!NewDomino)
    {
        return nullptr;
    }
    
    if (NewDomino->Mesh)
    {
        // 무게 중심을 아래로 내려서 오뚜기처럼 만듦
        NewDomino->Mesh->BodyInstance.COMNudge = FVector(0.0f, 0.0f, -100.0f);
        
        // 일어난 뒤에 젤리처럼 계속 흔들거리지 않도록, 회전할 때의 공기 저항(Damping)을 높여 금방 멈추게 함
        NewDomino->Mesh->SetAngularDamping(3.0f); 

        // 물리 엔진에 변경된 질량과 무게중심을 즉시 업데이트
        NewDomino->Mesh->BodyInstance.UpdateMassProperties();
    }
    
    NewDomino->SetPhysicsEnabled(true);
    
    // 생성된 도미노를 보드판이 관리하는 배열에 추가
    PlacedDominoes.Add(NewDomino);
    
    SceneCapture->ShowOnlyActorComponents(NewDomino);

    return NewDomino;
}

ADominoBlock* ADominoPuzzleBoard::TryRemoveDominoFromUV(FVector2D UV)
{
    if (bPuzzleRunning) return nullptr;

    FVector WorldPos;
    if (!ConvertUVToBoardWorld(UV, WorldPos)) return nullptr;

    ADominoBlock* DominoToRemove = nullptr;

    // 마우스를 아주 정밀하게 맞추지 않아도 클릭되도록 여유값(Tolerance)을 조금 더 늘렸습니다.
    const float ClickTolerance = 20.0f; 

    for (ADominoBlock* Domino : PlacedDominoes)
    {
        // 도미노 액터와 내부의 Mesh 컴포넌트가 유효한지 확인합니다.
        if (!IsValid(Domino) || !Domino->Mesh) continue;

        // 핵심: 액터 전체가 아니라, '눈에 보이는 실제 도미노 메시'의 정확한 월드 바운딩 박스를 가져옵니다.
        FBox MeshBox = Domino->Mesh->Bounds.GetBox();

        // 클릭하기 편하도록 박스의 상하좌우 크기를 여유값만큼 살짝 팽창시킵니다.
        MeshBox = MeshBox.ExpandBy(FVector(ClickTolerance, ClickTolerance, ClickTolerance));

        // 월드 좌표 기준, 클릭한 위치가 박스의 최소(Min) ~ 최대(Max) 영역 사이에 들어왔는지 아주 직관적으로 검사합니다.
        if (WorldPos.X >= MeshBox.Min.X && WorldPos.X <= MeshBox.Max.X &&
            WorldPos.Z >= MeshBox.Min.Z && WorldPos.Z <= MeshBox.Max.Z)
        {
            DominoToRemove = Domino;
            break;
        }
    }

    if (DominoToRemove)
    {
        PlacedDominoes.Remove(DominoToRemove);
        DominoToRemove->Destroy();
        return DominoToRemove;
    }

    return nullptr;
}

void ADominoPuzzleBoard::PushStartDomino()
{
    if (!StartDomino || !StartDomino->Mesh)
    {
        return;
    }

    // Side View 기준으로 좌/우 진행을 원하면 보통 X 방향 힘이 직관적
    // 도미노의 실제 전진축과 맞지 않으면 GetActorForwardVector 대신
    // FVector::ForwardVector 또는 보드 기준 축을 직접 사용해도 됨
    const FVector PushDir = StartDomino->GetActorForwardVector();
    StartDomino->Mesh->AddImpulse(PushDir * PushImpulse, NAME_None, true);
}

void ADominoPuzzleBoard::StartSimulation()
{
    if (bPuzzleRunning)
    {
        return;
    }

    bPuzzleRunning = true;
    
    // 배치된 오뚝이 도미노들을 평범한 도미노로 해제
    for (ADominoBlock* Domino : PlacedDominoes)
    {
        if (IsValid(Domino) && Domino->Mesh)
        {
            // 무게중심 오프셋을 0으로 만들어 원래 도미노의 무게중심으로 복구
            Domino->Mesh->BodyInstance.COMNudge = FVector::ZeroVector;
            
            // 멈추게 만들었던 회전 저항도 기본값(0.0f)으로 돌려놓음
            Domino->Mesh->SetAngularDamping(0.0f);

            // 바뀐 무게중심 업데이트
            Domino->Mesh->BodyInstance.UpdateMassProperties();
        }
    }

    if (StartDomino)
    {
        StartDomino->SetPhysicsEnabled(true);
    }

    if (EndDomino)
    {
        EndDomino->SetPhysicsEnabled(true);
    }

    for (ADominoBlock* Domino : PlacedDominoes)
    {
        if (IsValid(Domino))
        {
            Domino->SetPhysicsEnabled(true);
        }
    }

    PushStartDomino();

    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(
        TimerHandle,
        this,
        &ADominoPuzzleBoard::CheckPuzzleResult,
        ResultCheckDelay,
        false
    );
}

void ADominoPuzzleBoard::CheckPuzzleResult()
{
    const bool bSuccess = EndDomino && EndDomino->IsKnockedDown(0.65f);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Domino puzzle success"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Domino puzzle failed"));
    }
}

void ADominoPuzzleBoard::ResetPuzzle()
{
    bPuzzleRunning = false;

    if (StartDomino)
    {
        StartDomino->ResetDomino();
    }

    if (EndDomino)
    {
        EndDomino->ResetDomino();
    }

    for (ADominoBlock* Domino : PlacedDominoes)
    {
        if (IsValid(Domino))
        {
            Domino->Destroy();
        }
    }

    PlacedDominoes.Empty();
}