/*
** Surge Synthesizer is Free and Open Source Software
**
** Surge is made available under the Gnu General Public License, v3.0
** https://www.gnu.org/licenses/gpl-3.0.en.html
**
** Copyright 2004-2021 by various individuals as described by the Git transaction log
**
** All source at: https://github.com/surge-synthesizer/surge.git
**
** Surge was a commercial product from 2004-2018, with Copyright and ownership
** in that period held by Claes Johanson at Vember Audio. Claes made Surge
** open source in September 2018.
*/

#include "OscillatorBase.h"
#include "SurgeStorage.h"
#include "DSPUtils.h"
#include "SSESincDelayLine.h"
#include "BiquadFilter.h"
#include "OscillatorCommonFunctions.h"
#include <random>

class StringOscillator : public Oscillator
{
  public:
    enum str_params
    {
        str_exciter_mode,
        str_exciter_level,

        str_str1_decay,
        str_str2_decay,
        str_str2_detune,
        str_str_balance,

        str_stiffness,
    };

    enum exciter_modes
    {
        burst_noise,
        burst_pink_noise,
        burst_sine,
        burst_tri,
        burst_ramp,
        burst_square,
        burst_sweep,

        constant_noise,
        constant_pink_noise,
        constant_sine,
        constant_tri,
        constant_ramp,
        constant_square,
        constant_sweep,
        constant_audioin,
    };

    StringOscillator(SurgeStorage *s, OscillatorStorage *o, pdata *p)
        : Oscillator(s, o, p), lp(s), hp(s), noiseLp(s)
    {
        delayLine[0] = std::make_unique<SSESincDelayLine<16384>>();
        delayLine[1] = std::make_unique<SSESincDelayLine<16384>>();
    }

    ~StringOscillator();

    virtual void init(float pitch, bool is_display = false, bool nonzero_drift = true);
    virtual void init_ctrltypes(int scene, int oscnum) { init_ctrltypes(); };
    virtual void init_ctrltypes();
    virtual void init_default_values();
    virtual void process_block(float pitch, float drift = 0.f, bool stereo = false, bool FM = false,
                               float FMdepth = 0.f);

    template <bool FM, exciter_modes mode>
    void process_block_internal(float pitch, float drift, bool stereo, float FMdepth);

    float phase1 = 0, phase2 = 0;

    static constexpr float dustpos = 0.998, dustneg = 1.0 - dustpos;

    lag<float, true> examp, tap[2], t2level, feedback[2], tone, fmdepth;

    std::array<std::unique_ptr<SSESincDelayLine<16384>>, 2> delayLine;
    float priorSample[2] = {0, 0};
    Surge::Oscillator::DriftLFO driftLFO[2];
    Surge::Oscillator::CharacterFilter<float> charFilt;

    std::minstd_rand gen;
    std::uniform_real_distribution<float> urd;

    float dustBuffer[2][BLOCK_SIZE_OS];
    void fillDustBuffer(float tap0, float tap1);

    BiquadFilter lp, hp, noiseLp;
    void configureLpAndHpFromTone();

  private:
    int id_exciterlvl, id_str1decay, id_str2decay, id_str2detune, id_strbalance, id_stiffness;
};
