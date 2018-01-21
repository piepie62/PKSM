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

#include "editor.h"

u8 DPActiveFlag[] = {0x20, 0x83, 0xB8, 0xED};

bool isBattleBoxed(u8* mainbuf, int box, int slot)
{
	if (game_isgen7())
	{
		u8 team_lookup[72];
		memcpy(&team_lookup[0], &mainbuf[ofs.battleBoxes], 72);
		for (int i = 0; i < 72; i += 2)
		{
			if ((team_lookup[i] == (u8)slot) && (team_lookup[i + 1] == (u8)box))
			{
				return true;
			}
		}
	}
	
	return false;
}

u32 *getSaveOT(u8* mainbuf, u32* dst) {
	u16 src[ofs.nicknameLength];
	memcpy(src, &mainbuf[ofs.saveOT], ofs.nicknameLength);
	utf16_to_utf32(dst, src, ofs.nicknameLength);
	return dst;
}

u8 getSaveGender(u8* mainbuf) {
    return *(u8*)(mainbuf + ofs.saveGender);
}

u16 getSaveTID(u8* mainbuf) {
    return *(u16*)(mainbuf + ofs.saveTID);
}

u16 getSaveSID(u8* mainbuf) {
    return *(u16*)(mainbuf + ofs.saveSID);
}

u16 getSaveTSV(u8* mainbuf) {
	u16 TID = getSaveTID(mainbuf);
	u16 SID = getSaveSID(mainbuf);
	return (TID ^ SID) >> 4;
}

u32 getSaveSeed(u8* mainbuf, int index) {
    return *(u32*)(mainbuf + ofs.saveSeed + index*0x4);
}

void setWC(u8* mainbuf, u8* wcbuf, int i, int nInjected[]) {
	if (game_is3DS())
	{
		*(mainbuf + ofs.wondercardLocation - 0x100 + i / 8) |= 0x1 << (i % 8);
		memcpy((void*)(mainbuf + ofs.wondercardLocation + nInjected[0] * ofs.wondercardSize), (const void*)wcbuf, ofs.wondercardSize);

		if (game_getisORAS() && i == 2048)
		{
			*(mainbuf + 0x319B8 + 0) = 0xC2;
			*(mainbuf + 0x319B8 + 1) = 0x73;
			*(mainbuf + 0x319B8 + 2) = 0x5D;
			*(mainbuf + 0x319B8 + 3) = 0x22;
		}
	}
	
	else if (game_isgen5())
	{
		u32 seed = *(u32*)(mainbuf + 0x1D290);

		//decrypt
		u16 temp;
		for (int i = 0; i < 0xA90; i += 2) {
			memcpy(&temp, &mainbuf[0x1C800 + i], 2);
			temp ^= (pkx_lcrng(seed) >> 16);
			seed = pkx_lcrng(seed);
			memcpy(&mainbuf[0x1C800 + i], &temp, 2);
		}

		*(mainbuf + ofs.wondercardLocation - 0x100 + i / 8) |= 0x1 << (i & 7);
		memcpy((void*)(mainbuf + ofs.wondercardLocation + nInjected[0] * ofs.wondercardSize), (const void*)wcbuf, ofs.wondercardSize);

		//encrypt
		memcpy(&seed, &mainbuf[0x1D290], sizeof(u32));
		for (int i = 0; i < 0xA90; i += 2) {
			memcpy(&temp, &mainbuf[0x1C800 + i], 2);
			temp ^= (pkx_lcrng(seed) >> 16);
			seed = pkx_lcrng(seed);
			memcpy(&mainbuf[0x1C800 + i], &temp, 2);
		}
	}

	else if (game_isgen4()) {
		int game = game_get();
		int GBO = save_get_GBO();
		
		mainbuf[GBO + 72] = (u8)((mainbuf[GBO + 72] & 0xFE) | 1);
		if (game == GAME_HG || game == GAME_SS) {
			*(mainbuf + 0x9D3C + GBO + (2047 >> 3)) = 0x80;
			memcpy(&mainbuf[0x9E3C + GBO + nInjected[0] * ofs.wondercardSize], wcbuf, ofs.wondercardSize);
		}
		else if (game == GAME_PLATINUM) {
			*(mainbuf + 0xB4C0 + GBO + (2047 >> 3)) = 0x80;
			memcpy(&mainbuf[0xB5C0 + GBO + nInjected[0] * ofs.wondercardSize], wcbuf, ofs.wondercardSize);
		}
		else if (game == GAME_DIAMOND || game == GAME_PEARL) {
			memcpy(&mainbuf[0xA7D0 + GBO + nInjected[0] * 4], &DPActiveFlag[0], 4);
			memcpy(&mainbuf[0xA7FC + GBO + nInjected[0] * ofs.wondercardSize], wcbuf, ofs.wondercardSize);
		}
	}

	nInjected[0] += 1;
	if (nInjected[0] >= ofs.maxWondercards)
		nInjected[0] = 0;
}

void setSaveLanguage(u8* mainbuf, int i) {
	u8 langValues[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x07, 0x08, 0x09, 0x0A};
	memcpy(&mainbuf[ofs.saveLanguage], &langValues[i], sizeof(u8));
}

u8 getSaveLanguage(u8* mainbuf) {
	return mainbuf[ofs.saveLanguage];
}

