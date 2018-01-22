/*  This file is part of PKSM
>	Copyright (C) 2016/2017 Bernardo Giordano
>
>   This program is free software: you can redistribute it and/or modify
>   it under the terms of the GNU General Public License as published by
>   the Free Software Foundation, either version 3 of the License, or
>   (at your option) any later version.
>
>   This program is distributed in the hope that it will be useful,
>   but WITHOUT ANY WARRANTY; without even the implied warranty of
>   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
>   GNU General Public License for more details.
>
>   You should have received a copy of the GNU General Public License
>   along with this program.  If not, see <http://www.gnu.org/licenses/>.
>   See LICENSE for information.
*/

#include "game.h"

int game = 0;

int game_get() {
	return game;
}

void game_set(int value) {
	game = value;
}

bool game_isgen7() {
	return (game == GAME_SUN || game == GAME_MOON || game == GAME_US || game == GAME_UM);
}

bool game_isgen6() {
	return (game == GAME_X || game == GAME_Y || game == GAME_OR || game == GAME_AS);
}

bool game_isgen5() {
	return (game == GAME_B1 || game == GAME_B2 || game == GAME_W1 || game == GAME_W2);
}

bool game_isgen4() {
	return (game == GAME_DIAMOND || game == GAME_PEARL || game == GAME_PLATINUM || game == GAME_HG || game == GAME_SS);
}

bool game_is3DS() {
	return game_isgen6() || game_isgen7();
}

bool game_isDS() {
	return game_isgen5() || game_isgen4();
}

bool game_getisDP() {
	return (game == GAME_DIAMOND || game == GAME_PEARL);
}

bool game_getisPT() {
	return game == GAME_PLATINUM;
}

bool game_getisHGSS() {
	return (game == GAME_HG || game == GAME_SS);
}

bool game_getisBW() {
	return (game == GAME_B1 || game == GAME_W1);
}

bool game_getisB2W2() {
	return (game == GAME_B2 || game == GAME_W2);
}

bool game_getisXY() {
	return (game == GAME_X || game == GAME_Y);
}

bool game_getisORAS() {
	return (game == GAME_OR || game == GAME_AS);
}

bool game_getisSUMO() {
	return (game == GAME_SUN || game == GAME_MOON);
}

bool game_getisUSUM() {
	return (game == GAME_US || game == GAME_UM);
}

u8 game_get_country(u8* mainbuf) {
	return *(u8*)(mainbuf + perGameOffsets.saveCountry);
}

u8 game_get_region(u8* mainbuf) {
	return *(u8*)(mainbuf + perGameOffsets.saveRegion);
}

u8 game_get_console_region(u8* mainbuf) {
	return *(u8*)(mainbuf + perGameOffsets.consoleRegion);
}

u8 game_get_language(u8* mainbuf) {
	return *(u8*)(mainbuf + perGameOffsets.saveLanguage);	
}

