// #include "GameAudioDirectorComponent.h"
//
// #include "Kismet/GameplayStatics.h"
// #include "GameFramework/PlayerController.h"
// #include "Camera/PlayerCameraManager.h"
//
// UGameAudioDirectorComponent::UGameAudioDirectorComponent()
// {
//     PrimaryComponentTick.bCanEverTick = true;
// }
//
// void UGameAudioDirectorComponent::BeginPlay()
// {
//     Super::BeginPlay();
//
//     USoundCue* InitialCue = GetAmbienceCueForState(CurrentAmbienceState);
//     if (InitialCue)
//     {
//         AmbienceAC = UGameplayStatics::SpawnSound2D(
//             this,
//             InitialCue,
//             0.0f,
//             1.0f,
//             0.0f,
//             nullptr,
//             true,
//             false
//         );
//     }
//
//     AmbienceTargetVolume = 1.0f;
//     AmbienceFadeSpeed    = 1.0f;
// }
//
// void UGameAudioDirectorComponent::TickComponent(
//     float DeltaTime,
//     ELevelTick TickType,
//     FActorComponentTickFunction* ThisTickFunction)
// {
//     Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//
//     UpdateAmbience(DeltaTime);
// }
//
// // ========= AMBIENCE STATE =========
//
// USoundCue* UGameAudioDirectorComponent::GetAmbienceCueForState(EAudioAmbienceState State) const
// {
//     switch (State)
//     {
//     case EAudioAmbienceState::Idle:        return IdleAmbienceCue;
//     case EAudioAmbienceState::Exploration: return ExplorationAmbienceCue;
//     case EAudioAmbienceState::Suspense:    return SuspenseAmbienceCue;
//     case EAudioAmbienceState::Chase:       return ChaseAmbienceCue;
//     case EAudioAmbienceState::SafeRoom:    return SafeRoomAmbienceCue;
//     default:                               return nullptr;
//     }
// }
//
// void UGameAudioDirectorComponent::SetAmbienceState(EAudioAmbienceState NewState, float FadeTime)
// {
//     if (NewState == CurrentAmbienceState && AmbienceAC)
//     {
//         return;
//     }
//
//     CurrentAmbienceState = NewState;
//     USoundCue* NewCue    = GetAmbienceCueForState(NewState);
//
//     UWorld* World = GetWorld();
//     if (!World)
//     {
//         return;
//     }
//
//     if (!NewCue)
//     {
//         AmbienceTargetVolume = 0.0f;
//         AmbienceFadeSpeed    = (FadeTime > 0.01f) ? (1.0f / FadeTime) : 1000.0f;
//         return;
//     }
//
//     if (AmbienceAC && AmbienceAC->Sound != NewCue)
//     {
//         AmbienceAC->Stop();
//         AmbienceAC = nullptr;
//     }
//
//     if (!AmbienceAC)
//     {
//         AmbienceAC = UGameplayStatics::SpawnSound2D(
//             this,
//             NewCue,
//             0.0f,
//             1.0f,
//             0.0f,
//             nullptr,
//             true,
//             false
//         );
//     }
//
//     AmbienceTargetVolume = 1.0f;
//     AmbienceFadeSpeed    = (FadeTime > 0.01f) ? (1.0f / FadeTime) : 1000.0f;
// }
//
// void UGameAudioDirectorComponent::UpdateAmbience(float DeltaTime)
// {
//     if (!AmbienceAC)
//     {
//         return;
//     }
//
//     float StateScale = 1.0f;
//     if (const float* Found = AmbienceStateVolumeScale.Find(CurrentAmbienceState))
//     {
//         StateScale = *Found;
//     }
//
//     const float Desired = AmbienceTargetVolume * MasterVolume * StateScale;
//     const float Current = AmbienceAC->VolumeMultiplier;
//
//     if (FMath::IsNearlyEqual(Current, Desired, 0.001f))
//     {
//         AmbienceAC->SetVolumeMultiplier(Desired);
//         return;
//     }
//
//     const float NewVol = FMath::FInterpTo(
//         Current,
//         Desired,
//         DeltaTime,
//         AmbienceFadeSpeed
//     );
//
//     AmbienceAC->SetVolumeMultiplier(NewVol);
// }
//
// // ========= VOLUME =========
//
// void UGameAudioDirectorComponent::SetMasterVolume(float NewVolume)
// {
//     MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
// }
//
// void UGameAudioDirectorComponent::SetSFXVolume(float NewVolume)
// {
//     SFXVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
// }
//
// void UGameAudioDirectorComponent::SetUIVolume(float NewVolume)
// {
//     UIVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
// }
//
// void UGameAudioDirectorComponent::SetPlayerSFXVolume(float NewVolume)
// {
//     PlayerSFXVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
// }
//
// void UGameAudioDirectorComponent::SetEnemySFXVolume(float NewVolume)
// {
//     EnemySFXVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
// }
//
// void UGameAudioDirectorComponent::SetEnvironmentSFXVolume(float NewVolume)
// {
//     EnvironmentSFXVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
// }
//
// float UGameAudioDirectorComponent::GetCategoryVolume(EAudioSFXCategory Category) const
// {
//     switch (Category)
//     {
//     case EAudioSFXCategory::Player:      return PlayerSFXVolume;
//     case EAudioSFXCategory::Enemy:       return EnemySFXVolume;
//     case EAudioSFXCategory::Environment: return EnvironmentSFXVolume;
//     case EAudioSFXCategory::General:
//     default:                             return 1.0f;
//     }
// }
//
// // ========= SFX / UI =========
//
// void UGameAudioDirectorComponent::PlaySound2D(USoundBase* Sound, float Volume, float Pitch)
// {
//     if (!Sound) return;
//     const float FinalVolume = Volume * MasterVolume * SFXVolume;
//     UGameplayStatics::PlaySound2D(this, Sound, FinalVolume, Pitch);
// }
//
// void UGameAudioDirectorComponent::PlaySoundAtLocation(USoundBase* Sound, FVector Location,
//                                                       float Volume, float Pitch)
// {
//     if (!Sound) return;
//     UWorld* World = GetWorld();
//     if (!World) return;
//
//     const float FinalVolume = Volume * MasterVolume * SFXVolume;
//     UGameplayStatics::PlaySoundAtLocation(World, Sound, Location, FinalVolume, Pitch);
// }
//
// void UGameAudioDirectorComponent::PlayCategorizedSound2D(USoundBase* Sound, EAudioSFXCategory Category,
//                                                          float Volume, float Pitch)
// {
//     if (!Sound) return;
//     const float CatVol      = GetCategoryVolume(Category);
//     const float FinalVolume = Volume * MasterVolume * SFXVolume * CatVol;
//     UGameplayStatics::PlaySound2D(this, Sound, FinalVolume, Pitch);
// }
//
// void UGameAudioDirectorComponent::PlayCategorizedSoundAtLocation(USoundBase* Sound, EAudioSFXCategory Category,
//                                                                  FVector Location, float Volume, float Pitch)
// {
//     if (!Sound) return;
//     UWorld* World = GetWorld();
//     if (!World) return;
//
//     const float CatVol      = GetCategoryVolume(Category);
//     const float FinalVolume = Volume * MasterVolume * SFXVolume * CatVol;
//     UGameplayStatics::PlaySoundAtLocation(World, Sound, Location, FinalVolume, Pitch);
// }
//
// void UGameAudioDirectorComponent::PlaySoundRelativeToPlayer(USoundBase* Sound, FVector Offset,
//                                                             float Volume, float Pitch)
// {
//     if (!Sound) return;
//     UWorld* World = GetWorld();
//     if (!World) return;
//
//     APlayerController* PC = World->GetFirstPlayerController();
//     if (!PC) return;
//
//     APlayerCameraManager* CamMgr = PC->PlayerCameraManager;
//     if (!CamMgr) return;
//
//     const FVector CamLocation  = CamMgr->GetCameraLocation();
//     const FRotator CamRotation = CamMgr->GetCameraRotation();
//     const FVector WorldOffset  = CamRotation.RotateVector(Offset);
//     const FVector SpawnLocation = CamLocation + WorldOffset;
//
//     const float FinalVolume = Volume * MasterVolume * SFXVolume;
//     UGameplayStatics::PlaySoundAtLocation(World, Sound, SpawnLocation, FinalVolume, Pitch);
// }
//
// void UGameAudioDirectorComponent::PlayUISound(USoundBase* Sound, float Volume, float Pitch)
// {
//     if (!Sound) return;
//     const float FinalVolume = Volume * MasterVolume * UIVolume;
//     UGameplayStatics::PlaySound2D(this, Sound, FinalVolume, Pitch);
// }
//
// void UGameAudioDirectorComponent::PlayJumpscare(USoundBase* Stinger,
//                                                 float StingerVolume,
//                                                 float StingerPitch,
//                                                 float AmbienceDuck,
//                                                 float FadeBackTime)
// {
//     if (!Stinger)
//         return;
//
//     AmbienceDuck = FMath::Clamp(AmbienceDuck, 0.0f, 1.0f);
//
//     if (AmbienceAC)
//     {
//         AmbienceAC->SetVolumeMultiplier(MasterVolume * AmbienceDuck);
//         AmbienceTargetVolume = 1.0f;
//         AmbienceFadeSpeed    = (FadeBackTime > 0.01f) ? (1.0f / FadeBackTime) : 1000.0f;
//     }
//
//     PlaySoundRelativeToPlayer(Stinger, FVector(100.0f, 0.0f, 0.0f),
//                               StingerVolume, StingerPitch);
// }
//
// // ========= LOOPING SFX =========
//
// void UGameAudioDirectorComponent::StartLoopingSFX2D(USoundBase* Sound, float Volume, float Pitch)
// {
//     if (!Sound) return;
//
//     StopLoopingSFX2D();
//
//     const float FinalVolume = Volume * MasterVolume * SFXVolume;
//
//     LoopingSFX2D = UGameplayStatics::SpawnSound2D(
//         this,
//         Sound,
//         FinalVolume,
//         Pitch,
//         0.0f,
//         nullptr,
//         true,
//         false
//     );
// }
//
// void UGameAudioDirectorComponent::StopLoopingSFX2D()
// {
//     if (LoopingSFX2D)
//     {
//         LoopingSFX2D->Stop();
//         LoopingSFX2D = nullptr;
//     }
// }
//
// void UGameAudioDirectorComponent::StartLoopingSFXAtLocation(USoundBase* Sound, FVector Location,
//                                                             float Volume, float Pitch)
// {
//     if (!Sound) return;
//
//     StopLoopingSFX3D();
//
//     UWorld* World = GetWorld();
//     if (!World) return;
//
//     const float FinalVolume = Volume * MasterVolume * SFXVolume;
//
//     LoopingSFX3D = UGameplayStatics::SpawnSoundAtLocation(
//         World,
//         Sound,
//         Location,
//         FRotator::ZeroRotator,
//         FinalVolume,
//         Pitch,
//         0.0f,
//         nullptr,
//         nullptr,
//         true
//     );
// }
//
// void UGameAudioDirectorComponent::StopLoopingSFX3D()
// {
//     if (LoopingSFX3D)
//     {
//         LoopingSFX3D->Stop();
//         LoopingSFX3D = nullptr;
//     }
// }
//
// // ========= FOOTSTEPS =========
//
// USoundBase* UGameAudioDirectorComponent::ChooseRandomFromArray(const TArray<USoundBase*>& Array) const
// {
//     if (Array.Num() == 0)
//         return nullptr;
//
//     const int32 Index = FMath::RandRange(0, Array.Num() - 1);
//     return Array[Index];
// }
//
// void UGameAudioDirectorComponent::PlayFootstepAtLocation(FVector Location,
//                                                          EFootstepSurface Surface,
//                                                          float Volume,
//                                                          float Pitch)
// {
//     UWorld* World = GetWorld();
//     if (!World)
//         return;
//
//     const FFootstepSoundList* ListPtr = Footsteps_BySurface.Find(Surface);
//     if (!ListPtr)
//     {
//         ListPtr = Footsteps_BySurface.Find(EFootstepSurface::Default);
//     }
//     if (!ListPtr)
//         return;
//
//     USoundBase* Chosen = ChooseRandomFromArray(ListPtr->Sounds);
//     if (!Chosen)
//         return;
//
//     const float FinalVolume = Volume * MasterVolume * SFXVolume;
//     UGameplayStatics::PlaySoundAtLocation(World, Chosen, Location, FinalVolume, Pitch);
// }
#include "GameAudioDirectorComponent.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

