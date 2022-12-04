//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
// Item represents an existing item.
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////


#ifndef __OTSERV_ITEM_H__
#define __OTSERV_ITEM_H__

#include "thing.h"
#include "items.h"

#include <iostream>
#include <list>
#include <vector>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

class Creature;
class Player;
class Container;
class Depot;
class Teleport;
class TrashHolder;
class Mailbox;
class Door;
class MagicField;
class BedItem;

enum ITEMPROPERTY
{
	BLOCKSOLID = 0,
	HASHEIGHT,
	BLOCKPROJECTILE,
	BLOCKPATH,
	ISVERTICAL,
	ISHORIZONTAL,
	MOVEABLE,
	IMMOVABLEBLOCKSOLID,
	IMMOVABLEBLOCKPATH,
	IMMOVABLENOFIELDBLOCKPATH,
	NOFIELDBLOCKPATH,
	SUPPORTHANGABLE
};

enum TradeEvents_t
{
	ON_TRADE_TRANSFER,
	ON_TRADE_CANCEL,
};

enum ItemDecayState_t
{
	DECAYING_FALSE = 0,
	DECAYING_TRUE,
	DECAYING_PENDING
};

// from iomap.h
#pragma pack(1)
struct TeleportDest
{
	uint16_t _x;
	uint16_t _y;
	uint8_t _z;
};
#pragma pack()

enum AttrTypes_t
{
	//ATTR_DESCRIPTION = 1,
	//ATTR_EXT_FILE = 2,
	ATTR_TILE_FLAGS = 3,
	ATTR_ACTION_ID = 4,
	ATTR_UNIQUE_ID = 5,
	ATTR_TEXT = 6,
	ATTR_DESC = 7,
	ATTR_TELE_DEST = 8,
	ATTR_ITEM = 9,
	ATTR_DEPOT_ID = 10,
	//ATTR_EXT_SPAWN_FILE = 11,
	ATTR_RUNE_CHARGES = 12,
	//ATTR_EXT_HOUSE_FILE = 13,
	ATTR_HOUSEDOORID = 14,
	ATTR_COUNT = 15,
	ATTR_DURATION = 16,
	ATTR_DECAYING_STATE = 17,
	ATTR_WRITTENDATE = 18,
	ATTR_WRITTENBY = 19,
	ATTR_SLEEPERGUID = 20,
	ATTR_SLEEPSTART = 21,
	ATTR_CHARGES = 22,
	ATTR_NAME = 30,
	ATTR_PLURALNAME = 31,
	ATTR_ARTICLE = 32,
	ATTR_ATTACK = 33,
	ATTR_EXTRAATTACK = 34,
	ATTR_DEFENSE = 35,
	ATTR_EXTRADEFENSE = 36,
	ATTR_ARMOR = 37,
	ATTR_ATTACKSPEED = 38,
	ATTR_HITCHANCE = 39,
};

class ItemAttributes
{
	public:
		ItemAttributes()
		{
			m_attributes = 0;
			m_firstAttr = NULL;
		}

		virtual ~ItemAttributes()
		{
			if(m_firstAttr)
				deleteAttrs(m_firstAttr);
		}

		ItemAttributes(const ItemAttributes &i)
		{
			m_attributes = i.m_attributes;
			if(i.m_firstAttr)
				m_firstAttr = new Attribute(*i.m_firstAttr);
		}

		void setSpecialDescription(const std::string& desc) {setStrAttr(ATTR_ITEM_DESC, desc);}
		void resetSpecialDescription() {removeAttribute(ATTR_ITEM_DESC);}
		const std::string& getSpecialDescription() const {return getStrAttr(ATTR_ITEM_DESC);}

		void setText(const std::string& text) {setStrAttr(ATTR_ITEM_TEXT, text);}
		void resetText() {removeAttribute(ATTR_ITEM_TEXT);}
		const std::string& getText() const {return getStrAttr(ATTR_ITEM_TEXT);}

		void setDate(uint64_t n) {setIntAttr(ATTR_ITEM_WRITTENDATE, n);}
		void resetDate() {removeAttribute(ATTR_ITEM_WRITTENDATE);}
		time_t getDate() const {return (time_t)getIntAttr(ATTR_ITEM_WRITTENDATE);}

		void setWriter(std::string _writer) {setStrAttr(ATTR_ITEM_WRITTENBY, _writer);}
		void resetWriter() {removeAttribute(ATTR_ITEM_WRITTENBY);}
		const std::string& getWriter() const {return getStrAttr(ATTR_ITEM_WRITTENBY);}

		void setActionId(uint16_t n) {setIntAttr(ATTR_ITEM_ACTIONID, (uint16_t)std::max((uint16_t)100, n));}
		uint16_t getActionId() const {return getIntAttr(ATTR_ITEM_ACTIONID);}

