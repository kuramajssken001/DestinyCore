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

#include "WorldSession.h"
#include "WodGarrison.h"
#include "Creature.h"
#include "ClassHall.h"
#include "GarrisonAI.h"
#include "GarrisonMgr.h"
#include "GarrisonPackets.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Creature.h"
#include "SpellMgr.h"
#include "SpellInfo.h"

void WorldSession::HandleGetGarrisonInfo(WorldPackets::Garrison::GetGarrisonInfo& /*getGarrisonInfo*/)
{
    _player->SendGarrisonInfo();
}

void WorldSession::HandleGarrisonPurchaseBuilding(WorldPackets::Garrison::GarrisonPurchaseBuilding& garrisonPurchaseBuilding)
{
    if (!_player->GetNPCIfCanInteractWith(garrisonPurchaseBuilding.NpcGUID, UNIT_NPC_FLAG_GARRISON_ARCHITECT))
        return;

    if (Garrison* garrison = _player->GetGarrison(GARRISON_TYPE_GARRISON))
        garrison->ToWodGarrison()->PlaceBuilding(garrisonPurchaseBuilding.PlotInstanceID, garrisonPurchaseBuilding.BuildingID);
}

void WorldSession::HandleGarrisonCancelConstruction(WorldPackets::Garrison::GarrisonCancelConstruction& garrisonCancelConstruction)
{
    if (!_player->GetNPCIfCanInteractWith(garrisonCancelConstruction.NpcGUID, UNIT_NPC_FLAG_GARRISON_ARCHITECT))
        return;

    if (Garrison* garrison = _player->GetGarrison(GARRISON_TYPE_GARRISON))
        garrison->ToWodGarrison()->CancelBuildingConstruction(garrisonCancelConstruction.PlotInstanceID);
}

void WorldSession::HandleGarrisonCheckUpgradeable(WorldPackets::Garrison::GarrisonCheckUpgradeable& /*garrisonCheckUpgradeable*/)
{
    bool canUpgrade = false;
    if (Garrison* garrison = _player->GetGarrison(GARRISON_TYPE_GARRISON))
        canUpgrade = garrison->ToWodGarrison()->CanUpgrade(false);

    SendPacket(WorldPackets::Garrison::GarrisonCheckUpgradeableResult(canUpgrade).Write());
}

void WorldSession::HandleGarrisonUpgrade(WorldPackets::Garrison::GarrisonUpgrade& garrisonUpgrade)
{
    if (!_player->GetNPCIfCanInteractWith(garrisonUpgrade.NpcGUID, UNIT_NPC_FLAG_GARRISON_ARCHITECT))
        return;

    if (Garrison* garrison = _player->GetGarrison(GARRISON_TYPE_GARRISON))
    {
        /*bool result = */garrison->ToWodGarrison()->Upgrade();
        //SendPacket(WorldPackets::Garrison::GarrisonUpgradeResult().Write());
    }
}

void WorldSession::HandleGarrisonRequestBlueprintAndSpecializationData(WorldPackets::Garrison::GarrisonRequestBlueprintAndSpecializationData& /*garrisonRequestBlueprintAndSpecializationData*/)
{
    _player->SendGarrisonBlueprintAndSpecializationData();
}

void WorldSession::HandleGarrisonGetBuildingLandmarks(WorldPackets::Garrison::GarrisonGetBuildingLandmarks& /*garrisonGetBuildingLandmarks*/)
{
    if (Garrison* garrison = _player->GetGarrison(GARRISON_TYPE_GARRISON))
        garrison->ToWodGarrison()->SendBuildingLandmarks(_player);
}

void WorldSession::HandleGarrisonOpenMissionNpc(WorldPackets::Garrison::GarrisonOpenMissionNpcClient& garrisonOpenMissionNpcClient)
{
    if (!_player->GetNPCIfCanInteractWith(garrisonOpenMissionNpcClient.NpcGUID, UNIT_NPC_FLAG_GARRISON_MISSION_NPC))
        return;

    GarrisonType garType = GARRISON_TYPE_CLASS_HALL; // Todo : differenciate depending of NPC
    switch (garrisonOpenMissionNpcClient.NpcGUID.GetEntry())
    {
    case 80432:
    case 81546:
        garType = GARRISON_TYPE_GARRISON;
        break;
    default:
        garType = GARRISON_TYPE_CLASS_HALL;
        break;
    }
    Garrison const* garrison = _player->GetGarrison(garType);

    if (!garrison)
        return;

    if (garType == GARRISON_TYPE_CLASS_HALL)
    {
        SendPacket(WorldPackets::Garrison::ShowAdventureMap(garrisonOpenMissionNpcClient.NpcGUID).Write());
    }
    else
    {
        WorldPackets::Garrison::GarrisonOpenMissionNpc garrisonOpenMissionNpc;
        for (auto const& p : garrison->GetMissions())
        {
            garrisonOpenMissionNpc.Missions.push_back(p.first);
        }
        SendPacket(garrisonOpenMissionNpc.Write());
    }
}