UGameAudioDirectorComponent::UGameAudioDirectorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentAmbienceState      = EAudioAmbienceState::Idle;
    CurrentAmbienceCustomName = NAME_None;
}

void UGameAudioDirectorComponent::BeginPlay()
{
    Super::BeginPlay();

    USoundCue* InitialCue = nullptr;

    // Prefer custom state if set, else enum default
    if (!CurrentAmbienceCustomName.IsNone())
    {
        InitialCue = GetCustomAmbienceCue(CurrentAmbienceCustomName);
    }
    if (!InitialCue)
    {
        InitialCue = GetDefaultAmbienceCue(CurrentAmbienceState);
    }

    if (InitialCue)
    {
        AmbienceAC = UGameplayStatics::SpawnSound2D(
            this,
            InitialCue,
            0.0f, // start silent, fade in via UpdateAmbience
            1.0f,
            0.0f,
            nullptr,
            true,
            false
        );
    }

    AmbienceTargetVolume = 1.0f;
    AmbienceFadeSpeed    = 1.0f;
}

void UGameAudioDirectorComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateAmbience(DeltaTime);
}

// ========= AMBIENCE HELPERS =========

USoundCue* UGameAudioDirectorComponent::GetDefaultAmbienceCue(EAudioAmbienceState State) const
{
    if (USoundCue** Found = const_cast<TMap<EAudioAmbienceState, USoundCue*>&>(DefaultAmbienceCues).Find(State))
    {
        return *Found;
    }
    return nullptr;
}

