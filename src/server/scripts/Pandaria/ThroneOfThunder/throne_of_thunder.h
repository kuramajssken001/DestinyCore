/*
 * This file is part of the DestinyCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

enum DataTypes
{
    DATA_COUNCIL_OF_ELDERS,
    MAX_DATA
};

enum CreaturesIds
{
    BOSS_COUNCIL_SUL_THE_SANDCRAWLER = 69078,
    BOSS_COUNCIL_FROST_KING_MALAKK = 69131,
    BOSS_COUNCIL_HIGH_PRIESTESS_MARLI = 69132,
    BOSS_COUNCIL_KAZRAJIN = 69134,
    MOB_GARA_JAL = 69135,
    MOB_GARA_JALS_SOUL = 69182,

    NPC_TWISTED_FATE_HELPER = 662204,
};

class notPlayerPredicate
{
public:
    bool operator()(WorldObject* target) const
    {
        return target && (target->GetTypeId() != TYPEID_PLAYER || target->ToPlayer() && target->ToPlayer()->IsGameMaster());
    }
};
