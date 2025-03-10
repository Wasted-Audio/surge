#pragma once

/*
 * A utility wrapper around loading and drawables and stuff
 */

#include "juce_gui_basics/juce_gui_basics.h"

#include <vector>
#include <map>
#include <atomic>

class SurgeImageStore;

class SurgeImage
{
  public:
    SurgeImage(int resourceId);
    SurgeImage(const std::string &fname);
    ~SurgeImage();

    void addPNGForZoomLevel(const std::string &fname, int zoomLevel);
    void resolvePNGForZoomLevel(int zoomLevel);
    void setPhysicalZoomFactor(int zoomFactor);

    int resourceID = -1;
    std::string fname;

    void draw(juce::Graphics &g, float opacity,
              const juce::AffineTransform &transform = juce::AffineTransform()) const
    {
        juce::Graphics::ScopedSaveState gs(g);
        g.addTransform(scaleAdjustmentTransform());
        internalDrawableResolved()->draw(g, opacity, transform);
    }
    void drawAt(juce::Graphics &g, float x, float y, float opacity) const
    {
        juce::Graphics::ScopedSaveState gs(g);
        g.addTransform(scaleAdjustmentTransform());
        internalDrawableResolved()->drawAt(g, x, y, opacity);
    }
    void drawWithin(juce::Graphics &g, juce::Rectangle<float> destArea,
                    juce::RectanglePlacement placement, float opacity) const
    {
        juce::Graphics::ScopedSaveState gs(g);
        g.addTransform(scaleAdjustmentTransform());
        internalDrawableResolved()->drawWithin(g, destArea, placement, opacity);
    }

    std::unique_ptr<juce::Drawable> createCopy()
    {
        return internalDrawableResolved()->createCopy();
    }

    /*
     * I provide direct access to the drawable but be careful. Things like
     * changing zoom or skins can invalidate the pointer returned from here.
     */
    juce::Drawable *getDrawableButUseWithCaution() { return internalDrawableResolved(); }

  private:
    juce::Drawable *internalDrawableResolved() const;
    juce::AffineTransform scaleAdjustmentTransform() const;

    static std::atomic<int> instances;
    bool adjustForScale{false};
    int resolvedZoomFactor{100};

    /*
     * The zoom 100 bitmap and optional higher resolution bitmaps for zooms
     * map vs unordered is on purpose here - we need this ordered for our zoom search
     */
    std::map<int, std::pair<std::string, std::unique_ptr<SurgeImage>>> pngZooms;
    int currentPhysicalZoomFactor;

    std::unique_ptr<juce::Drawable> drawable;
    juce::Drawable *currentDrawable{nullptr};
};