float UGameAudioDirectorComponent::GetDefaultAmbienceVolumeScale(EAudioAmbienceState State) const
{
    if (float* Found = const_cast<TMap<EAudioAmbienceState, float>&>(DefaultAmbienceVolumeScale).Find(State))
    {
        return *Found;
    }
    return 1.0f;
}

USoundCue* UGameAudioDirectorComponent::GetCustomAmbienceCue(FName StateName) const
{
    if (StateName.IsNone())
    {
        return nullptr;
    }

    for (const FCustomAmbienceEntry& Entry : CustomAmbienceEntries)
    {
        if (Entry.StateName == StateName)
        {
            return Entry.Cue;
        }
    }
    return nullptr;
}

float UGameAudioDirectorComponent::GetCustomAmbienceVolumeScale(FName StateName) const
{
    if (StateName.IsNone())
    {
        return 1.0f;
    }

    for (const FCustomAmbienceEntry& Entry : CustomAmbienceEntries)
    {
        if (Entry.StateName == StateName)
        {
            return Entry.VolumeScale;
        }
    }
    return 1.0f;
}

void UGameAudioDirectorComponent::AddOrUpdateCustomAmbienceEntry(const FCustomAmbienceEntry& Entry)
{
    for (FCustomAmbienceEntry& Existing : CustomAmbienceEntries)
    {
        if (Existing.StateName == Entry.StateName)
        {
            Existing = Entry;
            return;
        }
    }
    CustomAmbienceEntries.Add(Entry);
}

