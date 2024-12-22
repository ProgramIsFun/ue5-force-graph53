// Copyright Epic Games, Inc. All Rights Reserved.


// #include "GameFramework/DefaultPawn.h"
#include "DefaultPawn2.h"

#include "utillllllssss.h"
#include "GameFramework/Controller.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PlayerInput.h"

//#include UE_INLINE_GENERATED_CPP_BY_NAME(DefaultPawn2)

FName ADefaultPawn2::MovementComponentName(TEXT("MovementComponent0"));
FName ADefaultPawn2::CollisionComponentName(TEXT("CollisionComponent0"));
FName ADefaultPawn2::MeshComponentName(TEXT("MeshComponent0"));


ADefaultPawn2::ADefaultPawn2(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetCanBeDamaged(true);
	
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	NetPriority = 3.0f;
	
	BaseEyeHeight = 0.0f;
	bCollideWhenPlacing = false;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(ADefaultPawn2::CollisionComponentName);
	CollisionComponent->InitSphereRadius(35.0f);
	CollisionComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	
	CollisionComponent->CanCharacterStepUpOn = ECB_No;
	CollisionComponent->SetShouldUpdatePhysicsVolume(true);
	CollisionComponent->SetCanEverAffectNavigation(false);
	CollisionComponent->bDynamicObstacle = true;
	
	RootComponent = CollisionComponent;
	
	MovementComponent = CreateDefaultSubobject<UPawnMovementComponent, UFloatingPawnMovement>(ADefaultPawn2::MovementComponentName);
	MovementComponent->UpdatedComponent = CollisionComponent;
	
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh;
		FConstructorStatics()
			: SphereMesh(TEXT("/Engine/EngineMeshes/Sphere")) {}
	};
	
	static FConstructorStatics ConstructorStatics;
	
	MeshComponent = CreateOptionalDefaultSubobject<UStaticMeshComponent>(ADefaultPawn2::MeshComponentName);
	if (MeshComponent)
	{
		MeshComponent->SetStaticMesh(ConstructorStatics.SphereMesh.Object);
		MeshComponent->AlwaysLoadOnClient = true;
		MeshComponent->AlwaysLoadOnServer = true;
		MeshComponent->bOwnerNoSee = true;
		MeshComponent->bCastDynamicShadow = true;
		MeshComponent->bAffectDynamicIndirectLighting = false;
		MeshComponent->bAffectDistanceFieldLighting = false;
		MeshComponent->bVisibleInRayTracing = false;
		MeshComponent->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		MeshComponent->SetupAttachment(RootComponent);
		MeshComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
		const float Scale = CollisionComponent->GetUnscaledSphereRadius() / 160.f; // @TODO: hardcoding known size of EngineMeshes.Sphere. Should use a unit sphere instead.
		MeshComponent->SetRelativeScale3D(FVector(Scale));
		MeshComponent->SetGenerateOverlapEvents(false);
		MeshComponent->SetCanEverAffectNavigation(false);
	}
	
	// This is the default pawn class, we want to have it be able to move out of the box.
	bAddDefaultMovementBindings = true;
	
	BaseTurnRate =45.f;
	BaseLookUpRate = 45.f;
	
}








void InitializeDefaultPawnInputBindings3()
{
	static bool bBindingsAdded = false;
	if (!bBindingsAdded)
	{
		bBindingsAdded = true;
		
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveForward", EKeys::W, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveForward", EKeys::S, -1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveForward", EKeys::Up, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveForward", EKeys::Down, -1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveForward", EKeys::Gamepad_LeftY, 1.f));

		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveRight", EKeys::A, -1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveRight", EKeys::D, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveRight", EKeys::Gamepad_LeftX, 1.f));

		// HACK: Android controller bindings in ini files seem to not work
		//  Direct overrides here some to work
#if !PLATFORM_ANDROID
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::Gamepad_LeftThumbstick, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::Gamepad_RightThumbstick, -1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::Gamepad_FaceButton_Bottom, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::LeftControl, -1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::SpaceBar, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::C, -1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::E, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::Q, -1.f));
#else
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::Gamepad_LeftTriggerAxis, -0.5f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::Gamepad_RightTriggerAxis, 0.5f));
#endif

		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_TurnRate", EKeys::Gamepad_RightX, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_TurnRate", EKeys::Left, -1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_TurnRate", EKeys::Right, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_Turn", EKeys::MouseX, 1.f));

		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_LookUpRate", EKeys::Gamepad_RightY, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_LookUp", EKeys::MouseY, -1.f));
	}
}



