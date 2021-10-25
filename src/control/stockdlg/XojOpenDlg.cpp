#include "XojOpenDlg.h"

#include <config.h>
#include <gio/gio.h>

#include "util/GtkDialogUtil.h"

#include "PathUtil.h"
#include "StringUtils.h"
#include "XojPreviewExtractor.h"
#include "i18n.h"

XojOpenDlg::XojOpenDlg(GtkWindow* win, Settings* settings): win(win), settings(settings) {
    dialog = gtk_file_chooser_dialog_new(_("Open file"), win, GTK_FILE_CHOOSER_ACTION_OPEN, _("_Cancel"),
                                         GTK_RESPONSE_CANCEL, _("_Open"), GTK_RESPONSE_OK, nullptr);
    fs::path currentFolder;
    if (!settings->getLastOpenPath().empty()) {
        currentFolder = settings->getLastOpenPath();
    } else {
        g_warning("lastOpenPath is not set!");
        currentFolder = g_get_home_dir();
    }
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), Util::toGFile(currentFolder).get(), nullptr);
}

XojOpenDlg::~XojOpenDlg() {
    if (dialog) {
        gtk_window_destroy(GTK_WINDOW(dialog));
    }
    dialog = nullptr;
}

void XojOpenDlg::addFilterAllFiles() {
    GtkFileFilter* filterAll = gtk_file_filter_new();
    gtk_file_filter_set_name(filterAll, _("All files"));
    gtk_file_filter_add_pattern(filterAll, "*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterAll);
}

void XojOpenDlg::addFilterPdf() {
    GtkFileFilter* filterPdf = gtk_file_filter_new();
    gtk_file_filter_set_name(filterPdf, _("PDF files"));
    gtk_file_filter_add_pattern(filterPdf, "*.pdf");
    gtk_file_filter_add_pattern(filterPdf, "*.PDF");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterPdf);
}

void XojOpenDlg::addFilterXoj() {
    GtkFileFilter* filterXoj = gtk_file_filter_new();
    gtk_file_filter_set_name(filterXoj, _("Xournal files"));
    gtk_file_filter_add_pattern(filterXoj, "*.xoj");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXoj);
}

void XojOpenDlg::addFilterXopp() {
    GtkFileFilter* filterXopp = gtk_file_filter_new();
    gtk_file_filter_set_name(filterXopp, _("Xournal++ files"));
    gtk_file_filter_add_pattern(filterXopp, "*.xopp");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXopp);
}

void XojOpenDlg::addFilterXopt() {
    GtkFileFilter* filterXopt = gtk_file_filter_new();
    gtk_file_filter_set_name(filterXopt, _("Xournal++ template"));
    gtk_file_filter_add_pattern(filterXopt, "*.xopt");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXopt);
}

auto XojOpenDlg::runDialog() -> fs::path {
    gtk_window_set_transient_for(GTK_WINDOW(dialog), win);
    if (wait_for_gtk_dialog_result(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
        gtk_window_destroy(GTK_WINDOW(dialog));
        dialog = nullptr;
        return fs::path{};
    }

    auto file = Util::fromGFile(Util::GOwned<GFile>(gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog))).get());
    settings->setLastOpenPath(file.parent_path());
    return file;
}

auto XojOpenDlg::showOpenTemplateDialog() -> fs::path {
    addFilterAllFiles();
    addFilterXopt();

    return runDialog();
}

auto XojOpenDlg::showOpenDialog(bool pdf, bool& attachPdf) -> fs::path {
    if (!pdf) {
        GtkFileFilter* filterSupported = gtk_file_filter_new();
        gtk_file_filter_set_name(filterSupported, _("Supported files"));
        gtk_file_filter_add_pattern(filterSupported, "*.xoj");
        gtk_file_filter_add_pattern(filterSupported, "*.xopp");
        gtk_file_filter_add_pattern(filterSupported, "*.xopt");
        gtk_file_filter_add_pattern(filterSupported, "*.pdf");
        gtk_file_filter_add_pattern(filterSupported, "*.PDF");
        gtk_file_filter_add_pattern(filterSupported, "*.moj");  // MrWriter
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterSupported);

        addFilterXoj();
        addFilterXopt();
        addFilterXopp();
    }

    addFilterPdf();
    addFilterAllFiles();

    GtkWidget* attachOpt = nullptr;
    if (pdf) {
        // Todo (gtk4, fabian): filechooser has to be initialized by a factory function
        // attachOpt = gtk_check_button_new_with_label(_("Attach file to the journal"));
        // g_object_ref(attachOpt);
        // gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(attachOpt), false);
        // gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), attachOpt);
    }

    // GtkWidget* image = gtk_image_new();
    // gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), image);
    // g_signal_connect(dialog, "update-preview", G_CALLBACK(updatePreviewCallback), nullptr);

    auto lastOpenPath = this->settings->getLastOpenPath();
    if (!lastOpenPath.empty()) {
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(this->dialog), Util::toGFile(lastOpenPath).get(), nullptr);
    }

    auto lastSavePath = this->settings->getLastSavePath();
    if (!lastSavePath.empty()) {
        gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(this->dialog), Util::toGFile(lastSavePath).get(),
                                             nullptr);
    }

    fs::path file = runDialog();

    if (attachOpt) {
        attachPdf = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(attachOpt));
        g_object_unref(attachOpt);
    }

    if (!file.empty()) {
        g_message("lastOpenPath set");
        this->settings->setLastOpenPath(file.parent_path());
    }

    return file;
}

void XojOpenDlg::updatePreviewCallback(GtkFileChooser* fileChooser, void* userData) {
    // Todo (gtk4, fabian): remove or find alternative in gtk4
}