void UGameAudioDirectorComponent::RemoveCustomAmbienceEntry(FName StateName)
{
    CustomAmbienceEntries.RemoveAll([StateName](const FCustomAmbienceEntry& Entry)
    {
        return Entry.StateName == StateName;
    });
}

// ========= AMBIENCE SETTERS =========

void UGameAudioDirectorComponent::SetAmbienceState(EAudioAmbienceState NewState, float FadeTime)
{
    if (NewState == CurrentAmbienceState && CurrentAmbienceCustomName.IsNone() && AmbienceAC)
    {
        return;
    }

    CurrentAmbienceState      = NewState;
    CurrentAmbienceCustomName = NAME_None; // using enum now

    USoundCue* NewCue = GetDefaultAmbienceCue(NewState);

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    if (!NewCue)
    {
        AmbienceTargetVolume = 0.0f;
        AmbienceFadeSpeed    = (FadeTime > 0.01f) ? (1.0f / FadeTime) : 1000.0f;
        return;
    }

    if (AmbienceAC && AmbienceAC->Sound != NewCue)
    {
        AmbienceAC->Stop();
        AmbienceAC = nullptr;
    }

    if (!AmbienceAC)
    {
        AmbienceAC = UGameplayStatics::SpawnSound2D(
            this,
            NewCue,
            0.0f,
            1.0f,
            0.0f,
            nullptr,
            true,
            false
        );
    }

    AmbienceTargetVolume = 1.0f;
    AmbienceFadeSpeed    = (FadeTime > 0.01f) ? (1.0f / FadeTime) : 1000.0f;
}

