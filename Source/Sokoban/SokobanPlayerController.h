#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SokobanPlayerController.generated.h"

class ASokobanBoard;

UCLASS()
class SOKOBAN_API ASokobanPlayerController
	: public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void MoveUp();
	void MoveDown();
	void MoveLeft();
	void MoveRight();
	
	void UndoMove();
	void RestartLevel();

	UPROPERTY()
	TObjectPtr<ASokobanBoard> Board;
};