void WorldSession::HandleGarrisonRequestScoutingMap(WorldPackets::Garrison::GarrisonRequestScoutingMap& scoutingMap)
{
    WorldPackets::Garrison::GarrisonScoutingMapResult result;
    result.ID = scoutingMap.ID;
    result.Active = true;
    SendPacket(result.Write());
}

void WorldSession::HandleGarrisonStartMission(WorldPackets::Garrison::GarrisonStartMission& startMission)
{
    if (!_player->GetNPCIfCanInteractWith(startMission.NpcGUID, UNIT_NPC_FLAG_GARRISON_MISSION_NPC))
        return;

    GarrMissionEntry const* missionEntry = sGarrMissionStore.LookupEntry(startMission.MissionID);
    if (!missionEntry)
        return;

    Garrison* garrison = _player->GetGarrison(GarrisonType(missionEntry->GarrTypeID));
    if (!garrison)
        return;

    garrison->StartMission(startMission.MissionID, startMission.Followers);
}

void WorldSession::HandleGarrisonCompleteMission(WorldPackets::Garrison::GarrisonCompleteMission& completeMission)
{
    if (!_player->GetNPCIfCanInteractWith(completeMission.NpcGUID, UNIT_NPC_FLAG_GARRISON_MISSION_NPC))
        return;

    GarrMissionEntry const* missionEntry = sGarrMissionStore.LookupEntry(completeMission.MissionID);
    if (!missionEntry)
        return;

    Garrison* garrison = _player->GetGarrison(GarrisonType(missionEntry->GarrTypeID));
    if (!garrison)
        return;

    garrison->CompleteMission(completeMission.MissionID);
}

void WorldSession::HandleGarrisonMissionBonusRoll(WorldPackets::Garrison::GarrisonMissionBonusRoll& missionBonusRoll)
{
    if (!_player->GetNPCIfCanInteractWith(missionBonusRoll.NpcGUID, UNIT_NPC_FLAG_GARRISON_MISSION_NPC))
        return;

    GarrMissionEntry const* missionEntry = sGarrMissionStore.LookupEntry(missionBonusRoll.MissionID);
    if (!missionEntry)
        return;

    Garrison* garrison = _player->GetGarrison(GarrisonType(missionEntry->GarrTypeID));
    if (!garrison)
        return;

    garrison->CalculateMissonBonusRoll(missionBonusRoll.MissionID);
}

void WorldSession::HandleRequestLandingPageShipmentInfoOpcode(WorldPackets::Garrison::GarrisonRequestLandingPageShipmentInfo& /*packet*/)
{
    if (!_player)
        return;

    Garrison* garrison = _player->GetGarrison(GarrisonType(GarrisonType::GARRISON_TYPE_CLASS_HALL));
    if (!garrison)
        return;

    //std::vector<MS::Garrison::GarrisonWorkOrder> l_WorkOrders = garrison->GetWorkOrders();

    WorldPacket l_Data(SMSG_GARRISON_LANDING_PAGE_SHIPMENT_INFO, 1024);

    l_Data << uint32(GarrisonType::GARRISON_TYPE_CLASS_HALL);

    l_Data << uint32(0); //l_WorkOrders.size()
    /*
    for (uint32 l_I = 0; l_I < l_WorkOrders.size(); ++l_I)
    {
        uint32 l_Duration = 0;

        CharShipmentEntry const* l_Entry = sCharShipmentStore.LookupEntry(l_WorkOrders[l_I].ShipmentID);

        if (l_Entry)
            l_Duration = l_Entry->Duration;

        /// @TODO http://www.mmo-champion.com/content/4662-Patch-6-1-Iron-Horde-Scrap-Meltdown-Garrison-Vendor-Rush-Orders-Blue-Posts
        l_Data << uint32(l_WorkOrders[l_I].ShipmentID);
        l_Data << uint64(l_WorkOrders[l_I].DatabaseID);
        l_Data << uint64(garrison->GetBuilding(l_WorkOrders[l_I].PlotInstanceID).FollowerAssigned);
        l_Data << uint32(l_WorkOrders[l_I].CreationTime);
        l_Data << uint32(l_WorkOrders[l_I].CompleteTime - l_WorkOrders[l_I].CreationTime);
        l_Data << uint32(0);                                    ///< Rewarded XP BuildingType
    }
    */
    SendPacket(&l_Data);
}

void WorldSession::HandleGarrisonAssignFollowerToBuilding(WorldPackets::Garrison::GarrisonAssignFollowerToBuilding& /*packet*/)
{ }

