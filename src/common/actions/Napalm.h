////////////////////////////////////////////////////////////////////////////////
//    Scorched3D (c) 2000-2003
//
//    This file is part of Scorched3D.
//
//    Scorched3D is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    Scorched3D is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Scorched3D; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
////////////////////////////////////////////////////////////////////////////////

#if !defined(__INCLUDE_Napalmh_INCLUDE__)
#define __INCLUDE_Napalmh_INCLUDE__

#include <engine/ActionReferenced.h>
#include <weapons/WeaponNapalm.h>
#include <common/Counter.h>
#include <landscapemap/DeformLandscape.h>
#include <list>

class GLTextureSet;
class Napalm : public ActionReferenced
{
public:
	struct NapalmEntry 
	{
		NapalmEntry(int x, int y, int o) : 
			offset(o), posX(x), posY(y) {}

		int offset;
		int posX, posY;
	};

	Napalm(int x, int y, Weapon *weapon, WeaponFireContext &weaponContext);
	virtual ~Napalm();

	virtual void init();
	virtual void simulate(fixed frameTime, bool &remove);
	virtual std::string getActionDetails();

	unsigned int getPlayerId() { return weaponContext_.getPlayerId(); }
	WeaponNapalm *getWeapon() { return weapon_; }

protected:
	int x_, y_;
	WeaponFireContext weaponContext_;
	WeaponNapalm *weapon_;
	Counter counter_;
	GLTextureSet *set_;

	// Not sent by wire
	bool hitWater_;
	fixed totalTime_, hurtTime_;
	fixed napalmTime_;
	fixed allowedNapalmTime_;
	fixed napalmHeight_;
	fixed stepTime_;
	fixed hurtStepTime_;
	fixed hurtPerSecond_;
	fixed groundScorchPer_;
	int effectRadius_;
	std::list<NapalmEntry *> napalmPoints_;

	fixed getHeight(int x, int y);
	void simulateAddStep();
	void simulateRmStep();
	void simulateDamage();

};

#endif