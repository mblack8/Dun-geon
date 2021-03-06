#include "Controller.h"
#include "Battle/Battle.h"
#include "Entities/EntityManager.h"
#include "Entities/Player.h"
#include "Misc/Direction.h"
#include "UI/BattleScreen.h"
#include "UI/Screen.h"
#include "Worlds/Field.h"
#include "Worlds/WorldManager.h"
#include <sstream>

namespace Player
{

Controller::Controller(Entities::EntityManager& entityManager,
                       Worlds::WorldManager& worldManager,
                       Entities::Player& playerEntity,
                       UI::Screen& screen)
    : m_EntityManager(entityManager),
      m_WorldManager(worldManager),
      m_PlayerEntity(playerEntity),
      m_Screen(screen)
{
}

bool Controller::TryMovePlayer(Direction dir)
{
    return m_EntityManager.TryMovePlayer(dir);
}

bool Controller::TryMovePlayerDiagonally(Direction first, Direction second)
{
    auto playerCoords  = m_EntityManager.CoordsOf(m_PlayerEntity);
    auto& room         = m_WorldManager.CurrentRoom();
    auto firstNeighbor = room.IsAtRoomEdge(playerCoords, first) ? nullptr : &room.FieldAt(playerCoords.Adjacent(first));
    auto secondNeighbor
        = room.IsAtRoomEdge(playerCoords, second) ? nullptr : &room.FieldAt(playerCoords.Adjacent(second));
    auto target = firstNeighbor == nullptr || secondNeighbor == nullptr
                      ? nullptr
                      : &room.FieldAt(playerCoords.Adjacent(first).Adjacent(second));
    if (target != nullptr && target->ForegroundEntity() != nullptr)
        return false;

    if (firstNeighbor == nullptr || firstNeighbor->ForegroundEntity() == nullptr)
        return m_EntityManager.TryMovePlayer(first) && m_EntityManager.TryMovePlayer(second);
    else if (secondNeighbor == nullptr || secondNeighbor->ForegroundEntity() == nullptr)
        return m_EntityManager.TryMovePlayer(second) && m_EntityManager.TryMovePlayer(first);
    else
        return false;
}

bool Controller::TryFight(Direction dir)
{
    // If no direction specified, use the player facing direction and ask for confirmation after
    bool deduced = false;
    if (dir == Direction::None)
    {
        deduced = true;
        dir     = m_PlayerEntity.FacingDirection;
    }

    auto approaching = m_EntityManager.Approaching(m_PlayerEntity, dir);
    if (approaching == nullptr || !approaching->Fightable())
        return false;

    if (deduced
        && !m_Screen.YesNoMessageBox("Are you sure you want to pick a fight with " + approaching->GetName() + "?"))
    {
        return true;
    }

    // TODO: try to remove the cast
    Entities::Character& targetedCharacter = dynamic_cast<Entities::Character&>(*approaching);

    Battle::Battle battle(m_PlayerEntity, targetedCharacter);
    m_Screen.OpenBattleScreen(battle);
    Battle::Battle::Result result = battle.DoBattle();
    m_Screen.CloseSubscreen();

    switch (result)
    {
    case Battle::Battle::Result::Victory:
    {
        int xpGain = targetedCharacter.CalculateXPReward();
        m_EntityManager.KillEntity(targetedCharacter);

        const auto oldPlayerStats = m_PlayerEntity.GetStats();
        bool leveledUp = m_PlayerEntity.GrantXP(xpGain);

        std::ostringstream message;
        message << m_PlayerEntity.GetName() << " has gained " << xpGain << " XP.";
        m_Screen.PostMessage(message.str());
        if (leveledUp)
        {
            m_Screen.DisplayLevelUp(oldPlayerStats, m_PlayerEntity.GetStats());
        }

        break;
    }
    case Battle::Battle::Result::GameOver:
    {
        // TODO: Parameterize class name
        std::ostringstream epitaph;
        epitaph << m_PlayerEntity.GetName() << " did not make it past\n"
                << "World " << m_WorldManager.CurrentWorld().GetWorldNumber() << " of the Dun-geon.\n"
                << "The duelist was defeated\n"
                << "in battle by " << targetedCharacter.GetName() << ".";
        m_Screen.GameOver(epitaph.str());
        break;
    }
    default:
        break;
    }

    return true;
}

void Controller::TurnPlayer(Direction dir)
{
    if (m_PlayerEntity.FacingDirection != dir)
    {
        m_PlayerEntity.FacingDirection = dir;
    }
}

} /* namespace Player */