		void setUniqueId(uint16_t n) {setIntAttr(ATTR_ITEM_UNIQUEID, (uint16_t)std::max((uint16_t)1000, n));}
		uint16_t getUniqueId() const {return getIntAttr(ATTR_ITEM_UNIQUEID);}

		void setCharges(uint16_t n) {setIntAttr(ATTR_ITEM_CHARGES, n);}
		uint16_t getCharges() const {return getIntAttr(ATTR_ITEM_CHARGES);}

		void setFluidType(uint16_t n) {setIntAttr(ATTR_ITEM_FLUIDTYPE, n);}
		uint16_t getFluidType() const {return getIntAttr(ATTR_ITEM_FLUIDTYPE);}

		void setOwner(uint32_t _owner) {setIntAttr(ATTR_ITEM_OWNER, _owner);}
		uint32_t getOwner() const {return getIntAttr(ATTR_ITEM_OWNER);}

		void setCorpseOwner(uint32_t _corpseOwner) {setIntAttr(ATTR_ITEM_CORPSEOWNER, _corpseOwner);}
		uint32_t getCorpseOwner() {return getIntAttr(ATTR_ITEM_CORPSEOWNER);}

		void setDuration(int32_t time) {setIntAttr(ATTR_ITEM_DURATION, time);}
		void decreaseDuration(int32_t time) {increaseIntAttr(ATTR_ITEM_DURATION, -time);}
		int32_t getDuration() const {return getIntAttr(ATTR_ITEM_DURATION);}

		void setDecaying(ItemDecayState_t decayState) {setIntAttr(ATTR_ITEM_DECAYING, decayState);}
		uint32_t getDecaying() const {return getIntAttr(ATTR_ITEM_DECAYING);}

	protected:
		enum itemAttrTypes
		{
			ATTR_ITEM_ACTIONID = 1,
			ATTR_ITEM_UNIQUEID = 2,
			ATTR_ITEM_DESC = 4,
			ATTR_ITEM_TEXT = 8,
			ATTR_ITEM_WRITTENDATE = 16,
			ATTR_ITEM_WRITTENBY = 32,

			ATTR_ITEM_NAME = 64,
			ATTR_ITEM_PLURALNAME = 128,
			ATTR_ITEM_ATTACK = 256,
			ATTR_ITEM_EXTRAATTACK = 512,
			ATTR_ITEM_DEFENSE = 1024,
			ATTR_ITEM_EXTRADEFENSE = 2048,
			ATTR_ITEM_ARMOR = 4096,
			ATTR_ITEM_ATTACKSPEED = 8192,
			ATTR_ITEM_HITCHANCE = 16384,
			ATTR_ITEM_SHOOTRANGE = 32768,

			ATTR_ITEM_OWNER = 65536,
			ATTR_ITEM_DURATION = 131072,
			ATTR_ITEM_DECAYING = 262144,
			ATTR_ITEM_CORPSEOWNER = 524288,
			ATTR_ITEM_CHARGES = 1048576,
			ATTR_ITEM_FLUIDTYPE = 2097152,
			ATTR_ITEM_DOORID = 4194304,

			ATTR_ITEM_ARTICLE = 8388608
		};

		bool hasAttribute(itemAttrTypes type) const;
		void removeAttribute(itemAttrTypes type);

	protected:
		static std::string emptyString;

		class Attribute
		{
			public:
				itemAttrTypes type;
				void* value;
				Attribute* next;
				Attribute(itemAttrTypes _type)
				{
					type = _type;
					value = NULL;
					next = NULL;
				}

				Attribute(const Attribute &i)
				{
					type = i.type;
					if(ItemAttributes::validateIntAttrType(type))
						value = i.value;
					else if(ItemAttributes::validateStrAttrType(type))
						value = (void*)new std::string( *((std::string*)i.value) );
					else
						value = NULL;

					next = NULL;
					if(i.next)
						next = new Attribute(*i.next);
				}
		};

		uint32_t m_attributes;
		Attribute* m_firstAttr;

		const std::string& getStrAttr(itemAttrTypes type) const;
		void setStrAttr(itemAttrTypes type, const std::string& value);

		uint32_t getIntAttr(itemAttrTypes type) const;
		void setIntAttr(itemAttrTypes type, int32_t value);
		void increaseIntAttr(itemAttrTypes type, int32_t value);

		static bool validateIntAttrType(itemAttrTypes type);
		static bool validateStrAttrType(itemAttrTypes type);

		void addAttr(Attribute* attr);
		Attribute* getAttrConst(itemAttrTypes type) const;
		Attribute* getAttr(itemAttrTypes type);

		void deleteAttrs(Attribute* attr);
};