void WorldSession::HandleGarrisonGenerateRecruits(WorldPackets::Garrison::GarrisonGenerateRecruits& generateRecruits)
{
    if (!_player)
        return;

    Garrison* garrison = _player->GetGarrison(GarrisonType::GARRISON_TYPE_CLASS_HALL);

    if (!garrison)
        return;

    if (Creature* unit = _player->GetNPCIfCanInteractWith(generateRecruits.NpcGUID, 0))
    {
        _player->Whisper(std::string("CMSG_GARRISON_GENERATE_RECRUITS"), Language::LANG_COMMON, _player);
        // if (unit->ToGarrisonNPCAI()) 
        //     unit->ToGarrisonNPCAI()->SendRecruitmentFollowersGenerated(_player, generateRecruits.AbiltyID ? generateRecruits.AbiltyID : generateRecruits.TraitID, 0, generateRecruits.TraitID ? true : false);
    }
}

void WorldSession::HandleGarrisonRecruitFollower(WorldPackets::Garrison::GarrisonRecruitsFollower& garrisonRecruitsFollower)
{
    if (_player == nullptr)
        return;

    Garrison* garrison = _player->GetGarrison(GarrisonType::GARRISON_TYPE_CLASS_HALL);

    if (!garrison)
        return;

    WorldPackets::Garrison::GarrisonRecruitFollowerResult result;
    std::unordered_map<uint64 /*dbId*/, Garrison::Follower> followers = garrison->GetFollowers();

    if (Creature* unit = _player->GetNPCIfCanInteractWith(garrisonRecruitsFollower.NpcGUID, 0))
    {
        result.resultID = uint32(GarrisonError::GARRISON_SUCCESS);

        for (auto& follower : followers)
        {
            if (follower.second.PacketInfo.GarrFollowerID == garrisonRecruitsFollower.FollowerID)
            {
                result.followers.push_back(follower.second.PacketInfo);
                garrison->AddFollower(garrisonRecruitsFollower.FollowerID);
                //l_Garrison->SetCanRecruitFollower(false);
                //m_Player->SetCharacterWorldState(CharacterWorldStates::GarrisonTavernBoolCanRecruitFollower, 0);
                break;
            }
        }
        SendPacket(result.Write());
    }
}

void WorldSession::HandleGarrisonSetFollowerInactive(WorldPackets::Garrison::GarrisonSetFollowerInactive& garrisonSetFollowerInactive)
{
    if (_player == nullptr)
        return;

    Garrison* garrison = _player->GetGarrison(GarrisonType::GARRISON_TYPE_CLASS_HALL);

    if (!garrison)
        return;

    garrison->ChangeFollowerActivationState(garrisonSetFollowerInactive.followerDBID, !garrisonSetFollowerInactive.desActivate);
}

