////////////////////////////////////////////////////////////////////////////////
//    Scorched3D (c) 2000-2004
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

#include <sky/Sky.h>
#include <landscapemap/LandscapeMaps.h>
#include <landscapedef/LandscapeDefn.h>
#include <client/ScorchedClient.h>

Sky::Sky()
{
}

Sky::~Sky()
{
}

void Sky::drawBackdrop()
{
	LandscapeDefinitionCache &defnCache = ScorchedClient::instance()->
		getLandscapeMaps().getDefinitions();

	sun_.setLightPosition();
	if (defnCache.getDefn()->roof->getType() == LandscapeDefnType::eRoofCavern)
	{
		roof_.draw();
	}
	else
	{
		dome_.drawBackdrop();
	}
}

void Sky::drawLayers()
{
	LandscapeDefinitionCache &defnCache = ScorchedClient::instance()->
		getLandscapeMaps().getDefinitions();

	if (defnCache.getDefn()->roof->getType() == LandscapeDefnType::eRoofCavern)
	{
	}
	else
	{
		dome_.drawLayers();
	}
}

void Sky::simulate(float frameTime)
{
	LandscapeDefinitionCache &defnCache = ScorchedClient::instance()->
		getLandscapeMaps().getDefinitions();
	if (defnCache.getDefn()->roof->getType() == LandscapeDefnType::eRoofCavern)
	{
	}
	else
	{
		dome_.simulate(frameTime);
	}
}

void Sky::flashSky()
{
	dome_.flash();
}

void Sky::generate()
{
	dome_.generate();
	roof_.generate();
	sun_.generate();
}