void parseHexEditor(u8* pkmn, int byteEntry) {
	if (!hax) {
		if (byteEntry == 0x08 || byteEntry == 0x09)
			checkMaxValueBetweenBounds(pkmn, byteEntry, 0x08, 2, ofs.maxSpecies);
		else if (byteEntry == 0x14)
			checkMaxValue(pkmn, byteEntry, pkmn[byteEntry], ofs.maxAbilities - 1);
		else if (byteEntry == 0x15)
			checkMaxValue(pkmn, byteEntry, pkmn[byteEntry], 1);
		else if (byteEntry == 0x1D)
			checkMaxValue(pkmn, byteEntry, pkmn[byteEntry], 1);
		else if (byteEntry == 0x36)
			checkMaxValue(pkmn, byteEntry, pkmn[byteEntry], 2);
		else if (byteEntry == 0x1E || byteEntry == 0x1F || byteEntry == 0x20 || byteEntry == 0x21 || byteEntry == 0x22 || byteEntry == 0x23) {
			int tot = 0;
			for (int i = 0; i < 6; i++)
				tot += pkx_get_ev(pkmn, i);
			if (tot < 510)
				pkmn[byteEntry]++;
		}
		else if (byteEntry == 0x62 || byteEntry == 0x63 || byteEntry == 0x64 || byteEntry == 0x65)
			checkMaxValue(pkmn, byteEntry, pkmn[byteEntry], 39);
		else if (byteEntry == 0x66 || byteEntry == 0x67 || byteEntry == 0x68 || byteEntry == 0x69)
			checkMaxValue(pkmn, byteEntry, pkmn[byteEntry], 2);
		else if (byteEntry == 0xD1 || byteEntry == 0xD4) // year
			checkMaxValue(pkmn, byteEntry, pkmn[byteEntry], 98);
		else if (byteEntry == 0xD2 || byteEntry == 0xD5) // month
			checkMaxValue(pkmn, byteEntry, pkmn[byteEntry], 11);
		else if (byteEntry == 0xD3 || byteEntry == 0xD6) // day; yes, this is shitty as fuck
			checkMaxValue(pkmn, byteEntry, pkmn[byteEntry], 30);
		else if (byteEntry == 0xDD) { // Met Level & OT Gender
			int metLV = pkmn[byteEntry] & 0x7f;
			if (metLV < 100) pkmn[byteEntry]++;
		}
		else if (byteEntry == 0xDE)
			checkMaxValue(pkmn, byteEntry, pkmn[byteEntry], 0x3E);
		else
			pkmn[byteEntry]++;
	} else if (byteEntry == 0x14) // ability needs a dedicated check for hax mode too
			checkMaxValue(pkmn, byteEntry, pkmn[byteEntry], ofs.maxAbilities);
		else 
			pkmn[byteEntry]++;
}

bool generating = false;

bool isGenerating() {
	return generating;
}

void generate(u8* mainbuf, bool isTeam, int box, int currentEntry, int page, int genEntry) {
	generating = true;
	
	FILE *fptr = fopen(game_isgen6() ? "romfs:/misc/living6.bin" : "romfs:/misc/living7.bin", "rt");
	fseek(fptr, 0, SEEK_END);
	u32 size = ftell(fptr);
	u8* livingbuf = (u8*)malloc(size);
	memset(livingbuf, 0, size);
	rewind(fptr);
	fread(livingbuf, size, 1, fptr);
	fclose(fptr);

	memcpy(&mainbuf[pkx_get_save_address((isTeam) ? (game_isgen4() ? 19 : (game_isgen5() ? 25 : 33)) : box, currentEntry)], &livingbuf[(page * 40 + genEntry) * ofs.pkmnLength], ofs.pkmnLength);
	u8 tempkmn[ofs.pkmnLength];
	pkx_get(mainbuf, (isTeam) ? (game_isgen4() ? 19 : (game_isgen5() ? 25 : 33)) : box, currentEntry, tempkmn);
	memcpy(&tempkmn[0xE3], &mainbuf[ofs.saveLanguage], 1); // nats

	// Correct Nickname of current language
	char nick[ofs.nicknameLength];
	memset(nick, 0, ofs.nicknameLength);
	
	pkx_set_tid(tempkmn, PKSM_Configuration.defaultTID);
	pkx_set_sid(tempkmn, PKSM_Configuration.defaultSID);
	memcpy(tempkmn + 0xB0, PKSM_Configuration.defaultOTName, ofs.nicknameLength);

	utf32_to_utf8((unsigned char*)nick, (uint32_t*)listSpecies.items[pkx_get_species(tempkmn)], ofs.nicknameLength);
	nick[ofs.nicknameLength - 1] = '\0';

	if (game_is3DS()) pkx_set_nickname(tempkmn, nick, 0x40);
	else if (game_isDS()) pkx_set_nickname(tempkmn, nick, 0x48);

	// Randomizing the encryption constant
	pkx_reroll_encryption_key(tempkmn);
	
	// set met date
	tempkmn[0xD4] = PKSM_Configuration.defaultYear;
	tempkmn[0xD5] = PKSM_Configuration.defaultMonth;
	tempkmn[0xD6] = PKSM_Configuration.defaultDay;
	
	setDex(mainbuf, tempkmn);
	pkx_set(mainbuf, (isTeam) ? (game_isgen4() ? 19 : (game_isgen5() ? 25 : 33)) : box, currentEntry, tempkmn);

	free(livingbuf);
	generating = false;
}

