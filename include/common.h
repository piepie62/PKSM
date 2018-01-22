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

#ifndef COMMON_H
#define COMMON_H

#include <3ds.h>
#include <citro3d.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <malloc.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "bank.h"
#include "camera.h"
#include "config.h"
#include "sections/events/events.h"
#include "dex.h"
#include "editor.h"
#include "sections/events/eventlists.h"
#include "fx.h"
#include "game.h"
#include "graphic.h"
#include "hex.h"
#include "hid.h"
#include "http.h"
#include "i18n.h"
#include "../source/memecrypto/source/memecrypto.h"
#include "../source/pp2d/pp2d.h"
#include "../source/quirc/quirc.h"
#include "sections/events/pgf.h"
#include "sections/events/pgt.h"
#include "pkx.h"
#include "save.h"
#include "sections/scripts/scripts.h"
#include "sha256.h"
#include "socket.h"
#include "spi.h"
#include "texman.h"
#include "sections/scripts/unicode.h"
#include "util.h"
#include "sections/events/wcman.h"
#include "sections/events/wcx.h"

struct ArrayUTF32 listMoves;
struct ArrayUTF32 listItems;
struct ArrayUTF32 listSpecies;

struct {
	u8 pkxLength;
	u16 totalSpecies;
	u16 totalMoves;
	u16 totalItems;
	
	u8 maxBalls;
	u8 maxBoxes;
	u16 maxAbilities;
	u16 maxSpecies;
	u16 maxMoveID; 
	u16 maxItemID;
	u8 maxWondercards;
	u8 nicknameLength;
	u32 wondercardLocation;
	u16 wondercardSize;
	u8 pkmnLength;
	u32 pokedex;
	u32 battleBoxes;
	u32 saveOT;
	u32 saveGender;
	u32 saveTID;
	u32 saveSID;
	u32 saveSeed;
	u32 saveLanguage;
	u32 saveRegion;
	u32 consoleRegion;
	u32 saveCountry;
	u32 saveSize;
	u32 boxSize;
} perGameOffsets;

#endif
