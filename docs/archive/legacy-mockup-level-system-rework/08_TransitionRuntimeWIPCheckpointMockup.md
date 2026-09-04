# 08. Transition Runtime WIP Checkpoint Mockup

> Status: WIP commit contract
> Target working branch: `feature/psw/level-system-rework`
> Reviewed local-change base HEAD: `8b6e66e30e59ccaf6687035978f5d31097628903`
> Depends on:
> - `34d6119ee7bf3b3c9683f46adf42cf7cbb8b044a:docs/wip/lobby-rework/06_StreamingLevelFoundationRefactorMockup.md`
> - `5f9781a2bf9a3a261ccf46dce0b28c4ff526457d:docs/wip/lobby-rework/07_StencilOwnershipRefactorMockup.md`
> Engine baseline: Unreal Engine 5.8

## 0. Authoritative commit scope

**This section is the authoritative staging manifest.**

Codex must stage exactly the paths below, with the three Config files staged only for the explicitly listed hunks. **Anything not listed here must remain unstaged.** Later sections explain why; they do not widen the commit scope.

### A. Stage these text/source paths completely

~~~text
Source/P_MA/Private/Convenience/MAHighlightComponent.cpp
Source/P_MA/Private/Convenience/MAHighlightComponent.h
Source/P_MA/Private/Debug/MACheatManager.cpp
Source/P_MA/Private/Debug/MACheatManager.h
Source/P_MA/Private/Level/Lobby/Hub/LobbyHubGameMode.cpp
Source/P_MA/Private/Level/Lobby/Hub/LobbyHubMagicCircle.cpp          [DELETE]
Source/P_MA/Private/Level/Lobby/Hub/LobbyHubMagicCircle.h            [DELETE]
Source/P_MA/Private/Level/Transition/MAMagicCircle.cpp
Source/P_MA/Private/Level/Transition/MAMagicCircle.h
Source/P_MA/Private/Level/Transition/MAWorldTransitionCoordinator.cpp [DELETE]
Source/P_MA/Private/Level/Transition/MAWorldTransitionCoordinator.h   [DELETE]
Source/P_MA/Private/Level/Transition/MAWorldTransitionMaskComponent.cpp [DELETE]
Source/P_MA/Private/Level/Transition/MAWorldTransitionMaskComponent.h   [DELETE]
Source/P_MA/Private/Level/Transition/MAWorldTransitionVisibilityComponent.cpp [DELETE]
Source/P_MA/Private/Level/Transition/MAWorldTransitionVisibilityComponent.h   [DELETE]
Source/P_MA/Private/Level/Transition/MASpaceTransitionMaskComponent.cpp
Source/P_MA/Private/Level/Transition/MASpaceTransitionMaskComponent.h
Source/P_MA/Private/Level/Transition/MASpaceTransitionSubsystem.cpp
Source/P_MA/Private/Level/Transition/MASpaceTransitionSubsystem.h
Source/P_MA/Private/Level/Transition/MASpaceTransitionTypes.h
Source/P_MA/Private/Level/Transition/MASpaceTransitionVisibilityComponent.cpp
Source/P_MA/Private/Level/Transition/MASpaceTransitionVisibilityComponent.h
Source/P_MA/Private/MAMaterialParams.h
Source/P_MA/Private/Player/MAPlayerCharacter.cpp
Source/P_MA/Private/Player/MAPlayerControllerBase.cpp
Source/P_MA/Private/Player/MAPlayerControllerBase.h
Source/P_MA/Private/MARenderStencil.h
~~~

### B. Stage these binary assets completely

~~~text
Content/PP_Highlight.uasset
Content/_Map/LobbyHubMap.umap
Content/_Map/MainMap1.umap
Content/_Map/WorldRoot.umap
Content/_WorkSpace/Level/LobbyHub/BP_MagicCircle.uasset
Content/_WorkSpace/Level/Transition/M_WorldTransitionMask.uasset
~~~

### C. Partially stage these Config files

Do not stage these files as whole-file changes.

#### `Config/DefaultEngine.ini`

Stage only:

~~~text
EditorStartupMap=/Game/_Map/WorldRoot.WorldRoot
GameDefaultMap=/Game/_Map/WorldRoot.WorldRoot

