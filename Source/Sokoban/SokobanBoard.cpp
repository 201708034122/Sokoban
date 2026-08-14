#include "SokobanBoard.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ASokobanBoard::ASokobanBoard()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    FloorInstances =
        CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FloorInstances"));
    FloorInstances->SetupAttachment(SceneRoot);

    WallInstances =
        CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallInstances"));
    WallInstances->SetupAttachment(SceneRoot);

    TargetInstances =
        CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TargetInstances"));
    TargetInstances->SetupAttachment(SceneRoot);

    BoxInstances =
        CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("BoxInstances"));
    BoxInstances->SetupAttachment(SceneRoot);

    PlayerInstances =
        CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("PlayerInstances"));
    PlayerInstances->SetupAttachment(SceneRoot);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    
    FloorInstances->SetCanEverAffectNavigation(false);
    WallInstances->SetCanEverAffectNavigation(false);
    TargetInstances->SetCanEverAffectNavigation(false);
    BoxInstances->SetCanEverAffectNavigation(false);
    PlayerInstances->SetCanEverAffectNavigation(false);

    if (CubeMesh.Succeeded())
    {
        FloorInstances->SetStaticMesh(CubeMesh.Object);
        WallInstances->SetStaticMesh(CubeMesh.Object);
        TargetInstances->SetStaticMesh(CubeMesh.Object);
        BoxInstances->SetStaticMesh(CubeMesh.Object);
        PlayerInstances->SetStaticMesh(CubeMesh.Object);
    }
}

void ASokobanBoard::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (LevelData.Tiles.IsEmpty())
    {
        InitializeTestLevel();
    }
    else
    {
        RebuildVisuals();
    }
}

void ASokobanBoard::BeginPlay()
{
    Super::BeginPlay();
    
    bRuntimeInitialized = true;
    ResetLevel();
}

bool ASokobanBoard::TryMove(FIntPoint Direction)
{
    const int32 DirectionLength =
        FMath::Abs(Direction.X) +
        FMath::Abs(Direction.Y);

    if (!bRuntimeInitialized || bLevelCompleted || DirectionLength != 1)
    {
        return false;
    }

    const FIntPoint TargetPosition =
        PlayerPosition + Direction;

    if (!IsWalkable(TargetPosition))
    {
        return false;
    }

    const int32 BoxIndex =
        FindBoxIndex(TargetPosition);

    FIntPoint BoxTargetPosition = TargetPosition;

    if (BoxIndex != INDEX_NONE)
    {
        BoxTargetPosition += Direction;

        if (!IsWalkable(BoxTargetPosition) ||
            FindBoxIndex(BoxTargetPosition) != INDEX_NONE)
        {
            return false;
        }
    }

    SaveSnapshot();

    if (BoxIndex != INDEX_NONE)
    {
        BoxPositions[BoxIndex] = BoxTargetPosition;
    }

    PlayerPosition = TargetPosition;
    ++MoveCount;

    RebuildVisuals();

    if (CheckLevelCompleted())
    {
        bLevelCompleted = true;
        OnLevelCompleted.Broadcast(MoveCount);
    }

    return true;
}

void ASokobanBoard::RebuildVisuals()
{
    FloorInstances->ClearInstances();
    WallInstances->ClearInstances();
    TargetInstances->ClearInstances();
    BoxInstances->ClearInstances();
    PlayerInstances->ClearInstances();

    const int32 ExpectedTileCount =
        LevelData.Rows * LevelData.Columns;

    if (LevelData.Rows <= 0 ||
        LevelData.Columns <= 0 ||
        LevelData.Tiles.Num() != ExpectedTileCount)
    {
        return;
    }

    for (int32 Row = 0; Row < LevelData.Rows; ++Row)
    {
        for (int32 Column = 0;
             Column < LevelData.Columns;
             ++Column)
        {
            const int32 TileIndex =
                Row * LevelData.Columns + Column;

            const ESokobanTileType Tile =
                LevelData.Tiles[TileIndex];

            if (Tile == ESokobanTileType::Empty)
            {
                continue;
            }

            const FIntPoint GridPosition(Column, Row);

            FloorInstances->AddInstance(FTransform(
                FRotator::ZeroRotator,
                GridToLocal(GridPosition, 0.0f),
                FVector(1.0f, 1.0f, 0.08f)));

            if (Tile == ESokobanTileType::Wall)
            {
                WallInstances->AddInstance(FTransform(
                    FRotator::ZeroRotator,
                    GridToLocal(GridPosition, 55.0f),
                    FVector(1.0f, 1.0f, 1.0f)));
            }
            else if (Tile == ESokobanTileType::Target)
            {
                TargetInstances->AddInstance(FTransform(
                    FRotator::ZeroRotator,
                    GridToLocal(GridPosition, 10.0f),
                    FVector(0.55f, 0.55f, 0.08f)));
            }
        }
    }

    const TArray<FIntPoint>& VisibleBoxPositions =
        bRuntimeInitialized
            ? BoxPositions
            : LevelData.BoxStarts;

    for (const FIntPoint& BoxPosition : VisibleBoxPositions)
    {
        BoxInstances->AddInstance(FTransform(
            FRotator::ZeroRotator,
            GridToLocal(BoxPosition, 55.0f),
            FVector(0.75f, 0.75f, 0.75f)));
    }

    PlayerInstances->AddInstance(FTransform(
        FRotator::ZeroRotator,
        GridToLocal(
            bRuntimeInitialized
                ? PlayerPosition
                : LevelData.PlayerStart,
            45.0f),
        FVector(0.55f, 0.55f, 0.9f)));
}

