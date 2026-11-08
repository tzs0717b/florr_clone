#pragma once
// Where the chat box sits: the transcript's panel, the line being typed and
// the channel strip over the panel.
//
// Pulled out of App (app_chat.cpp) so the touch HUD's tests can lay the two
// out against each other. They share the bottom-left corner of the screen:
// with a mouse the box hangs from the bottom edge, and with the touch
// controls up it has to stand clear of the stick instead -- the panel is 558
// wide and 300 tall, and left where it is on a phone it covers the stick and
// the approach pad under it, so a thumb going for the stick lands in the
// transcript or on the closed chat slot and opens the keyboard.
//
// The lift is the aqiangei fork's (as AshFlor lifts its chat over its stick):
// the slot goes above the stick's answering ring, and the transcript gets
// whatever height is left above the slot, up to kChatTouchPanelHeight.

#include <algorithm>

#include "shared/core/types.h"

namespace flix::ui {

// The box is pinned to the bottom-left corner, so every height here is
// measured up from the BOTTOM of the window.
//
// The panel the transcript sits on while the box is open. Its left edge is
// the tab strip's too.
inline constexpr double kChatX = 96.0;
inline constexpr double kChatPanelWidth = 558.0;
inline constexpr double kChatPanelUp = 349.0;   ///< top of the panel, from the bottom edge
inline constexpr double kChatPanelDown = 49.0;  ///< bottom of the panel, from the bottom edge

/// The channel strip over the panel.
inline constexpr double kChatTabsUp = 378.0;    ///< top of the strip, from the bottom edge
inline constexpr double kChatTabHeight = 24.0;

/// The input, under the panel's left edge. Its bottom sits 20 up; the panel
/// and the strip sit on top of it, so a change to its height moves them too.
inline constexpr double kChatFieldX = 100.0;
inline constexpr double kChatFieldUp = 44.0;    ///< top of the input, from the bottom edge
inline constexpr double kChatFieldWidth = 257.0;
inline constexpr double kChatFieldHeight = 24.0;

/// With the touch controls up: clear air between the stick's answering ring
/// and the slot, the slot's width, and the most transcript shown above it.
inline constexpr double kChatTouchGap = 12.0;
inline constexpr double kChatTouchFieldWidth = 220.0;
inline constexpr double kChatTouchPanelHeight = 180.0;
/// Below this much room the transcript is not worth a panel: it is drawn
/// without one (zero height), rather than falling back to the desktop spot
/// over the stick.
inline constexpr double kChatTouchMinPanel = 40.0;
/// Nothing is placed closer than this to the top edge.
inline constexpr double kChatTouchTopMargin = 8.0;

struct ChatLayout {
    Rect panel;     ///< the transcript's panel (zero height: no transcript)
    Rect field;     ///< the input slot
    double tabsTop; ///< top of the channel strip, drawn while the box is open
    bool lifted;    ///< placed clear of the touch stick

    /// The whole box as one region -- what a press must land outside of to
    /// close the chat, and inside of not to. The strip counts only while the
    /// box is open.
    Rect region(bool open) const {
        const double top = open ? std::min(tabsTop, panel.y) : panel.y;
        const double bottom = std::max(field.bottom(), panel.bottom());
        return {panel.x, top, std::max(panel.w, field.right() - panel.x), bottom - top};
    }
};

/// The box for a view `viewHeight` tall. `stickHitRadius` is the touch
/// stick's answering radius (MobileControls::stickHitRadius), zero -- the
/// default -- when the touch controls are not up.
inline ChatLayout chatLayout(double viewHeight, Vec2 stickCentre = {},
                             double stickHitRadius = 0.0) {
    ChatLayout out;
    out.panel = {kChatX, viewHeight - kChatPanelUp, kChatPanelWidth,
                 kChatPanelUp - kChatPanelDown};
    out.field = {kChatFieldX, viewHeight - kChatFieldUp, kChatFieldWidth, kChatFieldHeight};
    out.tabsTop = viewHeight - kChatTabsUp;
    out.lifted = false;
    if (stickHitRadius <= 0.0) return out;

    out.lifted = true;
    out.field.w = kChatTouchFieldWidth;
    out.field.y = std::max(kChatTouchTopMargin, stickCentre.y - stickHitRadius - kChatTouchGap -
                                                    out.field.h);
    // The strip sits over the panel, so the panel's top leaves room for it.
    const double panelBottom = out.field.y - 8.0;
    const double ceiling = kChatTouchTopMargin + kChatTabHeight + 4.0;
    const double panelTop = std::max(ceiling, panelBottom - kChatTouchPanelHeight);
    if (panelBottom - panelTop >= kChatTouchMinPanel) {
        out.panel = {kChatX, panelTop, kChatPanelWidth, panelBottom - panelTop};
    } else {
        out.panel = {kChatX, panelBottom, kChatPanelWidth, 0.0};
    }
    out.tabsTop = out.panel.y - 4.0 - kChatTabHeight;
    return out;
}

} // namespace flix::ui