void UGameAudioDirectorComponent::SetAmbienceStateByName(FName NewStateName, float FadeTime)
{
    if (NewStateName == CurrentAmbienceCustomName && AmbienceAC)
    {
        return;
    }

    CurrentAmbienceCustomName = NewStateName;
    // Optionally leave enum as-is (represents last default state)
    // CurrentAmbienceState = EAudioAmbienceState::Idle;

    USoundCue* NewCue = GetCustomAmbienceCue(NewStateName);

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    if (!NewCue)
    {
        AmbienceTargetVolume = 0.0f;
        AmbienceFadeSpeed    = (FadeTime > 0.01f) ? (1.0f / FadeTime) : 1000.0f;
        return;
    }

    if (AmbienceAC && AmbienceAC->Sound != NewCue)
    {
        AmbienceAC->Stop();
        AmbienceAC = nullptr;
    }

    if (!AmbienceAC)
    {
        AmbienceAC = UGameplayStatics::SpawnSound2D(
            this,
            NewCue,
            0.0f,
            1.0f,
            0.0f,
            nullptr,
            true,
            false
        );
    }

    AmbienceTargetVolume = 1.0f;
    AmbienceFadeSpeed    = (FadeTime > 0.01f) ? (1.0f / FadeTime) : 1000.0f;
}

void UGameAudioDirectorComponent::UpdateAmbience(float DeltaTime)
{
    if (!AmbienceAC)
    {
        return;
    }

    float StateScale = 1.0f;

    if (!CurrentAmbienceCustomName.IsNone())
    {
        StateScale = GetCustomAmbienceVolumeScale(CurrentAmbienceCustomName);
    }
    else
    {
        StateScale = GetDefaultAmbienceVolumeScale(CurrentAmbienceState);
    }

    const float Desired = AmbienceTargetVolume * MasterVolume * StateScale;
    const float Current = AmbienceAC->VolumeMultiplier;

    if (FMath::IsNearlyEqual(Current, Desired, 0.001f))
    {
        AmbienceAC->SetVolumeMultiplier(Desired);
        return;
    }

    const float NewVol = FMath::FInterpTo(
        Current,
        Desired,
        DeltaTime,
        AmbienceFadeSpeed
    );

    AmbienceAC->SetVolumeMultiplier(NewVol);
}

