#pragma once

#include <algorithm>
#include <vector>

#include "server/systems/loot.h"
#include "server/systems/spawning.h"
#include "shared/game/map_elements.h"

namespace flix {

/// Public ground pickups, independent of combat loot and the mob RNG stream.
class PyramidMapPickups {
public:
    static constexpr int kTarget = 24;
    static constexpr double kRefillMillis = 30000.0;
    static constexpr double kLifetimeSeconds = 600.0;

    void run(World& world, const WorldMaps& maps, const Terrain& terrain,
             const SpawnSystem& spawning, LootSystem& loot,
             const ContentRegistry& content, double nowMillis) {
        if (nowMillis < nextRefillMillis_) return;
        nextRefillMillis_ = nowMillis + kRefillMillis;
        const auto petal = content.petalIndex("pyramid_map");
        if (petal == kInvalidIndex) return;
        drops_.erase(std::remove_if(drops_.begin(), drops_.end(), [&](Entity e) {
            return !world.isAlive(e) || !world.has<DropItem>(e);
        }), drops_.end());

        for (int i = 0; i < maps.count(); ++i) {
            const MapData& map = maps.maps()[static_cast<std::size_t>(i)];
            if (map.templateId() != "pyramid") continue;
            const Realm realm = worldRealm(i);
            int count = 0;
            for (Entity e : drops_) {
                if (world.get<Transform>(e).realm == realm) ++count;
            }
            // A bounded search also handles a map with no reachable bands.
            for (int attempt = 0; count < kTarget && attempt < kTarget * 8; ++attempt) {
                Vec2 at;
                if (!spawning.randomBandPoint(terrain, realm, rng_, at)) break;
                const Vec2 safe = terrain.resolveCircle(at, kDropWallRadius, realm);
                if (distanceSq(at, safe) > 1.0 || terrain.outside(at, realm)) continue;
                const bool crowded = std::any_of(drops_.begin(), drops_.end(), [&](Entity e) {
                    const auto& other = world.get<Transform>(e);
                    return other.realm == realm && distanceSq(at, other.position) < 200.0 * 200.0;
                });
                if (crowded) continue;
                const Entity drop = loot.spawnDrop(world, petal, Rarity::Rare, at, realm, {});
                if (drop == NULL_ENTITY) break;
                auto& item = world.get<DropItem>(drop);
                item.singleCopy = true;
                item.humansOnly = true;
                world.get<Lifetime>(drop).remainingSeconds = kLifetimeSeconds;
                drops_.push_back(drop);
                ++count;
            }
        }
    }

private:
    double nextRefillMillis_ = 0.0;
    Rng rng_{0x505952414d4944ULL};
    std::vector<Entity> drops_;
};

} // namespace flix
