/*
** Surge Synthesizer is Free and Open Source Software
**
** Surge is made available under the Gnu General Public License, v3.0
** https://www.gnu.org/licenses/gpl-3.0.en.html
**
** Copyright 2004-2020 by various individuals as described by the Git transaction log
**
** All source at: https://github.com/surge-synthesizer/surge.git
**
** Surge was a commercial product from 2004-2018, with Copyright and ownership
** in that period held by Claes Johanson at Vember Audio. Claes made Surge
** open source in September 2018.
*/

#pragma once

#include <random>
#include "basic_dsp.h"

enum modsrctype
{
    mst_undefined = 0,
    mst_controller,
    mst_adsr,
    mst_lfo,
    mst_stepseq,
};

enum modsources
{
    ms_original = 0,
    ms_velocity,
    ms_keytrack,
    ms_polyaftertouch,
    ms_aftertouch,
    ms_pitchbend,
    ms_modwheel,
    ms_ctrl1,
    ms_ctrl2,
    ms_ctrl3,
    ms_ctrl4,
    ms_ctrl5,
    ms_ctrl6,
    ms_ctrl7,
    ms_ctrl8,
    ms_ampeg,
    ms_filtereg,
    ms_lfo1,
    ms_lfo2,
    ms_lfo3,
    ms_lfo4,
    ms_lfo5,
    ms_lfo6,
    ms_slfo1,
    ms_slfo2,
    ms_slfo3,
    ms_slfo4,
    ms_slfo5,
    ms_slfo6,
    ms_timbre,
    ms_releasevelocity,
    ms_random_bipolar,
    ms_random_unipolar,
    ms_alternate_bipolar,
    ms_alternate_unipolar,
    ms_breath,
    ms_expression,
    ms_sustain,
    ms_lowest_key,
    ms_highest_key,
    ms_latest_key,
    n_modsources,
};

const int modsource_display_order[n_modsources] = {
    ms_original,
    ms_velocity,
    ms_releasevelocity,
    ms_keytrack,
    ms_lowest_key,
    ms_highest_key,
    ms_latest_key,
    ms_polyaftertouch,
    ms_aftertouch,
    ms_modwheel,
    ms_breath,
    ms_expression,
    ms_sustain,
    ms_pitchbend,
    ms_timbre,
    ms_alternate_bipolar,
    ms_alternate_unipolar,
    ms_random_bipolar,
    ms_random_unipolar,
    ms_filtereg,
    ms_ampeg,
    ms_lfo1,
    ms_lfo2,
    ms_lfo3,
    ms_lfo4,
    ms_lfo5,
    ms_lfo6,
    ms_slfo1,
    ms_slfo2,
    ms_slfo3,
    ms_slfo4,
    ms_slfo5,
    ms_slfo6,
    ms_ctrl1,
    ms_ctrl2,
    ms_ctrl3,
    ms_ctrl4,
    ms_ctrl5,
    ms_ctrl6,
    ms_ctrl7,
    ms_ctrl8,
};

const int n_customcontrollers = 8;
extern float samplerate_inv;
extern float samplerate;

const char modsource_names_button[n_modsources][32] = {
    "Off",          "Velocity", "Keytrack",   "Poly AT",     "Channel AT",    "Pitch Bend",
    "Modwheel",     "Macro 1",  "Macro 2",    "Macro 3",     "Macro 4",       "Macro 5",
    "Macro 6",      "Macro 7",  "Macro 8",    "Amp EG",      "Filter EG",     "LFO 1",
    "LFO 2",        "LFO 3",    "LFO 4",      "LFO 5",       "LFO 6",         "S-LFO 1",
    "S-LFO 2",      "S-LFO 3",  "S-LFO 4",    "S-LFO 5",     "S-LFO 6",       "MPE Timbre",
    "Rel Velocity", "Random",   "RandUni",    "Alternate",   "Alternate Uni", "Breath",
    "Expression",   "Sustain",  "Lowest Key", "Highest Key", "Latest Key",
};

