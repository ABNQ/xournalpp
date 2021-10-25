/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "gui/GladeGui.h"

#include "AbstractToolItem.h"

class ToolButton: public AbstractToolItem {
public:
    ToolButton(ActionHandler* handler, std::string id, ActionType type, std::string iconName, std::string description,
               GtkWidget* menuitem = nullptr);
    ToolButton(ActionHandler* handler, std::string id, ActionType type, ActionGroup group, bool toolToggleOnlyEnable,
               std::string iconName, std::string description, GtkWidget* menuitem = nullptr);

    ~ToolButton() override = default;

public:
    /**
     * Register a popup menu entry, create a popup menu, if none is there
     *
     * @param name The name of the item
     * @param iconName To load an icon
     * @return The created menu item
     */
    GtkWidget* registerPopupMenuEntry(const std::string& name, const std::string& iconName = "");

    void updateDescription(const std::string& description);
    std::string getToolDisplayName() override;
    void setActive(bool active);

protected:
    GtkWidget* newItem() override;

    GtkWidget* getNewToolIcon() override;

private:
    std::string iconName;
    std::string description;
};