+ClassRedirects=(OldName="/Script/P_MA.MAWorldTransitionMaskComponent",NewName="/Script/P_MA.MASpaceTransitionMaskComponent")
+ClassRedirects=(OldName="/Script/P_MA.MAWorldTransitionVisibilityComponent",NewName="/Script/P_MA.MASpaceTransitionVisibilityComponent")
+ClassRedirects=(OldName="/Script/P_MA.MASpace",NewName="/Script/P_MA.MALevelRoot")
+StructRedirects=(OldName="/Script/P_MA.MASpaceRequest",NewName="/Script/P_MA.MASpaceTransitionRequest")
+PropertyRedirects=(OldName="/Script/P_MA.MASpaceRequest.MapAsset",NewName="/Script/P_MA.MASpaceTransitionRequest.DestinationMap")
+PropertyRedirects=(OldName="/Script/P_MA.MASpaceRequest.SlotTransform",NewName="/Script/P_MA.MASpaceTransitionRequest.DestinationSlotTransform")
+PropertyRedirects=(OldName="/Script/P_MA.MALevelRoot.TransitionAnchor",NewName="/Script/P_MA.MALevelRoot.TransitionCircle")
+PropertyRedirects=(OldName="/Script/P_MA.MAMagicCircle.WorldTransitionMaskComponent",NewName="/Script/P_MA.MAMagicCircle.SpaceTransitionMaskComponent")
~~~

Do not stage any other `DefaultEngine.ini` hunk, especially renderer target changes, CollisionProfile reserialization, or unrelated redirects.

#### `Config/DefaultGame.ini`

Stage only:

~~~text
+MapsToCook=(FilePath="/Game/_Map/MainMap1")
+MapsToCook=(FilePath="/Game/_Map/WorldRoot")
~~~

Do not stage unrelated AssetManager, culture, packaging, denylist, cook-directory, or formatting changes.

#### `Config/DefaultGameplayTags.ini`

Stage only the WorldTransition -> SpaceTransition sound migration:

~~~text
+GameplayTagRedirects=(OldTagName="Sound.WorldTransition",NewTagName="Sound.SpaceTransition")
+GameplayTagRedirects=(OldTagName="Sound.WorldTransition.Close",NewTagName="Sound.SpaceTransition.Close")
+GameplayTagRedirects=(OldTagName="Sound.WorldTransition.Open",NewTagName="Sound.SpaceTransition.Open")

-GameplayTagList=(Tag="Sound.WorldTransition",DevComment="World transition sound")
-GameplayTagList=(Tag="Sound.WorldTransition.Close",DevComment="")
-GameplayTagList=(Tag="Sound.WorldTransition.Open",DevComment="")
+GameplayTagList=(Tag="Sound.SpaceTransition",DevComment="Space transition sound")
+GameplayTagList=(Tag="Sound.SpaceTransition.Close",DevComment="")
+GameplayTagList=(Tag="Sound.SpaceTransition.Open",DevComment="")
~~~

Do not stage any other GameplayTag change.

### D. Explicitly leave these current local changes unstaged

~~~text
Content/_Map/MainMap.umap
docs/LevelSystemArchitecture.md
Config/DefaultEngine.ini unrelated hunks
Config/DefaultGame.ini unrelated hunks
Config/DefaultGameplayTags.ini unrelated hunks
LocalChangesReviewTool and generated review bundles
~~~

`MainMap.umap` is intentionally excluded even though it is currently modified.

### E. Staged-state acceptance rule

Before committing, inspect the staged diff only.

~~~text
git diff --cached --check
git diff --cached --name-status
git diff --cached --stat
~~~

The staged set must contain the 27 source/text paths from A, the 6 binary assets from B, and only the allowed hunks from the 3 Config files in C. There must be no staged path outside A/B/C.

If a dependency appears to require staging another path, **stop and report it instead of widening the commit scope.**

## 1. Purpose

This is not a final Transition Runtime approval commit. It is a buildable WIP checkpoint that preserves the currently working replacement as one checkout-safe state.

The checkpoint combines:

~~~text
Space Transition presentation replacement
+ 07 Stencil ownership contract
+ Persistent World transition runtime wiring
+ runtime assets/bootstrap required by the current implementation
~~~

The Streaming Level foundation itself is already in base HEAD `8b6e66e30e59ccaf6687035978f5d31097628903`; do not look for additional foundation files to stage in this WIP.

Stencil changes cannot be committed cleanly by themselves because the old `MAWorldTransitionVisibilityComponent` still calls the removed `UMAHighlightComponent::SetHighlightEnabled()` API. Therefore the old presentation/runtime replacement is kept together in this WIP.

This commit does **not** mean:

~~~text
Final transition architecture approved
Transition subsystem review passed
Network contract review passed
Asset/bootstrap cleanup complete
~~~

## 2. Stencil / Highlight contract

