#include "ImageOpenDlg.h"

#include <config.h>
#include <util/PathUtil.h>

#include "control/settings/Settings.h"

#include "util/GtkDialogUtil.h"

#include "Util.h"
#include "i18n.h"

auto ImageOpenDlg::show(GtkWindow* win, Settings* settings, bool localOnly, bool* attach) -> GFile* {
    GtkWidget* dialog = gtk_file_chooser_dialog_new(_("Open Image"), win, GTK_FILE_CHOOSER_ACTION_OPEN, _("_Cancel"),
                                                    GTK_RESPONSE_CANCEL, _("_Open"), GTK_RESPONSE_OK, nullptr);

    

    GtkFileFilter* filterSupported = gtk_file_filter_new();
    gtk_file_filter_set_name(filterSupported, _("Images"));
    gtk_file_filter_add_pixbuf_formats(filterSupported);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterSupported);

    if (!settings->getLastImagePath().empty()) {
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
                                            Util::toGFile(settings->getLastImagePath()).get(), nullptr);
    }

    GtkWidget* cbAttach = nullptr;
    if (attach) {
        // Todo (gtk4, fabian): Find gtk4 alternative;
        // cbAttach = gtk_check_button_new_with_label(_("Attach file to the journal"));
        // gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbAttach), false);
        // gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), cbAttach);
    }
// Todo (gtk4, fabian): Find gtk4 alternative;
    // GtkWidget* image = gtk_image_new();
    // gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), image);
    // g_signal_connect(dialog, "update-preview", G_CALLBACK(updatePreviewCallback), nullptr);

    gtk_window_set_transient_for(GTK_WINDOW(dialog), win);
    if (wait_for_gtk_dialog_result(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
        gtk_window_destroy(GTK_WINDOW(dialog));
        return nullptr;
    }
    GFile* file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
    if (attach) {
        *attach = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cbAttach));
    }

    // e.g. from last used files, there is no folder selected
    // in this case do not store the folder
    if (auto folder = Util::fromGFile(Util::GOwned<GFile>(gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog))).get());
        !folder.empty()) {
        settings->setLastImagePath(folder);
    }
    gtk_window_destroy(GTK_WINDOW(dialog));
    return file;
}

// Source: Empathy

auto ImageOpenDlg::pixbufScaleDownIfNecessary(GdkPixbuf* pixbuf, gint maxSize) -> GdkPixbuf* {
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);

    if (width > maxSize || height > maxSize) {
        double factor = static_cast<gdouble>(maxSize) / std::max(width, height);

        width = width * factor;
        height = height * factor;

        return gdk_pixbuf_scale_simple(pixbuf, width, height, GDK_INTERP_HYPER);
    }

    return static_cast<GdkPixbuf*>(g_object_ref(pixbuf));
}

void ImageOpenDlg::updatePreviewCallback(GtkFileChooser* fileChooser, void* userData) {
    // Todo (gtk4, fabian): Find gtk4 alternative;
}
