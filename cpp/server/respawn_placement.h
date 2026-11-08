#pragma once
#include "server/systems/movement.h"

namespace flix {
/// Jitter by walking from the corpse, never by teleporting to nearby floor
/// on the other side of a wall. Use the restored loadout's actual body radius.
inline Vec2 respawnPosition(const Terrain& terrain, Rng& rng, Realm realm,
                            Vec2 corpse, double bodyRadius, double reach) {
    Vec2 position = terrain.resolveCircle(corpse, bodyRadius, realm);
    const Vec2 offset = rng.insideCircle(reach);
    stepCollide(terrain, realm, position, offset, bodyRadius, 1.0, true, true);
    return position;
}
} // namespace flix
