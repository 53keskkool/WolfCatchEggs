#pragma once
#include "Entity/Component.h"
#include "Entity/Entity.h"

class EggComponent : public EntityComponent
{
public:
	EggComponent();

	void OnAdd(Entity* pEnt);
	void OnRemove();

private:

	void UpdateSizeVar();

	void OnScaleChanged(Variant* pDataObject);

	void OnRender(VariantList* pVList);
	void OnUpdate(VariantList* pVList);


	CL_Vec2f* m_pPos2d;
	CL_Vec2f* m_pSize2d;
	CL_Vec2f* m_pScale2d;
	float* m_pRotation;
	CL_Vec2f* m_pRotationCenter;
	float* m_pAlpha;
	float* m_pAngle; //angle to which egg should go
	float* m_pSpeed; //speed with which egg will go
	uint32* m_pFalling; //already lost it, now you can create a new egg
	float* m_pShelfLength; //length (X only) of the shelf to see when we should start falling
	float* m_pFloorY; //pos of the floor, so we know when egg gets broken

	SurfaceAnim* m_pTex = NULL;
	SurfaceAnim* m_pTexBroken = NULL;

	float m_alrdWent = 0; //how much did we go in X
	bool m_bRotatingRight = false;
	float m_rotationMod = 2.25f;

	bool m_bBroken = false;
	uint32 m_startFadeOutAt = 0;
};