const char modsource_names[n_modsources][32] = {
    "Off",
    "Velocity",
    "Keytrack",
    "Polyphonic Aftertouch",
    "Channel Aftertouch",
    "Pitch Bend",
    "Modulation Wheel",
    "Macro 1",
    "Macro 2",
    "Macro 3",
    "Macro 4",
    "Macro 5",
    "Macro 6",
    "Macro 7",
    "Macro 8",
    "Amp EG",
    "Filter EG",
    "Voice LFO 1",
    "Voice LFO 2",
    "Voice LFO 3",
    "Voice LFO 4",
    "Voice LFO 5",
    "Voice LFO 6",
    "Scene LFO 1",
    "Scene LFO 2",
    "Scene LFO 3",
    "Scene LFO 4",
    "Scene LFO 5",
    "Scene LFO 6",
    "MPE Timbre",
    "Release Velocity",
    "Random Bipolar",
    "Random Unipolar",
    "Alternate Bipolar",
    "Alternate Unipolar",
    "Breath",
    "Expression",
    "Sustain Pedal",
    "Lowest Key",
    "Highest Key",
    "Latest Key",
};

const char modsource_names_tag[n_modsources][32] = {
    "off",    "vel",    "keytrack",    "poly_at",    "chan_at",     "pbend",      "mwheel",
    "macro1", "macro2", "macro3",      "macro4",     "macro5",      "macro6",     "macro7",
    "macro8", "amp_eg", "filter_eg",   "lfo1",       "lfo2",        "lfo3",       "lfo4",
    "lfo5",   "lfo6",   "slfo1",       "slfo2",      "slfo3",       "slfo4",      "slfo5",
    "slfo6",  "timbre", "release_vel", "random",     "random_uni",  "alt",        "alt_uni",
    "breath", "expr",   "sustain",     "lowest_key", "highest_key", "latest_key",
};

inline bool isScenelevel(modsources ms)
{
    return (((ms <= ms_ctrl8) || ((ms >= ms_slfo1) && (ms <= ms_slfo6))) &&
            ((ms != ms_velocity) && (ms != ms_keytrack) && (ms != ms_polyaftertouch) &&
             (ms != ms_releasevelocity))) ||
           ((ms >= ms_breath) && (ms <= ms_latest_key));
}

inline bool canModulateMonophonicTarget(modsources ms)
{
    return isScenelevel(ms) || ms == ms_aftertouch;
}

inline bool isCustomController(modsources ms) { return (ms >= ms_ctrl1) && (ms <= ms_ctrl8); }

inline bool isEnvelope(modsources ms) { return (ms == ms_ampeg) || (ms == ms_filtereg); }

inline bool isLFO(modsources ms) { return (ms >= ms_lfo1) && (ms <= ms_slfo6); }

inline bool canModulateModulators(modsources ms) { return (ms != ms_ampeg) && (ms != ms_filtereg); }

inline bool isVoiceModulator(modsources ms) { return !((ms >= ms_slfo1) && (ms <= ms_slfo6)); }

inline bool canModulateVoiceModulators(modsources ms)
{
    return (ms <= ms_ctrl8) || ms == ms_timbre;
}

struct ModulationRouting
{
    int source_id;
    int destination_id;
    float depth;
    bool muted = false;
    int source_index{0};
};

class ModulationSource
{
  public:
    ModulationSource()
    {
        for (int i = 0; i < vecsize; ++i)
            voutput[i] = 0.f;
    }
    virtual ~ModulationSource() {}
    virtual const char *get_title() { return 0; }
    virtual int get_type() { return mst_undefined; }
    virtual void process_block() {}
    virtual void attack(){};
    virtual void release(){};
    virtual void reset(){};
    // override these if you support indices
    virtual void set_active_outputs(int ao) { active_outputs = ao; }
    virtual int get_active_outputs() { return active_outputs; }
    virtual float get_output(int which)
    {
        if (which == 0)
            return output;
        else if (which < vecsize)
            return voutput[which];
        else
            return 0.f;
    }
    virtual float get_output01(int which)
    {
        if (which == 0)
            return output;
        else if (which < vecsize)
            return voutput[which];
        else
            return 0.f;
    }
    virtual void set_output(int which, float f)
    {
        if (which == 0)
            output = f;
        else if (which < vecsize)
            voutput[which] = f;
    }

    virtual bool per_voice() { return false; }
    virtual bool is_bipolar() { return false; }
    virtual void set_bipolar(bool b) {}

  protected:
    int active_outputs{0};
    static constexpr int vecsize = 16;
    float output, voutput[vecsize];
};

class ControllerModulationSource : public ModulationSource
{
  public:
    // Smoothing and Shaping Behaviors
    enum SmoothingMode
    {
        LEGACY = -1, // This is (1) the exponential backoff and (2) not streamed.
        SLOW_EXP,    // Legacy with a sigma clamp
        FAST_EXP,    // Faster Legacy with a sigma clamp
        FAST_LINE,   // Linearly move
        DIRECT       // Apply the value directly
    } smoothingMode = LEGACY;

