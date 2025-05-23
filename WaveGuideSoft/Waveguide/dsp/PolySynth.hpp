#pragma once
#include <cstdint>
#include <span>
#include "DelayAllocator.hpp"

namespace dsp {

template<class T>
class PolySynth {
public:
    static constexpr uint32_t kNumPolyonic = 8;

    void Init(uint32_t sampleRate) {
        for (auto& note : notes_) {
            note.Init(sampleRate);
        }
        numUsedNotes_ = 0;
        numUnusedNotes_ = kNumPolyonic;
        for (uint32_t i = 0; i < kNumPolyonic; ++i) {
            unusedNotes_[i] = &notes_[i];
        }
    }

    void Process(std::span<float> buffer, std::span<float> auxBuffer) {
        if (numUsedNotes_ > 0) {
            bool shouldRemove = usedNotes_[0]->Process(buffer, auxBuffer);
            if (shouldRemove) {
                unusedNotes_[numUnusedNotes_++] = usedNotes_[0];
                std::swap(usedNotes_[0], usedNotes_[numUsedNotes_ - 1]);
                --numUsedNotes_;
                for (uint32_t i = 0; i < numUsedNotes_;) {
                    shouldRemove = usedNotes_[i]->AddTo(buffer, auxBuffer);
                    if (shouldRemove) {
                        unusedNotes_[numUnusedNotes_++] = usedNotes_[i];
                        std::swap(usedNotes_[i], usedNotes_[numUsedNotes_ - 1]);
                        --numUsedNotes_;
                    }
                    else {
                        ++i;
                    }
                }
            }
            else {
                for (uint32_t i = 1; i < numUsedNotes_;) {
                    bool shouldRemove = usedNotes_[i]->AddTo(buffer, auxBuffer);
                    if (shouldRemove) {
                        unusedNotes_[numUnusedNotes_++] = usedNotes_[i];
                        std::swap(usedNotes_[i], usedNotes_[numUsedNotes_ - 1]);
                        --numUsedNotes_;
                    }
                    else {
                        ++i;
                    }
                }
            }
        }
        else {
            std::fill_n(buffer.begin(), buffer.size(), 0);
        }
    }

    void NoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
        if (numUnusedNotes_ > 0) {
            auto* used = unusedNotes_[--numUnusedNotes_];
            used->NoteOn(channel, note, velocity / 127.0f);
            usedNotes_[numUsedNotes_++] = used;
        }
        else {
            notes_[roundrobinPos_].NoteOn(channel, note, velocity / 127.0f);
            ++roundrobinPos_;
            roundrobinPos_ &= (kNumPolyonic - 1);
        }
    }

    void NoteOff(uint8_t note) {
        for (uint32_t i = 0; i < numUsedNotes_; ++i) {
            if (usedNotes_[i]->IsPlaying(note)) {
                usedNotes_[i]->NoteOff();
            }
        }
    }

    void ForceStopAll() {
        for (uint32_t i = 0; i < numUsedNotes_; ++i) {
            usedNotes_[i]->Panic();
        }
        numUsedNotes_ = 0;
        numUnusedNotes_ = kNumPolyonic;
        for (uint32_t i = 0; i < kNumPolyonic; ++i) {
            unusedNotes_[i] = &notes_[i];
        }
    }

    std::span<T*> GetUsedNotes() { return std::span<T*>(usedNotes_, numUsedNotes_); }
    std::span<T> GetNotes() { return std::span<T>(notes_, kNumPolyonic); }
private:
    T notes_[kNumPolyonic];
    T* usedNotes_[kNumPolyonic]{};
    T* unusedNotes_[kNumPolyonic]{};
    uint32_t numUsedNotes_{};
    uint32_t numUnusedNotes_{};
    uint32_t roundrobinPos_{};
};

}