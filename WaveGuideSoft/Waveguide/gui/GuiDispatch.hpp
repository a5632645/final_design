#pragma once
#include <cstdint>
#include "usf/usf.hpp"
#include "obj/Styles.hpp"
#include "bsp/ControlIO.hpp"

namespace gui {

class CGuiDispatch;
struct CGuiObj {
    virtual ~CGuiObj() = default;
    virtual void Draw(StyleDrawer& display) = 0;
    virtual void OnSelect() = 0;
    virtual void OnTimeTick(uint32_t msEscape) {}
    void Redraw();
};

class CGuiDispatch {
public:
    static constexpr uint32_t kFps = 10;
    static constexpr uint32_t kMsPerFrame = 1000 / kFps;

    void Init();
    bool Update(uint32_t msEscape);
    void SetObj(CGuiObj& obj);
    CGuiObj* GetObj() { return obj_; }
    bool IsObjShowing(CGuiObj& obj) { return obj_ == &obj; }
    void SetNeedUpdate() { needUpdate_ = true; }

    // time tick
    void TimeTick(uint32_t msEscape);
    uint32_t GetMsEscape() { return msEscape_; }
private:
    bool IsNeedUpdate();

    bool needUpdate_ = true;
    CGuiObj* obj_ = nullptr;
    uint32_t msEscape_ = 0;
    uint32_t frameMsCount_ = kMsPerFrame;
};

extern CGuiDispatch GuiDispatch;

}