// NewUIBuffWindow.h: interface for the CNewUIBuffWindow class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "UI/NewUI/NewUIManager.h"

namespace SEASON3B
{
    class CNewUIBuffWindow : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_BUFF_STATUS = BITMAP_BUFFWINDOW_BEGIN,
            IMAGE_BUFF_STATUS2,
            IMAGE_BUFF_STATUS3,
            IMAGE_BUFF_SET_GUARD,   // Interface\newui_setguard (tools/hud/set_guard_icon.py)
            IMAGE_BUFF_GOLDEN_CURSE, // Interface\newui_goldencurse (tools/hud/golden_curse_icon.py), EFFECT_GOLDEN_CURSE
            IMAGE_BUFF_VEIL_WARD,     // Interface\newui_veilward (tools/hud/veil_icons.py), the Veil Ward (189)
            IMAGE_BUFF_VEIL_BLESSING, // Interface\newui_veilblessing (tools/hud/veil_icons.py), the Blessing of the Veil (185)
            // the state of the boss of the Illusion of Noria in the corner of the screen (tools/hud/boss_state_icons.py,
            // GameLogic::Events::IllusionOfNoria::RenderBossState): banished, ritual, awakened - in this order
            IMAGE_BOSS_BANISHED,
            IMAGE_BOSS_RITUAL,
            IMAGE_BOSS_AWAKENED,
        };

        enum BUFF_RENDER
        {
            BUFF_RENDER_ICON = 0,
            BUFF_RENDER_TOOLTIP
        };

    public:
        CNewUIBuffWindow();
        virtual ~CNewUIBuffWindow();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);
        void SetPos(int iScreenWidth);
        float GetIconsX() const;

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 5.3f

        void OpenningProcess();
        void ClosingProcess();

    private:
        void LoadImages();
        void UnloadImages();

        void BuffSort(std::list<eBuffState>& buffstate);
        void RenderBuffStatus(BUFF_RENDER renderstate);
        void RenderBuffIcon(eBuffState& eBuffType, float x, float y, float width, float height);
        void RenderBuffTooltip(eBuffClass& eBuffClassType, eBuffState& eBuffType, float x, float y);
        bool SetDisableRenderBuff(const eBuffState& _BuffState);

        CNewUIManager* m_pNewUIMng;
        POINT m_Pos;
    };
}
