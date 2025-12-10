#include <gtk/gtk.h>
#include <string>
#include <array>
#include <sstream>
#include <cstdlib>
#include <unistd.h>

// ───────────────────────────────────────────────
// Helpers
// ───────────────────────────────────────────────

static std::string exec_command(const std::string& cmd) {
    std::array<char, 256> buf{};
    std::string out;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";
    while (fgets(buf.data(), buf.size(), pipe)) {
        out += buf.data();
    }
    pclose(pipe);
    return out;
}

static const char* get_terminal() {
    const char* term = std::getenv("TERMINAL");
    if (term && *term) return term;
    // fallback
    return "xterm";
}

static void show_info_dialog(GtkWindow* parent, const char* title, const std::string& msg) {
    GtkWidget* dlg = gtk_message_dialog_new(
        parent,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "%s",
        title
    );
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dlg), "%s", msg.c_str());
    gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
}

static gboolean confirm(GtkWindow* parent, const char* title, const char* text) {
    GtkWidget* dlg = gtk_message_dialog_new(
        parent,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_QUESTION,
        GTK_BUTTONS_YES_NO,
        "%s",
        title
    );
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dlg), "%s", text);
    gint res = gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
    return (res == GTK_RESPONSE_YES);
}

// ───────────────────────────────────────────────
// Callbacks
// ───────────────────────────────────────────────

static void on_update_clicked(GtkButton* /*button*/, gpointer user_data) {
    GtkWindow* win = GTK_WINDOW(user_data);
    std::string term = get_terminal();
    std::string cmd = term + " -e sudo pacman -Syu";
    system(cmd.c_str());
    show_info_dialog(win, "Update", "System update command launched in terminal.");
}

static void on_reset_sound_clicked(GtkButton* /*button*/, gpointer user_data) {
    GtkWindow* win = GTK_WINDOW(user_data);

    // PipeWire + WirePlumber (user)
    int rc = system("systemctl --user restart pipewire.service wireplumber.service 2>/dev/null");

    if (rc != 0) {
        // fallback to PulseAudio
        system("pulseaudio -k 2>/dev/null");
    }

    show_info_dialog(win, "Sound Reset", "Audio services have been restarted.");
}

static void on_reset_network_clicked(GtkButton* /*button*/, gpointer user_data) {
    GtkWindow* win = GTK_WINDOW(user_data);
    std::string term = get_terminal();
    std::string cmd = term + " -e sudo systemctl restart NetworkManager";
    system(cmd.c_str());
    show_info_dialog(win, "Network Reset", "NetworkManager restart command launched in terminal.");
}

static void on_shutdown_clicked(GtkButton* /*button*/, gpointer user_data) {
    GtkWindow* win = GTK_WINDOW(user_data);
    if (!confirm(win, "Shutdown", "Are you sure you want to power off this computer?"))
        return;
    system("systemctl poweroff");
}

static void on_reboot_clicked(GtkButton* /*button*/, gpointer user_data) {
    GtkWindow* win = GTK_WINDOW(user_data);
    if (!confirm(win, "Restart", "Are you sure you want to restart this computer?"))
        return;
    system("systemctl reboot");
}

// ───────────────────────────────────────────────
// UI
// ───────────────────────────────────────────────

static GtkWidget* create_system_info_label() {
    std::ostringstream oss;

    // hostname
    char host[256] = {0};
    gethostname(host, sizeof(host)-1);

    std::string kernel = exec_command("uname -sr");
    std::string uptime = exec_command("uptime -p");

    // clean newlines
    auto trim_nl = [](std::string& s) {
        while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
    };
    trim_nl(kernel);
    trim_nl(uptime);

    oss << "Hostname: " << host << "\n";
    oss << "Kernel:   " << kernel << "\n";
    oss << "Uptime:   " << uptime << "\n";

    GtkWidget* label = gtk_label_new(oss.str().c_str());
    gtk_label_set_xalign(GTK_LABEL(label), 0.0); // left align
    gtk_widget_set_margin_top(label, 8);
    gtk_widget_set_margin_bottom(label, 8);
    gtk_widget_set_margin_start(label, 8);
    gtk_widget_set_margin_end(label, 8);

    // Use monospace font hint
    PangoFontDescription* font_desc = pango_font_description_from_string("monospace 10");
    gtk_widget_override_font(label, font_desc);
    pango_font_description_free(font_desc);

    return label;
}

static GtkWidget* create_main_window() {
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Command Center");
    gtk_window_set_default_size(GTK_WINDOW(window), 420, 260);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), nullptr);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Title
    GtkWidget* title = gtk_label_new("System Command Center");
    gtk_widget_set_margin_top(title, 8);
    gtk_widget_set_margin_bottom(title, 4);
    gtk_widget_set_margin_start(title, 8);
    gtk_widget_set_margin_end(title, 8);
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 0);

    // System info
    GtkWidget* info_label = create_system_info_label();
    gtk_box_pack_start(GTK_BOX(vbox), info_label, FALSE, FALSE, 0);

    // Buttons grid
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_widget_set_margin_top(grid, 8);
    gtk_widget_set_margin_bottom(grid, 8);
    gtk_widget_set_margin_start(grid, 8);
    gtk_widget_set_margin_end(grid, 8);
    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 0);

    GtkWidget* btn_update  = gtk_button_new_with_label("Update System");
    GtkWidget* btn_sound   = gtk_button_new_with_label("Reset Sound");
    GtkWidget* btn_net     = gtk_button_new_with_label("Reset Network");
    GtkWidget* btn_shutdown= gtk_button_new_with_label("Shutdown");
    GtkWidget* btn_reboot  = gtk_button_new_with_label("Restart");

    gtk_grid_attach(GTK_GRID(grid), btn_update,   0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_sound,    1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_net,      0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_shutdown, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_reboot,   0, 2, 2, 1);

    GtkWindow* win = GTK_WINDOW(window);
    g_signal_connect(btn_update,   "clicked", G_CALLBACK(on_update_clicked),   win);
    g_signal_connect(btn_sound,    "clicked", G_CALLBACK(on_reset_sound_clicked), win);
    g_signal_connect(btn_net,      "clicked", G_CALLBACK(on_reset_network_clicked), win);
    g_signal_connect(btn_shutdown, "clicked", G_CALLBACK(on_shutdown_clicked), win);
    g_signal_connect(btn_reboot,   "clicked", G_CALLBACK(on_reboot_clicked),   win);

    return window;
}

int main(int argc, char** argv) {
    gtk_init(&argc, &argv);

    GtkWidget* window = create_main_window();
    gtk_widget_show_all(window);

    gtk_main();
    return 0;
}