`FMARenderStencil` remains a stateless helper with only these external operations:

~~~cpp
FMARenderStencil::SetHighlightValue(...)
FMARenderStencil::SetTransitionVisible(...)
~~~

Stencil allocation:

~~~text
low 7 bits = Highlight
bit 7      = TransitionVisible

0      = no highlight
1..36  = hue highlight
37     = white highlight
~~~

Final application:

~~~text
Transition ON  -> ERSM_255
Transition OFF -> ERSM_Default
Final Stencil != 0 -> RenderCustomDepth ON
Final Stencil == 0 -> RenderCustomDepth OFF
~~~

`PP_Highlight` must use `CustomStencil % 128` for every Highlight decision. Raw stencil must not bypass that low-bit extraction. This fixes Transition-only stencil `128` being interpreted as a red highlight.

## 3. Transition Visibility contract

`UMASpaceTransitionVisibilityComponent` owns only:

~~~text
registered Primitive targets
Transition visible bit on/off
~~~

Public:

~~~cpp
AddTarget(...)
~~~

Private:

~~~cpp
SetVisibleThroughTransition(...)
~~~

`SetVisibleThroughTransition()` stays private and `UMASpaceTransitionMaskComponent` stays its friend because Mask is the only controller of that operation.

Do not restore:

~~~text
SetHighlighter
Highlighter
FTargetState
SavedTargetStates
Stencil snapshot/restore
Highlight disable/enable
~~~

## 4. Transition Mask checkpoint contract

`UMASpaceTransitionMaskComponent` currently preserves:

~~~text
Close/Open phase
SetClosedState / ReleaseClosedState
PostProcessVolume lifetime
Transition radius interpolation
VisibilityComponent collection
Close/Open completion delegate
~~~

The transition material interprets bit 7 only:

~~~text
CustomStencil >= 128
-> transition-visible subject
~~~

The Mask state machine and World-wide Visibility collection are **not finally approved by this WIP**. They are intentionally preserved for later detailed review.

## 5. Runtime replacement included by the manifest

The old runtime is removed:

~~~text
MAWorldTransitionCoordinator
MAWorldTransitionMaskComponent
MAWorldTransitionVisibilityComponent
LobbyHubMagicCircle
~~~

The current replacement is preserved:

~~~text
MASpaceTransitionSubsystem
MASpaceTransitionTypes
MASpaceTransitionMaskComponent
MASpaceTransitionVisibilityComponent
MAMagicCircle shared Hub/Battle implementation
PlayerController SpaceTransition RPC wiring
CheatManager transition test commands
~~~

Current RPC/debug names are preserved as WIP:

~~~text
ClientPrepareSpaceTransition
ClientCloseSpaceTransition
ClientCommitSpaceTransition
ClientCancelSpaceTransition
ServerNotifySpaceTransitionProgress

CloseSpaceTransition
OpenSpaceTransition
TravelToTransitionTestMap
~~~

Do not refactor unreviewed Subsystem/Network orchestration merely because it is included in this checkpoint.

## 6. Asset/bootstrap meaning

The binary assets in the authoritative manifest have these roles:

~~~text
WorldRoot      = Persistent UWorld
LobbyHubMap    = initial streamed level
MainMap1       = transition test destination
BP_MagicCircle = shared Magic Circle asset
PP_Highlight   = low-7-bit highlight consumer
M_WorldTransitionMask = bit-7 transition consumer
~~~

`Content/_Map/MainMap.umap` is not part of this checkpoint.

## 7. Validation before commit

Required:

~~~text
Project build passes
git diff --check passes
git diff --cached --check passes
~~~

Already validated runtime behavior must remain intact:

~~~text
Highlight works normally
Transition-only stencil does not produce red Highlight
Player and Magic Circle pass the transition mask
WorldRoot -> LobbyHubMap initial execution works
TravelToTransitionTestMap can enter MainMap1
~~~

Do not expand into new test infrastructure for this WIP.

## 8. Commit message and status

Recommended commit message:

~~~text
WIP: checkpoint persistent space transition runtime
~~~

This is intentionally a WIP checkpoint. Later review order remains:

~~~text
1. Transition Mask / Presentation detailed review
2. Transition Types / Network RPC review
3. UMASpaceTransitionSubsystem orchestration review
4. Asset/bootstrap/config redirect cleanup
5. LevelSystemArchitecture final update
6. follow-up cleanup commit if needed
~~~

The purpose of this checkpoint is to preserve the currently working intermediate state without accidentally staging unrelated local changes.