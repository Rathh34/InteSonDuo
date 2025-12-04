// #pragma once
//
// #include "CoreMinimal.h"
// #include "Components/ActorComponent.h"
// #include "Sound/SoundCue.h"
// #include "Sound/SoundBase.h"
// #include "Components/AudioComponent.h"
// #include "GameAudioDirectorComponent.generated.h"
//
// UENUM(BlueprintType)
// enum class EFootstepSurface : uint8
// {
//     Default UMETA(DisplayName="Default"),
//     Wood    UMETA(DisplayName="Wood"),
//     Stone   UMETA(DisplayName="Stone"),
//     Metal   UMETA(DisplayName="Metal"),
//     Carpet  UMETA(DisplayName="Carpet")
// };
//
// USTRUCT(BlueprintType)
// struct FFootstepSoundList
// {
//     GENERATED_BODY()
//
//     /** All sounds that can be randomly picked for this surface. */
//     UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Footsteps",
//               meta=(ToolTip="List of possible footstep sounds for a given surface."))
//     TArray<USoundBase*> Sounds;
// };
//
// UENUM(BlueprintType)
// enum class EAudioAmbienceState : uint8
// {
//     Idle        UMETA(DisplayName="Idle"),
//     Exploration UMETA(DisplayName="Exploration"),
//     Suspense    UMETA(DisplayName="Suspense"),
//     Chase       UMETA(DisplayName="Chase"),
//     SafeRoom    UMETA(DisplayName="Safe Room")
// };
//
// UENUM(BlueprintType)
// enum class EAudioSFXCategory : uint8
// {
//     General     UMETA(DisplayName="General"),
//     Player      UMETA(DisplayName="Player"),
//     Enemy       UMETA(DisplayName="Enemy"),
//     Environment UMETA(DisplayName="Environment")
// };
//
// /**
//  * Game audio director: one ambience SoundCue per state, plus SFX/UI/footsteps/jumpscares/looping.
//  * Attach this to the Player (or a central Audio Manager actor) and drive it from Blueprints only.
//  */
// UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent), Blueprintable)
// class GAMEAUDIODIRECTOR_API UGameAudioDirectorComponent : public UActorComponent
// {
//     GENERATED_BODY()
//
// public:
//     UGameAudioDirectorComponent();
//
//     virtual void BeginPlay() override;
//     virtual void TickComponent(
//         float DeltaTime,
//         ELevelTick TickType,
//         FActorComponentTickFunction* ThisTickFunction
//     ) override;
//
//     // ========= AMBIENCE STATE =========
//
//     /** Set high-level ambience state (Idle / Exploration / Suspense / Chase / SafeRoom). */
//     UFUNCTION(BlueprintCallable, Category="Audio Director|Ambience",
//               meta=(ToolTip="Set the current ambience state (Idle, Exploration, Suspense, Chase, SafeRoom) and crossfade to its SoundCue."))
//     void SetAmbienceState(EAudioAmbienceState NewState,
//                           float FadeTime = 1.0f);
//
//     /** Get the current ambience state. */
//     UFUNCTION(BlueprintPure, Category="Audio Director|Ambience",
//               meta=(ToolTip="Return the current ambience state."))
//     EAudioAmbienceState GetAmbienceState() const { return CurrentAmbienceState; }
//
//     // ========= VOLUME =========
//
//     /** Set overall master volume for all sounds (0 = mute, 1 = full). */
//     UFUNCTION(BlueprintCallable, Category="Audio Director|Volume",
//               meta=(ToolTip="Set overall master volume for ALL sounds. Values are clamped 0-1."))
//     void SetMasterVolume(float NewVolume);
//
//     /** Set main SFX volume (world and gameplay sounds). */
//     UFUNCTION(BlueprintCallable, Category="Audio Director|Volume",
//               meta=(ToolTip="Set main SFX volume for all non-UI sounds. Values are clamped 0-1."))
//     void SetSFXVolume(float NewVolume);
//
//     /** Set UI SFX volume (menus, buttons, HUD). */
//     UFUNCTION(BlueprintCallable, Category="Audio Director|Volume",
//               meta=(ToolTip="Set UI volume used by PlayUISound. Values are clamped 0-1."))
//     void SetUIVolume(float NewVolume);
//
//     /** Set extra volume multiplier for player-related SFX. */
//     UFUNCTION(BlueprintCallable, Category="Audio Director|Volume",
//               meta=(ToolTip="Set extra volume multiplier for Player SFX category."))
//     void SetPlayerSFXVolume(float NewVolume);
//
//     /** Set extra volume multiplier for enemy-related SFX. */
//     UFUNCTION(BlueprintCallable, Category="Audio Director|Volume",
//               meta=(ToolTip="Set extra volume multiplier for Enemy SFX category."))
//     void SetEnemySFXVolume(float NewVolume);
//
//     /** Set extra volume multiplier for environment SFX. */
//     UFUNCTION(BlueprintCallable, Category="Audio Director|Volume",
//               meta=(ToolTip="Set extra volume multiplier for Environment SFX category."))
//     void SetEnvironmentSFXVolume(float NewVolume);
//
//     // ========= SFX / UI =========
//
//     /** Play a non-spatial 2D sound (global SFX, HUD whooshes, etc.). */
//     UFUNCTION(BlueprintCallable, Category="Audio Director|Play 2D",
//               meta=(ToolTip="Play a non-spatial (2D) sound. Final volume = Volume * Master * SFX."))
//     void PlaySound2D(USoundBase* Sound,
//                      float Volume = 1.0f,
//                      float Pitch = 1.0f);
//
//     /** Play a 3D sound at a world location (doors, impacts, distant sounds). */
//     UFUNCTION(BlueprintCallable, Category="Audio Director|Play 3D",
//               meta=(ToolTip="Play a 3D sound at a specific world location. Final volume = Volume * Master * SFX."))
//     void PlaySoundAtLocation(USoundBase* Sound,
//                              FVector Location,
//                              float Volume = 1.0f,
//                              float Pitch = 1.0f);
//
//     /** Play a 2D sound using a specific SFX category (Player, Enemy, Environment). */
//     UFUNCTION(BlueprintCallable, Category="Audio Director|Play 2D",
//               meta=(ToolTip="Play a 2D sound with category mixing (Player/Enemy/Environment). Final volume = Volume * Master * SFX * CategoryVolume."))
//     void PlayCategorizedSound2D(USoundBase* Sound,
//                                 EAudioSFXCategory Category,
//                                 float Volume = 1.0f,
//                                 float Pitch = 1.0f);
//
//     /** Play a 3D sound at a location using a specific SFX category. */
//     UFUNCTION(BlueprintCallable, Category="Audio Director|Play 3D",
//               meta=(ToolTip="Play a 3D sound with category mixing at a location. Final volume = Volume * Master * SFX * CategoryVolume."))
//     void PlayCategorizedSoundAtLocation(USoundBase* Sound,
//                                         EAudioSFXCategory Category,
//                                         FVector Location,
//                                         float Volume = 1.0f,
//                                         float Pitch = 1.0f);
//
//     /**
//      * Play a 3D sound relative to the player camera.
//      * Offset is in camera local space (X=forward, Y=right, Z=up).
//      */
//     UFUNCTION(BlueprintCallable, Category="Audio Director|Play Relative To Player",
//               meta=(ToolTip="Play a 3D sound relative to the player camera. Offset is in camera local space (X=forward, Y=right, Z=up)."))
//     void PlaySoundRelativeToPlayer(USoundBase* Sound,
//                                    FVector Offset,
//                                    float Volume = 1.0f,
//                                    float Pitch = 1.0f);
//
//     /** Play a 2D UI sound (buttons, menus, notifications). */
//     UFUNCTION(BlueprintCallable, Category="Audio Director|Play UI",
//               meta=(ToolTip="Play a non-spatial UI sound. Final volume = Volume * Master * UI."))
//     void PlayUISound(USoundBase* Sound,
//                      float Volume = 1.0f,
//                      float Pitch = 1.0f);
//
//     /**
//      * Play a jumpscare stinger and temporarily duck ambience.
//      * StingerVolume/Pitch control the stinger; AmbienceDuck lowers ambience, then fades back.
//      */
//     UFUNCTION(BlueprintCallable, Category="Audio Director|Jumpscare",
//               meta=(ToolTip="Play a jumpscare stinger in front of the player and duck ambience, then fade ambience back."))
//     void PlayJumpscare(USoundBase* Stinger,
//                        float StingerVolume = 1.0f,
//                        float StingerPitch  = 1.0f,
//                        float AmbienceDuck  = 0.3f,
//                        float FadeBackTime  = 1.0f);
//
//     // ========= LOOPING SFX =========
//
//     /** Start a looping 2D SFX that can be stopped later (e.g. hum, hiss). */
//     UFUNCTION(BlueprintCallable, Category="Audio Director|Looping",
//               meta=(ToolTip="Start a looping 2D sound (e.g. hum) that can be stopped with StopLoopingSFX2D."))
//     void StartLoopingSFX2D(USoundBase* Sound,
//                            float Volume = 1.0f,
//                            float Pitch = 1.0f);
//
//     /** Stop the currently playing looping 2D SFX if any. */
//     UFUNCTION(BlueprintCallable, Category="Audio Director|Looping",
//               meta=(ToolTip="Stop the currently playing looping 2D SFX (if any)."))
//     void StopLoopingSFX2D();
//
//     /** Start a looping 3D SFX at a world location. */
//     UFUNCTION(BlueprintCallable, Category="Audio Director|Looping",
//               meta=(ToolTip="Start a looping 3D sound at a given location that can be stopped with StopLoopingSFX3D."))
//     void StartLoopingSFXAtLocation(USoundBase* Sound,
//                                    FVector Location,
//                                    float Volume = 1.0f,
//                                    float Pitch = 1.0f);
//
//     /** Stop the currently playing looping 3D SFX if any. */
//     UFUNCTION(BlueprintCallable, Category="Audio Director|Looping",
//               meta=(ToolTip="Stop the currently playing looping 3D SFX (if any)."))
//     void StopLoopingSFX3D();
//
//     // ========= FOOTSTEPS =========
//
//     /**
//      * Play a footstep at a world location.
//      * Surface selects which footstep list is used; Volume/Pitch optionally override mix.
//      */
//     UFUNCTION(BlueprintCallable, Category="Audio Director|Footsteps",
//               meta=(ToolTip="Play a footstep sound at a world location using a surface type. If Volume=1, final volume = Master * SFX."))
//     void PlayFootstepAtLocation(FVector Location,
//                                 EFootstepSurface Surface,
//                                 float Volume = 1.0f,
//                                 float Pitch = 1.0f);
//
// protected:
//     // Ambience state
//     UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Ambience",
//               meta=(ToolTip="Current ambience state. Controls which ambience SoundCue is used."))
//     EAudioAmbienceState CurrentAmbienceState = EAudioAmbienceState::Idle;
//
//     // One cue per ambience state
//     UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Ambience",
//               meta=(ToolTip="Ambience used while in Idle state (very low or no tension)."))
//     USoundCue* IdleAmbienceCue = nullptr;
//
//     UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Ambience",
//               meta=(ToolTip="Ambience used while in Exploration state (normal wandering)."))
//     USoundCue* ExplorationAmbienceCue = nullptr;
//
//     UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Ambience",
//               meta=(ToolTip="Ambience used while in Suspense state (something feels wrong)."))
//     USoundCue* SuspenseAmbienceCue = nullptr;
//
//     UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Ambience",
//               meta=(ToolTip="Ambience used while in Chase state (high tension)."))
//     USoundCue* ChaseAmbienceCue = nullptr;
//
//     UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Ambience",
//               meta=(ToolTip="Ambience used while in Safe Room state (extra calm, safe area)."))
//     USoundCue* SafeRoomAmbienceCue = nullptr;
//
//     /** Per-state ambience volume scale (1.0 = normal). */
//     UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Ambience",
//               meta=(ToolTip="Extra volume multiplier per ambience state (e.g. make Chase louder than Exploration)."))
//     TMap<EAudioAmbienceState, float> AmbienceStateVolumeScale;
//
//     UPROPERTY(Transient)
//     UAudioComponent* AmbienceAC = nullptr;
//
//     float AmbienceTargetVolume = 1.0f;
//     float AmbienceFadeSpeed    = 1.0f;
//
//     // Volume
//     UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Volume",
//               meta=(ClampMin="0.0", ClampMax="1.0",
//                    ToolTip="Overall master volume for all sounds (0=mute, 1=full)."))
//     float MasterVolume = 1.0f;
//
//     UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Volume",
//               meta=(ClampMin="0.0", ClampMax="1.0",
//                    ToolTip="Main SFX volume multiplier for non-UI sounds."))
//     float SFXVolume = 1.0f;
//
//     UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Volume",
//               meta=(ClampMin="0.0", ClampMax="1.0",
//                    ToolTip="UI volume multiplier used by PlayUISound."))
//     float UIVolume = 1.0f;
//
//     UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Volume",
//               meta=(ClampMin="0.0", ClampMax="1.0",
//                    ToolTip="Extra volume multiplier for Player SFX category."))
//     float PlayerSFXVolume = 1.0f;
//
//     UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Volume",
//               meta=(ClampMin="0.0", ClampMax="1.0",
//                    ToolTip="Extra volume multiplier for Enemy SFX category."))
//     float EnemySFXVolume = 1.0f;
//
//     UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Volume",
//               meta=(ClampMin="0.0", ClampMax="1.0",
//                    ToolTip="Extra volume multiplier for Environment SFX category."))
//     float EnvironmentSFXVolume = 1.0f;
//
//     // Looping SFX
//     UPROPERTY(Transient)
//     UAudioComponent* LoopingSFX2D = nullptr;
//
//     UPROPERTY(Transient)
//     UAudioComponent* LoopingSFX3D = nullptr;
//
//     // Footsteps
//     UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Footsteps",
//               meta=(ToolTip="Footstep sounds per surface type. If a surface is missing, Default is used as fallback."))
//     TMap<EFootstepSurface, FFootstepSoundList> Footsteps_BySurface;
//
//     // Internal helpers
//     USoundCue* GetAmbienceCueForState(EAudioAmbienceState State) const;
//     void UpdateAmbience(float DeltaTime);
//
//     float GetCategoryVolume(EAudioSFXCategory Category) const;
//     USoundBase* ChooseRandomFromArray(const TArray<USoundBase*>& Array) const;
// };
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "GameAudioDirectorComponent.generated.h"

