// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabber.h"

#define OUT

// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();

	FindPhysicsHandle();
	SetupInputComponent();
}

// Look for attached Physics Handle
void UGrabber::FindPhysicsHandle()
{
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (!PhysicsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("%s Missing Physics Handle Component."), *GetOwner()->GetName())
	}
}

// Look for attached Input Component and bind the actions
void UGrabber::SetupInputComponent()
{
	InputComponent = GetOwner()->FindComponentByClass<UInputComponent>();
	if (InputComponent)
	{
		InputComponent->BindAction("Grab", EInputEvent::IE_Pressed, this, &UGrabber::CheckForGrab);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s Missing Physics Handle Component."), *GetOwner()->GetName())
	}
}

// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PhysicsHandle) { return; }
	if (PhysicsHandle->GrabbedComponent)
	{
		FVector PlayerLocation;
		FRotator PlayerRotation;
		GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(OUT PlayerLocation, OUT PlayerRotation);

		FVector LineTraceStart = PlayerLocation;
		FVector LineTraceEnd = PlayerLocation + PlayerRotation.Vector() * GrabDistance;

		PhysicsHandle->SetTargetLocation(LineTraceEnd);
	}
}

// Try to grab, or release if already grabbing
void UGrabber::CheckForGrab()
{
	if (CurrentlyGrabbing)
	{
		Release();
	}
	else
	{
		Grab();
	}
}

// Try to grab
void UGrabber::Grab()
{
	FHitResult HitResult = GetFirstPhysicsBodyInReach();
	UPrimitiveComponent* ComponentToGrab = HitResult.GetComponent();

	if (HitResult.GetActor())
	{
		if (!PhysicsHandle) { return; }
		PhysicsHandle->GrabComponent(
			ComponentToGrab,
			NAME_None,
			ComponentToGrab->GetOwner()->GetActorLocation(),
			true // Allow rotation
		);
		CurrentlyGrabbing = true;
	}
}

// Release grabbed object
void UGrabber::Release()
{
	if (!PhysicsHandle) { return; }
	PhysicsHandle->ReleaseComponent();
	CurrentlyGrabbing = false;
}

FHitResult UGrabber::GetFirstPhysicsBodyInReach() const
{
	FVector PlayerLocation;
	FRotator PlayerRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(OUT PlayerLocation, OUT PlayerRotation);

	FVector LineTraceStart = PlayerLocation;
	FVector LineTraceEnd = PlayerLocation + PlayerRotation.Vector() * GrabDistance;

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByObjectType(
		OUT HitResult,
		LineTraceStart,
		LineTraceEnd,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody),
		FCollisionQueryParams(FName(TEXT("")), false, GetOwner())
	);

	return HitResult;
}