void game_fill_offsets()
{
	// init
	perGameOffsets.totalSpecies = 807;
	perGameOffsets.pkxLength = 232;
	perGameOffsets.totalMoves = 728;
	perGameOffsets.totalItems = 959;
	
	perGameOffsets.maxBalls = 0;
	perGameOffsets.maxBoxes = 31;
	perGameOffsets.maxAbilities = 0;
	perGameOffsets.maxSpecies = 0;
	perGameOffsets.maxMoveID = 0;
	perGameOffsets.maxItemID = 0;
	perGameOffsets.maxWondercards = 0;
	perGameOffsets.nicknameLength = 26;
	perGameOffsets.wondercardLocation = 0;
	perGameOffsets.wondercardSize = 264;
	perGameOffsets.pkmnLength = 232;
	perGameOffsets.pokedex = 0;
	perGameOffsets.battleBoxes = 0;
	perGameOffsets.saveOT = 0;
	perGameOffsets.saveGender = 0;
	perGameOffsets.saveTID = 0;
	perGameOffsets.saveSID = 0;
	perGameOffsets.saveSeed = 0;
	perGameOffsets.saveLanguage = 0;
	perGameOffsets.saveRegion = 0;
	perGameOffsets.consoleRegion = 0;
	perGameOffsets.saveCountry = 0;
	perGameOffsets.saveSize = 0;
	perGameOffsets.boxSize = 0;
	
	if (game_getisUSUM())
	{
		perGameOffsets.maxBalls = 26;
		perGameOffsets.maxBoxes = 32;
		perGameOffsets.maxAbilities = 233;
		perGameOffsets.maxSpecies = 806; // blocks zeraora, TODO: 807
		perGameOffsets.maxMoveID = 728;
		perGameOffsets.maxItemID = 959;
		perGameOffsets.maxWondercards = 48;
		perGameOffsets.nicknameLength = 26;
		perGameOffsets.wondercardLocation = 0x66300;
		perGameOffsets.wondercardSize = 264;
		perGameOffsets.pkmnLength = 232;
		perGameOffsets.pokedex = 0x2C00;
		perGameOffsets.battleBoxes = 0x50C4;
		perGameOffsets.saveOT = 0x1438;
		perGameOffsets.saveGender = 0x1405;
		perGameOffsets.saveTID = 0x1400;
		perGameOffsets.saveSID = 0x1402;
		perGameOffsets.saveSeed = 0x651DC;
		perGameOffsets.saveLanguage = 0x1435;
		perGameOffsets.saveRegion = 0x142E;
		perGameOffsets.consoleRegion = 0x1434;
		perGameOffsets.saveCountry = 0x142F;
		perGameOffsets.saveSize = 0x6CC00;
		perGameOffsets.boxSize = 0x36600;
	}
	else if (game_getisSUMO())
	{
		perGameOffsets.maxBalls = 26;
		perGameOffsets.maxBoxes = 32;
		perGameOffsets.maxAbilities = 232;
		perGameOffsets.maxSpecies = 802;
		perGameOffsets.maxMoveID = 720;
		perGameOffsets.maxItemID = 920;
		perGameOffsets.maxWondercards = 48;
		perGameOffsets.nicknameLength = 26;
		perGameOffsets.wondercardLocation = 0x65D00;
		perGameOffsets.wondercardSize = 264;
		perGameOffsets.pkmnLength = 232;
		perGameOffsets.pokedex = 0x2A00;
		perGameOffsets.battleBoxes = 0x4CC4;
		perGameOffsets.saveOT = 0x1238;
		perGameOffsets.saveGender = 0x1205;
		perGameOffsets.saveTID = 0x1200;
		perGameOffsets.saveSID = 0x1202;
		perGameOffsets.saveSeed = 0x6B5DC;
		perGameOffsets.saveLanguage = 0x1235;
		perGameOffsets.saveRegion = 0x122E;
		perGameOffsets.consoleRegion = 0x1234;
		perGameOffsets.saveCountry = 0x122F;
		perGameOffsets.saveSize = 0x6BE00;
		perGameOffsets.boxSize = 0x36600;
	}
	else if (game_getisORAS())
	{
		perGameOffsets.maxBalls = 25;
		perGameOffsets.maxBoxes = 31;
		perGameOffsets.maxAbilities = 191;
		perGameOffsets.maxSpecies = 721;
		perGameOffsets.maxMoveID = 621;
		perGameOffsets.maxItemID = 775;
		perGameOffsets.maxWondercards = 24;
		perGameOffsets.nicknameLength = 26;
		perGameOffsets.wondercardLocation = 0x1CD00;
		perGameOffsets.wondercardSize = 264;
		perGameOffsets.pkmnLength = 232;
		perGameOffsets.pokedex = 0x1500;
		//ofs.battleBoxes =
		perGameOffsets.saveOT = 0x14048;
		perGameOffsets.saveGender = 0x14005;
		perGameOffsets.saveTID = 0x14000;
		perGameOffsets.saveSID = 0x14002;
		//ofs.saveSeed = 
		perGameOffsets.saveLanguage = 0x1402D;
		perGameOffsets.saveRegion = 0x14026;
		perGameOffsets.consoleRegion = 0x1402C;
		perGameOffsets.saveCountry = 0x14027;
		perGameOffsets.saveSize = 0x76000;
		perGameOffsets.boxSize = 0x34AD0;
	}
	else if (game_getisXY())
	{
		perGameOffsets.maxBalls = 25;
		perGameOffsets.maxBoxes = 31;
		perGameOffsets.maxAbilities = 188;
		perGameOffsets.maxSpecies = 721;
		perGameOffsets.maxMoveID = 617;
		perGameOffsets.maxItemID = 717;
		perGameOffsets.maxWondercards = 24;
		perGameOffsets.nicknameLength = 26;
		perGameOffsets.wondercardLocation = 0x1BD00;
		perGameOffsets.wondercardSize = 264;
		perGameOffsets.pkmnLength = 232;
		perGameOffsets.pokedex = 0x1500;
		//ofs.battleBoxes =
		perGameOffsets.saveOT = 0x14048;
		perGameOffsets.saveGender = 0x14005;
		perGameOffsets.saveTID = 0x14000;
		perGameOffsets.saveSID = 0x14002;
		//ofs.saveSeed =
		perGameOffsets.saveLanguage = 0x1402D;
		perGameOffsets.saveRegion = 0x14026;
		perGameOffsets.consoleRegion = 0x1402C;
		perGameOffsets.saveCountry = 0x14027;
		perGameOffsets.saveSize = 0x65600;
		perGameOffsets.boxSize = 0x34AD0;
	}
	else if (game_isgen5())
	{
		perGameOffsets.maxBalls = 25;
		perGameOffsets.maxBoxes = 24;
		perGameOffsets.maxAbilities = 164;
		perGameOffsets.maxSpecies = 649;
		perGameOffsets.maxMoveID = 559;
		if (game_getisBW())
			perGameOffsets.maxItemID = 632;
		else if (game_getisB2W2())
			perGameOffsets.maxItemID = 638;
		perGameOffsets.maxWondercards = 12;
		perGameOffsets.nicknameLength = 22; // ?
		perGameOffsets.wondercardLocation = 0x1C900;
		perGameOffsets.wondercardSize = 204;
		perGameOffsets.pkmnLength = 136;
		if (game_getisBW())
			perGameOffsets.pokedex = 0x21600;
		else if (game_getisB2W2())
			perGameOffsets.pokedex = 0x21400;
		if (game_getisBW())
			perGameOffsets.battleBoxes = 0x20a00;
		else if (game_getisB2W2())
			perGameOffsets.battleBoxes = 0x20900;
		perGameOffsets.saveOT = 0x19404;
		perGameOffsets.saveGender = 0x19421;
		perGameOffsets.saveTID = 0x19414;
		perGameOffsets.saveSID = 0x19416;
		// ofs.saveSeed =
		perGameOffsets.saveLanguage = 0x1941e;
		// ofs.saveRegion =
		// ofs.consoleRegion =
		// ofs.saveCountry =
		perGameOffsets.saveSize = 0x80000;
		perGameOffsets.boxSize = 0x18000;
	}
	else if (game_isgen4())
	{
		perGameOffsets.maxBalls = 25;
		//ofs.maxBoxes =
		perGameOffsets.maxAbilities = 123;
		perGameOffsets.maxSpecies = 493;
		//ofs.maxMoveID = 
		//ofs.maxItemID = 
		perGameOffsets.maxWondercards = 8;
		// ofs.nicknameLength =
		// ofs.wondercardLocation =
		perGameOffsets.wondercardSize = 260;
		// ofs.pkmnLength =
		// ofs.pokedex =
		// ofs.battleBoxes =
		// ofs.saveOT =
		// ofs.saveGender =
		// ofs.saveTID =
		// ofs.saveSID =
		// ofs.saveSeed =
		// ofs.saveLanguage =
		// ofs.saveRegion =
		// ofs.consoleRegion =
		// ofs.saveCountry =
		// ofs.saveSize =
		// ofs.boxSize = 		
	}
}
