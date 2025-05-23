#include "GuiDispatch.hpp"
#include "bsp/ControlIO.hpp"
#include "obj/Main.hpp"
#include "obj/ReedDebug.hpp"
#include "obj/BowedDebug.hpp"

using bsp::CControlIO;

extern OLEDDisplay display;

namespace gui {

CGuiDispatch GuiDispatch;

void CGuiDispatch::Init() {
    SetObj(Main);
}

bool CGuiDispatch::Update(uint32_t msEscape)
{
    TimeTick(msEscape);
    if (!IsNeedUpdate()) {
        return false;
    }
    needUpdate_ = false;

    StyleDrawer drawer{ display };
    if (obj_ != nullptr) {
        obj_->Draw(drawer);
    }
    return true;
}

void CGuiDispatch::SetObj(CGuiObj& obj) {
    using bsp::ControlIO;
    if (&obj != obj_) {
        obj_ = &obj;
        ControlIO.ResetButtonCallbacks();
        ControlIO.ResetEncoderCallbacks();
        ControlIO.SetButtonCallback(bsp::ButtonId::kBtnMain, [](bsp::ButtonEventArgs state) {
            if (state.IsAttack()) {
                GuiDispatch.SetObj(Main);
            } 
        });
        obj.OnSelect();
        SetNeedUpdate();
    }
}

bool CGuiDispatch::IsNeedUpdate()
{
    if (frameMsCount_ < kMsPerFrame) {
        return false;
    }
    frameMsCount_ = 0;
    return needUpdate_;
}

void CGuiDispatch::TimeTick(uint32_t msEscape) {
    msEscape_ = msEscape;
    frameMsCount_ += msEscape;
    
    if (obj_ != nullptr) {
        obj_->OnTimeTick(msEscape);
    }
}

void CGuiObj::Redraw() {
    if (GuiDispatch.IsObjShowing(*this)) {
        GuiDispatch.SetNeedUpdate();
    }
}

}