UENUM(BlueprintType)
enum class EFootstepSurface : uint8
{
    Default UMETA(DisplayName="Default"),
    Wood    UMETA(DisplayName="Wood"),
    Stone   UMETA(DisplayName="Stone"),
    Metal   UMETA(DisplayName="Metal"),
    Carpet  UMETA(DisplayName="Carpet")
};

USTRUCT(BlueprintType)
struct FFootstepSoundList
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Footsteps",
              meta=(ToolTip="List of possible footstep sounds for a given surface."))
    TArray<USoundBase*> Sounds;
};

UENUM(BlueprintType)
enum class EAudioAmbienceState : uint8
{
    Idle        UMETA(DisplayName="Idle"),
    Exploration UMETA(DisplayName="Exploration"),
    Suspense    UMETA(DisplayName="Suspense"),
    Chase       UMETA(DisplayName="Chase"),
    SafeRoom    UMETA(DisplayName="Safe Room")
};

UENUM(BlueprintType)
enum class EAudioSFXCategory : uint8
{
    General     UMETA(DisplayName="General"),
    Player      UMETA(DisplayName="Player"),
    Enemy       UMETA(DisplayName="Enemy"),
    Environment UMETA(DisplayName="Environment")
};

USTRUCT(BlueprintType)
struct FCustomAmbienceEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio Director|Ambience",
              meta=(ToolTip="Custom ambience state name (e.g. BossFight, PuzzleRoom)."))
    FName StateName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio Director|Ambience",
              meta=(ToolTip="Looping SoundCue to use for this custom ambience state."))
    USoundCue* Cue = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio Director|Ambience",
              meta=(ClampMin="0.0", ToolTip="Extra volume multiplier for this custom ambience state (1.0 = normal)."))
    float VolumeScale = 1.0f;
};

