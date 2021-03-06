#include "RoomLayout.h"
#include "Entities/StaticEntities.h"
#include "Misc/Coords.h"
#include "Misc/Direction.h"
#include "Misc/Exceptions.h"
#include "Misc/RNG.h"
#include "RoomGenerationParameters.h"
#include "UI/CameraStyle.h"
#include <algorithm>
#include <map>
#include <vector>

namespace Worlds::Generation
{

RoomLayout::RoomLayout(const RoomGenerationParameters& parameters)
    : m_Width(0),
      m_Height(0),
      m_Parameters(parameters),
      m_CameraStyle(UI::CameraStyle::Fixed),
      m_VisionRadius(0),
      m_NPCSpawnChance(0)
{
}

int RoomLayout::WriteToFields(std::vector<std::vector<Field>>& fields) const
{
    int accessibleFieldCount = 0;
    fields.clear();
    fields.resize(m_Width);
    for (Coords::Scalar i = 0; i < m_Width; i++)
    {
        for (Coords::Scalar j = 0; j < m_Height; j++)
        {
            fields[i].emplace_back(Coords(i, j));
            switch (m_Map[i][j])
            {
            case FieldType::Accessible:
                fields[i][j].MakeAccessible();
                accessibleFieldCount++;
                break;
            case FieldType::Wall:
                fields[i][j].PlaceEntity(Entities::Wall);
                break;
            case FieldType::Column:
                fields[i][j].PlaceEntity(Entities::Column);
                break;
            default:
                break;
            }
        }
    }

    return accessibleFieldCount;
}

const std::map<Direction, Coords>& RoomLayout::GetEntrances() const
{
    return m_Entrances;
}

UI::CameraStyle RoomLayout::GetCameraStyle() const
{
    return m_CameraStyle;
}

int RoomLayout::GetVisionRadius() const
{
    return m_VisionRadius;
}

double RoomLayout::GetNPCSpawnChance() const
{
    return m_NPCSpawnChance;
}

void RoomLayout::GenerateAttributes()
{
}

std::vector<Direction> RoomLayout::GenerateEntranceDirections() const
{
    // Select all directions that aren't forbidden
    std::vector<Direction> directions, viable;
    for (const auto& dir : Direction::All)
    {
        if (m_Parameters.EntranceInfo.count(dir) == 0 || m_Parameters.EntranceInfo.at(dir) == true)
        {
            viable.push_back(dir);
        }
    }
    // Select those that are forced or pass RNG
    for (const auto& dir : viable)
    {
        if ((m_Parameters.EntranceInfo.count(dir) > 0 && m_Parameters.EntranceInfo.at(dir) == true) ||
            (m_Parameters.EntranceInfo.count(dir) == 0 && RNG::Chance(m_Parameters.OptionalEntranceChance)))
        {
            directions.push_back(dir);
        }
    }
    // If we happened to only get at most one but the room must continue, generate a second entrance anyway
    if (directions.size() <= 1 && viable.size() > 1 && m_Parameters.ForceContinue)
    {
        if (!directions.empty())
        {
            // Remove the direction of arrival from the viable pool
            auto whereWeCameFrom = std::find(viable.begin(), viable.end(), directions.front());
            viable.erase(whereWeCameFrom);
        }
        auto randomDir = viable[RNG::RandomInt(viable.size())];
        directions.push_back(randomDir);
    }
    return directions;
}

Coords RoomLayout::GenerateEntranceCoords(Direction dir) const
{
    constexpr static const int MinimumCornerDistance = 3;
    switch (dir())
    {
    case Direction::Value::Up:
        return Coords(RNG::RandomInt(MinimumCornerDistance, m_Width - MinimumCornerDistance), 0);
    case Direction::Value::Right:
        return Coords(m_Width - 1, RNG::RandomInt(MinimumCornerDistance, m_Height - MinimumCornerDistance));
    case Direction::Value::Down:
        return Coords(RNG::RandomInt(MinimumCornerDistance, m_Width - MinimumCornerDistance), m_Height - 1);
    case Direction::Value::Left:
        return Coords(0, RNG::RandomInt(MinimumCornerDistance, m_Height - MinimumCornerDistance));
    default:
        throw InvalidPositionException("Attempted to generate entrance coords for direction None");
    }
}

void RoomLayout::DrawMapLine(Coords from, Coords to, FieldType value)
{
    for (const auto& pos : from.StraightPath(to))
    {
        m_Map[pos.X][pos.Y] = value;
    }
}

void RoomLayout::DrawMapBox(Coords center, Coords::Scalar radius, FieldType value)
{
    for (Coords::Scalar i = 0; i <= radius; i++)
    {
        for (Coords::Scalar j = 0; j <= radius; j++)
        {
            Coords::Scalar left = center.X - i;
            Coords::Scalar right = center.X + i;
            Coords::Scalar up = center.Y - j;
            Coords::Scalar down = center.Y + j;
            if (left < m_Width && left >= 0)
            {
                if (up < m_Height && up >= 0) m_Map[left][up] = value;
                if (down < m_Height && down >= 0) m_Map[left][down] = value;
            }
            if (right < m_Width && right >= 0)
            {
                if (up < m_Height && up >= 0) m_Map[right][up] = value;
                if (down < m_Height && down >= 0) m_Map[right][down] = value;
            }
        }
    }
}

} /* namespace Worlds::Generation */
