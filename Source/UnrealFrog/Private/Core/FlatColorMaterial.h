// Copyright UnrealFrog Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"

#if WITH_EDITORONLY_DATA
#include "Materials/MaterialExpressionVectorParameter.h"
#endif

/**
 * Returns a UMaterial with a "Color" VectorParameter connected to BaseColor.
 *
 * Strategy:
 *   1. Try loading the persistent .uasset at /Game/Materials/M_FlatColor
 *      (created via Tools/CreateMaterial/create_flat_color_material.py)
 *   2. If not found (asset not yet created), fall back to runtime creation
 *      (editor-only — returns nullptr in packaged builds)
 *
 * To create the persistent asset, run:
 *   UnrealEditor-Cmd <project> -ExecutePythonScript=Tools/CreateMaterial/create_flat_color_material.py
 */
inline UMaterial* GetOrCreateFlatColorMaterial()
{
	static TWeakObjectPtr<UMaterial> CachedMaterial;

	if (UMaterial* Existing = CachedMaterial.Get())
	{
		return Existing;
	}

	// Strategy 1: Load persistent .uasset (works in packaged builds)
	UMaterial* Mat = LoadObject<UMaterial>(
		nullptr, TEXT("/Game/Materials/M_FlatColor.M_FlatColor"));
	if (Mat)
	{
		CachedMaterial = Mat;
		return Mat;
	}

#if WITH_EDITORONLY_DATA
	// Strategy 2: Runtime creation (editor-only fallback)
	Mat = NewObject<UMaterial>(
		GetTransientPackage(), TEXT("M_FlatColor_Runtime"));
	Mat->AddToRoot(); // prevent GC

	UMaterialExpressionVectorParameter* Param =
		NewObject<UMaterialExpressionVectorParameter>(Mat);
	Param->ParameterName = TEXT("Color");
	Param->DefaultValue = FLinearColor::White;

	Mat->GetExpressionCollection().AddExpression(Param);
	Mat->GetEditorOnlyData()->BaseColor.Expression = Param;

	Mat->PreEditChange(nullptr);
	Mat->PostEditChange();

	CachedMaterial = Mat;
	return Mat;
#else
	return nullptr;
#endif
}