/**
 * Game audio director: enum-based default ambience states + custom name-based states,
 * plus SFX/UI/footsteps/jumpscares/looping.
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent), Blueprintable)
class GAMEAUDIODIRECTOR_API UGameAudioDirectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGameAudioDirectorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(
        float DeltaTime,
        ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction
    ) override;

    // ========= AMBIENCE STATE =========

    /** Set ambience using built-in enum state (Idle, Exploration, Suspense, Chase, SafeRoom). */
    UFUNCTION(BlueprintCallable, Category="Audio Director|Ambience",
              meta=(ToolTip="Set ambience using built-in enum state (Idle, Exploration, Suspense, Chase, SafeRoom). Uses DefaultAmbienceCues/Volumes."))
    void SetAmbienceState(EAudioAmbienceState NewState, float FadeTime = 1.0f);

    /** Set ambience using a custom state name (e.g. BossFight, PuzzleRoom) from CustomAmbienceEntries. */
    UFUNCTION(BlueprintCallable, Category="Audio Director|Ambience",
              meta=(ToolTip="Set ambience using a custom state name (e.g. BossFight, PuzzleRoom) defined in CustomAmbienceEntries."))
    void SetAmbienceStateByName(FName NewStateName, float FadeTime = 1.0f);

    /** Get the current enum ambience state (for defaults). */
    UFUNCTION(BlueprintPure, Category="Audio Director|Ambience",
              meta=(ToolTip="Return the current enum ambience state (for default states)."))
    EAudioAmbienceState GetAmbienceState() const { return CurrentAmbienceState; }

    /** Get the current custom ambience state name (if using name-based state). */
    UFUNCTION(BlueprintPure, Category="Audio Director|Ambience",
              meta=(ToolTip="Return the current custom ambience state name, or None if using enum-based default."))
    FName GetCustomAmbienceStateName() const { return CurrentAmbienceCustomName; }

    /** Add or update a custom ambience entry (StateName, Cue, VolumeScale). */
    UFUNCTION(BlueprintCallable, Category="Audio Director|Ambience",
              meta=(ToolTip="Add or update a custom ambience entry (StateName, Cue, VolumeScale) in CustomAmbienceEntries."))
    void AddOrUpdateCustomAmbienceEntry(const FCustomAmbienceEntry& Entry);

    /** Remove any custom ambience entry with the given StateName. */
    UFUNCTION(BlueprintCallable, Category="Audio Director|Ambience",
              meta=(ToolTip="Remove any custom ambience entry that matches the given StateName."))
    void RemoveCustomAmbienceEntry(FName StateName);

    // ========= VOLUME =========

    UFUNCTION(BlueprintCallable, Category="Audio Director|Volume",
              meta=(ToolTip="Set overall master volume for all sounds (0=mute, 1=full)."))
    void SetMasterVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category="Audio Director|Volume",
              meta=(ToolTip="Set main SFX volume for non-UI sounds."))
    void SetSFXVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category="Audio Director|Volume",
              meta=(ToolTip="Set UI volume used by PlayUISound."))
    void SetUIVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category="Audio Director|Volume",
              meta=(ToolTip="Set extra volume multiplier for Player SFX category."))
    void SetPlayerSFXVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category="Audio Director|Volume",
              meta=(ToolTip="Set extra volume multiplier for Enemy SFX category."))
    void SetEnemySFXVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category="Audio Director|Volume",
              meta=(ToolTip="Set extra volume multiplier for Environment SFX category."))
    void SetEnvironmentSFXVolume(float NewVolume);

    // ========= SFX / UI =========

    UFUNCTION(BlueprintCallable, Category="Audio Director|Play 2D",
              meta=(ToolTip="Play a non-spatial (2D) sound. Final volume = Volume * Master * SFX."))
    void PlaySound2D(USoundBase* Sound,
                     float Volume = 1.0f,
                     float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category="Audio Director|Play 3D",
              meta=(ToolTip="Play a 3D sound at a specific world location. Final volume = Volume * Master * SFX."))
    void PlaySoundAtLocation(USoundBase* Sound,
                             FVector Location,
                             float Volume = 1.0f,
                             float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category="Audio Director|Play 2D",
              meta=(ToolTip="Play a 2D sound with category mixing (Player/Enemy/Environment). Final volume = Volume * Master * SFX * CategoryVolume."))
    void PlayCategorizedSound2D(USoundBase* Sound,
                                EAudioSFXCategory Category,
                                float Volume = 1.0f,
                                float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category="Audio Director|Play 3D",
              meta=(ToolTip="Play a 3D sound with category mixing at a location. Final volume = Volume * Master * SFX * CategoryVolume."))
    void PlayCategorizedSoundAtLocation(USoundBase* Sound,
                                        EAudioSFXCategory Category,
                                        FVector Location,
                                        float Volume = 1.0f,
                                        float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category="Audio Director|Play Relative To Player",
              meta=(ToolTip="Play a 3D sound relative to the player camera. Offset is in camera local space (X=forward, Y=right, Z=up)."))
    void PlaySoundRelativeToPlayer(USoundBase* Sound,
                                   FVector Offset,
                                   float Volume = 1.0f,
                                   float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category="Audio Director|Play UI",
              meta=(ToolTip="Play a non-spatial UI sound. Final volume = Volume * Master * UI."))
    void PlayUISound(USoundBase* Sound,
                     float Volume = 1.0f,
                     float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category="Audio Director|Jumpscare",
              meta=(ToolTip="Play a jumpscare stinger in front of the player and duck ambience, then fade ambience back."))
    void PlayJumpscare(USoundBase* Stinger,
                       float StingerVolume = 1.0f,
                       float StingerPitch  = 1.0f,
                       float AmbienceDuck  = 0.3f,
                       float FadeBackTime  = 1.0f);

    // ========= LOOPING SFX =========

    UFUNCTION(BlueprintCallable, Category="Audio Director|Looping",
              meta=(ToolTip="Start a looping 2D sound (e.g. hum) that can be stopped with StopLoopingSFX2D."))
    void StartLoopingSFX2D(USoundBase* Sound,
                           float Volume = 1.0f,
                           float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category="Audio Director|Looping",
              meta=(ToolTip="Stop the currently playing looping 2D SFX (if any)."))
    void StopLoopingSFX2D();

    UFUNCTION(BlueprintCallable, Category="Audio Director|Looping",
              meta=(ToolTip="Start a looping 3D sound at a given location that can be stopped with StopLoopingSFX3D."))
    void StartLoopingSFXAtLocation(USoundBase* Sound,
                                   FVector Location,
                                   float Volume = 1.0f,
                                   float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category="Audio Director|Looping",
              meta=(ToolTip="Stop the currently playing looping 3D SFX (if any)."))
    void StopLoopingSFX3D();

    // ========= FOOTSTEPS =========

    UFUNCTION(BlueprintCallable, Category="Audio Director|Footsteps",
              meta=(ToolTip="Play a footstep sound at a world location using a surface type. If Volume=1, final volume = Master * SFX."))
    void PlayFootstepAtLocation(FVector Location,
                                EFootstepSurface Surface,
                                float Volume = 1.0f,
                                float Pitch = 1.0f);