// ========= VOLUME =========

void UGameAudioDirectorComponent::SetMasterVolume(float NewVolume)
{
    MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
}

void UGameAudioDirectorComponent::SetSFXVolume(float NewVolume)
{
    SFXVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
}

void UGameAudioDirectorComponent::SetUIVolume(float NewVolume)
{
    UIVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
}

void UGameAudioDirectorComponent::SetPlayerSFXVolume(float NewVolume)
{
    PlayerSFXVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
}

void UGameAudioDirectorComponent::SetEnemySFXVolume(float NewVolume)
{
    EnemySFXVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
}

void UGameAudioDirectorComponent::SetEnvironmentSFXVolume(float NewVolume)
{
    EnvironmentSFXVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
}

float UGameAudioDirectorComponent::GetCategoryVolume(EAudioSFXCategory Category) const
{
    switch (Category)
    {
    case EAudioSFXCategory::Player:      return PlayerSFXVolume;
    case EAudioSFXCategory::Enemy:       return EnemySFXVolume;
    case EAudioSFXCategory::Environment: return EnvironmentSFXVolume;
    case EAudioSFXCategory::General:
    default:                             return 1.0f;
    }
}

// ========= SFX / UI =========

void UGameAudioDirectorComponent::PlaySound2D(USoundBase* Sound, float Volume, float Pitch)
{
    if (!Sound) return;
    const float FinalVolume = Volume * MasterVolume * SFXVolume;
    UGameplayStatics::PlaySound2D(this, Sound, FinalVolume, Pitch);
}

void UGameAudioDirectorComponent::PlaySoundAtLocation(USoundBase* Sound, FVector Location,
                                                      float Volume, float Pitch)
{
    if (!Sound) return;
    UWorld* World = GetWorld();
    if (!World) return;

    const float FinalVolume = Volume * MasterVolume * SFXVolume;
    UGameplayStatics::PlaySoundAtLocation(World, Sound, Location, FinalVolume, Pitch);
}

void UGameAudioDirectorComponent::PlayCategorizedSound2D(USoundBase* Sound, EAudioSFXCategory Category,
                                                         float Volume, float Pitch)
{
    if (!Sound) return;
    const float CatVol      = GetCategoryVolume(Category);
    const float FinalVolume = Volume * MasterVolume * SFXVolume * CatVol;
    UGameplayStatics::PlaySound2D(this, Sound, FinalVolume, Pitch);
}

void UGameAudioDirectorComponent::PlayCategorizedSoundAtLocation(USoundBase* Sound, EAudioSFXCategory Category,
                                                                 FVector Location, float Volume, float Pitch)
{
    if (!Sound) return;
    UWorld* World = GetWorld();
    if (!World) return;

    const float CatVol      = GetCategoryVolume(Category);
    const float FinalVolume = Volume * MasterVolume * SFXVolume * CatVol;
    UGameplayStatics::PlaySoundAtLocation(World, Sound, Location, FinalVolume, Pitch);
}

void UGameAudioDirectorComponent::PlaySoundRelativeToPlayer(USoundBase* Sound, FVector Offset,
                                                            float Volume, float Pitch)
{
    if (!Sound) return;
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APlayerCameraManager* CamMgr = PC->PlayerCameraManager;
    if (!CamMgr) return;

    const FVector CamLocation  = CamMgr->GetCameraLocation();
    const FRotator CamRotation = CamMgr->GetCameraRotation();
    const FVector WorldOffset  = CamRotation.RotateVector(Offset);
    const FVector SpawnLocation = CamLocation + WorldOffset;

    const float FinalVolume = Volume * MasterVolume * SFXVolume;
    UGameplayStatics::PlaySoundAtLocation(World, Sound, SpawnLocation, FinalVolume, Pitch);
}