class Item : virtual public Thing, public ItemAttributes
{
	public:
		//Factory member to create item of right type based on type
		static Item* CreateItem(const uint16_t _type, uint16_t _count = 1);
		static Item* CreateItem(PropStream& propStream);
		static Items items;

		// Constructor for items
		Item(const uint16_t _type, uint16_t _count = 0);
		Item(const Item &i);
		virtual Item* clone() const;
		virtual void copyAttributes(Item* item);

		virtual ~Item();

		virtual Item* getItem() {return this;}
		virtual const Item* getItem() const {return this;}
		virtual Container* getContainer() {return NULL;}
		virtual const Container* getContainer() const {return NULL;}
		virtual Teleport* getTeleport() {return NULL;}
		virtual const Teleport* getTeleport() const {return NULL;}
		virtual TrashHolder* getTrashHolder() {return NULL;}
		virtual const TrashHolder* getTrashHolder() const {return NULL;}
		virtual Mailbox* getMailbox() {return NULL;}
		virtual const Mailbox* getMailbox() const {return NULL;}
		virtual Door* getDoor() {return NULL;}
		virtual const Door* getDoor() const {return NULL;}
		virtual MagicField* getMagicField() {return NULL;}
		virtual const MagicField* getMagicField() const {return NULL;}
		virtual BedItem* getBed() {return NULL;}
		virtual const BedItem* getBed() const {return NULL;}

		static std::string getDescription(const ItemType& it, int32_t lookDistance, const Item* item = NULL, int32_t subType = -1, bool addArticle = true);
		static std::string getNameDescription(const ItemType& it, const Item* item = NULL, int32_t subType = -1, bool addArticle = true);
		static std::string getWeightDescription(const ItemType& it, double weight, uint32_t count = 1);

		virtual std::string getDescription(int32_t lookDistance) const;
		std::string getNameDescription() const;
		std::string getWeightDescription() const;

		//serialization
		virtual bool readAttr(AttrTypes_t attr, PropStream& propStream);
		virtual bool unserializeAttr(PropStream& propStream);
		virtual bool serializeAttr(PropWriteStream& propWriteStream);
		virtual bool unserializeItemNode(FileLoader& f, NODE node, PropStream& propStream);

		virtual bool isPushable() const {return !isNotMoveable();}
		virtual int32_t getThrowRange() const {return (isPickupable() ? 15 : 2);}

		uint16_t getID() const {return id;}
		uint16_t getClientID() const {return items[id].clientId;}
		void setID(uint16_t newid);

		const std::string& getName() const {return getStrAttr(ATTR_ITEM_NAME) != "" ? getStrAttr(ATTR_ITEM_NAME) : items[id].name;}
		void setName(std::string name) {setStrAttr(ATTR_ITEM_NAME, name);}

		const std::string& getPluralName() const {return getStrAttr(ATTR_ITEM_PLURALNAME) != "" ? getStrAttr(ATTR_ITEM_PLURALNAME) : items[id].pluralName;}
		void setPluralName(std::string pluralname) {setStrAttr(ATTR_ITEM_PLURALNAME, pluralname);}

		int32_t getAttack() const {return hasAttribute(ATTR_ITEM_ATTACK) ? getIntAttr(ATTR_ITEM_ATTACK) : items[id].attack;}
		void setAttack(int32_t attack) {setIntAttr(ATTR_ITEM_ATTACK, attack);}

		int32_t getExtraAttack() const {return hasAttribute(ATTR_ITEM_EXTRAATTACK) ? getIntAttr(ATTR_ITEM_EXTRAATTACK) : items[id].extraAttack;}
		void setExtraAttack(int32_t extraattack) {setIntAttr(ATTR_ITEM_EXTRAATTACK, extraattack);}

		int32_t getDefense() const {return hasAttribute(ATTR_ITEM_DEFENSE) ? getIntAttr(ATTR_ITEM_DEFENSE) : items[id].defense;}
		void setDefense(int32_t defense) {setIntAttr(ATTR_ITEM_DEFENSE, defense);}

		int32_t getExtraDefense() const {return hasAttribute(ATTR_ITEM_EXTRADEFENSE) ? getIntAttr(ATTR_ITEM_EXTRADEFENSE) : items[id].extraDefense;}
		void setExtraDefense(int32_t extradefense) { setIntAttr(ATTR_ITEM_EXTRADEFENSE, extradefense);}

		int32_t getArmor() const {return hasAttribute(ATTR_ITEM_ARMOR) ? getIntAttr(ATTR_ITEM_ARMOR) : items[id].armor;}
		void setArmor(int32_t armor) {setIntAttr(ATTR_ITEM_ARMOR, armor);}

		uint32_t getAttackSpeed() const {return hasAttribute(ATTR_ITEM_ATTACKSPEED) ? getIntAttr(ATTR_ITEM_ATTACKSPEED) : items[id].attackSpeed;}
		void setAttackSpeed(int32_t attackspeed) {setIntAttr(ATTR_ITEM_ATTACKSPEED, attackspeed);}

