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

#include "AbstractItem.h"

class AbstractToolItem: public AbstractItem {
public:
    AbstractToolItem(std::string id, ActionHandler* handler, ActionType type, GtkWidget* menuitem = nullptr);
    virtual ~AbstractToolItem();

public:
    void selected(ActionGroup group, ActionType action) override;
    virtual GtkWidget* createItem(bool horizontal);
    virtual GtkWidget* createTmpItem(bool horizontal);
    void setPopupMenu(GtkWidget* popupMenu);

    bool isUsed() const;
    void setUsed(bool used);

    static void toolButtonCallback(GtkButton* toolbutton, AbstractToolItem* item);

    virtual std::string getToolDisplayName() = 0;

    /**
     * Returns: (transfer floating)
     */
    virtual GtkWidget* getNewToolIcon() = 0;

    /**
     * Enable / Disable the tool item
     */
    void enable(bool enabled) override;

protected:
    virtual GtkWidget* newItem() = 0;

public:
protected:
    GtkWidget* item = nullptr;
    GtkWidget* popupMenu = nullptr;

    bool toolToggleButtonActive = false;
    bool toolToggleOnlyEnable = false;

    /**
     * This item is already somewhere in the toolbar
     */
    bool used = false;
};