bool ASokobanBoard::IsInsideGrid(
    const FIntPoint& Position) const
{
    return Position.X >= 0 &&
        Position.X < LevelData.Columns &&
        Position.Y >= 0 &&
        Position.Y < LevelData.Rows;
}

bool ASokobanBoard::IsWalkable(
    const FIntPoint& Position) const
{
    if (!IsInsideGrid(Position))
    {
        return false;
    }

    const int32 TileIndex =
        Position.Y * LevelData.Columns + Position.X;

    const ESokobanTileType Tile =
        LevelData.Tiles[TileIndex];

    return Tile == ESokobanTileType::Floor ||
        Tile == ESokobanTileType::Target;
}

int32 ASokobanBoard::FindBoxIndex(
    const FIntPoint& Position) const
{
    return BoxPositions.IndexOfByKey(Position);
}

void ASokobanBoard::ResetLevel()
{
    PlayerPosition = LevelData.PlayerStart;
    BoxPositions = LevelData.BoxStarts;
    MoveCount = 0;
    UndoStack.Empty();
    bLevelCompleted = false;

    RebuildVisuals();
}

void ASokobanBoard::InitializeTestLevel()
{
    LevelData.Rows = 7;
    LevelData.Columns = 9;
    LevelData.Tiles.Init(
        ESokobanTileType::Floor,
        LevelData.Rows * LevelData.Columns);

    for (int32 Row = 0; Row < LevelData.Rows; ++Row)
    {
        for (int32 Column = 0; Column < LevelData.Columns; ++Column)
        {
            const bool bIsBoundary =
                Row == 0 ||
                Row == LevelData.Rows - 1 ||
                Column == 0 ||
                Column == LevelData.Columns - 1;

            if (bIsBoundary)
            {
                const int32 TileIndex =
                    Row * LevelData.Columns + Column;

                LevelData.Tiles[TileIndex] =
                    ESokobanTileType::Wall;
            }
        }
    }

    LevelData.Tiles[2 * LevelData.Columns + 6] =
        ESokobanTileType::Target;

    LevelData.PlayerStart = FIntPoint(2, 2);

    LevelData.BoxStarts.Empty();
    LevelData.BoxStarts.Add(FIntPoint(4, 2));

    RebuildVisuals();
}

bool ASokobanBoard::UndoMove()
{
    if (UndoStack.IsEmpty())
    {
        return false;
    }

    const FSokobanSnapshot Snapshot = UndoStack.Pop();

    PlayerPosition = Snapshot.PlayerPosition;
    BoxPositions = Snapshot.BoxPositions;
    MoveCount = Snapshot.MoveCount;
    bLevelCompleted = false;

    RebuildVisuals();
    return true;
}

FVector ASokobanBoard::GridToLocal(
    const FIntPoint& GridPosition,
    float Height) const
{
    return FVector(
        GridPosition.X * TileSize,
        GridPosition.Y * TileSize,
        Height);
}

void ASokobanBoard::SaveSnapshot()
{
    FSokobanSnapshot Snapshot;
    Snapshot.PlayerPosition = PlayerPosition;
    Snapshot.BoxPositions = BoxPositions;
    Snapshot.MoveCount = MoveCount;

    UndoStack.Add(MoveTemp(Snapshot));
}

bool ASokobanBoard::CheckLevelCompleted() const
{
    if (BoxPositions.IsEmpty())
    {
        return false;
    }

    int32 TargetCount = 0;

    for (const ESokobanTileType Tile : LevelData.Tiles)
    {
        if (Tile == ESokobanTileType::Target)
        {
            ++TargetCount;
        }
    }

    if (TargetCount != BoxPositions.Num())
    {
        return false;
    }

    for (const FIntPoint& BoxPosition : BoxPositions)
    {
        const int32 TileIndex =
            BoxPosition.Y * LevelData.Columns + BoxPosition.X;

        if (!LevelData.Tiles.IsValidIndex(TileIndex) ||
            LevelData.Tiles[TileIndex] != ESokobanTileType::Target)
        {
            return false;
        }
    }

    return true;
}
