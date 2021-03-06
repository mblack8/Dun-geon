#include "Nameplate.h"
#include "ColorPairs.h"
#include "FillBar.h"
#include "Misc/Utils.h"
#include "Screen.h"

namespace UI::Components
{

Nameplate::Nameplate(const Entities::Character& character, int xPos, int yPos, int width, bool isTitleOnTop)
    : m_Character(character),
      m_Width(width),
      m_IsTitleOnTop(isTitleOnTop),
      m_Window(newwin(Height, width, yPos, xPos)),
      HealthBar(m_Window,
                16,
                8,
                2,
                m_Character.GetStats().Health,
                m_Character.GetStats().MaxHealth,
                ColorPairs::WhiteOnGreen),
      ManaBar(m_Window, 16, 8, 3, m_Character.GetStats().Mana, m_Character.GetStats().MaxMana, ColorPairs::WhiteOnBlue)
{
}

Nameplate::~Nameplate()
{
    wclear(m_Window);
    wrefresh(m_Window);
    delwin(m_Window);
}

void Nameplate::Draw()
{
    werase(m_Window);
    DrawBorder();

    int row = 0;

    // Top row
    row++;
    mvwprintw(m_Window, row, 4, "Level %d", m_Character.GetStats().Level);
    mvwprintw(
        m_Window, row, m_Width - 4 - m_Character.GetDescription().size(), "%s", m_Character.GetDescription().c_str());

    // Middle row
    row++;
    mvwprintw(m_Window, row, 4, "HP:");
    HealthBar.Draw();

    // Bottom row
    row++;
    mvwprintw(m_Window, row, 4, "MP:");
    ManaBar.Draw();

    wrefresh(m_Window);
}

void Nameplate::FlashBorder(short colorPair, int count, int periodMs)
{
    for (int i = 0; i < count; i++)
    {
        if (i != 0)
            Sleep(periodMs);

        wattron(m_Window, COLOR_PAIR(colorPair));
        DrawBorder();

        Sleep(periodMs);

        wattroff(m_Window, A_COLOR);
        DrawBorder();
    }
}

void Nameplate::DrawBorder()
{
    box(m_Window, 0, 0);
    std::string title = " " + m_Character.GetName() + " ";
    wattron(m_Window, A_REVERSE);
    Screen::PrintCenter(m_Window, title, m_IsTitleOnTop ? 0 : Height - 1);
    wattroff(m_Window, A_REVERSE);
    wrefresh(m_Window);
}

} // namespace UI::Components