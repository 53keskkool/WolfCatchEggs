#pragma once
#include "Entity/Component.h"
#include "Entity/Entity.h"

class WolfComponent : public EntityComponent
{
public:
	WolfComponent();

	void OnAdd(Entity* pEnt);
	void OnRemove();

	void OnEggFall(VariantList* pVList);

private:

	void UpdateSizeVar();

	void OnScaleChanged(Variant* pDataObject);
	void OnPause(VariantList* pVList);
	void OnGameOver(VariantList* pVList);
	void OnArcadeInput(VariantList* pVList);
	//touch stuff
	void OnTouchStart(VariantList* pVList);
	void OnOverMove(VariantList* pVList);
	void OnOverEnd(VariantList* pVList);

	void OnRender(VariantList* pVList);
	void OnUpdate(VariantList* pVList);


	CL_Vec2f* m_pPos2d;
	CL_Vec2f* m_pSize2d;
	CL_Vec2f* m_pScale2d;

	SurfaceAnim* m_pWolfUp = NULL;
	SurfaceAnim* m_pWolfDown = NULL;

	bool m_bLookingUp = true;
	bool m_bLookingRight = true;

	bool m_bTouching = false;
	CL_Vec2f m_touchPos = CL_Vec2f(0, 0);
	float m_touchUpDelim = 0;
	float m_touchRightDelim = 0;

	bool m_bGameOver = false;
	bool m_bPaused = false;
};