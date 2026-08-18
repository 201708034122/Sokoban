#include "SokobanPlayerController.h"

#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "SokobanBoard.h"

void ASokobanPlayerController::BeginPlay()
{
    Super::BeginPlay();

    for (TActorIterator<ASokobanBoard> It(GetWorld()); It; ++It)
    {
        Board = *It;
        break;
    }

    if (!Board)
    {
        UE_LOG(LogTemp, Error, TEXT("Sokoban board was not found."));
    }

    FInputModeGameAndUI InputMode;
    InputMode.SetHideCursorDuringCapture(false);

    SetInputMode(InputMode);
    bShowMouseCursor = true;
}

void ASokobanPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    InputComponent->BindKey(
        EKeys::W,
        IE_Pressed,
        this,
        &ASokobanPlayerController::MoveUp);

    InputComponent->BindKey(
        EKeys::Up,
        IE_Pressed,
        this,
        &ASokobanPlayerController::MoveUp);

    InputComponent->BindKey(
        EKeys::S,
        IE_Pressed,
        this,
        &ASokobanPlayerController::MoveDown);

    InputComponent->BindKey(
        EKeys::Down,
        IE_Pressed,
        this,
        &ASokobanPlayerController::MoveDown);

    InputComponent->BindKey(
        EKeys::A,
        IE_Pressed,
        this,
        &ASokobanPlayerController::MoveLeft);

    InputComponent->BindKey(
        EKeys::Left,
        IE_Pressed,
        this,
        &ASokobanPlayerController::MoveLeft);

    InputComponent->BindKey(
        EKeys::D,
        IE_Pressed,
        this,
        &ASokobanPlayerController::MoveRight);

    InputComponent->BindKey(
        EKeys::Right,
        IE_Pressed,
        this,
        &ASokobanPlayerController::MoveRight);
    
    InputComponent->BindKey(
        EKeys::Z,
        IE_Pressed,
        this,
        &ASokobanPlayerController::UndoMove);

    InputComponent->BindKey(
        EKeys::R,
        IE_Pressed,
        this,
        &ASokobanPlayerController::RestartLevel);
}

void ASokobanPlayerController::MoveUp()
{
    if (Board)
    {
        Board->TryMove(FIntPoint(0, -1));
    }
}

void ASokobanPlayerController::MoveDown()
{
    if (Board)
    {
        Board->TryMove(FIntPoint(0, 1));
    }
}

void ASokobanPlayerController::MoveLeft()
{
    if (Board)
    {
        Board->TryMove(FIntPoint(-1, 0));
    }
}

void ASokobanPlayerController::MoveRight()
{
    if (Board)
    {
        Board->TryMove(FIntPoint(1, 0));
    }
}

void ASokobanPlayerController::UndoMove()
{
    if (Board)
    {
        Board->UndoMove();
    }
}

void ASokobanPlayerController::RestartLevel()
{
    if (Board)
    {
        Board->ResetLevel();
    }
}