void UGameAudioDirectorComponent::PlayUISound(USoundBase* Sound, float Volume, float Pitch)
{
    if (!Sound) return;
    const float FinalVolume = Volume * MasterVolume * UIVolume;
    UGameplayStatics::PlaySound2D(this, Sound, FinalVolume, Pitch);
}

void UGameAudioDirectorComponent::PlayJumpscare(USoundBase* Stinger,
                                                float StingerVolume,
                                                float StingerPitch,
                                                float AmbienceDuck,
                                                float FadeBackTime)
{
    if (!Stinger)
        return;

    AmbienceDuck = FMath::Clamp(AmbienceDuck, 0.0f, 1.0f);

    if (AmbienceAC)
    {
        AmbienceAC->SetVolumeMultiplier(MasterVolume * AmbienceDuck);
        AmbienceTargetVolume = 1.0f;
        AmbienceFadeSpeed    = (FadeBackTime > 0.01f) ? (1.0f / FadeBackTime) : 1000.0f;
    }

    PlaySoundRelativeToPlayer(Stinger, FVector(100.0f, 0.0f, 0.0f),
                              StingerVolume, StingerPitch);
}

// ========= LOOPING SFX =========

void UGameAudioDirectorComponent::StartLoopingSFX2D(USoundBase* Sound, float Volume, float Pitch)
{
    if (!Sound) return;

    StopLoopingSFX2D();

    const float FinalVolume = Volume * MasterVolume * SFXVolume;

    LoopingSFX2D = UGameplayStatics::SpawnSound2D(
        this,
        Sound,
        FinalVolume,
        Pitch,
        0.0f,
        nullptr,
        true,
        false
    );
}

void UGameAudioDirectorComponent::StopLoopingSFX2D()
{
    if (LoopingSFX2D)
    {
        LoopingSFX2D->Stop();
        LoopingSFX2D = nullptr;
    }
}

void UGameAudioDirectorComponent::StartLoopingSFXAtLocation(USoundBase* Sound, FVector Location,
                                                            float Volume, float Pitch)
{
    if (!Sound) return;

    StopLoopingSFX3D();

    UWorld* World = GetWorld();
    if (!World) return;

    const float FinalVolume = Volume * MasterVolume * SFXVolume;

    LoopingSFX3D = UGameplayStatics::SpawnSoundAtLocation(
        World,
        Sound,
        Location,
        FRotator::ZeroRotator,
        FinalVolume,
        Pitch,
        0.0f,
        nullptr,
        nullptr,
        true
    );
}

void UGameAudioDirectorComponent::StopLoopingSFX3D()
{
    if (LoopingSFX3D)
    {
        LoopingSFX3D->Stop();
        LoopingSFX3D = nullptr;
    }
}

// ========= FOOTSTEPS =========

USoundBase* UGameAudioDirectorComponent::ChooseRandomFromArray(const TArray<USoundBase*>& Array) const
{
    if (Array.Num() == 0)
        return nullptr;

    const int32 Index = FMath::RandRange(0, Array.Num() - 1);
    return Array[Index];
}

void UGameAudioDirectorComponent::PlayFootstepAtLocation(FVector Location,
                                                         EFootstepSurface Surface,
                                                         float Volume,
                                                         float Pitch)
{
    UWorld* World = GetWorld();
    if (!World)
        return;

    const FFootstepSoundList* ListPtr = Footsteps_BySurface.Find(Surface);
    if (!ListPtr)
    {
        ListPtr = Footsteps_BySurface.Find(EFootstepSurface::Default);
    }
    if (!ListPtr)
        return;

    USoundBase* Chosen = ChooseRandomFromArray(ListPtr->Sounds);
    if (!Chosen)
        return;

    const float FinalVolume = Volume * MasterVolume * SFXVolume;
    UGameplayStatics::PlaySoundAtLocation(World, Chosen, Location, FinalVolume, Pitch);
}
