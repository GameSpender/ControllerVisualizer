#pragma once

class TeamRules {
public:
    static bool canDamage(int attacker, int target) {
        return attacker != target;  // simplest form
    }
};
