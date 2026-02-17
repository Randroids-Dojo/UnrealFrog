// Copyright UnrealFrog Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class AActor;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * Factory for building multi-part 3D models from engine primitives.
 *
 * Each Build* function attaches visual UStaticMeshComponents to the
 * owner actor's existing RootComponent. All sub-components have
 * collision disabled -- only the actor's own collision primitive
 * handles physics.
 *
 * Ported from WebFrogger's model factory functions (createFrog, createCar, etc.)
 * with Art Director adjustments for UE camera distance and PBR lighting.
 *
 * Coordinate mapping from Three.js to UE:
 *   Three.js X  -> UE X  (left-right)
 *   Three.js Y  -> UE Z  (up from ground)
 *   Three.js Z  -> UE -Y (forward in Three.js is -Z; forward in UE grid is +Y)
 *
 * Scale: 1.0 in Three.js = 100 UU in UE. Engine primitives are 100 UU at
 * scale 1.0, so Three.js dimensions map directly to UE scale factors.
 */
struct FModelFactory
{
	// -- Public Build Functions ------------------------------------------------
	// Each returns the first (body) mesh component created.
	// All child components are attached to Owner->GetRootComponent().

	/** Frog: body, belly, 2 eyes, 2 pupils, 4 legs (10 components). */
	static UStaticMeshComponent* BuildFrogModel(AActor* Owner);

	/** Car: body, cabin, 4 wheels (6 components). BodyColor varies per lane. */
	static UStaticMeshComponent* BuildCarModel(AActor* Owner, FLinearColor BodyColor);

	/** Truck: cab, trailer, 6 wheels (8 components). */
	static UStaticMeshComponent* BuildTruckModel(AActor* Owner);

	/** Bus: body, 3 windows, 4 wheels (8 components). */
	static UStaticMeshComponent* BuildBusModel(AActor* Owner);

	/** Log: single fat cylinder (1 component). End caps removed — looked like wheels from above. */
	static UStaticMeshComponent* BuildLogModel(AActor* Owner, int32 WidthCells);

	/** Turtle group: per turtle -- shell (dome), base, head. Width in grid cells. */
	static UStaticMeshComponent* BuildTurtleGroupModel(AActor* Owner, int32 WidthCells);

	/** Lily pad: pad disc, flower (2 components). For home slot indicators. */
	static UStaticMeshComponent* BuildLilyPadModel(AActor* Owner);

	// -- Mesh Accessors -------------------------------------------------------
	// Cached LoadObject calls for engine primitives.

	static UStaticMesh* GetCubeMesh();
	static UStaticMesh* GetSphereMesh();
	static UStaticMesh* GetCylinderMesh();

private:
	/**
	 * Create a UStaticMeshComponent, attach to Owner's root, set mesh/position/
	 * scale/color, disable collision, register. Returns the new component.
	 *
	 * @param Owner           Actor to attach to
	 * @param Name            Unique component name within the actor
	 * @param Mesh            Engine primitive mesh (Cube, Sphere, Cylinder)
	 * @param RelativeLocation Offset from actor root in UE units
	 * @param Scale           World scale (1.0 = 100 UU for engine primitives)
	 * @param Color           Flat color via FlatColorMaterial DynamicMaterialInstance
	 */
	static UStaticMeshComponent* AddMeshComponent(
		AActor* Owner,
		const TCHAR* Name,
		UStaticMesh* Mesh,
		FVector RelativeLocation,
		FVector Scale,
		FLinearColor Color);

	/**
	 * Same as AddMeshComponent but also applies a relative rotation.
	 * Used for cylinders that need to be rotated (wheels, log body, etc.)
	 */
	static UStaticMeshComponent* AddMeshComponentRotated(
		AActor* Owner,
		const TCHAR* Name,
		UStaticMesh* Mesh,
		FVector RelativeLocation,
		FRotator RelativeRotation,
		FVector Scale,
		FLinearColor Color);
};
