//Copyright Paul Reiche, Fred Ford. 1992-2002

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "genall.h"
#include "../lander.h"
#include "../planets.h"
#include "../../build.h"
#include "../../comm.h"
#include "../../encount.h"
#include "../../globdata.h"
#include "../../ipdisp.h"
#include "../../nameref.h"
#include "../../setup.h"
#include "../../state.h"
#include "libs/mathlib.h"


static bool GenerateOrz_generatePlanets (SOLARSYS_STATE *solarSys);
static bool GenerateOrz_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);
static COUNT GenerateOrz_generateEnergy (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT whichNode);
static bool GenerateOrz_pickupEnergy (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT whichNode);


const GenerateFunctions generateOrzFunctions = {
	/* .initNpcs         = */ GenerateDefault_initNpcs,
	/* .reinitNpcs       = */ GenerateDefault_reinitNpcs,
	/* .uninitNpcs       = */ GenerateDefault_uninitNpcs,
	/* .generatePlanets  = */ GenerateOrz_generatePlanets,
	/* .generateMoons    = */ GenerateDefault_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GenerateOrz_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateOrz_generateEnergy,
	/* .generateLife     = */ GenerateDefault_generateLife,
	/* .pickupMinerals   = */ GenerateDefault_pickupMinerals,
	/* .pickupEnergy     = */ GenerateOrz_pickupEnergy,
	/* .pickupLife       = */ GenerateDefault_pickupLife,
};


static bool
GenerateOrz_generatePlanets (SOLARSYS_STATE *solarSys)
{
	COUNT angle;

	GenerateDefault_generatePlanets (solarSys);

	if (CurStarDescPtr->Index == ORZ_DEFINED)
	{
		solarSys->PlanetDesc[0].data_index = WATER_WORLD;
		solarSys->PlanetDesc[0].radius = EARTH_RADIUS * 156L / 100;
		solarSys->PlanetDesc[0].NumPlanets = 0;
		angle = ARCTAN (solarSys->PlanetDesc[0].location.x,
				solarSys->PlanetDesc[0].location.y);
		solarSys->PlanetDesc[0].location.x =
				COSINE (angle, solarSys->PlanetDesc[0].radius);
		solarSys->PlanetDesc[0].location.y =
				SINE (angle, solarSys->PlanetDesc[0].radius);
		ComputeSpeed(&solarSys->PlanetDesc[0], FALSE, 1);
	}

	return true;
}

