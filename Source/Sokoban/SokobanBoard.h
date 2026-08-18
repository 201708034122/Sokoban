#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SokobanBoard.generated.h"

class UInstancedStaticMeshComponent;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FSokobanLevelCompleted,
    int32,
    MoveCount);

UENUM(BlueprintType)
enum class ESokobanTileType : uint8
{
    Empty,
    Floor,
    Wall,
    Target
};

USTRUCT(BlueprintType)
struct FSokobanLevelData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sokoban")
    int32 Rows = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sokoban")
    int32 Columns = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sokoban")
    TArray<ESokobanTileType> Tiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sokoban")
    FIntPoint PlayerStart = FIntPoint(1, 1);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sokoban")
    TArray<FIntPoint> BoxStarts;
};

struct FSokobanSnapshot
{
    FIntPoint PlayerPosition;
    TArray<FIntPoint> BoxPositions;
    int32 MoveCount = 0;
};

UCLASS()
class SOKOBAN_API ASokobanBoard : public AActor
{
    GENERATED_BODY()

public:
    ASokobanBoard();

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Sokoban|Game")
    bool TryMove(FIntPoint Direction);

    UFUNCTION(BlueprintCallable, Category = "Sokoban|Game")
    void ResetLevel();
    
    UFUNCTION(BlueprintCallable, Category = "Sokoban|Editor")
    bool SetEditorTileAtWorldLocation(
        FVector WorldLocation,
        ESokobanTileType TileType);

    UFUNCTION(BlueprintCallable, Category = "Sokoban|Editor")
    bool PlaceEditorBoxAtWorldLocation(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Sokoban|Editor")
    bool PlaceEditorPlayerAtWorldLocation(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Sokoban|Editor")
    bool EraseEditorCellAtWorldLocation(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Sokoban|Editor")
    void ClearEditorLevel();
    
    UFUNCTION(BlueprintCallable, Category = "Sokoban|Editor")
    bool ValidateEditorLevel(FText& OutErrorMessage) const;

    UFUNCTION(CallInEditor, Category = "Sokoban")
    void InitializeTestLevel();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sokoban")
    FSokobanLevelData LevelData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sokoban")
    float TileSize = 110.0f;
    
    UFUNCTION(BlueprintCallable, Category = "Sokoban|Game")
    bool UndoMove();

    UFUNCTION(BlueprintPure, Category = "Sokoban|Game")
    int32 GetMoveCount() const
    {
        return MoveCount;
    }

    UFUNCTION(BlueprintPure, Category = "Sokoban|Game")
    bool IsLevelCompleted() const
    {
        return bLevelCompleted;
    }

    UPROPERTY(BlueprintAssignable, Category = "Sokoban|Game")
    FSokobanLevelCompleted OnLevelCompleted;

protected:
    UPROPERTY(VisibleAnywhere, Category = "Sokoban|Visual")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Sokoban|Visual")
    TObjectPtr<UInstancedStaticMeshComponent> FloorInstances;

    UPROPERTY(VisibleAnywhere, Category = "Sokoban|Visual")
    TObjectPtr<UInstancedStaticMeshComponent> WallInstances;

    UPROPERTY(VisibleAnywhere, Category = "Sokoban|Visual")
    TObjectPtr<UInstancedStaticMeshComponent> TargetInstances;

    UPROPERTY(VisibleAnywhere, Category = "Sokoban|Visual")
    TObjectPtr<UInstancedStaticMeshComponent> BoxInstances;

    UPROPERTY(VisibleAnywhere, Category = "Sokoban|Visual")
    TObjectPtr<UInstancedStaticMeshComponent> PlayerInstances;

private:
    void RebuildVisuals();
    FVector GridToLocal(const FIntPoint& GridPosition, float Height) const;
    bool IsInsideGrid(const FIntPoint& Position) const;
    bool IsWalkable(const FIntPoint& Position) const;
    int32 FindBoxIndex(const FIntPoint& Position) const;
    void SaveSnapshot();
    bool CheckLevelCompleted() const;
    bool WorldToGrid(
        const FVector& WorldLocation,
        FIntPoint& OutGridPosition) const;

    TArray<FSokobanSnapshot> UndoStack;
    int32 MoveCount = 0;

    FIntPoint PlayerPosition;
    TArray<FIntPoint> BoxPositions;
    bool bRuntimeInitialized = false;
    bool bLevelCompleted = false;
};