void WorldSession::HandleGarrisonGetShipmentInfo(WorldPackets::Garrison::GarrisonRequestShipmentInfo& garrisonRequestShipmentInfo)
{
    if (!_player)
        return;

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(garrisonRequestShipmentInfo.NpcGUID, UNIT_NPC_FLAG_SHIPMENT_CRAFTER);
    if (!unit)
        return;

    uint32 l_ShipmentID = 0;
    uint32 l_OrderAvailable = 0;
    uint32 l_PlotInstanceID = 0;
    uint32 l_PendingWorkOrderCount = 0;

    for (uint32 i = 0; i < sCharShipmentStore.GetNumRows(); ++i)
    {
        if (CharShipmentEntry const* charShipmentData = sCharShipmentStore.LookupEntry(i))
        {
            if (charShipmentData->ShipmentContainerID == unit->GetShipmentContainerID())
            {
                l_ShipmentID = charShipmentData->ID;
                l_OrderAvailable = charShipmentData->MaxShipments;
                CharShipmentContainerEntry const* charShipmentContainerData = sCharShipmentContainerStore.LookupEntry(charShipmentData->ShipmentContainerID);

                Garrison* garrison = _player->GetGarrison(GarrisonType(charShipmentContainerData->GarrTypeID));

                if (!garrison || !_player->IsInGarrison())
                    return;
                WorldPacket l_Response(SMSG_GET_SHIPMENT_INFO_RESPONSE, 1024);
                l_Response.WriteBit(true);
                l_Response.FlushBits();
                l_Response << uint32(l_ShipmentID);
                l_Response << uint32(l_OrderAvailable);
                l_Response << uint32(l_PendingWorkOrderCount);
                l_Response << uint32(l_PlotInstanceID);
                SendPacket(&l_Response);
                break;
            }
        }
    }



    //if (unit->AI())
    //    l_ShipmentID = unit->OnShipmentIDRequest();



    /*
    l_PlotInstanceID = garrison->GetCreaturePlotInstanceID(garrisonRequestShipmentInfo.NpcGUID);

    if (!!l_PlotInstanceID)
    {
        l_OrderAvailable = garrison->GetBuildingMaxWorkOrder(l_PlotInstanceID);

        uint32 l_BuildingID = garrison->GetBuilding(l_PlotInstanceID).BuildingID;

        if (l_BuildingID)
        {
            if (unit->AI())
                l_ShipmentID = unit->AI()->OnShipmentIDRequest(_player);

            if (l_ShipmentID == -1) ///< Comparison of integers of different signs: 'uint32' (aka 'unsigned int') and 'int'
                l_ShipmentID = sGarrisonShipmentManager->GetShipmentIDForBuilding(l_BuildingID, _player, false);
        }
    }

    bool l_Success = !!l_ShipmentID && !!l_PlotInstanceID;

    WorldPacket l_Response(SMSG_GET_SHIPMENT_INFO_RESPONSE, 1024);
    l_Response.WriteBit(l_Success);
    l_Response.FlushBits();

    if (l_Success)
    {
        std::vector<MS::Garrison::GarrisonWorkOrder> l_WorkOrders = garrison->GetWorkOrders();

        uint32 l_PendingWorkOrderCount = std::count_if(l_WorkOrders.begin(), l_WorkOrders.end(), [l_PlotInstanceID](const MS::Garrison::GarrisonWorkOrder & p_Order) -> bool
        {
            return p_Order.PlotInstanceID == l_PlotInstanceID;
        });


        l_Response << uint32(l_ShipmentID);
        l_Response << uint32(l_OrderAvailable);
        l_Response << uint32(l_PendingWorkOrderCount);
        l_Response << uint32(l_PlotInstanceID);

        for (uint32 l_I = 0; l_I < l_WorkOrders.size(); ++l_I)
        {
            if (l_WorkOrders[l_I].PlotInstanceID != l_PlotInstanceID)
                continue;

            l_Response << uint32(l_WorkOrders[l_I].ShipmentID);
            l_Response << uint64(l_WorkOrders[l_I].DatabaseID);
            l_Response << uint64(0);                                    ///< 6.1.x FollowerID
            l_Response << uint32(l_WorkOrders[l_I].CreationTime);
            l_Response << uint32(l_WorkOrders[l_I].CompleteTime - l_WorkOrders[l_I].CreationTime);
            l_Response << uint32(0);                                    ///< 6.1.x Rewarded XP

        }
    }
    else
    {
        l_Response << uint32(0);
        l_Response << uint32(0);
        l_Response << uint32(0);
        l_Response << uint32(0);
    }

    SendPacket(&l_Response); */
}

void WorldSession::HandleGarrisonResearchTalent(WorldPackets::Garrison::GarrisonResearchTalent& researchTalent)
{
    printf("HandleGarrisonResearchTalent GarrTalentID=%d \n", researchTalent.GarrTalentID);
    WorldPackets::Garrison::GarrisonResearchTalentResult result;
    result.Result = uint32(GarrisonError::GARRISON_SUCCESS);
    result.GarrTypeId = uint32(GarrisonType::GARRISON_TYPE_CLASS_HALL);
    result.GarrTalentID = researchTalent.GarrTalentID;
    result.StartTime = uint32(2254525440);
    result.Unk1 = uint32(0);
    SendPacket(result.Write());
}

void WorldSession::HandleGarrisonRequestClassSpecCategoryInfo(WorldPackets::Garrison::GarrisonRequestClassSpecCategoryInfo& requestClassSpecCategoryInfo)
{
    printf("HandleGarrisonRequestClassSpecCategoryInfo GarrFollowerTypeId=%d \n", requestClassSpecCategoryInfo.GarrFollowerTypeId);
    WorldPackets::Garrison::GarrisonFollowerCategories result;
    result.GarrFollowerTypeId = requestClassSpecCategoryInfo.GarrFollowerTypeId;
    result.CategoryInfoCount = 0;

    SendPacket(result.Write());
}

void WorldSession::HandleGarrisonCreateShipmentOpcode(WorldPackets::Garrison::GarrisonCreateShipment& createShipment)
{
    printf("HandleGarrisonCreateShipmentOpcode npcId=%d \n", createShipment.NpcGUID.GetEntry());
    if (!createShipment.Count)
        createShipment.Count = 1;
    if (!_player)
        return;

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(createShipment.NpcGUID, UNIT_NPC_FLAG_SHIPMENT_CRAFTER);
    if (!unit)
        return;

    WorldPackets::Garrison::GarrisonCreateShipmentResponse result;
    result.ShipmentID = 0;
    result.ShipmentRecID = 0;
    result.Result = 1;
    SendPacket(result.Write());
}