void pokemonEditor(u8* mainbuf) {
	int game = game_get();
	
	int lookup[] = {0, 1, 2, 4, 5, 3};
	
	int modeFlag = ED_STANDARD;
	bool isTeam = false;
	int speed = 0;
	int box = 0;
	int currentEntry = 0;
	int menuEntry = 0;
	int byteEntry = 0;
	int boxmax = ofs.maxBoxes - 1;
	int touchExecuting = 0;
	int oldEntry = 0;
	
	wchar_t* descriptions[ofs.pkmnLength];
	
	u8* pkmn = (u8*)malloc(ofs.pkmnLength * sizeof(u8));
	int ability = (int)pkx_get_ability_number(pkmn);

	while (aptMainLoop()) {
		hidScanInput();
		touchPosition touch;
		hidTouchRead(&touch);
		
		if (!isTeam) {
			oldEntry = currentEntry;
			calcCurrentEntryMorePages(&currentEntry, &box, boxmax + 1, 29, 6);
			if (oldEntry != currentEntry)
				touchExecuting = currentEntry;
		}
		
		if (hidKeysDown() & KEY_B) 
			break;
		
		if (hidKeysDown() & KEY_X) {
			isTeam = isTeam ? false : true;
			if (isTeam)
				currentEntry = currentEntry > 5 ? 5 : currentEntry;
		}

 		if (hidKeysDown() & KEY_TOUCH) {
			if (touch.px > 210 && touch.px < 320 && touch.py > 0 && touch.py < 210) {
				currentEntry = -1;
				if (!isTeam)
					touchExecuting = -1;
				isTeam = true;
			}
			if (touch.px > 0 && touch.px < 210 && touch.py > 0 && touch.py < 210) {
				currentEntry = -1;
				if (isTeam)
					touchExecuting = -1;
				isTeam = false;
			}
			
			if (touch.px > 7 && touch.px < 23 && touch.py > 17 && touch.py < 37) {
				if (box > 0) 
					box--;
				else if (box == 0) 
					box = boxmax;
			}
			if (touch.px > 185 && touch.px < 201 && touch.py > 17 && touch.py < 37) {
				if (box < boxmax) 
					box++;
				else if (box == boxmax) 
					box = 0;
			}
			
			if (touch.px > 0 && touch.px < 210 && touch.py > 210 && touch.py < 240)
				modeFlag = (modeFlag == ED_STANDARD) ? ED_SEED : ED_STANDARD;
			
			if (touch.px > 280 && touch.px < 318 && touch.py > 210 && touch.py < 240) 
				break;
		}
		
 		if (hidKeysHeld() & KEY_TOUCH) {
			if (!isTeam) {
				int x_start, y_start = 45;	
				for (int i = 0; i < 5; i++) {
					x_start = 4;
					for (int j = 0; j < 6; j++) {
						if ((touch.px > x_start) && (touch.px < (x_start + 34)) && (touch.py > y_start) && (touch.py < (y_start + 30))) {
							currentEntry = i * 6 + j;
							if (touchExecuting == currentEntry + 40)
								touchExecuting += 40;
							else
								touchExecuting = currentEntry;
						}
						x_start += 34;
					}
					y_start += 30;
				}
			} else {
				for (int i = 0, j = 0, k = 0; i < 6; i++) {
					if (touch.px > 214 + j && touch.px < 265 + j && touch.py > 40 + k && touch.py < 85 + k) {
						currentEntry = i;
						if (touchExecuting == currentEntry + 40)
							touchExecuting += 40;
						else
							touchExecuting = currentEntry;
					}
					if (i % 2 == 0) {
						j = 52;
						k += 20;
					} else {
						j = 0;
						k += 25;
					}
				}
			}
		}
		
		// QR injector
		if ((hidKeysHeld() & KEY_L) && (hidKeysHeld() & KEY_R) && game_is3DS())
		{
			init_qr(mainbuf, MODE_PKX);
		}

		if (((hidKeysDown() & KEY_Y) || ((hidKeysDown() & KEY_TOUCH) && touch.px > 240 && touch.px < 276 && touch.py > 210 && touch.py < 240)) && !isTeam) {
			if (!socket_init())
				break;
			
			// work in temporary variable
			int tempVett[2]; // box, currentEntry
			tempVett[0] = box;
			tempVett[1] = currentEntry;

			do {
				hidScanInput();
				calcCurrentEntryMorePages(&tempVett[1], &tempVett[0], boxmax + 1, 29, 6);

				if (hidKeysDown() & KEY_X) {
					u8 toBeChecked[ofs.pkmnLength];
					pkx_get(mainbuf, tempVett[0], tempVett[1], toBeChecked);
					if (pkx_get_species(toBeChecked)) {
						if (!socket_is_legality_address_set())
							socket_set_legality_address(true);
						
						else if (socket_is_legality_address_set())
							processLegality(toBeChecked);
					}
				}
			
				process_pkx(mainbuf, tempVett);
				printPKViewer(mainbuf, pkmn, isTeam, tempVett[1], menuEntry, tempVett[0], ED_OTA, 0, 0);	
			} while (aptMainLoop() && !(hidKeysDown() & KEY_B));
			socket_shutdown();
			
			//swap
			box = tempVett[0];
			currentEntry = tempVett[1];
		}

		if (!(hidKeysDown() & KEY_TOUCH) && !(hidKeysHeld() & KEY_TOUCH) && touchExecuting >= 0 && touchExecuting / 40 == 0)// && !teamChanged)
			touchExecuting += 40;

		if (currentEntry < 0) {
			currentEntry = 0;
			touchExecuting = 0;
		}

		if (((hidKeysDown() & KEY_A) || touchExecuting / 40 == 2) && !isBattleBoxed(mainbuf, box, currentEntry)) {
			touchExecuting = currentEntry;

			pkx_get(mainbuf, (isTeam) ? (game_isgen4() ? 19 : (game_isgen5() ? 25 : 33)) : box, currentEntry, pkmn);
			bool operationDone = false;

			touchExecuting = menuEntry;
			while (aptMainLoop() && (pkx_get_species(pkmn) > 0 && pkx_get_species(pkmn) <= ofs.maxSpecies) && !operationDone && !(hidKeysDown() & KEY_B)) {
				hidScanInput();
				hidTouchRead(&touch);
				oldEntry = menuEntry;
				menuEntry = calcCurrentEntryOneScreen(menuEntry, 4, 1);
				if (oldEntry != menuEntry)
					touchExecuting = menuEntry;
				
				printPKViewer(mainbuf, pkmn, isTeam, currentEntry, menuEntry, box, ED_MENU, 0, 0);
				
				if (hidKeysHeld() & KEY_TOUCH) {
					if (touch.px > 242 && touch.px < 283 && touch.py > 5 && touch.py < 25 && !isTeam && pkx_get_ball(pkmn) != CHERISH_BALL) {
						pkx_set_tid(pkmn, getSaveTID(mainbuf));
						pkx_set_sid(pkmn, getSaveSID(mainbuf));

						pkmn[0xE3] = getSaveLanguage(mainbuf); // nationality
						pkmn[0x93] = 0; // current handler to 0
						pkmn[0xA2] = 0; // held trainer friendship to 0
						
						pkmn[0xE0] = game_get_country(mainbuf);
						pkmn[0xE1] = game_get_region(mainbuf);
						pkmn[0xE2] = game_get_console_region(mainbuf);
						pkmn[0xE3] = game_get_language(mainbuf);

						u32 ot32[ofs.nicknameLength];
						u8 ot[ofs.nicknameLength];
						memset(ot32, 0, ofs.nicknameLength*sizeof(u32));
						memset(ot, 0, ofs.nicknameLength);
						getSaveOT(mainbuf, ot32);
						utf32_to_utf8(ot, ot32, ofs.nicknameLength);

						if (game_is3DS()) pkx_set_nickname(pkmn, (char*)ot, 0xB0);
						else if (game_isDS()) pkx_set_nickname(pkmn, (char*)ot, 0x68);
						pkx_set_ot_gender(pkmn, (getSaveGender(mainbuf)));

						pkx_set(mainbuf, (isTeam) ? (game_isgen4() ? 19 : (game_isgen5() ? 25 : 33)) : box, currentEntry, pkmn);
						operationDone = true;
						break;
					}
					
					if (touch.px > 208 && touch.px < 317) {
						for (int i = 0; i < 5; i++) {
							if (touch.py > 42 + 27 * i + i*4 && touch.py < 69 + 27 * i + i*4) {
								menuEntry = i;
								if (touchExecuting == menuEntry + 40)
									touchExecuting += 40;
								else
									touchExecuting = menuEntry;
							}
						}
					}
				}
				if (!(hidKeysDown() & KEY_TOUCH) && !(hidKeysHeld() & KEY_TOUCH) && touchExecuting / 40 == 0)
					touchExecuting += 40;

				if (((hidKeysDown() & KEY_A) || touchExecuting / 40 == 2) && menuEntry < 4) {
					touchExecuting = menuEntry;
					switch (menuEntry) {
						case 0: {
							while (aptMainLoop() && !operationDone && !isTeam) {
								hidScanInput();
								touchPosition touch;
								hidTouchRead(&touch);

								if (hidKeysDown() & KEY_B) {
									operationDone = true;
									break;
								}
								
								// hex editor
								if ((hidKeysDown() & KEY_TOUCH) && touch.px > 290 && touch.px < 320 && touch.py > 0 && touch.py < 24)  {
									fillSectors(sector);
									fillDescriptions(descriptions);
									hax = false;
									
									while(aptMainLoop() && !(hidKeysDown() & KEY_B)) {
										hidScanInput();
										hidTouchRead(&touch);
										byteEntry = calcCurrentEntryOneScreen(byteEntry, ofs.pkmnLength - 1, 16);

										if (hidKeysDown() & KEY_TOUCH) {
											if (touch.px > 0 && touch.px < 20 && touch.py > 0 && touch.py < 20) pattern[0] = true;
											if (touch.px > 300 && touch.px < 320 && touch.py > 0 && touch.py < 20) pattern[1] = true;
											if (touch.px > 0 && touch.px < 20 && touch.py > 220 && touch.py < 240) pattern[2] = true;
											if (touch.px > 300 && touch.px < 320 && touch.py > 220 && touch.py < 240) pattern[3] = true;
										}
										
										if (pattern[0] && pattern[1] && pattern[2] && pattern[3]) {
											if (!hax)
												fillSectorsHaxMode(sector);
											else
												fillSectors(sector);
											hax = (hax) ? false : true;
											for (int i = 0; i < 4; i++) pattern[i] = false;
										}

										if (sector[byteEntry][1]) {
											if (byteEntry == 0x30 || byteEntry == 0x31 || byteEntry == 0x32 || byteEntry == 0x33 || byteEntry == 0x34 || byteEntry == 0x35 || byteEntry == 0x36) {
												for (int i = 0; i < 8; i++) {
													if ((hidKeysDown() & KEY_TOUCH) && touch.px > 90 && touch.px < 103 && touch.py > 70 + i*17 && touch.py < 83 + i*17 && !(byteEntry == 0x36 && i > 1))
														pkx_set_ribbons(pkmn, byteEntry - 0x30, i, !pkx_get_ribbons(pkmn, byteEntry - 0x30, i));
												}
											}
											if (byteEntry == 0x77) {
												if ((hidKeysDown() & KEY_TOUCH) && touch.px > 90 && touch.px < 103 && touch.py > 70 && touch.py < 83)
													pkx_set_flag(pkmn, 0x77, 7, !pkx_get_nickname_flag(pkmn));
												if ((hidKeysDown() & KEY_TOUCH) && touch.px > 90 && touch.px < 103 && touch.py > 70 + 17 && touch.py < 83 + 17)
													pkx_set_flag(pkmn, 0x77, 6, !pkx_is_egg(pkmn));
											}
											if (byteEntry == 0xDD) {
												if ((hidKeysDown() & KEY_TOUCH) && touch.px > 100 - 3 && touch.px < 100 + 15 && touch.py > 89 - 6 && touch.py < 89 + 14)
													pkx_set_flag(pkmn, 0xdd, 7, !pkx_get_ot_gender(pkmn));
											}
											if (byteEntry == 0xDE) {
												for (int i = 0; i < 6; i++) {
													if ((hidKeysDown() & KEY_TOUCH) && touch.px > 90 && touch.px < 103 && touch.py > 70 + i * 17 && touch.py < 83 + i * 17)
														pkx_set_hti(pkmn, i, !pkx_get_hti(pkmn, i));
												}
											}
											if (byteEntry == 0x1D) {
												if ((hidKeysDown() & KEY_TOUCH) && touch.px > 90 && touch.px < 103 && touch.py > 70 && touch.py < 83)
													pkx_set_flag(pkmn, 0x1D, 0, !((pkmn[byteEntry] & 1) == 1));
											}
										}

										bool downPlus = ((hidKeysDown() & KEY_TOUCH) && touch.px > 247 && touch.px < 264 && touch.py > 31 && touch.py < 49) || (hidKeysDown() & KEY_A);
										bool downMinus = ((hidKeysDown() & KEY_TOUCH) && touch.px > 224 && touch.px < 241 && touch.py > 31 && touch.py < 49) || (hidKeysDown() & KEY_X);
										bool heldPlus = ((hidKeysHeld() & KEY_TOUCH) && touch.px > 247 && touch.px < 264 && touch.py > 31 && touch.py < 49) || (hidKeysHeld() & KEY_A);
										bool heldMinus = ((hidKeysHeld() & KEY_TOUCH) && touch.px > 224 && touch.px < 241 && touch.py > 31 && touch.py < 49) || (hidKeysHeld() & KEY_X);
										
										if (heldMinus && heldPlus)
											speed = 0;
										else if (sector[byteEntry][0] && downMinus) {
											if (pkmn[byteEntry] > 0)
												pkmn[byteEntry]--;
										}
										else if (sector[byteEntry][0] && heldMinus) {
											if (speed < -30 && pkmn[byteEntry] > 0)
												pkmn[byteEntry]--;
											else
												speed--;
										}
										else if (sector[byteEntry][0] && downPlus) {
											if (pkmn[byteEntry] < 0xFF)
												parseHexEditor(pkmn, byteEntry);
										}
										else if (sector[byteEntry][0] && heldPlus) {
											if (speed > 30 && pkmn[byteEntry] < 0xFF)
												parseHexEditor(pkmn, byteEntry);
											else
												speed++;
										}
										else
											speed = 0;
										
										printPKEditor(pkmn, byteEntry, 0, 0, ED_HEX, descriptions);
									}
								}
								
								if (hidKeysDown() & KEY_TOUCH) {
									if (touch.px > 280 && touch.px < 318 && touch.py > 210 && touch.py < 240) 
										break;
			
									if (touch.px > 206 && touch.px < 317 && touch.py > 110 && touch.py < 141) {
										while(aptMainLoop() && !(hidKeysDown() & KEY_B)) {
											hidScanInput();
											touchPosition touch;
											hidTouchRead(&touch);
											
											if (hidKeysDown() & KEY_TOUCH) {
												if (touch.px > 280 && touch.px < 318 && touch.py > 210 && touch.py < 240) 
													break;
												if (touch.px > 291 && touch.px < 316 && touch.py > 175 && touch.py < 187) {
													int hpEntry = 0;
													while(aptMainLoop() && !(hidKeysDown() & KEY_B)) {
														hidScanInput();
														hpEntry = calcCurrentEntryOneScreen(hpEntry, 15, 4);
	
														if (hidKeysDown() & KEY_A) {
															pkx_set_hp_type(pkmn, (u8)hpEntry);
															break;
														}
														
														printPKEditor(pkmn, hpEntry, 0, 0, ED_HIDDENPOWER, descriptions);
													}
												}
											}
											
											if (hidKeysDown() & KEY_TOUCH) {
												for (int i = 0; i < 6; i++) {
													int iv = pkx_get_iv(pkmn, lookup[i]);
													int ev = pkx_get_ev(pkmn, lookup[i]);
													int oldev = ev;
													int tot = 0;
													
													if (touch.px > 96 && touch.px < 109 && touch.py > 49 + i * 20 && touch.py < 62 + i * 20)
														pkx_set_iv(pkmn, (iv > 0) ? iv - 1 : 31, lookup[i]);
													if (touch.px > 139 && touch.px < 152 && touch.py > 49 + i * 20 && touch.py < 62 + i * 20)
														pkx_set_iv(pkmn, (iv < 31) ? iv + 1 : 0, lookup[i]);
													if (touch.px > 177 && touch.px < 190 && touch.py > 49 + i * 20 && touch.py < 62 + i * 20)
														pkx_set_ev(pkmn, (ev > 0) ? ev - 1 : 252, lookup[i]);
													if (touch.px > 218 && touch.px < 231 && touch.py > 49 + i * 20 && touch.py < 62 + i * 20)
														pkx_set_ev(pkmn, (ev < 252) ? ev + 1 : 0, lookup[i]);
													
													for (int i = 0; i < 6; i++)
														tot += pkx_get_ev(pkmn, i);
													if (tot > 510)
														pkx_set_ev(pkmn, oldev, lookup[i]);
												}
											}
											
											else if (hidKeysHeld() & KEY_TOUCH) {
												bool touched = false;

												for (int i = 0; i < 6; i++) {
													if (touch.px > 96 && touch.px < 109 && touch.py > 49 + i * 20 && touch.py < 62 + i * 20) {
														touched = true;
														if (speed < -30 && pkx_get_iv(pkmn, lookup[i]) > 0)
															pkx_set_iv(pkmn, pkx_get_iv(pkmn, lookup[i]) - 1, lookup[i]);
														else
															speed--;
													}
													else if (touch.px > 139 && touch.px < 152 && touch.py > 49 + i * 20 && touch.py < 62 + i * 20) {
														touched = true;
														if (speed > 30 && pkx_get_iv(pkmn, lookup[i]) < 31)
															pkx_set_iv(pkmn, pkx_get_iv(pkmn, lookup[i]) + 1, lookup[i]);
														else
															speed++;
													}
													else if (touch.px > 177 && touch.px < 190 && touch.py > 49 + i * 20 && touch.py < 62 + i * 20) {
														touched = true;
														if (speed < -30 && pkx_get_ev(pkmn, lookup[i]) > 0)
															pkx_set_ev(pkmn, pkx_get_ev(pkmn, lookup[i]) - 1, lookup[i]);
														else
															speed--;
													}
													else if (touch.px > 218 && touch.px < 231 && touch.py > 49 + i * 20 && touch.py < 62 + i * 20) {
														touched = true;
														if (speed > 30 && pkx_get_ev(pkmn, lookup[i]) < 252) {
															int tot = 0;
															for (int i = 0; i < 6; i++)
																tot += pkx_get_ev(pkmn, i);
															if (tot < 510)
																pkx_set_ev(pkmn, pkx_get_ev(pkmn, lookup[i]) + 1, lookup[i]);
														}
														else
															speed++;
													}
												}

												if (!touched)
													speed = 0;
											}
											else
												speed = 0;
											
											printPKEditor(pkmn, 0, 0, 0, ED_STATS, descriptions);
										}
									}
									
									if (touch.px > 180 && touch.px < 195 && touch.py > 51 && touch.py < 63) {
										int natureEntry = 0;
										while(aptMainLoop() && !(hidKeysDown() & KEY_B)) {
											hidScanInput();
											natureEntry = calcCurrentEntryOneScreen(natureEntry, 24, 5);
											
											if (hidKeysDown() & KEY_A) {
												pkx_set_nature(pkmn, natureEntry);
												break;
											}
											
											printPKEditor(pkmn, natureEntry, 0, 0, ED_NATURES, descriptions);
										}
									}
									
									if (touch.px > 0 && touch.px < 26 && touch.py > 0 && touch.py < 20) {
										int ballEntry = 0;
										while(aptMainLoop() && !(hidKeysDown() & KEY_B)) {
											hidScanInput();
											ballEntry = calcCurrentEntryOneScreen(ballEntry, ofs.maxBalls - 1, 6);

											if (hidKeysDown() & KEY_A) {
												pkx_set_ball(pkmn, (u8)ballEntry + 1);
												break;
											}
											
											printPKEditor(pkmn, ballEntry, 0, 0, ED_BALLS, descriptions);
										}
									}
									
									if (touch.px > 227 && touch.px < 300 && touch.py > 24 && touch.py < 92) {
										u16 species = pkx_get_species(pkmn);
										FormData *forms = pkx_get_legal_form_data(species, game);
										if (forms->editable) {
											int numforms = forms->max - forms->min + 1;
											int columns;
											if (numforms <= 16)
												columns = 4;
											else
												columns = 6;
											
											u8 form = pkx_get_form(pkmn);
											int formEntry = form >= forms->min && form <= forms->max ? form - forms->min : 0;
											while(aptMainLoop() && !(hidKeysDown() & KEY_B)) {
												hidScanInput();

												if (hidKeysDown() & KEY_RIGHT)
													if (formEntry + 1 < numforms) 
														formEntry++;
												
												if (hidKeysDown() & KEY_LEFT)
													if (formEntry > 0) 
														formEntry--;
												
												if (hidKeysDown() & KEY_UP)
													if (formEntry >= columns) 
														formEntry -= columns;
												
												if (hidKeysDown() & KEY_DOWN)
													if (formEntry + columns < numforms)
														formEntry += columns;
													
												if (hidKeysDown() & KEY_A) {
													pkx_set_form(pkmn, (u8)(formEntry + forms->min));
													pkx_set_ability(pkmn, ability);
													break;
												}
												
												printPKEditor(pkmn, formEntry, (int)species, 0, ED_FORMS, descriptions);
											}
										}
										free(forms);
									}

									if (touch.px > 180 && touch.px < 195 && touch.py > 71 && touch.py < 83) {
										ability = (ability + 1) % 3;
										pkx_set_ability(pkmn, ability);
									}
									
									if (touch.px > 156 && touch.px < 174 && touch.py > 0 && touch.py < 20) {
										if (pkx_get_gender(pkmn) != 2) 
											pkx_set_gender(pkmn, ((pkx_get_gender(pkmn) == 0) ? 1 : 0));
									}
									
									if (touch.px > 180 && touch.px < 195 && touch.py > 111 && touch.py < 123)
										pkx_set_shiny(pkmn, pkx_is_shiny(pkmn) ? false : true);
									
									if (touch.px > 180 && touch.px < 195 && touch.py > 131 && touch.py < 143) {
										if (!(pkx_get_pokerus(pkmn)))
											pkx_set_pokerus(pkmn);
										else 
											*(pkmn + 0x2B) = 0x00;
									}
									
									if (touch.px > 180 && touch.px < 195 && touch.py > 171 && touch.py < 183) {
										static SwkbdState swkbd;
										char nick[ofs.nicknameLength];
										memset(nick, 0, ofs.nicknameLength);

										SwkbdButton button = SWKBD_BUTTON_NONE;
										swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, 12);
										swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, SWKBD_FILTER_DIGITS | SWKBD_FILTER_AT | SWKBD_FILTER_PERCENT | SWKBD_FILTER_BACKSLASH | SWKBD_FILTER_PROFANITY, 2);
										swkbdSetFeatures(&swkbd, SWKBD_MULTILINE);

										// Fix for swkKeyboard being in UTF8 only (char*)
										i18n_initTextSwkbd(&swkbd, S_EDITOR_TEXT_CANCEL, S_EDITOR_TEXT_SET, S_EDITOR_TEXT_ENTER_NICKNAME_POKEMON);

										button = swkbdInputText(&swkbd, nick, ofs.nicknameLength);
										nick[ofs.nicknameLength - 1] = '\0';

										if (button != SWKBD_BUTTON_NONE) {
											if (game_is3DS()) pkx_set_nickname(pkmn, nick, 0x40);
											else if (game_isDS()) pkx_set_nickname(pkmn, nick, 0x48);
										}
									}
									
									if (touch.px > 180 && touch.px < 195 && touch.py > 151 && touch.py < 163) {
										static SwkbdState swkbd;
										char nick[ofs.nicknameLength];
										memset(nick, 0, ofs.nicknameLength);
										
										SwkbdButton button = SWKBD_BUTTON_NONE;
										swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, 12);
										swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, SWKBD_FILTER_DIGITS | SWKBD_FILTER_AT | SWKBD_FILTER_PERCENT | SWKBD_FILTER_BACKSLASH | SWKBD_FILTER_PROFANITY, 2);
										swkbdSetFeatures(&swkbd, SWKBD_MULTILINE);

										// Fix for swkKeyboard being in UTF8 only (char*)
										i18n_initTextSwkbd(&swkbd, S_EDITOR_TEXT_CANCEL, S_EDITOR_TEXT_SET, S_EDITOR_TEXT_ENTER_TRAINER_NAME);

										button = swkbdInputText(&swkbd, nick, ofs.nicknameLength);
										nick[ofs.nicknameLength - 1] = '\0';

										if (button != SWKBD_BUTTON_NONE) {
											if (game_is3DS()) pkx_set_nickname(pkmn, nick, 0xB0);
											else if (game_isDS()) pkx_set_nickname(pkmn, nick, 0x68);
										}
									}
									
									if (touch.px > 206 && touch.px < 315 && touch.py > 172 && touch.py < 203) {
										pkx_set_as_it_is(mainbuf, (isTeam) ? (game_isgen4() ? 19 : (game_isgen5() ? 25 : 33)) : box, currentEntry, pkmn);
										operationDone = true;
										break;
									}
									
									if (touch.px > 206 && touch.px < 317 && touch.py > 141 && touch.py < 172) {
										int *movesSorted = listMoves.sortedItemsID;
										int moveEntry = 0;
										int entryBottom = 0;
										int page = 0, maxpages = ofs.totalMoves/40 + 1;

										while (aptMainLoop() && !(hidKeysDown() & KEY_B)) {
											hidScanInput();
											touchPosition touch;
											hidTouchRead(&touch);
											calcCurrentEntryMorePagesReversed(&moveEntry, &page, maxpages, 39, 20);

											if (hidKeysHeld() & KEY_TOUCH) {
												if (touch.px > 0 && touch.px < 198 && touch.py > 25 && touch.py < 45)   entryBottom = 0;
												if (touch.px > 0 && touch.px < 198 && touch.py > 45 && touch.py < 65)   entryBottom = 1;
												if (touch.px > 0 && touch.px < 198 && touch.py > 65 && touch.py < 85)   entryBottom = 2;
												if (touch.px > 0 && touch.px < 198 && touch.py > 85 && touch.py < 105)  entryBottom = 3;
												if (touch.px > 0 && touch.px < 198 && touch.py > 129 && touch.py < 149) entryBottom = 4;
												if (touch.px > 0 && touch.px < 198 && touch.py > 149 && touch.py < 169) entryBottom = 5;
												if (touch.px > 0 && touch.px < 198 && touch.py > 169 && touch.py < 189) entryBottom = 6;
												if (touch.px > 0 && touch.px < 198 && touch.py > 189 && touch.py < 209) entryBottom = 7;

												if (touch.px > 280 && touch.px < 318 && touch.py > 210 && touch.py < 240)
													break;
											}

											if (hidKeysDown() & KEY_A) {
												if (movesSorted[moveEntry + page*40] <= ofs.maxMoveID)
												{
													if (entryBottom < 4)
														pkx_set_move(pkmn, movesSorted[moveEntry + page * 40], entryBottom);
													else
														pkx_set_egg_move(pkmn, movesSorted[moveEntry + page * 40], entryBottom - 4);													
												}
											}

											printPKEditor(pkmn, moveEntry, page, entryBottom, ED_MOVES, descriptions);
										}
									}
									
									if (touch.px > 180 && touch.px < 195 && touch.py > 90 && touch.py < 102) {
										int *itemsSorted = listItems.sortedItemsID;
										int itemEntry = 0;
										int page = 0, maxpages = ofs.totalItems/40 + 1;
										
										while (aptMainLoop() && !(hidKeysDown() & KEY_B)) {
											hidScanInput();
											calcCurrentEntryMorePagesReversed(&itemEntry, &page, maxpages, 39, 20);
											
											if (hidKeysDown() & KEY_A && itemsSorted[itemEntry + page * 40] <= ofs.maxItemID) {
												pkx_set_item(pkmn, itemsSorted[itemEntry + page * 40]);
												break;
											}
											
											printPKEditor(pkmn, itemEntry, page, 0, ED_ITEMS, descriptions);
										}
									}
								}
								
								if (hidKeysDown() & KEY_TOUCH) {
									if (touch.px > 180 && touch.px < 193 && touch.py > 29 && touch.py < 42)
										pkx_set_level(pkmn, (pkx_get_level(pkmn) < 100) ? pkx_get_level(pkmn) + 1 : 1);
									
									if (touch.px > 137 && touch.px < 150 && touch.py > 29 && touch.py < 42)
										pkx_set_level(pkmn, (pkx_get_level(pkmn) > 1) ? pkx_get_level(pkmn) - 1 : 100);
									
									if (touch.px > 137 && touch.px < 150 && touch.py > 189 && touch.py < 202) {
										if (pkx_is_egg(pkmn))
											pkx_set_ot_friendship(pkmn, (pkx_get_ot_friendship(pkmn) > 0) ? pkx_get_ot_friendship(pkmn) - 1 : 255);
										else
											pkx_set_friendship(pkmn, (pkx_get_friendship(pkmn) > 0) ? pkx_get_friendship(pkmn) - 1 : 255);
									}

									if (touch.px > 180 && touch.px < 193 && touch.py > 189 && touch.py < 202) {
										if (pkx_is_egg(pkmn))
											pkx_set_ot_friendship(pkmn, (pkx_get_ot_friendship(pkmn) < 255) ? pkx_get_ot_friendship(pkmn) + 1 : 0);
										else
											pkx_set_friendship(pkmn, (pkx_get_friendship(pkmn) < 255) ? pkx_get_friendship(pkmn) + 1 : 0);
									}
								}

								if (hidKeysHeld() & KEY_TOUCH) {
									if (touch.px > 180 && touch.px < 193 && touch.py > 29 && touch.py < 42) {
										if (speed > 30 && pkx_get_level(pkmn) < 100)
											pkx_set_level(pkmn, pkx_get_level(pkmn) + 1);
										else
											speed++;
									}
									else if (touch.px > 137 && touch.px < 150 && touch.py > 29 && touch.py < 42) {
										if (speed < -30 && pkx_get_level(pkmn) > 1)
											pkx_set_level(pkmn, pkx_get_level(pkmn) - 1);
										else
											speed--;
									}
									else if (touch.px > 137 && touch.px < 150 && touch.py > 189 && touch.py < 202) {
										if (pkx_is_egg(pkmn)) {
											if (speed < -30 && pkx_get_ot_friendship(pkmn) > 0)
												pkx_set_ot_friendship(pkmn, pkx_get_ot_friendship(pkmn) - 1);
											else
												speed--;
										}
										else {
											if (speed < -30 && pkx_get_friendship(pkmn) > 0)
												pkx_set_friendship(pkmn, pkx_get_friendship(pkmn) - 1);
											else
												speed--;
										}
									}
									else if (touch.px > 180 && touch.px < 193 && touch.py > 189 && touch.py < 202) {
										if (pkx_is_egg(pkmn)) {
											if (speed > 30 && pkx_get_ot_friendship(pkmn) < 255)
												pkx_set_ot_friendship(pkmn, pkx_get_ot_friendship(pkmn) + 1);
											else
												speed++;
										}
										else {
											if (speed > 30 && pkx_get_friendship(pkmn) < 255)
												pkx_set_friendship(pkmn, pkx_get_friendship(pkmn) + 1);
											else
												speed++;
										}
									}
									else
										speed = 0;
								}
								else
									speed = 0;
								
								printPKEditor(pkmn, 0, 0, 0, ED_BASE, descriptions);
							}
							break;
						}
						case 1 : {
							isTeam = false;
							int cloneEntry = currentEntry;
							while(aptMainLoop() && !operationDone && !(hidKeysDown() & KEY_B)) {
								hidScanInput();
								touchPosition touch;
								hidTouchRead(&touch);
								if (!isTeam)
									calcCurrentEntryMorePages(&cloneEntry, &box, boxmax + 1, 29, 6);

								if (hidKeysDown() & KEY_TOUCH) {
									if (touch.px > 210 && touch.px < 320 && touch.py > 0 && touch.py < 210) {
										cloneEntry = 0;
										isTeam = true;
									}
									if (touch.px > 0 && touch.px < 210 && touch.py > 0 && touch.py < 210) {
										cloneEntry = 0;
										isTeam = false;
									}
									
									if (touch.px > 7 && touch.px < 23 && touch.py > 17 && touch.py < 37) {
										if (box > 0) 
											box--;
										else if (box == 0) 
											box = boxmax;
									}
									if (touch.px > 185 && touch.px < 201 && touch.py > 17 && touch.py < 37) {
										if (box < boxmax) 
											box++;
										else if (box == boxmax) 
											box = 0;
									}
									if (touch.px > 280 && touch.px < 318 && touch.py > 210 && touch.py < 240) 
										break;
								}
			
								if (hidKeysHeld() & KEY_TOUCH) {
									if (!isTeam) {
										int x_start, y_start = 45;
										for (int i = 0; i < 5; i++) {
											x_start = 4;
											for (int j = 0; j < 6; j++) {
												if ((touch.px > x_start) && (touch.px < (x_start + 34)) && (touch.py > y_start) && (touch.py < (y_start + 30)))
													cloneEntry = i * 6 + j;
												x_start += 34;
											}
											y_start += 30;
										}
									} else {
										if (touch.px > 214 && touch.px < 265 && touch.py > 40 && touch.py < 85) cloneEntry = 0;
										if (touch.px > 266 && touch.px < 317 && touch.py > 60 && touch.py < 105) cloneEntry = 1;
										if (touch.px > 214 && touch.px < 265 && touch.py > 85 && touch.py < 130) cloneEntry = 2;
										if (touch.px > 266 && touch.px < 317 && touch.py > 105 && touch.py < 150) cloneEntry = 3;
										if (touch.px > 214 && touch.px < 265 && touch.py > 130 && touch.py < 175) cloneEntry = 4;
										if (touch.px > 266 && touch.px < 317 && touch.py > 150 && touch.py < 195) cloneEntry = 5;
									}
								}
								if ((hidKeysDown() & KEY_A) && !isTeam) {
									pkx_set_as_it_is(mainbuf, box, cloneEntry, pkmn);
									operationDone = true;
									currentEntry = cloneEntry;
									break;
								}

								printPKViewer(mainbuf, pkmn, isTeam, cloneEntry, menuEntry, box, ED_CLONE, 0, 0);
							}
							break;
						}
						case 2 : {
							if (!isTeam && confirmDisp(i18n(S_EDITOR_Q_CONFIRM_RELEASE))) {
								memset(pkmn, 0, ofs.pkmnLength);
								pkx_set_as_it_is(mainbuf, box, currentEntry, pkmn);
								operationDone = true;
							}
							break;
						}
						case 3 : {
							if (!isTeam) {
								int genEntry = ((int)pkx_get_species(pkmn) - 1) % 40;
								int page = ((int)pkx_get_species(pkmn) - 1) / 40, maxpages = ofs.maxSpecies / 40 + 1;
								
								while (aptMainLoop() && !(hidKeysDown() & KEY_B)) {
									hidScanInput();
									calcCurrentEntryMorePages(&genEntry, &page, maxpages, 39, 8);
									
									if (hidKeysDown() & KEY_A) {
										if (page*40 + genEntry < ofs.maxSpecies) {
											generate(mainbuf, isTeam, box, currentEntry, page, genEntry);
											operationDone = true;
										}
										break;
									}
									printPKViewer(mainbuf, pkmn, isTeam, currentEntry, menuEntry, box, ED_GENERATE, genEntry, page);
								}
							}
							break;
						}
					}
				}
				
				if (((hidKeysDown() & KEY_A) || touchExecuting / 40 == 2) && menuEntry == 4) {
					touchExecuting = currentEntry;
					break;
				}
				if (operationDone || (hidKeysDown() & KEY_B)) {
					touchExecuting = currentEntry;
					break;
				}
			}
			if (!pkx_get_species(pkmn) && !isTeam && !operationDone) {
				int genEntry = 0;
				int page = 0, maxpages = ofs.maxSpecies / 40 + 1;
				
				while (aptMainLoop() && !(hidKeysDown() & KEY_B)) {
					hidScanInput();
					calcCurrentEntryMorePages(&genEntry, &page, maxpages, 39, 8);
					
					if (hidKeysDown() & KEY_A) {
						if (page*40 + genEntry < ofs.maxSpecies) {
							generate(mainbuf, isTeam, box, currentEntry, page, genEntry);
							operationDone = true;
						}
						break;
					}
					
					printPKViewer(mainbuf, pkmn, isTeam, currentEntry, menuEntry, box, ED_GENERATE, genEntry, page);
				}
			}
		}

		printPKViewer(mainbuf, pkmn, isTeam, currentEntry, menuEntry, box, modeFlag, 0, 0);
	}
	free(pkmn);
}
