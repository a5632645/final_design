#include "Preset.hpp"
#include "bsp/ControlIO.hpp"

namespace gui::internal {

struct TestBuffer {
    char buff[12];
};
static TestBuffer test[16]{};

void CPreset::Draw(StyleDrawer& drawer) {
    drawer.display.Fill(OledColorEnum::kOledBLACK);
    drawer.DrawTitleBar("预设");

    if (!isSaving_) {
        drawer.DrawBottomOptions(0, "保存");
        drawer.DrawBottomOptions(1, "加载");
        auto b = drawer.display.GetDrawAera();
        b.RemovedFromTop(12);
        b.RemovedFromBottom(12);

        auto numShow = std::min(kNumCanDisplay, numTotalListItems_ - listTopPos_);
        b.h = 12;
        for (int32_t i = 0; i < numShow; ++i) {
            auto idx = i + listTopPos_;
            drawer.display.FormatString(b.x, b.y, "{}/{}: {}", idx + 1, numTotalListItems_, test[idx].buff);
            if (i == listPos_) {
                drawer.InverseFrame(b);
            }
            b.Translated(0, b.h);
        }
    }
    else {
        auto b = drawer.display.GetDrawAera();
        b.RemovedFromTop(12);
        drawer.display.setColor(OledColorEnum::kOledWHITE);
        std::string_view view{editBuffer_, editPos_};
        auto w = drawer.display.FormantToInternalBuffer("名称:{}", view);
        drawer.display.drawString(b.x, b.y, std::string_view{w.data(), w.length()});
        int16_t xOffset = drawer.display.getStringWidth(std::string_view{w.data(), w.length()});
        char currentChar = editBuffer_[editPos_];
        int16_t length = drawer.display.getStringWidth(std::string_view{&currentChar, 1});
        drawer.InverseFrame(Rectange{xOffset, b.y, length, 12});
        drawer.display.drawString(xOffset, b.y, std::string_view{editBuffer_ + editPos_, 11 - editPos_});

        drawer.DrawBottomOptions(0, "确定");
        drawer.DrawBottomOptions(1, "取消");
    }
}

inline void CPreset::OnSelect() {
    numTotalListItems_ = 16;

    auto& io = bsp::ControlIO;
    io.SetButtonCallback(bsp::ButtonId::kBtn0, [](bsp::ButtonEventArgs args) {
        if (!args.IsAttack()) return;

        if (Preset.isSaving_) { // 确认编辑
            Preset.isSaving_ = false;
            for (int32_t i = 0; i < 11; ++i) {
                if (Preset.editBuffer_[i] == 0) {
                    Preset.editBuffer_[i] = ' ';
                }
            }
            Preset.editBuffer_[11] = 0;

            int32_t idx = Preset.listTopPos_ + Preset.listPos_;
            std::copy_n(Preset.editBuffer_, 12, test[idx].buff);
            Preset.Redraw();
            return;
        }

        // 保存
        int32_t idx = Preset.listTopPos_ + Preset.listPos_;
        Preset.EditText(test[idx].buff, 12);
    });
    io.SetButtonCallback(bsp::ButtonId::kBtn1, [](bsp::ButtonEventArgs args) {
        if (!args.IsAttack()) return;

        if (Preset.isSaving_) { // 取消编辑
            Preset.isSaving_ = false;
            Preset.Redraw();
            return;
        }

        // 加载
    });
    io.SetEncoderCallback(bsp::EncoderId::kParam, [](int32_t dvalue) {
        if (Preset.isSaving_) {
            Preset.editPos_ += dvalue;
            if (Preset.editPos_ < 0) Preset.editPos_ = 0;
            if (Preset.editPos_ >= 11) Preset.editPos_ = 10; // '\0'不能编辑
            Preset.Redraw();
            return;
        }

        Preset.listPos_ += dvalue;
        auto e = std::min(Preset.numTotalListItems_ - Preset.listTopPos_, kNumCanDisplay);
        if (Preset.listPos_ < 0) {
            auto up = -Preset.listPos_;
            Preset.listPos_ = 0;
            Preset.listTopPos_ -= up;
            if (Preset.listTopPos_ < 0) {
                Preset.listTopPos_ = 0;
            }
        }
        else if (Preset.listPos_ >= e) {
            auto down = Preset.listPos_ - e + 1;
            Preset.listTopPos_ += down;
            if (Preset.listTopPos_ + kNumCanDisplay >= Preset.numTotalListItems_) {
                auto up = Preset.numTotalListItems_ - Preset.listTopPos_ - kNumCanDisplay;
                Preset.listTopPos_ += up;
                if (Preset.listTopPos_ < 0) {
                    Preset.listTopPos_ = 0;
                }
            }
            Preset.listPos_ = e - 1;
            if (Preset.listPos_ < 0) {
                Preset.listPos_ = 0;
            }
        }
        Preset.Redraw();
    });
    io.SetEncoderCallback(bsp::EncoderId::kValue, [](int32_t dvalue) {
        if (!Preset.isSaving_) return;

        auto& c = Preset.editBuffer_[Preset.editPos_];
        c += dvalue;
        if (c < 0) c = 0;
        if (c > 127) c = 127;
        Preset.Redraw();
    });
}

void CPreset::EditText(char* ptr, int32_t length) {
    std::copy_n(ptr, length, editBuffer_);
    for (int32_t i = 0; i < 11; ++i) {
        if (editBuffer_[i] == 0) {
            editBuffer_[i] = ' ';
        }
    }
    editBuffer_[11] = 0;
    editBufferLength_ = length;
    isSaving_ = true;
    Preset.Redraw();
}
}