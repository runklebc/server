// GroupHandler.cpp - didn't actually find GroupHandler.h in the Elysium core

// Method currently in Elysium core which handles switching a player into an empty raid group
// Skeleton for HandleGroupChangeSwapSubGroupOpcode()
void WorldSession::HandleGroupChangeSubGroupOpcode(WorldPacket & recv_data)
{
    std::string name;
    uint8 groupNr;
    recv_data >> name;

    recv_data >> groupNr;

    if (groupNr >= MAX_RAID_SUBGROUPS)
        return;

    // we will get correct pointer for group here, so we don't have to check if group is BG raid
    Group *group = GetPlayer()->GetGroup();
    if (!group)
        return;

    /** error handling **/
    if (!group->IsLeader(GetPlayer()->GetObjectGuid()) &&
            !group->IsAssistant(GetPlayer()->GetObjectGuid()))
        return;

    if (!group->HasFreeSlotSubGroup(groupNr))
        return;
    /********************/

    // everything is fine, do it
    if (Player* player = sObjectMgr.GetPlayer(name.c_str()))
        group->ChangeMembersGroup(player, groupNr);
    else
    {
        if (ObjectGuid guid = sObjectMgr.GetPlayerGuidByName(name.c_str()))
            group->ChangeMembersGroup(guid, groupNr);
    }
}

// --------------------------------------------------------------------------------------

// Method to be added to Elysium core which handles swapping players in a raid
void WorldSession::HandleGroupChangeSwapSubGroupOpcode(WorldPacket & recv_data)
{
    std::string name1;
    std::string name2;


    recv_data >> name1;
    recv_data >> name2;

    // we will get correct pointer for group here, so we don't have to check if group is BG raid
    Group *group = GetPlayer()->GetGroup();
    if (!group)
        return;

    /** error handling **/
    if (!group->IsLeader(GetPlayer()->GetObjectGuid()) &&
        !group->IsAssistant(GetPlayer()->GetObjectGuid()))
        return;

    // everything is fine, do it
    if (Player* player1 = sObjectMgr.GetPlayer(name1.c_str()) && 
	Player* player2 = sObjectMgr.GetPlayer(name2.c_str()))
    {
        group->SwapRaidSubgroup(player1, player2);
    }
    else
    {
        if (ObjectGuid guid1 = sObjectMgr.GetPlayerGuidByName(name1.c_str()) &&
	    ObjectGuid guid2 = sObjectMgr.GetPlayerGuidByName(name2.c_str()))
            group->SwapRaidSubgroup(guid1, guid2);
    }
}

// Method to be added to Elysium project that actually does the swapping
void Group::SwapRaidSubGroup(Player *player1, Player *player2)
{
    if (!player1 || !player2 || !isRaidGroup())
        return;

    uint8 groupA = player1->GetSubGroup();
    uint8 groupB = player2->GetSubGroup();
	
    if (groupA == groupB)
        return;

   // player1 should always be going to groupB
   // player2 should always be going to groupA
	
   /* 	not really sure what _setMembersGroup(..) is checking behind the scenes
	looks like there's a line checking "if (slot == m_memberSlots.end()) return false;"
	is this checking to see if the SubGroup we're trying to move to is full? if so this might be a problem.
	
	maybe we call SubGroupCounterDecrease(..) here to trick the core into thinking each 
	SubGroup temporarily has 1 less person in it and don't call it later	
	
	SubGroupCounterDecrease(groupA);
        SubGroupCounterDecrease(groupB);
	
	TODO - decide where to call SubGroupCounterDecrease(..)
   */

    SubGroupCounterDecrease(groupA);
    SubGroupCounterDecrease(groupB);

    if (_setMembersGroup(player1->GetObjectGuid(), groupB) &&
	_setMembersGroup(player2->GetObjectGuid(), groupA))
    {
        if (player1->GetGroup() == this && player2->GetGroup() == this)
	{
            player1->GetGroupRef().setSubGroup(groupB);
	    player2->GetGroupRef().setSubGroup(groupA);
	}
        //if player is in BG raid, it is possible that he is also in normal raid - and that normal raid is stored in m_originalGroup reference
        else
        {
            groupA = player1->GetOriginalSubGroup();
	    groupB = player2->GetOriginalSubGroup();
			
            player1->GetOriginalGroupRef().setSubGroup(groupB);
	    player2->GetOriginalGroupRef().setSubGroup(groupA);
        }
		
	/* TODO - decide where to call SubGroupCounterDecrease(..)
	   we should call this on both groups so we don't try to put 6 players in a SubGroup
	   if we decide to call these two lines earlier in the method we shouldn't call them here	   
	*/
        //SubGroupCounterDecrease(groupA);
	//SubGroupCounterDecrease(groupB);	    

	// Not sure EXACTLY what SendUpdate() does but I'm hoping that nothing 
	// happens until this is called, because I don't want to put 6 people in groupB on accident
        SendUpdate();
    }
}

/**************************************************************************************/
// Opcodes.cpp line 693

// Change from
/*0x280*/  StoreOpcode(CMSG_GROUP_SWAP_SUB_GROUP,         "CMSG_GROUP_SWAP_SUB_GROUP",        STATUS_NEVER,     PACKET_PROCESS_MAX_TYPE,      &WorldSession::Handle_NULL);

// Change to
/*0x280*/  StoreOpcode(CMSG_GROUP_SWAP_SUB_GROUP,         "CMSG_GROUP_SWAP_SUB_GROUP",        STATUS_NEVER,     PACKET_PROCESS_MAX_TYPE,      &WorldSession::HandleGroupChangeSwapSubGroupOpcode);