		int32_t getHitChance() const {return hasAttribute(ATTR_ITEM_HITCHANCE) ? getIntAttr(ATTR_ITEM_HITCHANCE) : items[id].hitChance;}
		void setHitChance(int32_t hitchance) {setIntAttr(ATTR_ITEM_HITCHANCE, hitchance);}

		int32_t getShootRange() const {return hasAttribute(ATTR_ITEM_SHOOTRANGE) ? getIntAttr(ATTR_ITEM_SHOOTRANGE) : items[id].shootRange;}
		void setShootRange(int32_t shootrange) {setIntAttr(ATTR_ITEM_HITCHANCE, shootrange);}

		const std::string& getArticle() const {return getStrAttr(ATTR_ITEM_ARTICLE) != "" ? getStrAttr(ATTR_ITEM_ARTICLE) : items[id].article;}
		void setArticle(std::string article) {setStrAttr(ATTR_ITEM_ARTICLE, article);}

		virtual double getWeight() const;

		bool isReadable() const {return items[id].canReadText;}
		bool canWriteText() const {return items[id].canWriteText;}
		int32_t getMaxWriteLength() const {return items[id].maxTextLen;}

		int32_t getSlotPosition() const {return items[id].slotPosition;}
		WeaponType_t getWeaponType() const {return items[id].weaponType;}
		Ammo_t getAmmoType() const {return items[id].ammoType;}

		int32_t getWorth() const {return getItemCount() * items[id].worth;}
		void getLight(LightInfo& lightInfo);

		bool hasProperty(enum ITEMPROPERTY prop) const;
		bool hasCharges() const {return items[id].charges != 0;}
		bool forceSerialize() const {return items[id].forceSerialize || canWriteText() || isContainer() || isBed() || isDoor();}
		bool isGroundTile() const {return items[id].isGroundTile();}
		bool isContainer() const {return items[id].isContainer();}
		bool isSplash() const {return items[id].isSplash();}
		bool isFluidContainer() const {return (items[id].isFluidContainer());}
		bool isDoor() const {return items[id].isDoor();}
		bool isMagicField() const {return items[id].isMagicField();}
		bool isTeleport() const {return items[id].isTeleport();}
		bool isKey() const {return items[id].isKey();}
		bool isDepot() const {return items[id].isDepot();}
		bool isMailbox() const {return items[id].isMailbox();}
		bool isTrashHolder() const {return items[id].isTrashHolder();}
		bool isBed() const {return items[id].isBed();}
		bool isRune() const {return items[id].isRune();}
		bool isBlocking() const {return items[id].blockSolid;}
		bool isStackable() const {return items[id].stackable;}
		bool isAlwaysOnTop() const {return items[id].alwaysOnTop;}
		bool isNotMoveable() const {return !items[id].moveable;}
		bool isPickupable() const {return items[id].pickupable;}
		bool isUseable() const {return items[id].useable;}
		bool isHangable() const {return items[id].isHangable;}
		bool isRoteable() const {const ItemType& it = items[id]; return it.rotable && it.rotateTo;}
		bool isWeapon() const {return (items[id].weaponType != WEAPON_NONE);}

		bool floorChangeDown() const {return items[id].floorChangeDown;}
		bool floorChangeNorth() const {return items[id].floorChangeNorth;}
		bool floorChangeSouth() const {return items[id].floorChangeSouth;}
		bool floorChangeEast() const {return items[id].floorChangeEast;}
		bool floorChangeWest() const {return items[id].floorChangeWest;}

		// get the number of items
		uint16_t getItemCount() const {return count;}
		void setItemCount(uint16_t n) {count = n;}

		void setDefaultSubtype();
		bool hasSubType() const;
		uint16_t getSubType() const;
		void setSubType(uint16_t n);

		void setUniqueId(uint16_t n);

		void setDefaultDuration()
		{
			uint32_t duration = getDefaultDuration();
			if(duration != 0)
				setDuration(duration);
		}
		uint32_t getDefaultDuration() const {return items[id].decayTime * 1000;}
		bool canDecay();

		virtual bool canRemove() const {return true;}
		virtual bool canTransform() const {return true;}
		virtual void onRemoved() {}
		virtual bool onTradeEvent(TradeEvents_t event, Player* owner){return true;}

		virtual void __startDecaying();

		bool isLoadedFromMap() const {return loadedFromMap;}
		void setLoadedFromMap(bool value) {loadedFromMap = value;}

	protected:
		std::string getWeightDescription(double weight) const;
		uint16_t id;  // the same id as in ItemType
		uint8_t count; // number of stacked items

		bool loadedFromMap;
		//Don't add variables here, use the ItemAttribute class.
};

#endif
