#include "SM64DS_2.h"
#include "SM64DS_Common.h"
#include <nds/debug.h>

asm("repl_0202d0f8 = MySafeLoadOverlay");

bool USED = false;

void int_to_string(int num, char* str) {
    if (num == 0) {
        *str = '0';
        *(str + 1) = '\0';
        return;
    }

    int is_negative = num < 0;
    if (is_negative) {
        num = -num;
    }

    int i = 0;
    while (num > 0) {
        int digit = num % 10;
        *(str + i) = digit + '0';
        num /= 10;
        i++;
    }

    if (is_negative) {
        *(str + i) = '-';
        i++;
    }

    *(str + i) = '\0';

    // Reverse the string
    int len = i;
    for (int j = 0; j < len / 2; j++) {
        char temp = *(str + j);
        *(str + j) = *(str + len - j - 1);
        *(str + len - j - 1) = temp;
    }
}





void hook_020e50ac() 
{

	if (PLAYER_ARR[0]->isUnderwater && !USED) {
		USED = true;
		//LoadLevel(0x32,4,1,0);
		//LoadLevelLogic(0xc,0,0,0,'\0');
		// load the boo level without loading screen (i think ?)
		//FUN_0202aaec(6,0,5,6,13);
		
		/*(&LEVEL_OVL_MAP)[6] = (&LEVEL_OVL_MAP)[6]*/
		
		STAR_ID_SELECTED = '\x01';
		PREVIOUS_LEVEL_ID = '\x01';
		RETURN_ENTRANCE_ON_DEAH = '\r';
		LEVEL_ID_TO_LOAD = '\x06';

		// FUN_0202d690(0, 6);
		
		//FUN_0202ded4(6);
	}
}

asm("repl_02018bc4 = MyFSInternalReadFile");
extern "C" int MyFSInternalReadFile(int param_1, int ramAddress, int param_3) {
	char buf[16];
	int_to_string(ramAddress, &buf[0]);
	nocashMessage("MyFSInternalReadFile ");
	nocashMessage(buf);
	nocashMessage("\n");
	
	return FS_Internal_readFile(param_1, ramAddress, param_3);
}

void log_number(char *text, int value) {
	nocashMessage("[INFO] ");
	nocashMessage(text);
	nocashMessage(" ");
	char buf[16];
	int_to_string(value, &buf[0]);
	nocashMessage(buf);
	nocashMessage("\n");
}


void apply_number(char *address, int value, int byte_count) {
	for(char i = 0 ; i < byte_count ; i++) {
		*(address + i) += (value >> 8*i) & 0xFF;
	}
	/**(address) 			= (value >> 0) & 0xFF;  // Extract the least significant byte
	*(address + 0x1) 	= (value >> 8) & 0xFF;  // Extract the second least significant byte
	*(address + 0x2)	= (value >> 16) & 0xFF; // Extract the third least significant byte
	*(address + 0x3) 	= (value >> 24) & 0xFF; // Extract the most significant byte*/
}

int value_by_address(char *address, int byte_count) {
	int sum = 0;
	for(char i = 0 ; i < byte_count ; i++) {
		sum += *(address + i) << 8*i;
	}
	return sum;
	// return *(address + 0) + (*(address + 1) << 8) + (*(address + 2) << 16) + (*(address + 3) << 24);
}

