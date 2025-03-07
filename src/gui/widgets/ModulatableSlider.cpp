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

#include "ModulatableSlider.h"
#include "SurgeGUIEditor.h"
#include "SurgeImageStore.h"
#include "basic_dsp.h"
#include "SurgeGUIUtils.h"
#include "SurgeImage.h"

namespace Surge
{
namespace Widgets
{

ModulatableSlider::MoveRateState ModulatableSlider::sliderMoveRateState =
    ModulatableSlider::MoveRateState::kUnInitialized;
ModulatableSlider::ModulatableSlider() { setRepaintsOnMouseActivity(true); }

ModulatableSlider::~ModulatableSlider() {}

void ModulatableSlider::updateLocationState()
{
    /*
     * The underlying image contains multiple types in a grid for the tray
     */
    trayTypeX = 0;
    trayTypeY = 0;
    if (orientation == ParamConfig::kHorizontal)
    {
        if (isSemitone)
            trayTypeY = 2;
        else if (isBipolarFn())
            trayTypeY = 1;
        if (isLightStyle)
            trayTypeY += 3;
    }
    else
    {
        if (isBipolarFn())
            trayTypeY = 1;
        if (isMiniVertical)
            trayTypeY = 2;
    }

    switch (modulationState)
    {
    case ModulatableControlInterface::UNMODULATED:
        trayTypeX = 0;
        break;
    case ModulatableControlInterface::MODULATED_BY_OTHER:
        trayTypeX = 1;
        break;
    case ModulatableControlInterface::MODULATED_BY_ACTIVE:
        trayTypeX = 2;
        break;
    }

    if (orientation == ParamConfig::kVertical)
    {
        trayPosition = juce::AffineTransform().translated(2, 2);
        trayw = 16;
        trayh = 75;
        range = isMiniVertical ? 39 : 56;

        handleX0 = 0;
        handleY0 = 9;

        handleCX = 7;
        handleMX = handleCX;
        barNMX = handleMX;
        modHandleX = 24;

        handleCY = (1 - quantizedDisplayValue) * range + handleY0;
        // When modulating and this is relevant, value == QDV always
        handleMY = limit01(1 - (value + modValue)) * range + handleY0;
        barNMY = limit01(1 - (value - modValue)) * range + handleY0;
        if (!isModulationBipolar)
            barNMY = handleMY;

        labelRect = juce::Rectangle<int>();
        handleSize = juce::Rectangle<int>().withWidth(15).withHeight(20);
    }
    else
    {
        trayPosition = juce::AffineTransform().translated(2, 5);
        trayw = 133;
        trayh = 14;
        range = 112;

        handleX0 = (trayw - range) * 0.5;
        handleY0 = 0;

        handleCX = range * quantizedDisplayValue + handleX0;
        handleMX = range * limit01(value + modValue) + handleX0;
        barNMX = range * limit01(value - modValue) + handleX0;
        if (!isModulationBipolar)
            barNMX = handleMX;

        handleCY = 6;
        handleMY = handleCY;
        barNMY = handleMY;
        modHandleX = 28;
        drawLabel = true;
        // This calculation is a little dicey but is correct
        labelRect = juce::Rectangle<int>()
                        .withX(handleX0)
                        .withY(trayh / 2)
                        .withHeight(trayh)
                        .withWidth(range)
                        .withTrimmedRight(1)
                        .withTrimmedLeft(1);
        handleSize = juce::Rectangle<int>().withWidth(20).withHeight(15);
    }

    if (skin && skinControl &&
        skin->propertyValue(skinControl, Surge::Skin::Component::HIDE_SLIDER_LABEL, "") == "true")
    {
        drawLabel = false;
    }
}

void ModulatableSlider::paint(juce::Graphics &g)
{
    jassert(pTray);
    if (!pTray)
        return;

    float activationOpacity = 1.0;
    if ((hasDeactivatedFn && isDeactivatedFn()) || isDeactivated)
    {
        activationOpacity = 0.35;
    }
    updateLocationState();

    // Draw the tray
    {
        juce::Graphics::ScopedSaveState gs(g);
        auto t = juce::AffineTransform();
        t = t.translated(-trayTypeX * trayw, -trayTypeY * trayh);

        g.addTransform(trayPosition);
        g.reduceClipRegion(0, 0, trayw, trayh);
        pTray->draw(g, activationOpacity, t);
    }

    // Draw the modulation bar
    if (isEditingModulation)
    {
        juce::Graphics::ScopedSaveState gs(g);

        g.addTransform(trayPosition);
        g.setColour(skin->getColor(Colors::Slider::Modulation::Positive));
        g.drawLine(handleCX, handleCY, handleMX, handleMY, 2);
        g.setColour(skin->getColor(Colors::Slider::Modulation::Negative));
        g.drawLine(handleCX, handleCY, barNMX, barNMY, 2);
    }
    // Draw the label
    if (drawLabel)
    {
        juce::Graphics::ScopedSaveState gs(g);
        g.addTransform(trayPosition);
        auto tl = label;
        if (hasLabelFn)
        {
            tl = labelFn();
        }
        // FIXME FONT AND JUSTIFICATION
        g.setFont(font);
        if (isLightStyle)
            g.setColour(skin->getColor(Colors::Slider::Label::Light));
        else
            g.setColour(skin->getColor(Colors::Slider::Label::Dark));
        g.drawText(tl, labelRect, juce::Justification::top | juce::Justification::right);
    }

    // Draw the handles
    {
        juce::Graphics::ScopedSaveState gs(g);
        g.addTransform(trayPosition);

        auto q = handleSize.withCentre(juce::Point<int>(handleCX, handleCY));
        auto moveTo = juce::AffineTransform().translated(q.getTopLeft());
        auto t = juce::AffineTransform().translated(-1, -1);
        g.addTransform(moveTo);
        g.reduceClipRegion(handleSize.expanded(2));
        pHandle->draw(g, activationOpacity, t);

        if (isHovered && pHandleHover)
            pHandleHover->draw(g, activationOpacity, t);

        if (pTempoSyncHandle && isTemposync)
            pTempoSyncHandle->draw(g, activationOpacity, t);

        if (isHovered && isTemposync && pTempoSyncHoverHandle)
            pTempoSyncHoverHandle->draw(g, activationOpacity, t);
    }

    if (isEditingModulation)
    {
        juce::Graphics::ScopedSaveState gs(g);
        g.addTransform(trayPosition);

        auto q = handleSize.withCentre(juce::Point<int>(handleMX, handleMY));
        auto moveTo = juce::AffineTransform().translated(q.getTopLeft());
        auto t = juce::AffineTransform().translated(-1 - modHandleX, -1);
        g.addTransform(moveTo);
        g.reduceClipRegion(handleSize);
        pHandle->draw(g, activationOpacity, t);
    }
}

void ModulatableSlider::onSkinChanged()
{
    if (orientation == ParamConfig::kHorizontal)
    {
        pTray = associatedBitmapStore->getImage(IDB_SLIDER_HORIZ_BG);
        pHandle = associatedBitmapStore->getImage(IDB_SLIDER_HORIZ_HANDLE);
        pHandleHover = associatedBitmapStore->getImageByStringID(
            skin->hoverImageIdForResource(IDB_SLIDER_HORIZ_HANDLE, GUI::Skin::HOVER));
        pTempoSyncHandle =
            associatedBitmapStore->getImageByStringID("TEMPOSYNC_HORIZONTAL_OVERLAY");
        pTempoSyncHoverHandle =
            associatedBitmapStore->getImageByStringID("TEMPOSYNC_HORIZONTAL_HOVER_OVERLAY");
    }
    else
    {
        pTray = associatedBitmapStore->getImage(IDB_SLIDER_VERT_BG);
        pHandle = associatedBitmapStore->getImage(IDB_SLIDER_VERT_HANDLE);
        pHandleHover = associatedBitmapStore->getImageByStringID(
            skin->hoverImageIdForResource(IDB_SLIDER_VERT_HANDLE, GUI::Skin::HOVER));

        pTempoSyncHandle = associatedBitmapStore->getImageByStringID("TEMPOSYNC_VERTICAL_OVERLAY");
        pTempoSyncHoverHandle =
            associatedBitmapStore->getImageByStringID("TEMPOSYNC_VERTICAL_HOVER_OVERLAY");
    }

    if (skinControl.get())
    {
        auto htr = skin->propertyValue(skinControl, Surge::Skin::Component::SLIDER_TRAY);
        if (htr.isJust())
        {
            pTray = associatedBitmapStore->getImageByStringID(htr.fromJust());
        }
        auto hi = skin->propertyValue(skinControl, Surge::Skin::Component::HANDLE_IMAGE);
        if (hi.isJust())
        {
            pHandle = associatedBitmapStore->getImageByStringID(hi.fromJust());
        }
        auto ho = skin->propertyValue(skinControl, Surge::Skin::Component::HANDLE_HOVER_IMAGE);
        if (ho.isJust())
        {
            pHandleHover = associatedBitmapStore->getImageByStringID(ho.fromJust());
        }
        auto ht = skin->propertyValue(skinControl, Surge::Skin::Component::HANDLE_TEMPOSYNC_IMAGE);
        if (ht.isJust())
        {
            pTempoSyncHandle = associatedBitmapStore->getImageByStringID(ht.fromJust());
        }

        auto hth =
            skin->propertyValue(skinControl, Surge::Skin::Component::HANDLE_TEMPOSYNC_HOVER_IMAGE);
        if (hth.isJust())
        {
            pTempoSyncHoverHandle = associatedBitmapStore->getImageByStringID(hth.fromJust());
        }
    }

#if I_HAVE_A_SKIN_API_FOR_THIS_AT_DRAWABLE_LEVEL
    if (!pHandleHover)
        pHandleHover = skin->hoverBitmapOverlayForBackgroundBitmap(
            skinControl, dynamic_cast<CScalableBitmap *>(pHandle), associatedBitmapStore,
            Surge::GUI::Skin::HoverType::HOVER);
#endif
}

void ModulatableSlider::mouseEnter(const juce::MouseEvent &event)
{
    isHovered = true;
    auto sge = firstListenerOfType<SurgeGUIEditor>();
    if (sge)
    {
        sge->sliderHoverStart(getTag());
    }
}

void ModulatableSlider::mouseExit(const juce::MouseEvent &event) { endHover(); }

void ModulatableSlider::endHover()
{
    isHovered = false;
    auto sge = firstListenerOfType<SurgeGUIEditor>();
    if (sge)
    {
        sge->sliderHoverEnd(getTag());
    }
    repaint();
}

void ModulatableSlider::mouseDrag(const juce::MouseEvent &event)
{
    auto p = event.getMouseDownPosition();
    float distance = event.position.getX() - p.getX();
    if (orientation == ParamConfig::kVertical)
        distance = -(event.position.getY() - p.getY());

    if (distance == 0 && editTypeWas == NOEDIT)
        return;

    float dDistance = distance - lastDistance;
    lastDistance = distance;

    if (editTypeWas == NOEDIT)
    {
        notifyBeginEdit();
    }
    editTypeWas = DRAG;
    updateLocationState();

    float dMouse = 1.f / range;
    if (event.mods.isShiftDown())
        dMouse = dMouse * 0.1;

    switch (sliderMoveRateState)
    {
    case kExact:
        break;
    case kSlow:
        dMouse *= 0.3;
        break;
    case kMedium:
        dMouse *= 0.7;
        break;
    case kLegacy:
    default:
        dMouse *= 0.3 * legacyMoveRate;
        break;
    }

    if (isEditingModulation)
    {
        modValue = modValue + dMouse * dDistance;
        modValue = limitpm1(modValue);
    }
    else
    {
        value = value + dMouse * dDistance;
        value = limit01(value);
    }

    notifyValueChanged();
    updateInfowindowContents(isEditingModulation);
    repaint();
}

void ModulatableSlider::mouseDown(const juce::MouseEvent &event)
{
    if (event.mods.isPopupMenu())
    {
        editTypeWas = NOEDIT;
        notifyControlModifierClicked(event.mods);
        return;
    }

    if (!Surge::GUI::showCursor(storage))
    {
        juce::Desktop::getInstance().getMainMouseSource().enableUnboundedMouseMovement(true);
    }

    valueOnMouseDown = value;
    modValueOnMouseDown = modValue;
    lastDistance = 0.f;
    editTypeWas = NOEDIT;
    showInfowindow(isEditingModulation);
}

void ModulatableSlider::mouseUp(const juce::MouseEvent &event)
{
    /*
     * Why "soon" and not "now"? Well JUCE will deliver me a mouse up as first step
     * of a double click... but if ive dragged im not in a doubleclick
     */
    if (editTypeWas == DRAG)
    {
        hideInfowindowNow();
    }
    else
    {
        hideInfowindowSoon();
    }

    if (!Surge::GUI::showCursor(storage))
    {
        juce::Desktop::getInstance().getMainMouseSource().enableUnboundedMouseMovement(false);
        if (editTypeWas == DRAG)
        {
            updateLocationState();
            auto p = juce::Point<float>(handleCX, handleCY);
            if (isEditingModulation)
                p = juce::Point<float>(handleMX, handleMY);
            p = localPointToGlobal(p);
            juce::Desktop::getInstance().getMainMouseSource().setScreenPosition(p);
        }
    }

    if (editTypeWas != DRAG)
    {
        editTypeWas = NOEDIT;
        return;
    }

    if (event.mods.isAltDown())
    {
        if (isEditingModulation)
            modValue = modValueOnMouseDown;
        else
            value = valueOnMouseDown;
        notifyValueChanged();
    }

    notifyEndEdit();
    updateInfowindowContents(isEditingModulation);
    editTypeWas = NOEDIT;
}

void ModulatableSlider::mouseDoubleClick(const juce::MouseEvent &event)
{
    editTypeWas = DOUBLECLICK;

    notifyBeginEdit();
    notifyControlModifierDoubleClicked(event.mods);
    notifyValueChanged();
    notifyEndEdit();

    updateInfowindowContents(isEditingModulation);
    showInfowindowSelfDismiss(isEditingModulation);
    repaint();
}

void ModulatableSlider::mouseWheelMove(const juce::MouseEvent &event,
                                       const juce::MouseWheelDetails &wheel)
{
    /*
     * If I choose based on horiz/vert it only works on trackpads, so just add
     */
    float delta = wheel.deltaX - (wheel.isReversed ? 1 : -1) * wheel.deltaY;

    if (delta == 0)
    {
        return;
    }

    showInfowindowSelfDismiss(isEditingModulation);

    // Revert to classic scrolling behavior if Ctrl is pressed down
    if (intRange && isStepped && !event.mods.isCtrlDown())
    {
        delta = 1.f / intRange * (delta > 0 ? 1 : -1);
    }

    else
    {

#if MAC
        float speed = 1.2;
#else
        float speed = 0.42666;
#endif

        if (event.mods.isShiftDown())
        {
            speed /= 10.f;
        }

        delta *= speed;
    }

    editTypeWas = WHEEL;
    notifyBeginEdit();

    if (isEditingModulation)
    {
        modValue = limitpm1(modValue + delta);
    }
    else
    {
        value = limit01(value + delta);
    }

    notifyValueChanged();
    updateInfowindowContents(isEditingModulation);
    notifyEndEdit();
    repaint();
}

#if SURGE_JUCE_ACCESSIBLE

struct ModulatableSliderAH : public juce::AccessibilityHandler
{
    struct MSValue : public juce::AccessibilityValueInterface
    {
        explicit MSValue(ModulatableSlider *s) : slider(s) {}

