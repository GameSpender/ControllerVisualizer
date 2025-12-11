// CollisionLayers.h
#pragma once

namespace CollisionLayer {
    constexpr int All = -1; // Collide with everything
    constexpr int Player = 1 << 0; // 0b00001
    constexpr int Enemy = 1 << 1; // 0b00010
    constexpr int World = 1 << 2; // 0b00100
    constexpr int Object = 1 << 3; // 0b01000
    constexpr int Projectile = 1 << 4; // 0b10000
}
