#include "LatexSettingsPanel.h"

#include <fstream>
#include <variant>

#include "control/latex/LatexGenerator.h"

#include "GtkDialogUtil.h"
#include "PathUtil.h"
#include "filesystem.h"
#include "i18n.h"

LatexSettingsPanel::LatexSettingsPanel(GladeSearchpath* gladeSearchPath):
        GladeGui(gladeSearchPath, "latexSettings.glade", "latexSettingsPanel"),
        cbAutoDepCheck(GTK_TOGGLE_BUTTON(this->get("latexSettingsRunCheck"))),
        globalTemplateChooser(GTK_FILE_CHOOSER(this->get("latexSettingsTemplateFile"))) {
    g_object_ref(this->cbAutoDepCheck);
    g_object_ref(this->globalTemplateChooser);
    g_signal_connect(this->get("latexSettingsTestBtn"), "clicked",
                     G_CALLBACK(+[](GtkWidget*, LatexSettingsPanel* self) { self->checkDeps(); }), this);
}

LatexSettingsPanel::~LatexSettingsPanel() {
    g_object_unref(this->cbAutoDepCheck);
    g_object_unref(this->globalTemplateChooser);
}

void LatexSettingsPanel::load(const LatexSettings& settings) {
    gtk_toggle_button_set_active(this->cbAutoDepCheck, settings.autoCheckDependencies);
    if (!settings.globalTemplatePath.empty()) {
        gtk_file_chooser_set_file(this->globalTemplateChooser, Util::toGFile(settings.globalTemplatePath).get(),
                                  nullptr);
    }

    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(this->get("latexSettingsGenCmd"))),
                              settings.genCmd.c_str(), settings.genCmd.size());
}

void LatexSettingsPanel::save(LatexSettings& settings) {
    settings.autoCheckDependencies = gtk_toggle_button_get_active(this->cbAutoDepCheck);
    settings.globalTemplatePath =
            Util::fromGFile(Util::GOwned<GFile>(gtk_file_chooser_get_file(this->globalTemplateChooser)).get());
    settings.genCmd = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(this->get("latexSettingsGenCmd"))));
}

void LatexSettingsPanel::show(GtkWindow* parent) {}

void LatexSettingsPanel::checkDeps() {
    LatexSettings settings;
    this->save(settings);
    std::string msg;

    if (fs::is_regular_file(settings.globalTemplatePath)) {
        // Assume the file is encoded as UTF-8 (open in binary mode to avoid surprises)
        std::ifstream is(settings.globalTemplatePath, std::ios_base::binary);
        if (!is.is_open()) {
            msg = FS(_F("Unable to open global template file at {1}. Does it exist?") %
                     settings.globalTemplatePath.u8string().c_str());
        } else {
            std::string templ(std::istreambuf_iterator<char>(is), {});
            std::string sample = LatexGenerator::templateSub("x^2", templ, 0x000000U);
            auto const& tmpDir = Util::getTmpDirSubfolder("tex");
            auto result = LatexGenerator(settings).asyncRun(tmpDir, sample);
            if (auto* proc = std::get_if<GSubprocess*>(&result)) {
                GError* err = nullptr;
                if (g_subprocess_wait_check(*proc, nullptr, &err)) {
                    msg = _("Sample LaTeX file generated successfully.");
                } else {
                    msg = FS(_F("Error: {1}. Please check the contents of {2}") % err->message %
                             tmpDir.u8string().c_str());
                    g_error_free(err);
                }
                g_object_unref(*proc);
            } else if (auto* err = std::get_if<LatexGenerator::GenError>(&result)) {
                msg = err->message;
            }
        }
    } else {
        msg = FS(_F("Error: {1} is not a regular file. Please check your LaTeX template file settings. ") %
                 settings.globalTemplatePath.u8string().c_str());
    }
    GtkWidget* dialog =
            gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg.c_str());
    wait_for_gtk_dialog_result(GTK_DIALOG(dialog));
    gtk_window_destroy(GTK_WINDOW(dialog));
}