void ADefaultPawn2::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	if (bAddDefaultMovementBindings)
	{
		InitializeDefaultPawnInputBindings3();

		PlayerInputComponent->BindAxis("DefaultPawn_MoveForward", this, &ADefaultPawn2::MoveForward);
		PlayerInputComponent->BindAxis("DefaultPawn_MoveRight", this, &ADefaultPawn2::MoveRight);
		
		PlayerInputComponent->BindAxis("DefaultPawn_MoveUp", this, &ADefaultPawn2::MoveUp_World);


		PlayerInputComponent->BindAxis("DefaultPawn_Turn", this, &ADefaultPawn2::AddControllerYawInput);
		PlayerInputComponent->BindAxis("DefaultPawn_TurnRate", this, &ADefaultPawn2::TurnAtRate);

		PlayerInputComponent->BindAxis("DefaultPawn_LookUp", this, &ADefaultPawn2::AddControllerPitchInput);
		PlayerInputComponent->BindAxis("DefaultPawn_LookUpRate", this, &ADefaultPawn2::LookUpAtRate2);
	}

	PlayerInputComponent->BindAction("N8888888", IE_Pressed, this, &ADefaultPawn2::increase_speed);
	PlayerInputComponent->BindAction("K66666", IE_Pressed, this, &ADefaultPawn2::button_s_pressed_779);
}


void ADefaultPawn2::AddControllerPitchInput(float Val)
{
	// If not, should look around.
	if(should_look_around)
	{
		
	}else
	{
		return;
	}

	
	if (Val != 0.f && Controller && Controller->IsLocalPlayerController())
	{
		APlayerController* const PC = CastChecked<APlayerController>(Controller);
		PC->AddPitchInput(Val);
	}
}

void ADefaultPawn2::AddControllerYawInput(float Val)
{
	// If not, should look around.
	if(should_look_around)
	{
		
	}else
	{
		return;
	}

	if (Val != 0.f && Controller && Controller->IsLocalPlayerController())
	{
		APlayerController* const PC = CastChecked<APlayerController>(Controller);
		PC->AddYawInput(Val);
	}
}


void ADefaultPawn2::UpdateNavigationRelevance()
{
	if (CollisionComponent)
	{
		CollisionComponent->SetCanEverAffectNavigation(bCanAffectNavigationGeneration);
	}
}

void ADefaultPawn2::MoveRight(float Val)
{
	if (Val != 0.f)
	{
		if (Controller)
		{
			FRotator const ControlSpaceRot = Controller->GetControlRotation();

			// transform to world space and add it
			AddMovementInput( FRotationMatrix(ControlSpaceRot).GetScaledAxis( EAxis::Y ), Val );
		}
	}
}

void ADefaultPawn2::MoveForward(float Val)
{
	if (Val != 0.f)
	{
		if (Controller)
		{
			FRotator const ControlSpaceRot = Controller->GetControlRotation();


			// One is the fault I modified it here.
			// ll2("val111111111: " + FString::SanitizeFloat(Val),true,2);
			float Val2 = Val * speed_forward;
			
			// transform to world space and add it
			AddMovementInput(
				FRotationMatrix(ControlSpaceRot).GetScaledAxis( EAxis::X ),
				Val2
				);
		}
	}
}

void ADefaultPawn2::MoveUp_World(float Val)
{
	if (Val != 0.f)
	{
		AddMovementInput(FVector::UpVector, Val);
	}
}

void ADefaultPawn2::TurnAtRate(float Rate)
{

	if (1)
	{
		// calculate delta for this frame from the rate information
		AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
		// ll("TurnAtRate11"+FString::SanitizeFloat(Rate)
			// ,true,2);

	}
}

void ADefaultPawn2::LookUpAtRate2(float Rate)
{
	if (1)
	{
		// calculate delta for this frame from the rate information
		AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
		// ll("LookUpAtRate2"+FString::SanitizeFloat(Rate)
		// 	,true,2);
	}
	
}

void ADefaultPawn2::stop_looking_around881()
{
	should_look_around = false;
}

void ADefaultPawn2::resume_looking_around881()
{
	should_look_around = true;
}

UPawnMovementComponent* ADefaultPawn2::GetMovementComponent() const
{
	return MovementComponent;
}

void ADefaultPawn2::increase_speed()
{
	// ll("increase_speed"+FString::SanitizeFloat(speed_forward)
	// 	,true,2);
	speed_forward += 0.1f;
	
	ll("increase_speed"+FString::SanitizeFloat(speed_forward)
		,true,2);
}