/**
	The overlays have a fixed address in memory, therefore the a lot of things are hardcoded inside them
	We need to update the addresses to take in account the dyanmic address given.
**/
void update_level(char *CURRENT_ADDRESS, int ORIGINAL_ADDRESS) {
	
	
	// 60 4 Address of the CLPS chunk
	int value_by_address_ = value_by_address(CURRENT_ADDRESS+0x60, 4);
	log_number("value_by_address_", value_by_address_);
	int CLPS_address_offset = value_by_address_ - ORIGINAL_ADDRESS; // 34925364 - 34925216 = 148 = 0x94
	log_number("CLPS_address_offset", CLPS_address_offset);
	apply_number(CURRENT_ADDRESS+0x60, (int) CURRENT_ADDRESS + CLPS_address_offset, 4);
	
	if(
		*(CURRENT_ADDRESS+CLPS_address_offset) 		!= 'C' ||
		*(CURRENT_ADDRESS+CLPS_address_offset+1) 	!= 'L' ||
		*(CURRENT_ADDRESS+CLPS_address_offset+2) 	!= 'P' ||
		*(CURRENT_ADDRESS+CLPS_address_offset+3) 	!= 'S') {
		nocashMessage("[Error] The CLPS string could not be found.");
		return;
	}
	
	// CLP_ADDRESS+0x06 2	Number of entries
	int clp_entries = value_by_address(CURRENT_ADDRESS+CLPS_address_offset+0x6, 2);
	log_number("clp_entries", clp_entries);
	
	// 64 4 Address of the 'misc' objects table
	int object_table_offset = value_by_address(CURRENT_ADDRESS+0x64, 4) - ORIGINAL_ADDRESS; // 34925540 - 34925216 = 324 = 0x144
	log_number("object_table_offset", object_table_offset);
	
	// 70 4 Address of the level area data
	int area_address_offset = value_by_address(CURRENT_ADDRESS+0x70, 4) - ORIGINAL_ADDRESS; // 34925820 - 34925216 = 604 = 0x25C
	log_number("area_address_offset", area_address_offset);
	
	// 74 1 Number of level areas
	int area_count = value_by_address(CURRENT_ADDRESS + 0x74, 1); // 0x1
	log_number("area_count", area_count);
	
	// For each area (12 bytes), we need to update the first 4 bytes (Address of the objects table)
	for(int i = 0 ; i < area_count; i ++) {
		int area_object_table_address_offset = value_by_address(CURRENT_ADDRESS+area_address_offset+i*12, 4) - ORIGINAL_ADDRESS; // for area 1: 34925832 - 34925216 = 616 = 0x268
		if(area_object_table_address_offset < 0) {
			nocashMessage("[WARNING] area_object_table_address_offset < 0\n");
		}
		else {		
			log_number("area_object_table_address_offset", area_object_table_address_offset);
			int address_table = (int) CURRENT_ADDRESS + area_object_table_address_offset;
			apply_number(CURRENT_ADDRESS+area_address_offset+i*12, address_table, 4); 
			
			// We need to update the OBJECT TABLES
			int object_table_count = value_by_address(CURRENT_ADDRESS + area_object_table_address_offset, 4); // for area 1: 0x0B = 11
			log_number("object_table_count", object_table_count);
			
			for(int j = 0 ; j < object_table_count; j ++) {
				int area_object_table_offset = value_by_address(CURRENT_ADDRESS + area_object_table_address_offset + 0x4 + j*8, 4) - ORIGINAL_ADDRESS; // area 1, table 1, 34925840 - 34925216 = 624 = 0x270
				log_number("area_object_table_offset", area_object_table_offset);
				apply_number(CURRENT_ADDRESS + area_object_table_address_offset + 0x4 + j*8, (int) CURRENT_ADDRESS + area_object_table_offset, 4);
			}
			
			
		}
		
		
		// TODO: the texture animation data 
	}
	

	
}


extern "C" void MySafeLoadOverlay(int levelID)

{
  int overlayID;
  nocashMessage("MySafeLoadOverlay\n");


	int ORIGINAL = 34925216;
	int diff;
  
  if (DAT_0209f278 != 0) {
    if (CURRENT_LEVEL_OVERLAY != LEVEL_OVL_MAP[levelID]) {
      OS_Terminate();
    }
    DAT_0209f278 = 0;
    return;
  }
  overlayID = LEVEL_OVL_MAP[levelID];
  if (overlayID != -1) {
	  if(levelID==6) {
			// still loading the overlay
			FS_loadOverlay(overlayID);
			
			DAT_0209f340 = LoadFile(0x080a);			
			update_level(DAT_0209f340, ORIGINAL);
			RAM_ADDRESS_LEVELS[levelID] = DAT_0209f340; // RAM_ADDRESS_LEVELS: 0x02092208
			
		  
	  } else {
		  FS_loadOverlay(overlayID);
	  }
	
    CURRENT_LEVEL_OVERLAY = overlayID;
  }
  FUN_0202df70(FS_loadOverlay,levelID);
  return;
}