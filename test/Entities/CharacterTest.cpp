#define BOOST_TEST_MODULE Entities.Character
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "Entities/Character.h"
#include "Misc/Coords.h"
#include "Misc/Direction.h"

BOOST_AUTO_TEST_CASE(Move)
{
    Entities::Character character("Name");
    Coords originalCoords = character.GetCoords();
    BOOST_REQUIRE_EQUAL(character.GetLastMoveDirection(), Direction::None);
    const Direction direction = Direction::Right;
    character.Move(direction);
    // Check that character's coords and last move direction have changed
    BOOST_CHECK_EQUAL(character.GetCoords(), originalCoords.Adjacent(direction));
    BOOST_CHECK_EQUAL(character.GetLastMoveDirection(), direction);
}