// Copyright Epic Games, Inc. All Rights Reserved.

#include "VRLerpTestCharacter.h"
#include "VRLerpTestProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"

#include "HeadMountedDisplayFunctionLibrary.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AVRLerpTestCharacter

AVRLerpTestCharacter::AVRLerpTestCharacter()
{
	// Character doesnt have a rifle at start
	bHasRifle = false;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	//FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = false;
	FirstPersonCameraComponent->bLockToHmd = false;//�����Ő��䂵�Ȃ��悤�ɂ���

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

}

void AVRLerpTestCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	//Tick��L���ɂ���
	PrimaryActorTick.bCanEverTick = true;
}

void AVRLerpTestCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FRotator vrOrientation;
	FVector vrPosition;

	// ���_���Z�b�g�t���O��true�Ȃ烊�Z�b�g���������s
	if (shouldResetViewFlug)
	{
		// HMD�̌��݂̉�]���擾
		UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(vrOrientation, vrPosition);

		// �n���ꂽ���Z�b�g�̉�]�l���g���āA���Z�b�g����
		m_initialOrientationOffset = FRotator(
			vrOrientation.Pitch - m_resetOrientation.Pitch,
			vrOrientation.Yaw - m_resetOrientation.Yaw,
			vrOrientation.Roll - m_resetOrientation.Roll
		);

		shouldResetViewFlug = false; // �t���O��false�ɂ���
	}

	// HMD�̌��݂̉�]�ƈʒu���擾
	UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(vrOrientation, vrPosition);

	// ��]�̕␳�iYaw�APitch�ARoll�����Z�b�g������Ԃ̃I�t�Z�b�g��K�p�j
	FRotator adjustedOrientation = vrOrientation - m_initialOrientationOffset;

	// ���݂̉�]���l�����ɕϊ�
	FQuat currentOrientation = adjustedOrientation.Quaternion();

	// �O�̃t���[���̉�]�ƌ��݂̉�]���Ԃ���
	FQuat smoothedOrientation = FQuat::Slerp(m_previousOrientation, currentOrientation, 0.1f);

	// ��Ԍ�̉�]���g�p
	FRotator smoothedRotator = smoothedOrientation.Rotator();

	// �J�����̉�]��ύX
	FirstPersonCameraComponent->SetWorldRotation(smoothedRotator);

	// ���݂̉�]�����̃t���[���p�ɕۑ�
	m_previousOrientation = smoothedOrientation;
}

void AVRLerpTestCharacter::TriggerViewReset(FRotator NewOrientation)
{
	m_resetOrientation = NewOrientation;
	shouldResetViewFlug = true; // �O������Ăяo���Ď��_���Z�b�g���g���K�[
}

//////////////////////////////////////////////////////////////////////////// Input

void AVRLerpTestCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AVRLerpTestCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AVRLerpTestCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AVRLerpTestCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AVRLerpTestCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AVRLerpTestCharacter::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool AVRLerpTestCharacter::GetHasRifle()
{
	return bHasRifle;
}