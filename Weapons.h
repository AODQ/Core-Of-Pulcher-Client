#ifndef WEAPONS_H_
#define WEAPONS_H_
#pragma once

enum class Weapon {
  Manshredder, Dopplerbeam, Tornet, Grannibal,
  Zeus, Pericaliya, Volnias, Fetus, PMF, Fists, size
};

enum class Weapon_Status {
  Selected, Unselected, No_Ammo, No_Weapon, size
};

#endif