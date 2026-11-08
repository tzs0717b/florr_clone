#pragma once
#include <algorithm>
#include <vector>
#include "client/world_view.h"

namespace flix {

inline bool needsTopBossBar(Rarity rarity, double health, bool prestige) {
    if (!(health > 0.0)) return false;
    if (rarity == Rarity::Unique || rarity == Rarity::Apex) return true;
    return rarity == Rarity::Super && !prestige && health < 1.0;
}

/// Keep stable rows, give rare bosses priority, and bound the HUD footprint.
inline void compactBossBars(std::vector<const RemoteEntity*>& bosses, bool prestige,
                            double width, double height) {
    bosses.erase(std::remove_if(bosses.begin(), bosses.end(), [prestige](const RemoteEntity* mob) {
        return !needsTopBossBar(mob->rarity, mob->healthFraction, prestige);
    }), bosses.end());
    std::sort(bosses.begin(), bosses.end(), [](const RemoteEntity* a, const RemoteEntity* b) {
        if (a->rarity != b->rarity) return rarityIndex(a->rarity) > rarityIndex(b->rarity);
        return a->netId < b->netId;
    });
    const std::size_t limit = width < 800.0 || height < 720.0 ? 1 : 2;
    if (bosses.size() > limit) bosses.resize(limit);
}

} // namespace flix
