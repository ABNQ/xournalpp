#include "FontButton.h"

#include <locale>
#include <sstream>
#include <utility>

#include <config.h>

#include "PathUtil.h"
#include "i18n.h"

using std::string;

FontButton::FontButton(ActionHandler* handler, GladeGui* gui, string id, ActionType type, string description,
                       GtkWidget* menuitem):
        AbstractToolItem(std::move(id), handler, type, menuitem) {
    this->gui = gui;
    this->description = std::move(description);
}

void FontButton::activated(GdkEvent* event, GtkWidget* menuitem, GtkButton* toolbutton) {
    GtkFontButton* button = GTK_FONT_BUTTON(fontButton);

    string name = Util::GOwned<char[]>(gtk_font_chooser_get_font(GTK_FONT_CHOOSER(button))).get();

    auto pos = name.find_last_of(' ');
    this->font.setName(name.substr(0, pos));
    this->font.setSize(std::stod(name.substr(pos + 1)));

    handler->actionPerformed(ACTION_FONT_BUTTON_CHANGED, GROUP_NOGROUP, event, menuitem, nullptr, true);
}

void FontButton::setFontFontButton(GtkWidget* fontButton, XojFont& font) {
    GtkFontButton* button = GTK_FONT_BUTTON(fontButton);
    // Fixing locale to make format of font-size string independent of localization setting
    std::stringstream fontSizeStream;
    fontSizeStream.imbue(std::locale("C"));
    fontSizeStream << font.getSize();
    string name = font.getName() + " " + fontSizeStream.str();
    gtk_font_chooser_set_font(GTK_FONT_CHOOSER(button), name.c_str());
}

void FontButton::setFont(XojFont& font) {
    this->font = font;
    if (this->fontButton == nullptr) {
        return;
    }

    setFontFontButton(this->fontButton, font);
}

auto FontButton::getFont() -> XojFont {
    // essentially, copy the font object to prevent a memory leak.
    XojFont newfont;
    newfont.setName(font.getName());
    newfont.setSize(font.getSize());

    return newfont;
}

auto FontButton::getToolDisplayName() -> string { return _("Font"); }

auto FontButton::getNewToolIcon() -> GtkWidget* { return gtk_image_new_from_icon_name("font-x-generic"); }

auto FontButton::createItem(bool horizontal) -> GtkWidget* {
    if (this->item) {
        return this->item;
    }

    this->item = newItem();
    g_object_ref(this->item);
    g_signal_connect(fontButton, "font_set", G_CALLBACK(&toolButtonCallback), this);
    return this->item;
}

auto FontButton::createTmpItem(bool horizontal) -> GtkWidget* {
    GtkWidget* fontButton = newFontButton();
    gtk_widget_set_tooltip_text(fontButton, this->description.c_str());

    if (!this->font.getName().empty()) {
        setFontFontButton(fontButton, this->font);
    }

    return fontButton;
}

void FontButton::showFontDialog() {
    if (this->fontButton == nullptr) {
        newItem();
    }
}

auto FontButton::newFontButton() -> GtkWidget* {
    GtkWidget* w = gtk_font_button_new();
    gtk_font_button_set_use_font(GTK_FONT_BUTTON(w), true);
    return w;
}

auto FontButton::newItem() -> GtkWidget* {
    if (this->fontButton) {
        g_object_unref(this->fontButton);
    }
    this->fontButton = newFontButton();
    gtk_widget_set_tooltip_text(this->fontButton, this->description.c_str());

    g_signal_connect(this->fontButton, "font-set", G_CALLBACK(&toolButtonCallback), this);

    if (!this->font.getName().empty()) {
        setFont(this->font);
    }

    return this->fontButton;
}
