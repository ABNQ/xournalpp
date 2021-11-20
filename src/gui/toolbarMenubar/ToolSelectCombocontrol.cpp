#include "ToolSelectCombocontrol.h"

#include <utility>

#include <config.h>

#include "Gtk4Util.h"
#include "ToolMenuHandler.h"
#include "i18n.h"

using std::string;

ToolSelectCombocontrol::ToolSelectCombocontrol(ToolMenuHandler* toolMenuHandler, ActionHandler* handler, string id):
        ToolButton(handler, std::move(id), ACTION_TOOL_SELECT_RECT, GROUP_TOOL, true,
                   toolMenuHandler->iconName("combo-selection"), _("Selection Combo")),
        toolMenuHandler(toolMenuHandler),
        popup(gtk_popover_new()) {
    addMenuitem(_("Select Rectangle"), toolMenuHandler->iconName("select-rect"), ACTION_TOOL_SELECT_RECT, GROUP_TOOL);
    addMenuitem(_("Select Region"), toolMenuHandler->iconName("select-lasso"), ACTION_TOOL_SELECT_REGION, GROUP_TOOL);
    addMenuitem(_("Select Object"), toolMenuHandler->iconName("object-select"), ACTION_TOOL_SELECT_OBJECT, GROUP_TOOL);
    addMenuitem(_("Play Object"), toolMenuHandler->iconName("object-play"), ACTION_TOOL_PLAY_OBJECT, GROUP_TOOL);

    setPopupMenu(popup);
}

ToolSelectCombocontrol::~ToolSelectCombocontrol() = default;

void ToolSelectCombocontrol::addMenuitem(const string& text, const string& icon, ActionType type, ActionGroup group) {
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget* label = gtk_label_new(text.c_str());

    gtk_box_append(GTK_BOX(box), gtk_image_new_from_icon_name(icon.c_str()));
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_box_append(GTK_BOX(box), label);

    gtk_popover_set_child(GTK_POPOVER(popup), label);
    toolMenuHandler->registerMenupoint(label, type, group);
}

void ToolSelectCombocontrol::selected(ActionGroup group, ActionType action) {
    if (this->item) {
        if (!GTK_IS_TOGGLE_BUTTON(this->item)) {
            g_warning("selected action %i which is not a toggle action! 2", action);
            return;
        }

        string description;

        if (action == ACTION_TOOL_SELECT_RECT && this->action != ACTION_TOOL_SELECT_RECT) {
            this->action = ACTION_TOOL_SELECT_RECT;
            gtk_image_set_from_icon_name(GTK_IMAGE(iconWidget), toolMenuHandler->iconName("select-rect").c_str());

            description = _("Select Rectangle");
        } else if (action == ACTION_TOOL_SELECT_REGION && this->action != ACTION_TOOL_SELECT_REGION) {
            this->action = ACTION_TOOL_SELECT_REGION;
            gtk_image_set_from_icon_name(GTK_IMAGE(iconWidget), toolMenuHandler->iconName("select-lasso").c_str());

            description = _("Select Region");
        } else if (action == ACTION_TOOL_SELECT_OBJECT && this->action != ACTION_TOOL_SELECT_OBJECT) {
            this->action = ACTION_TOOL_SELECT_OBJECT;
            gtk_image_set_from_icon_name(GTK_IMAGE(iconWidget), toolMenuHandler->iconName("object-select").c_str());

            description = _("Select Object");
        } else if (action == ACTION_TOOL_PLAY_OBJECT && this->action != ACTION_TOOL_PLAY_OBJECT) {
            this->action = ACTION_TOOL_PLAY_OBJECT;
            gtk_image_set_from_icon_name(GTK_IMAGE(iconWidget), toolMenuHandler->iconName("object-play").c_str());

            description = _("Play Object");
        }
        gtk_widget_set_tooltip_text(GTK_WIDGET(item), description.c_str());


        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(this->item)) != (this->action == action)) {
            this->toolToggleButtonActive = (this->action == action);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->item), this->toolToggleButtonActive);
        }
    }
}

auto ToolSelectCombocontrol::newItem() -> GtkWidget* {
    labelWidget = gtk_label_new(_("Select Rectangle"));
    iconWidget = gtk_image_new_from_icon_name(toolMenuHandler->iconName("select-rect").c_str());

    auto it = gtk_menu_button_new();
    gtk_menu_button_set_label(GTK_MENU_BUTTON(it), "test0");
    gtk_menu_button_set_child(GTK_MENU_BUTTON(it), iconWidget);
    gtk_menu_button_set_popover(GTK_MENU_BUTTON(it), popupMenu);
    return it;
}
