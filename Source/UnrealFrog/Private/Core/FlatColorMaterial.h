// Copyright UnrealFrog Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Materials/Material.h"

#if WITH_EDITORONLY_DATA
#include "Materials/MaterialExpressionVectorParameter.h"
#endif

/**
 * Returns a runtime-created UMaterial with a "Color" VectorParameter
 * connected to BaseColor. Created once and cached for the session lifetime.
 *
 * Editor-only: returns nullptr in non-editor (packaged game) builds.
 * For packaged builds, create the material as a persistent .uasset via
 * Tools/CreateMaterial/create_flat_color_material.py.
 */
inline UMaterial* GetOrCreateFlatColorMaterial()
{
#if WITH_EDITORONLY_DATA
	static TWeakObjectPtr<UMaterial> CachedMaterial;

	if (UMaterial* Existing = CachedMaterial.Get())
	{
		return Existing;
	}

	UMaterial* Mat = NewObject<UMaterial>(
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
