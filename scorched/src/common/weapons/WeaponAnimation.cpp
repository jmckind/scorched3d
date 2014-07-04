////////////////////////////////////////////////////////////////////////////////
//    Scorched3D (c) 2000-2013
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
//    You should have received a copy of the GNU General Public License along
//    with this program; if not, write to the Free Software Foundation, Inc.,
//    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
////////////////////////////////////////////////////////////////////////////////

#include <weapons/WeaponAnimation.h>
#include <weapons/Accessory.h>
#include <engine/ActionController.h>
#include <common/Defines.h>
#ifndef S3D_SERVER
#include <client/ScorchedClient.h>
#include <uiactions/UIParticleAction.h>
#endif

REGISTER_ACCESSORY_SOURCE(WeaponAnimation);

WeaponAnimation::WeaponAnimation() :
	Weapon("WeaponAnimation", 
		"Used to generate the particle effects like the laser beam death or the small blue ring that flashes when you die.")
{
	addChildXMLEntry("particle", &particles_);
}

WeaponAnimation::~WeaponAnimation()
{

}

void WeaponAnimation::fireWeapon(ScorchedContext &context,
	WeaponFireContext &weaponContext, FixedVector &position, FixedVector &velocity)
{
#ifndef S3D_SERVER
	if (!context.getServerMode())
	{
		UIParticleAction *particleAction = new UIParticleAction(position, particles_);
		ScorchedClient::instance()->getClientUISync().addActionFromClient(particleAction);
	}
#endif // #ifndef S3D_SERVER
}