        ModulatableSlider *slider;

        bool isReadOnly() const override { return false; }
        double getCurrentValue() const override { return slider->getValue(); }
        void setValue(double newValue) override
        {
            slider->setValue(newValue);
            slider->setQuantitizedDisplayValue(newValue);
            slider->repaint();
            slider->notifyValueChanged();
        }
        virtual juce::String getCurrentValueAsString() const override
        {
            auto sge = slider->firstListenerOfType<SurgeGUIEditor>();
            if (sge)
            {
                return sge->getDisplayForTag(slider->getTag());
            }
            return std::to_string(slider->getValue());
        }
        virtual void setValueAsString(const juce::String &newValue) override
        {
            auto sge = slider->firstListenerOfType<SurgeGUIEditor>();
            if (sge)
            {
                float f = sge->getF01FromString(slider->getTag(), newValue.toStdString());
                setValue(f);
                return;
            }
            setValue(newValue.getDoubleValue());
        }
        AccessibleValueRange getRange() const override { return {{0, 1}, 0.01}; }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MSValue);
    };

    explicit ModulatableSliderAH(ModulatableSlider *s)
        : slider(s), juce::AccessibilityHandler(
                         *s, juce::AccessibilityRole::slider,
                         juce::AccessibilityActions().addAction(
                             juce::AccessibilityActionType::showMenu,
                             [this]() { this->showMenu(); }),
                         AccessibilityHandler::Interfaces{std::make_unique<MSValue>(s)})
    {
    }
    void resetToDefault()
    {
        slider->notifyControlModifierDoubleClicked(juce::ModifierKeys());
        slider->setQuantitizedDisplayValue(slider->getValue());
        slider->repaint();
    }

    void showMenu()
    {
        auto m = juce::ModifierKeys().withFlags(juce::ModifierKeys::rightButtonModifier);
        slider->notifyControlModifierClicked(m);
    }

    ModulatableSlider *slider;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulatableSliderAH);
};

std::unique_ptr<juce::AccessibilityHandler> ModulatableSlider::createAccessibilityHandler()
{
    return std::make_unique<ModulatableSliderAH>(this);
}
#endif

} // namespace Widgets
} // namespace Surge