#include "SkinColors.h"

namespace Colors
{
namespace AboutPage
{
const Surge::Skin::Color Text("about.page.text", 255, 255, 255),
    ColumnText("about.page.column.text", 255, 144, 0), Link("about.page.link", 45, 134, 254),
    LinkHover("about.page.hover.link", 96, 196, 255);
}

namespace Dialog
{
const Surge::Skin::Color Background("dialog.background", 205, 206, 212),
    Border("dialog.border", 151, 151, 151);

namespace Titlebar
{
const Surge::Skin::Color Background("dialog.titlebar.background", 151, 151, 151),
    Text("dialog.titlebar.text", 255, 255, 255);
}
namespace Button
{
const Surge::Skin::Color Background("dialog.button.background", 227, 227, 227),
    Border("dialog.button.border", 151, 151, 151), Text("dialog.button.text", 0, 0, 0),
    BackgroundHover("dialog.button.hover.background", 241, 187, 114),
    BorderHover("dialog.button.hover.border", 151, 151, 151),
    TextHover("dialog.button.hover.text", 0, 0, 0),
    BackgroundPressed("dialog.button.pressed.background", 128, 73, 0),
    BorderPressed("dialog.button.pressed.border", 151, 151, 151),
    TextPressed("dialog.button.pressed.text", 255, 147, 0);
}
namespace Checkbox
{
const Surge::Skin::Color Background("dialog.checkbox.background", 255, 255, 255),
    Border("dialog.checkbox.border", 0, 0, 0), Tick("dialog.checkbox.tick", 0, 0, 0);
}
namespace Entry
{
const Surge::Skin::Color Background("dialog.textfield.background", 255, 255, 255),
    Border("dialog.textfield.border", 160, 160, 160),
    Focus("dialog.textfield.focus", 170, 170, 230), Text("dialog.textfield.text", 0, 0, 0);
}
namespace Label
{
const Surge::Skin::Color Error("dialog.label.text.error", 255, 0, 0),
    Text("dialog.label.text", 0, 0, 0);
}
} // namespace Dialog

namespace Effect
{
namespace Label
{
const Surge::Skin::Color Text("effect.label.text", 76, 76, 76),
    Separator("effect.label.separator", 106, 106, 106);
}
namespace Preset
{
const Surge::Skin::Color Name("effect.label.presetname", 0, 0, 0);
}
namespace Menu
{
const Surge::Skin::Color Text("effect.menu.text", 0, 0, 0),
    TextHover("effect.menu.text.hover", 0, 0, 0);
}
namespace Grid
{
const Surge::Skin::Color Border("effect.grid.border", 0, 0, 0);

namespace Scene
{
const Surge::Skin::Color Background("effect.grid.scene.background", 255, 255, 255),
    Border("effect.grid.scene.border", 0, 0, 0), Text("effect.grid.scene.text", 0, 0, 0),
    BackgroundHover("effect.grid.scene.background.hover", 212, 212, 212),
    BorderHover("effect.grid.scene.border.hover", 0, 0, 0),
    TextHover("effect.grid.scene.text.hover", 0, 0, 0);
}
namespace Unselected
{
const Surge::Skin::Color Background("effect.grid.unselected.background", 151, 151, 151),
    Border("effect.grid.unselected.border", 0, 0, 0),
    Text("effect.grid.unselected.text", 255, 255, 255),
    BackgroundHover("effect.grid.unselected.background.hover", 192, 192, 192),
    BorderHover("effect.grid.unselected.border.hover", 0, 0, 0),
    TextHover("effect.grid.unselected.text.hover", 255, 255, 255);
}
namespace Selected
{
const Surge::Skin::Color Background("effect.grid.selected.background", 255, 255, 255),
    Border("effect.grid.selected.border", 0, 0, 0), Text("effect.grid.selected.text", 32, 32, 32),
    BackgroundHover("effect.grid.selected.background.hover", 192, 192, 192),
    BorderHover("effect.grid.selected.border.hover", 0, 0, 0),
    TextHover("effect.grid.selected.text.hover", 32, 32, 32);
}
namespace Bypassed
{
const Surge::Skin::Color Background("effect.grid.bypassed.background", 57, 59, 69),
    Border("effect.grid.bypassed.border", 0, 0, 0),
    Text("effect.grid.bypassed.text", 151, 151, 151),
    BackgroundHover("effect.grid.bypassed.background.hover", 96, 98, 108),
    BorderHover("effect.grid.bypassed.border.hover", 0, 0, 0),
    TextHover("effect.grid.bypassed.text.hover", 151, 151, 151);
}
namespace BypassedSelected
{
const Surge::Skin::Color Background("effect.grid.bypassed.selected.background", 57, 59, 69),
    Border("effect.grid.bypassed.selected.border", 0, 0, 0),
    Text("effect.grid.bypassed.selected.text", 255, 255, 255),
    BackgroundHover("effect.grid.bypassed.selected.background.hover", 96, 98, 108),
    BorderHover("effect.grid.bypassed.selected.border.hover", 0, 0, 0),
    TextHover("effect.grid.bypassed.selected.text.hover", 151, 151, 151);
}
} // namespace Grid
} // namespace Effect

namespace FormulaEditor
{
const Surge::Skin::Color Background("formulaeditor.background", 245, 246, 238),
    Highlight("formulaeditor.highlight", 192, 192, 192), Text("formulaeditor.text", 0, 0, 0),
    LineNumBackground("formulaeditor.linenumber.background", 151, 151, 151),
    LineNumText("formulaeditor.linenumber.text", 0, 0, 0);

namespace Lua
{
const Surge::Skin::Color Bracket("formulaeditor.lua.comment", 57, 59, 69),
    Comment("formulaeditor.lua.comment", 151, 151, 151),
    Error("formulaeditor.lua.error", 255, 0, 0),
    Identifier("formulaeditor.lua.identifier", 38, 109, 190),
    Interpunction("formulaeditor.lua.interpunction", 17, 17, 17),
    Keyword("formulaeditor.lua.keyword", 83, 123, 0),
    Number("formulaeditor.lua.number", 203, 75, 22),
    String("formulaeditor.lua.string", 151, 117, 0);
} // namespace Lua
} // namespace FormulaEditor

namespace InfoWindow
{
const Surge::Skin::Color Background("infowindow.background", 255, 255, 255),
    Border("infowindow.border", 0, 0, 0), Text("infowindow.text", 0, 0, 0);

namespace Modulation
{
const Surge::Skin::Color Negative("infowindow.text.modulation.negative", 0, 0, 0),
    Positive("infowindow.text.modulation.positive", 0, 0, 0),
    ValueNegative("infowindow.text.modulation.value.negative", 0, 0, 0),
    ValuePositive("infowindow.text.modulation.value.positive", 0, 0, 0);
}
} // namespace InfoWindow

namespace LFO
{
namespace Title
{
const Surge::Skin::Color Text("lfo.title.text", 0, 0, 0, 192);
}

namespace Type
{
const Surge::Skin::Color Text("lfo.type.unselected.text", 0, 0, 0),
    SelectedText("lfo.type.selected.text", 0, 0, 0),
    SelectedBackground("lfo.type.selected.background", 255, 152, 21);
}
namespace StepSeq
{
const Surge::Skin::Color Background("lfo.stepseq.background", 255, 144, 0),
    ColumnShadow("lfo.stepseq.column.shadow", 109, 109, 125),
    DragLine("lfo.stepseq.drag.line", 221, 221, 255),
    Envelope("lfo.stepseq.envelope", 109, 109, 125),
    TriggerClick("lfo.stepseq.trigger.click", 50, 84, 131), Wave("lfo.stepseq.wave", 255, 255, 255);

namespace Button
{
const Surge::Skin::Color Background("lfo.stepseq.button.background", 227, 227, 227),
    Border("lfo.stepseq.button.border", 151, 151, 151),
    Hover("lfo.stepseq.button.hover", 241, 187, 114), Arrow("lfo.stepseq.button.arrow", 0, 0, 0),
    ArrowHover("lfo.stepseq.button.arrow.hover", 0, 0, 0);
}
namespace InfoWindow
{
const Surge::Skin::Color Background("lfo.stepseq.popup.background", 255, 255, 255),
    Border("lfo.stepseq.popup.border", 0, 0, 0), Text("lfo.stepseq.popup.text", 0, 0, 0);
}
namespace Loop
{
const Surge::Skin::Color Marker("lfo.stepseq.loop.markers", 18, 52, 99),
    PrimaryStep("lfo.stepseq.loop.step.primary", 169, 208, 239),
    SecondaryStep("lfo.stepseq.loop.step.secondary", 154, 191, 224),
    OutsidePrimaryStep("lfo.stepseq.loop.outside.step.primary", 222, 222, 222),
    OutsideSecondaryStep("lfo.stepseq.loop.outside.step.secondary", 208, 208, 208);
}
namespace Step
{
const Surge::Skin::Color Fill("lfo.stepseq.step.fill", 18, 52, 99),
    FillDeactivated("lfo.stepseq.step.fill.deactivated", 46, 134, 255),
    FillOutside("lfo.stepseq.step.fill.outside", 127, 127, 127);
}
} // namespace StepSeq
namespace Waveform
{
const Surge::Skin::Color Background("lfo.waveform.background", 255, 144, 0),
    Bounds("lfo.waveform.bounds", 224, 128, 0), Center("lfo.waveform.center", 224, 128, 0),
    Dots("lfo.waveform.dots", 192, 112, 0), Envelope("lfo.waveform.envelope", 176, 96, 0),
    Wave("lfo.waveform.wave", 0, 0, 0),
    DeactivatedWave("lfo.waveform.wave.deactivated", 0, 0, 0, 96),
    GhostedWave("lfo.waveform.wave.ghosted", 0, 0, 0, 96);

namespace Ruler
{
const Surge::Skin::Color Text("lfo.waveform.ruler.text", 0, 0, 0),
    Ticks("lfo.waveform.ruler.ticks", 0, 0, 0),
    ExtendedTicks("lfo.waveform.ruler.ticks.extended", 224, 128, 0),
    SmallTicks("lfo.waveform.ruler.ticks.small", 176, 96, 0);
}
} // namespace Waveform
} // namespace LFO

namespace Menu
{
const Surge::Skin::Color Name("menu.name", 0, 0, 0), NameHover("menu.name.hover", 60, 20, 0),
    NameDeactivated("menu.name.deactivated", 180, 180, 180), Value("menu.value", 0, 0, 0),
    ValueHover("menu.value.hover", 60, 20, 0),
    ValueDeactivated("menu.value.deactivated", 180, 180, 180);

const Surge::Skin::Color FilterValue("filtermenu.value", 255, 154, 16),
    FilterValueHover("filtermenu.value.hover", 255, 255, 255);
} // namespace Menu

namespace ModSource
{
namespace Unused
{
const Surge::Skin::Color Background("modbutton.unused.fill", 18, 52, 99),
    Border("modbutton.unused.frame", 32, 93, 176),
    BorderHover("modbutton.unused.frame.hover", 144, 216, 255),
    Text("modbutton.unused.text", 46, 134, 254),
    TextHover("modbutton.unused.text.hover", 144, 216, 255);
}
namespace Used
{
const Surge::Skin::Color Background("modbutton.used.fill", 18, 52, 99),
    Border("modbutton.used.frame", 64, 173, 255),
    BorderHover("modbutton.used.frame.hover", 192, 235, 255),
    Text("modbutton.used.text", 64, 173, 255),
    TextHover("modbutton.used.text.hover", 192, 235, 255),
    UsedModHover("modbutton.used.text.usedmod.hover", 154, 225, 95);
}
namespace Selected
{
const Surge::Skin::Color Background("modbutton.selected.fill", 35, 100, 192),
    Border("modbutton.selected.frame", 46, 134, 254),
    BorderHover("modbutton.selected.frame.hover", 192, 235, 255),
    Text("modbutton.selected.text", 18, 52, 99),
    TextHover("modbutton.selected.text.hover", 192, 235, 255),
    UsedModHover("modbutton.selected.text.usedmod.hover", 154, 225, 95);
namespace Used
{
const Surge::Skin::Color Background("modbutton.selected.used.fill", 35, 100, 192),
    Border("modbutton.selected.used.frame", 144, 216, 255),
    BorderHover("modbutton.selected.used.frame.hover", 185, 255, 255),
    Text("modbutton.selected.used.text", 144, 216, 255),
    TextHover("modbutton.selected.used.text.hover", 185, 255, 255),
    UsedModHover("modbutton.selected.used.text.usedmod.hover", 154, 225, 95);
}
} // namespace Selected
namespace Armed
{
const Surge::Skin::Color Background("modbutton.armed.fill", 116, 172, 72),
    Border("modbutton.armed.frame", 173, 255, 107),
    BorderHover("modbutton.armed.frame.hover", 208, 255, 255),
    Text("modbutton.armed.text", 173, 255, 107),
    TextHover("modbutton.armed.text.hover", 208, 255, 255),
    UsedModHover("modbutton.armed.text.usedmod.hover", 46, 134, 254);
}
namespace Macro
{
const Surge::Skin::Color Background("modbutton.macro.slider.background", 32, 93, 176),
    Fill("modbutton.macro.slider.fill", 46, 134, 254),
    CurrentValue("modbutton.macro.slider.currentvalue", 255, 144, 0);

}
} // namespace ModSource

namespace MSEGEditor
{
const Surge::Skin::Color Background("msegeditor.background", 17, 17, 17),
    Curve("msegeditor.curve", 255, 255, 255),
    DeformCurve("msegeditor.curve.deformed", 128, 128, 128),
    CurveHighlight("msegeditor.curve.highlight", 255, 144, 0),
    Panel("msegeditor.panel", 205, 206, 212), Text("msegeditor.panel.text", 0, 0, 0);

namespace Axis
{
const Surge::Skin::Color Line("msegeditor.axis.line", 255, 255, 255, 128),
    Text("msegeditor.axis.text", 180, 180, 180),
    SecondaryText("msegeditor.axis.text.secondary", 180, 180, 180, 192);
}
namespace GradientFill
{
const Surge::Skin::Color StartColor("msegeditor.fill.gradient.start", 255, 144, 0, 128),
    EndColor("msegeditor.fill.gradient.end", 255, 144, 0, 16);
}
namespace Grid
{
const Surge::Skin::Color Primary("msegeditor.grid.primary", 255, 255, 255, 128),
    SecondaryHorizontal("msegeditor.grid.secondary.horizontal", 255, 255, 255, 32),
    SecondaryVertical("msegeditor.grid.secondary.vertical", 255, 255, 255, 24);
}
namespace Loop
{
const Surge::Skin::Color Marker("msegeditor.loop.marker", 255, 147, 0, 144),
    RegionAxis("msegeditor.loop.region.axis", 255, 255, 255, 48),
    RegionFill("msegeditor.loop.region.fill", 32, 32, 32),
    RegionBorder("msegeditor.loop.region.border", 255, 144, 0, 144);
}
namespace NumberField
{
const Surge::Skin::Color Text("msegeditor.numberfield.text", 0, 0, 0),
    TextHover("msegeditor.numberfield.text.hover", 0, 0, 0);
}
} // namespace MSEGEditor

namespace NumberField
{
const Surge::Skin::Color Text("numberfield.text", 0, 0, 0),
    TextHover("numberfield.text.hover", 0, 0, 0);
}

namespace Osc
{
namespace Display
{
const Surge::Skin::Color Bounds("osc.line.bounds", 70, 70, 70),
    Center("osc.line.center", 90, 90, 90), AnimatedWave("osc.waveform.animated", 255, 255, 255),
    Wave("osc.waveform", 255, 144, 0), Dots("osc.waveform.dots", 64, 64, 64);
}
namespace Filename
{
const Surge::Skin::Color Background("osc.wavename.background", 255, 160, 16),
    BackgroundHover("osc.wavename.background.hover", 255, 160, 16),
    Frame("osc.wavename.frame", 255, 160, 16), FrameHover("osc.wavename.frame.hover", 255, 160, 16),
    Text("osc.wavename.text", 0, 0, 0), TextHover("osc.wavename.text.hover", 255, 255, 255);
}
namespace Type
{
// 158 should be 144 but VSTGUI for some reason makes text come off more reddish than it should be
const Surge::Skin::Color Text("osc.type.text", 255, 158, 0),
    TextHover("osc.type.text.hover", 255, 255, 255);
} // namespace Type
} // namespace Osc

namespace Overlay
{
const Surge::Skin::Color Background("editor.overlay.background", 0, 0, 0, 204);
}

namespace PatchBrowser
{
const Surge::Skin::Color Text("patchbrowser.text", 0, 0, 0);
}

namespace Scene
{
namespace PitchBendRange
{
const Surge::Skin::Color Text("scene.pbrange.text", 255, 147, 0),
    TextHover("scene.pbrange.text.hover", 255, 255, 255);
}
namespace SplitPoint
{
const Surge::Skin::Color Text("scene.split_poly.text", 0, 0, 0),
    TextHover("scene.split_poly.text.hover", 0, 0, 0);
}
namespace KeytrackRoot
{
const Surge::Skin::Color Text("scene.keytrackroot.text", 0, 0, 0),
    TextHover("scene.keytrackroot.text.hover", 0, 0, 0);
}
} // namespace Scene

namespace Slider
{
namespace Label
{
const Surge::Skin::Color Light("slider.light.label", 255, 255, 255),
    Dark("slider.dark.label", 0, 0, 0);
}
namespace Modulation
{
const Surge::Skin::Color Positive("slider.modulation.positive", 173, 255, 107),
    Negative("slider.modulation.negative", 173, 255, 107);
}
} // namespace Slider

namespace VuMeter
{
const Surge::Skin::Color Background("vumeter.background", 21, 21, 21),
    Border("vumeter.border", 205, 206, 212);

}
} // namespace Colors
