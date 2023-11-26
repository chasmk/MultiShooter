// Copyright Epic Games, Inc. All Rights Reserved.

#include "MenuSystem1Character.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

//////////////////////////////////////////////////////////////////////////
// AMenuSystem1Character

AMenuSystem1Character::AMenuSystem1Character() :
    //��ʼ��ί�У����ѻص���������ȥ��AMenuSystemCharacter = ThisClass,ʹ�ø�����
    CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
    FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
    JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete))
{
    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    // set our turn rates for input
    BaseTurnRate = 45.f;
    BaseLookUpRate = 45.f;

    // Don't rotate when the controller rotates. Let that just affect the camera.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
    GetCharacterMovement()->JumpZVelocity = 600.f;
    GetCharacterMovement()->AirControl = 0.2f;

    // Create a camera boom (pulls in towards the player if there is a collision)
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
    CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

    // Create a follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
    FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

    // Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
    // are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

    //��ȡ������ϵͳ����
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem)
    {
        OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                15.f,
                FColor::Blue,
                FString::Printf(TEXT("Found subsystem %s  AppId: %s"), *OnlineSubsystem->GetSubsystemName().ToString(), *OnlineSubsystem->GetAppId())
            );
        }
    }

}

//////////////////////////////////////////////////////////////////////////
// Input

void AMenuSystem1Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    // Set up gameplay key bindings
    check(PlayerInputComponent);
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

    PlayerInputComponent->BindAxis("MoveForward", this, &AMenuSystem1Character::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AMenuSystem1Character::MoveRight);

    // We have 2 versions of the rotation bindings to handle different kinds of devices differently
    // "turn" handles devices that provide an absolute delta, such as a mouse.
    // "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("TurnRate", this, &AMenuSystem1Character::TurnAtRate);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("LookUpRate", this, &AMenuSystem1Character::LookUpAtRate);

    // handle touch devices
    PlayerInputComponent->BindTouch(IE_Pressed, this, &AMenuSystem1Character::TouchStarted);
    PlayerInputComponent->BindTouch(IE_Released, this, &AMenuSystem1Character::TouchStopped);

    // VR headset functionality
    PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AMenuSystem1Character::OnResetVR);
}

void AMenuSystem1Character::CreateGameSession()
{
    // Called when pressing the 1 key
    if (!OnlineSessionInterface.IsValid())// ���ù���ָ���Ƿ�Ϊ�� 
    {
        return;
    }
    //Ѱ���Ѿ������ĻỰ
    auto ExistingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
    if (ExistingSession != nullptr)//���У������٣��Ա�����������Ự
    {
        OnlineSessionInterface->DestroySession(NAME_GameSession);
    }
    //�����Ƕ����delegate��ӵ�OnlineSessionInterface��delegate list��
    OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
    //�����������»Ự
    TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
    SessionSettings->bIsLANMatch = false;//���޾�����
    SessionSettings->NumPublicConnections = 4;
    SessionSettings->bAllowJoinInProgress = true;
    SessionSettings->bAllowJoinViaPresence = true;
    SessionSettings->bShouldAdvertise = true;
    SessionSettings->bUsesPresence = true;
    SessionSettings->bUseLobbiesIfAvailable = true;// very important,fix  bug
    SessionSettings->Set(FName("MatchType"), FString("FreeForAlljyj"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    
    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    OnlineSessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings);
}

void AMenuSystem1Character::JoinGameSession()
{
    // Find game sessions
    if (!OnlineSessionInterface.IsValid())
    {
        return;
    }

    OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
    //����һ�������Ự�ı���ָ��
    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->MaxSearchResults = 10000;
    SessionSearch->bIsLanQuery = false;
    SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
    //ʹ�øñ��������Ự
    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    OnlineSessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
}

void AMenuSystem1Character::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    // ��ȡ��ǰʱ��
    FDateTime Now = FDateTime::Now();
    double Timestamp = Now.ToUnixTimestamp();
    FString DateTimeString = Now.ToString(TEXT("%Y-%m-%d %H:%M:%S"));
    if (bWasSuccessful)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                15.f,
                FColor::Blue,
                FString::Printf(TEXT("Created session: %s"), *SessionName.ToString())
            );
        }

        UWorld* World = GetWorld();
        if (World)
        {//ת��lobby�ؿ�����Ϊlisten server
            World->ServerTravel(FString("/Game/ThirdPersonCPP/Maps/Lobby?listen"));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                15.f,
                FColor::Red,
                FString(TEXT("Failed to create session!"))
            );
        }
    }
}

void AMenuSystem1Character::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (!OnlineSessionInterface.IsValid())
    {
        return;
    }
    for (auto Result : SessionSearch->SearchResults)
    {
        FString Id = Result.GetSessionIdStr();
        FString User = Result.Session.OwningUserName;
        FString MatchType;
        Result.Session.SessionSettings.Get(FName("MatchType"), MatchType);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                15.f,
                FColor::Cyan,
                FString::Printf(TEXT("Id: %s, User: %s"), *Id, *User)
            );
        }
        if (MatchType == FString("FreeForAlljyj"))
        {//�ҵ�������׼�������session
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(
                    -1,
                    15.f,
                    FColor::Green,
                    FString::Printf(TEXT("Join Match Type: %s"), *MatchType)
                );
            }
            //��delegate����ӿڣ�����join session��ͻᴥ��callback����
            OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
            
            const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
            OnlineSessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, Result);
        }
    }
}

void AMenuSystem1Character::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (!OnlineSessionInterface.IsValid())
    {
        return;
    }
    FString Address;
    if (OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, Address))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                15.f,
                FColor::Yellow ,
                FString::Printf(TEXT("Connect string: %s"), *Address)
            );
        }
        APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
        if (PlayerController)
        {
            PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
        }
    }
}


void AMenuSystem1Character::OnResetVR()
{
    // If MenuSystem1 is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in MenuSystem1.Build.cs is not automatically propagated
    // and a linker error will result.
    // You will need to either:
    //		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
    // or:
    //		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
    UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AMenuSystem1Character::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
        Jump();
}

void AMenuSystem1Character::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
        StopJumping();
}

void AMenuSystem1Character::TurnAtRate(float Rate)
{
    // calculate delta for this frame from the rate information
    AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMenuSystem1Character::LookUpAtRate(float Rate)
{
    // calculate delta for this frame from the rate information
    AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMenuSystem1Character::MoveForward(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        // find out which way is forward
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        // get forward vector
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void AMenuSystem1Character::MoveRight(float Value)
{
    if ( (Controller != nullptr) && (Value != 0.0f) )
    {
        // find out which way is right
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
    
        // get right vector 
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        // add movement in that direction
        AddMovementInput(Direction, Value);
    }
}