static bool
GenerateOrz_generateOrbital (SOLARSYS_STATE *solarSys, PLANET_DESC *world)
{
	if ((CurStarDescPtr->Index == ORZ_DEFINED
			&& matchWorld (solarSys, world, 0, MATCH_PLANET))
			|| (CurStarDescPtr->Index == TAALO_PROTECTOR_DEFINED
			&& matchWorld (solarSys, world, 1, 2)
			&& !GET_GAME_STATE (TAALO_PROTECTOR)))
	{
		COUNT i;

		if ((CurStarDescPtr->Index == ORZ_DEFINED
				|| !GET_GAME_STATE (TAALO_UNPROTECTED))
				&& ActivateStarShip (ORZ_SHIP, SPHERE_TRACKING))
		{
			NotifyOthers (ORZ_SHIP, IPNL_ALL_CLEAR);
			PutGroupInfo (GROUPS_RANDOM, GROUP_SAVE_IP);
			ReinitQueue (&GLOBAL (ip_group_q));
			assert (CountLinks (&GLOBAL (npc_built_ship_q)) == 0);

			if (CurStarDescPtr->Index == ORZ_DEFINED)
			{
				CloneShipFragment (ORZ_SHIP,
						&GLOBAL (npc_built_ship_q), INFINITE_FLEET);
				SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, 1 << 7);
			}
			else
			{
				for (i = 0; i < 14; ++i)
				{
					CloneShipFragment (ORZ_SHIP,
							&GLOBAL (npc_built_ship_q), 0);
				}
				SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, 1 << 6);
			}
			GLOBAL (CurrentActivity) |= START_INTERPLANETARY;
			InitCommunication (ORZ_CONVERSATION);

			if (GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD))
				return true;

			{
				BOOLEAN OrzSurvivors;

				OrzSurvivors = GetHeadLink (&GLOBAL (npc_built_ship_q))
						&& (CurStarDescPtr->Index == ORZ_DEFINED
						|| !GET_GAME_STATE (TAALO_UNPROTECTED));

				GLOBAL (CurrentActivity) &= ~START_INTERPLANETARY;
				ReinitQueue (&GLOBAL (npc_built_ship_q));
				GetGroupInfo (GROUPS_RANDOM, GROUP_LOAD_IP);

				if (OrzSurvivors)
					return true;

				LockMutex (GraphicsLock);
				RepairSISBorder ();
				UnlockMutex (GraphicsLock);
			}
		}

		SET_GAME_STATE (TAALO_UNPROTECTED, 1);
		if (CurStarDescPtr->Index == TAALO_PROTECTOR_DEFINED)
		{
			LoadStdLanderFont (&solarSys->SysInfo.PlanetInfo);
			solarSys->PlanetSideFrame[1] =
					CaptureDrawable (
					LoadGraphic (TAALO_DEVICE_MASK_PMAP_ANIM));
			solarSys->SysInfo.PlanetInfo.DiscoveryString =
					CaptureStringTable (
					LoadStringTable (TAALO_DEVICE_STRTAB));
		}
		else
		{
			LoadStdLanderFont (&solarSys->SysInfo.PlanetInfo);
			solarSys->PlanetSideFrame[1] =
					CaptureDrawable (LoadGraphic (RUINS_MASK_PMAP_ANIM));
			solarSys->SysInfo.PlanetInfo.DiscoveryString =
					CaptureStringTable (LoadStringTable (RUINS_STRTAB));
		}
	}

	GenerateDefault_generateOrbital (solarSys, world);

	return true;
}

static COUNT
GenerateOrz_generateEnergy (SOLARSYS_STATE *solarSys, PLANET_DESC *world,
		COUNT whichNode)
{
	if (CurStarDescPtr->Index == TAALO_PROTECTOR_DEFINED
			&& matchWorld (solarSys, world, 1, 2))
	{
		// This check is redundant since the retrieval bit will keep the
		// node from showing up again
		if (GET_GAME_STATE (TAALO_PROTECTOR))
		{	// already picked up
			return 0;
		}

		return GenerateDefault_generateArtifact (solarSys, whichNode);
	}

	if (CurStarDescPtr->Index == ORZ_DEFINED
			&& matchWorld (solarSys, world, 0, MATCH_PLANET))
	{
		return GenerateDefault_generateRuins (solarSys, whichNode);
	}

	return 0;
}

static bool
GenerateOrz_pickupEnergy (SOLARSYS_STATE *solarSys, PLANET_DESC *world,
		COUNT whichNode)
{
	if (CurStarDescPtr->Index == TAALO_PROTECTOR_DEFINED
			&& matchWorld (solarSys, world, 1, 2))
	{
		assert (!GET_GAME_STATE (TAALO_PROTECTOR) && whichNode == 0);

		GenerateDefault_landerReport (solarSys);
		SetLanderTakeoff ();

		SET_GAME_STATE (TAALO_PROTECTOR, 1);
		SET_GAME_STATE (TAALO_PROTECTOR_ON_SHIP, 1);

		return true; // picked up
	}

	if (CurStarDescPtr->Index == ORZ_DEFINED
			&& matchWorld (solarSys, world, 0, MATCH_PLANET))
	{
		// Standard ruins report
		GenerateDefault_landerReportCycle (solarSys);
		return false;
	}

	(void) whichNode;
	return false;
}
