#pragma once
#include <array>
#include "server/systems/movement.h"

namespace flix {
/// Move each half through the same full-body wall guard as normal movement.
inline std::array<Vec2, 2> splitterPositions(const Terrain& terrain, Realm realm,
                                          Vec2 centre, double radius) {
    std::array<Vec2, 2> positions{centre, centre};
    stepCollide(terrain, realm, positions[0], Vec2{-radius, 0}, radius, 1.0, true, true);
    stepCollide(terrain, realm, positions[1], Vec2{radius, 0}, radius, 1.0, true, true);
    return positions;
}
} // namespace flix
