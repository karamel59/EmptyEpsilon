#include "weaponTube.h"
#include "spaceObjects/EMPMissile.h"
#include "spaceObjects/homingMissile.h"
#include "spaceObjects/mine.h"
#include "spaceObjects/nuke.h"
#include "spaceObjects/spaceship.h"

WeaponTube::WeaponTube()
{
    parent = nullptr;
    
    type_loaded = MW_None;
    state = WTS_Empty;
    delay = 0.0;
    tube_index = 0;
}

void WeaponTube::setParent(SpaceShip* parent)
{
    assert(!this->parent);
    this->parent = parent;

    parent->registerMemberReplication(&type_loaded);
    parent->registerMemberReplication(&state);
    parent->registerMemberReplication(&delay, 0.5);
}

void WeaponTube::setIndex(int index)
{
    tube_index = index;
}

void WeaponTube::startLoad(EMissileWeapons type)
{
    if (type <= MW_None || type >= MW_Count)
        return;
    if (state != WTS_Empty)
        return;
    if (parent->weapon_storage[type] <= 0)
        return;
        
    state = WTS_Loading;
    delay = parent->tube_load_time;
    type_loaded = type;
    parent->weapon_storage[type]--;
}

void WeaponTube::startUnload()
{
    if (state == WTS_Loaded)
    {
        state = WTS_Unloading;
        delay = parent->tube_load_time;
    }
}

void WeaponTube::fire(float target_angle)
{
    //TOFIX: This does not really belong here.
    if (parent->scanned_by_player == SS_NotScanned)
    {
        P<SpaceShip> ship = parent->getTarget();
        if (parent->getTarget() && (!ship || ship->scanned_by_player != SS_NotScanned))
            parent->scanned_by_player = SS_FriendOrFoeIdentified;
    }

    if (parent->docking_state != DS_NotDocking) return;
    if (parent->current_warp > 0.0) return;
    if (state != WTS_Loaded) return;

    sf::Vector2f fireLocation = parent->getPosition() + sf::rotateVector(parent->ship_template->model_data->getTubePosition2D(tube_index), parent->getRotation());
    switch(type_loaded)
    {
    case MW_Homing:
        {
            P<HomingMissile> missile = new HomingMissile();
            missile->owner = parent;
            missile->setFactionId(parent->getFactionId());
            missile->target_id = parent->target_id;
            missile->setPosition(fireLocation);
            missile->setRotation(parent->getRotation());
            missile->target_angle = target_angle;
        }
        break;
    case MW_Nuke:
        {
            P<Nuke> missile = new Nuke();
            missile->owner = parent;
            missile->setFactionId(parent->getFactionId());
            missile->target_id = parent->target_id;
            missile->setPosition(fireLocation);
            missile->setRotation(parent->getRotation());
            missile->target_angle = target_angle;
        }
        break;
    case MW_Mine:
        {
            P<Mine> missile = new Mine();
            missile->owner = parent;
            missile->setFactionId(parent->getFactionId());
            missile->setPosition(fireLocation);
            missile->setRotation(parent->getRotation());
            missile->eject();
        }
        break;
    case MW_EMP:
        {
            P<EMPMissile> missile = new EMPMissile();
            missile->owner = parent;
            missile->setFactionId(parent->getFactionId());
            missile->target_id = parent->target_id;
            missile->setPosition(fireLocation);
            missile->setRotation(parent->getRotation());
            missile->target_angle = target_angle;
        }
        break;
    default:
        break;
    }
    state = WTS_Empty;
    type_loaded = MW_None;
}

void WeaponTube::forceUnload()
{
    if (state != WTS_Empty && type_loaded != MW_None)
    {
        state = WTS_Empty;
        if (parent->weapon_storage[type_loaded] < parent->weapon_storage_max[type_loaded])
            parent->weapon_storage[type_loaded] ++;
        type_loaded = MW_None;
    }
}

void WeaponTube::update(float delta)
{
    if (delay > 0.0)
    {
        delay -= delta * parent->getSystemEffectiveness(SYS_MissileSystem);
    }else{
        switch(state)
        {
        case WTS_Loading:
            state = WTS_Loaded;
            break;
        case WTS_Unloading:
            state = WTS_Empty;
            if (parent->weapon_storage[type_loaded] < parent->weapon_storage_max[type_loaded])
                parent->weapon_storage[type_loaded] ++;
            type_loaded = MW_None;
            break;
        default:
            break;
        }
    }
}

bool WeaponTube::isEmpty()
{
    return state == WTS_Empty;
}

bool WeaponTube::isLoaded()
{
    return state == WTS_Loaded;
}

bool WeaponTube::isLoading()
{
    return state == WTS_Loading;
}

bool WeaponTube::isUnloading()
{
    return state == WTS_Unloading;
}

float WeaponTube::getLoadProgress()
{
    return 1.0 - delay / parent->tube_load_time;
}

float WeaponTube::getUnloadProgress()
{
    return delay / parent->tube_load_time;
}

EMissileWeapons WeaponTube::getLoadType()
{
    return type_loaded;
}
