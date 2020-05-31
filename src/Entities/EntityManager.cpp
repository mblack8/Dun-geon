#include "EntityManager.h"
#include "Character.h"
#include "Entity.h"
#include "Misc/Direction.h"
#include "Player.h"
#include "Worlds/Field.h"
#include "Worlds/Room.h"
#include "Worlds/WorldManager.h"
#include <memory>

namespace Entities
{

EntityManager::EntityManager(Worlds::WorldManager& worldManager, Player& player)
    : m_WorldManager(worldManager),
      m_Player(player)
{
}

void EntityManager::Store(Worlds::Room& room, Entity& entity)
{
    // Storing pointers is safe because both rooms and entities are tied to the world's lifespan
    m_EntityStorage[&room].emplace_back(&entity);
}

bool EntityManager::TryMovePlayer(Direction dir)
{
    if (CanCharacterMove(m_Player, dir))
    {
        Pluck(m_Player, m_WorldManager.CurrentRoom());
        m_Player.Move(dir);
        Place(m_Player, m_WorldManager.CurrentRoom());
        CycleCurrentRoom();
        return true;
    }
    else if (m_WorldManager.CurrentRoom().IsAtRoomEdge(m_Player.GetCoords(), dir))
    {
        Direction nextRoomEntranceDir = dir.Opposite();
        Pluck(m_Player, m_WorldManager.CurrentRoom());
        Worlds::Room& nextRoom = m_WorldManager.SwitchRoom(dir);
        Coords newCoords = nextRoom
                               .Entrance(nextRoomEntranceDir)
                               ->GetCoords();
        m_Player.SetCoords(newCoords);
        m_Player.SetLastMoveDirection(dir);
        Place(m_Player, nextRoom);
        CycleCurrentRoom();
        return true;
    }

    m_Player.SetLastMoveDirection(dir);
    return false;
}

const Entity* EntityManager::Approaching(const Character& approachingCharacter) const
{
    const Worlds::Field* approachedField = AdjacentField(approachingCharacter, approachingCharacter.GetLastMoveDirection());
    return approachedField != nullptr ? approachedField->ForegroundEntity() : nullptr;
}

bool EntityManager::CanCharacterMove(const Character& character, Direction dir) const
{
    if (dir == Direction::None) return true;

    const Worlds::Field* targetField = AdjacentField(character, dir);
    if (targetField != nullptr && targetField->ForegroundEntity() == nullptr)
    {
        return true;
    }

    return false;
}

const std::array<const Worlds::Field*, 4> EntityManager::AdjacentFields(
    const Entity& entity) const
{
    Coords coords = entity.GetCoords();
    const Worlds::Room& currentRoom = m_WorldManager.CurrentRoom();
    std::array<const Worlds::Field*, 4> fields;
    for (const auto& dir : Direction::All)
    {
        fields[dir.ToInt()] = !currentRoom.IsAtRoomEdge(coords, dir)
                                  ? &currentRoom.FieldAt(coords.Adjacent(dir))
                                  : nullptr;
    }
    return fields;
}

const Worlds::Field* EntityManager::AdjacentField(
    const Entity& entity,
    Direction direction) const
{
    return direction != Direction::None
               ? AdjacentFields(entity)[direction.ToInt()]
               : nullptr;
}

void EntityManager::Cycle(Worlds::Room& room)
{
    for (auto& entity : m_EntityStorage[&room])
    {
        Pluck(*entity, room);
        //TODO: Implement NPC behavior
        //TODO: Resolve movement conflicts between player and NPCs or two NPCs
        Place(*entity, room);
    }
}

void EntityManager::Place(Entity& entity, Worlds::Room& room)
{
    auto& field = room.FieldAt(entity.GetCoords());
    field.PlaceEntity(entity);
}

void EntityManager::Pluck(Entity& entity, Worlds::Room& room)
{
    auto& field = room.FieldAt(entity.GetCoords());
    entity.IsBlocking() ? field.VacateForeground() : field.VacateBackground();
}

} /* namespace Entities */
