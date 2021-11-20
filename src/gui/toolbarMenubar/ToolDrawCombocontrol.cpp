#include "ToolDrawCombocontrol.h"

#include <utility>

#include <config.h>

#include "Gtk4Util.h"
#include "ToolMenuHandler.h"
#include "i18n.h"

using std::string;

class ToolDrawType {
public:
    ToolDrawType(string name, string icon, ActionType type): name(std::move(name)), icon(std::move(icon)), type(type) {}

    string name;
    string icon;
    ActionType type;
};

ToolDrawCombocontrol::ToolDrawCombocontrol(ToolMenuHandler* toolMenuHandler, ActionHandler* handler, string id):
        ToolButton(handler, std::move(id), ACTION_TOOL_DRAW_RECT, GROUP_RULER, false,
                   toolMenuHandler->iconName("combo-drawing-type"), _("Drawing Type Combo")),
        toolMenuHandler(toolMenuHandler) {
    setPopupMenu(gtk_popover_new());

    drawTypes.push_back(
            new ToolDrawType(_("Draw Rectangle"), toolMenuHandler->iconName("draw-rect"), ACTION_TOOL_DRAW_RECT));
    drawTypes.push_back(
            new ToolDrawType(_("Draw Ellipse"), toolMenuHandler->iconName("draw-ellipse"), ACTION_TOOL_DRAW_ELLIPSE));
    drawTypes.push_back(
            new ToolDrawType(_("Draw Arrow"), toolMenuHandler->iconName("draw-arrow"), ACTION_TOOL_DRAW_ARROW));
    drawTypes.push_back(new ToolDrawType(_("Draw Line"), toolMenuHandler->iconName("draw-line"), ACTION_RULER));
    drawTypes.push_back(new ToolDrawType(_("Draw coordinate system"),
                                         toolMenuHandler->iconName("draw-coordinate-system"),
                                         ACTION_TOOL_DRAW_COORDINATE_SYSTEM));
    drawTypes.push_back(
            new ToolDrawType(_("Draw Spline"), toolMenuHandler->iconName("draw-spline"), ACTION_TOOL_DRAW_SPLINE));
    drawTypes.push_back(new ToolDrawType(_("Stroke recognizer"), toolMenuHandler->iconName("shape-recognizer"),
                                         ACTION_SHAPE_RECOGNIZER));

    for (ToolDrawType* t: drawTypes) { createMenuItem(t->name, t->icon, t->type); }
}

ToolDrawCombocontrol::~ToolDrawCombocontrol() {
    for (ToolDrawType* t: drawTypes) { delete t; }
    this->drawTypes.clear();
    this->toolMenuHandler = nullptr;
}

void ToolDrawCombocontrol::createMenuItem(const string& name, const string& icon, ActionType type) {
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_append(GTK_BOX(box), gtk_image_new_from_icon_name(icon.c_str()));
    gtk_box_append(GTK_BOX(box), gtk_label_new(name.c_str()));
    gtk_popover_menu_add_child(GTK_POPOVER_MENU(popupMenu), box, name.c_str());
    toolMenuHandler->registerMenupoint(box, type, GROUP_RULER);
}

void ToolDrawCombocontrol::selected(ActionGroup group, ActionType action) {
    if (!this->item) {
        return;
    }

    if (!GTK_IS_TOGGLE_BUTTON(this->item)) {
        g_warning("ToolDrawCombocontrol: selected action %i which is not a toggle action!", action);
        return;
    }

    string description;

    for (ToolDrawType* t: drawTypes) {
        if (action == t->type && this->action != t->type) {
            this->action = t->type;
            gtk_image_set_from_icon_name(GTK_IMAGE(iconWidget), t->icon.c_str());
            description = t->name;
            break;
        }
    }

    gtk_widget_set_tooltip_text(GTK_WIDGET(item), description.c_str());
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(this->item)) != (this->action == action)) {
        this->toolToggleButtonActive = (this->action == action);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->item), this->toolToggleButtonActive);
    }
}

auto ToolDrawCombocontrol::newItem() -> GtkWidget* {
    iconWidget = gtk_image_new_from_icon_name(drawTypes[0]->icon.c_str());

    GtkWidget* it = gtk_button_new();
    gtk_menu_button_set_child(GTK_MENU_BUTTON(it), iconWidget);
    gtk_menu_button_set_label(GTK_MENU_BUTTON(it), _("Draw Rectangle"));
    gtk_menu_button_set_popover(GTK_MENU_BUTTON(it), popupMenu);
    return it;
}