    ControllerModulationSource()
    {
        target = 0.f;
        output = 0.f;
        bipolar = false;
        changed = true;
        smoothingMode = LEGACY;
    }
    ControllerModulationSource(SmoothingMode mode) : ControllerModulationSource()
    {
        smoothingMode = mode;
    }

    virtual ~ControllerModulationSource() {}
    void set_target(float f)
    {
        target = f;
        startingpoint = output;
        changed = true;
    }

    void init(float f)
    {
        target = f;
        output = f;
        startingpoint = f;
        changed = true;
    }

    void set_target01(float f, bool updatechanged = true)
    {
        if (bipolar)
            target = 2.f * f - 1.f;
        else
            target = f;
        startingpoint = output;
        if (updatechanged)
            changed = true;
    }

    virtual float get_output01(int i) override
    {
        if (bipolar)
            return 0.5f + 0.5f * output;
        return output;
    }

    virtual float get_target01()
    {
        if (bipolar)
            return 0.5f + 0.5f * target;
        return target;
    }

    virtual bool has_changed(bool reset)
    {
        if (changed)
        {
            if (reset)
                changed = false;
            return true;
        }
        return false;
    }

    virtual void reset() override
    {
        target = 0.f;
        output = 0.f;
        bipolar = false;
    }
    inline void processSmoothing(SmoothingMode mode, float sigma)
    {
        if (mode == LEGACY || mode == SLOW_EXP || mode == FAST_EXP)
        {
            float b = fabs(target - output);
            if (b < sigma && mode != LEGACY)
            {
                output = target;
            }
            else
            {
                float a = (mode == FAST_EXP ? 0.99f : 0.9f) * 44100 * samplerate_inv * b;
                output = (1 - a) * output + a * target;
            }
            return;
        };
        if (mode == FAST_LINE)
        {
            /*
             * Apply a constant change until we get there.
             * Rate is set so we cover the entire range (0,1)
             * in 50 blocks at 44k
             */
            float sampf = samplerate / 44100;
            float da = (target - startingpoint) / (50 * sampf);
            float b = target - output;
            if (fabs(b) < fabs(da))
            {
                output = target;
            }
            else
            {
                output += da;
            }
        }
        if (mode == DIRECT)
        {
            output = target;
        }
    }
    virtual void process_block() override
    {
        processSmoothing(smoothingMode, smoothingMode == FAST_EXP ? 0.005f : 0.0025f);
    }

    virtual bool process_block_until_close(float sigma)
    {
        if (smoothingMode == LEGACY)
            processSmoothing(SLOW_EXP, sigma);
        else
            processSmoothing(smoothingMode, sigma);

        return (output != target); // continue
    }

    virtual bool is_bipolar() override { return bipolar; }
    virtual void set_bipolar(bool b) override { bipolar = b; }

    float target, startingpoint;
    int id; // can be used to assign the controller to a parameter id
    bool bipolar;
    bool changed;
};

class RandomModulationSource : public ModulationSource
{
  public:
    RandomModulationSource(bool bp) : bipolar(bp)
    {
        std::random_device rd;
        gen = std::minstd_rand(rd());
        if (bp)
        {
            dis = std::uniform_real_distribution<float>(-1.0, 1.0);
            norm = std::normal_distribution<float>(0, 0.33); // stddev
        }
        else
        {
            dis = std::uniform_real_distribution<float>(0.0, 1.0);
            norm = std::normal_distribution<float>(0, 0.33); // stddev
        }
    }

    int get_active_outputs() override
    {
        return 2; // bipolar can't support lognormal obvs
    }

    float get_output(int which) override { return output[which]; }

    virtual void attack() override
    {
        if (bipolar)
        {
            output[0] = dis(gen);
            output[1] = limitpm1(norm(gen));
        }
        else
        {
            output[0] = dis(gen);
            output[1] = limit01(fabs(norm(gen)));
        }
    }

    float output[2];
    bool bipolar{false};
    std::minstd_rand gen;
    std::uniform_real_distribution<float> dis;
    std::normal_distribution<float> norm;
};

class AlternateModulationSource : public ModulationSource
{
  public:
    AlternateModulationSource(bool bp) : state(false)
    {
        if (bp)
            nv = -1;
        else
            nv = 0;

        pv = 1;
    }

    virtual void attack() override
    {
        if (state)
            output = pv;
        else
            output = nv;
        state = !state;
    }

    bool state;
    float nv, pv;
};
