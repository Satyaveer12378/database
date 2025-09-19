// test_eventbox.c
// Compile:
//   gcc `pkg-config --cflags --libs gtk+-3.0` -g test_eventbox.c -o test_eventbox
// Run:
//   ./test_eventbox

#include <gtk/gtk.h>
#include <stdlib.h>

const char *css =
  "window { background: #222222; }"

  /* Force visible rectangle behind eventbox with class label-box */
  "eventbox.label-box {"
  "  background-color: #2e3436;"
  "  background-image: none;"
  "  border: 2px solid rgba(255,255,255,0.06);"
  "  border-radius: 6px;"
  "  padding: 6px 8px;"
  "  min-width: 140px;"
  "}"
  /* label text inside eventbox */
  "eventbox.label-box label {"
  "  color: white;"
  "  font-size: 14px;"
  "  font-weight: 800;"
  "}"
  /* value button */
  ".button.value-rect {"
  "  background-color: #000000;"
  "  background-image: none;"
  "  border-radius: 6px;"
  "  border: 2px solid rgba(255,255,255,0.06);"
  "  padding: 8px;"
  "  font-size: 18px;"
  "  color: #00ff66;"
  "}"
  ".button.value-rect.selected {"
  "  background-color: #ffffff;"
  "  color: #333333;"
  "  border: 2px solid #e6b04d;"
  "}";

static void on_value_clicked(GtkButton *btn, gpointer _)
{
    /* toggle selected class for demo */
    GtkStyleContext *ctx = gtk_widget_get_style_context(GTK_WIDGET(btn));
    if (gtk_style_context_has_class(ctx, "selected"))
        gtk_style_context_remove_class(ctx, "selected");
    else
        gtk_style_context_add_class(ctx, "selected");

    const char *menu = g_object_get_data(G_OBJECT(btn), "menu");
    g_print("Clicked value for %s -> %s\n", menu ? menu : "(null)", gtk_button_get_label(btn));
}

static GtkWidget *create_value_row(const char *menu_text, const char *value_text)
{
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);

    /* EventBox (must draw its own background) */
    GtkWidget *menu_box = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(menu_box), TRUE);

    /* add class and name as double-safety */
    gtk_style_context_add_class(gtk_widget_get_style_context(menu_box), "label-box");
    gtk_widget_set_name(menu_box, "label-box"); /* fallback id selector - harmless */

    GtkWidget *label = gtk_label_new(menu_text);
    gtk_container_add(GTK_CONTAINER(menu_box), label);

    /* Value (button) */
    GtkWidget *btn = gtk_button_new_with_label(value_text);
    gtk_style_context_add_class(gtk_widget_get_style_context(btn), "value-rect");
    g_object_set_data_full(G_OBJECT(btn), "menu", g_strdup(menu_text), g_free);
    g_signal_connect(btn, "clicked", G_CALLBACK(on_value_clicked), NULL);

    /* size requests to ensure visible rectangle */
    gtk_widget_set_size_request(menu_box, 150, 36);
    gtk_widget_set_size_request(btn, 150, 44);

    gtk_box_pack_start(GTK_BOX(vbox), menu_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 0);

    gtk_widget_show_all(vbox);
    return vbox;
}

static void load_css(void)
{
    GtkCssProvider *prov = gtk_css_provider_new();
    GError *err = NULL;
    if (!gtk_css_provider_load_from_data(prov, css, -1, &err)) {
        g_printerr("CSS load failed: %s\n", err ? err->message : "(null)");
        g_clear_error(&err);
    } else {
        g_print("CSS loaded OK\n");
        /* Use APPLICATION priority so theme doesn't override our background */
        gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                                 GTK_STYLE_PROVIDER(prov),
                                                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
    g_object_unref(prov);
}

static void on_activate(GtkApplication *app, gpointer _)
{
    GtkWidget *w = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(w), 800, 480);
    gtk_window_set_title(GTK_WINDOW(w), "EventBox test");

    load_css();

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_container_add(GTK_CONTAINER(w), hbox);

    GtkWidget *left = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_hexpand(left, TRUE);
    gtk_widget_set_vexpand(left, TRUE);
    gtk_box_pack_start(GTK_BOX(hbox), left, TRUE, TRUE, 12);

    GtkWidget *right = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(hbox), right, FALSE, FALSE, 12);

    /* create 5 rows */
    for (int i = 1; i <= 5; ++i) {
        char menu[32], val[32];
        g_snprintf(menu, sizeof(menu), "MENU-%d", i);
        g_snprintf(val, sizeof(val), "%d", 100 + (i==1 ? 1 : 0)); /* first = 101 for visibility */
        GtkWidget *row = create_value_row(menu, val);
        gtk_box_pack_start(GTK_BOX(right), row, FALSE, FALSE, 8);
    }

    gtk_widget_show_all(w);
}

int main(int argc, char **argv)
{
    GtkApplication *app = gtk_application_new("org.test.eventbox", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(app, argc, argv);
    g_object_unref(app);
    return status;
}

