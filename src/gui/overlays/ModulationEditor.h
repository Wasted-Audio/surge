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

#ifndef SURGE_XT_MODULATIONEDITOR_H
#define SURGE_XT_MODULATIONEDITOR_H

#include "juce_gui_basics/juce_gui_basics.h"

class SurgeGUIEditor;
class SurgeSynthesizer;

namespace Surge
{
namespace Overlays
{
class ModulationListBoxModel;

class ModulationEditor : public juce::Component
{
  public:
    ModulationEditor(SurgeGUIEditor *ed, SurgeSynthesizer *s);
    ~ModulationEditor();

    std::unique_ptr<juce::ListBox> listBox;
    std::unique_ptr<ModulationListBoxModel> listBoxModel;
    std::unique_ptr<juce::TextEditor> textBox;
    SurgeGUIEditor *ed;
    SurgeSynthesizer *synth;

    void paint(juce::Graphics &g) override;
    void resized() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationEditor);
};

} // namespace Overlays
} // namespace Surge

#endif // SURGE_XT_MODULATIONEDITOR_H