protected:
    // Ambience state (enum defaults)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Ambience",
              meta=(ToolTip="Current enum ambience state (used for default ambience mapping)."))
    EAudioAmbienceState CurrentAmbienceState = EAudioAmbienceState::Idle;

    // Current custom ambience name (if using name-based custom state; None if using enum)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Ambience",
              meta=(ToolTip="Current custom ambience state name. If None, the enum-based state is used."))
    FName CurrentAmbienceCustomName = NAME_None;

    /** Enum-based default ambience cues (Idle/Exploration/Suspense/Chase/SafeRoom). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Ambience",
              meta=(ToolTip="Default ambience cues per enum state. Populate Idle/Exploration/Suspense/Chase/SafeRoom here."))
    TMap<EAudioAmbienceState, USoundCue*> DefaultAmbienceCues;

    /** Enum-based default ambience volume scale (1.0 = normal). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Ambience",
              meta=(ToolTip="Extra volume multipliers per enum ambience state (1.0 = normal)."))
    TMap<EAudioAmbienceState, float> DefaultAmbienceVolumeScale;

    /** Custom ambience states that you can add/remove in Blueprint (name → cue + volume scale). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio Director|Ambience",
              meta=(ToolTip="Custom ambience states. Each maps a StateName to a looping SoundCue and volume scale."))
    TArray<FCustomAmbienceEntry> CustomAmbienceEntries;

    UPROPERTY(Transient)
    UAudioComponent* AmbienceAC = nullptr;

    float AmbienceTargetVolume = 1.0f;
    float AmbienceFadeSpeed    = 1.0f;

    // Volume
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Volume",
              meta=(ClampMin="0.0", ClampMax="1.0",
                   ToolTip="Overall master volume for all sounds (0=mute, 1=full)."))
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Volume",
              meta=(ClampMin="0.0", ClampMax="1.0",
                   ToolTip="Main SFX volume multiplier for non-UI sounds."))
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Volume",
              meta=(ClampMin="0.0", ClampMax="1.0",
                   ToolTip="UI volume multiplier used by PlayUISound."))
    float UIVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Volume",
              meta=(ClampMin="0.0", ClampMax="1.0",
                   ToolTip="Extra volume multiplier for Player SFX category."))
    float PlayerSFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Volume",
              meta=(ClampMin="0.0", ClampMax="1.0",
                   ToolTip="Extra volume multiplier for Enemy SFX category."))
    float EnemySFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Volume",
              meta=(ClampMin="0.0", ClampMax="1.0",
                   ToolTip="Extra volume multiplier for Environment SFX category."))
    float EnvironmentSFXVolume = 1.0f;

    // Looping SFX
    UPROPERTY(Transient)
    UAudioComponent* LoopingSFX2D = nullptr;

    UPROPERTY(Transient)
    UAudioComponent* LoopingSFX3D = nullptr;

    // Footsteps
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Director|Footsteps",
              meta=(ToolTip="Footstep sounds per surface type. If a surface is missing, Default is used as fallback."))
    TMap<EFootstepSurface, FFootstepSoundList> Footsteps_BySurface;

    // Internal helpers
    USoundCue* GetDefaultAmbienceCue(EAudioAmbienceState State) const;
    float GetDefaultAmbienceVolumeScale(EAudioAmbienceState State) const;

    USoundCue* GetCustomAmbienceCue(FName StateName) const;
    float GetCustomAmbienceVolumeScale(FName StateName) const;

    void UpdateAmbience(float DeltaTime);

    float GetCategoryVolume(EAudioSFXCategory Category) const;
    USoundBase* ChooseRandomFromArray(const TArray<USoundBase*>& Array) const;
};
