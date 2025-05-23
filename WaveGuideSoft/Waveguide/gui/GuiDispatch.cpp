#include "GuiDispatch.hpp"
#include "bsp/UC1638.hpp"
#include "bsp/ControlIO.hpp"
#include "obj/Main.hpp"
#include "obj/Settings.hpp"

using bsp::UC1638;
using bsp::ControlIO;

namespace gui {

CGuiDispatch GuiDispatch;

void CGuiDispatch::Init() {
    // SetObj(Main);
    SetObj(Settings);
}

bool CGuiDispatch::Update(uint32_t msEscape)
{
    TimeTick(msEscape);
    if (!IsNeedUpdate()) {
        return false;
    }
    needUpdate_ = false;

    auto& display = UC1638.display;
    StyleDrawer drawer{ display };
    if (obj_ != nullptr) {
        obj_->Draw(drawer);
    }
    return true;
}

void CGuiDispatch::SetObj(CGuiObj& obj) {
    if (&obj != obj_) {
        obj_ = &obj;
        ControlIO.ResetButtonCallbacks();
        ControlIO.ResetEncoderCallbacks();
        ControlIO.SetButtonCallback(bsp::ButtonId::kBtnMain, [](bsp::ButtonEventArgs state) {
            if (state.IsAttack()) {
                GuiDispatch.SetObj(Main);
            } 
        });
        ControlIO.SetButtonCallback(bsp::ButtonId::kOctAdd, [](bsp::ButtonEventArgs args) {
            if (args.IsAttack()) {
                Main.SetOctave(Main.octave + 1);
            }
        });
        ControlIO.SetButtonCallback(bsp::ButtonId::kOctSub, [](bsp::ButtonEventArgs args) {
            if (args.IsAttack()) {
                Main.SetOctave(Main.octave - 1);
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