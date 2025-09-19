// menu.c  -- drop this file in place of your current menu.c
// GTK3 version

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

/* CSS for the menu panel (GTK3-compatible) */
const char *css =
  "window { background: #000000; }"

  /* rectangle behind menu label (EventBox) */
  "eventbox.label-box {"
  "  background-color: #2e3436;"
  "  background-image: none;"
  "  border: 2px solid rgba(255, 255, 255, 0.06);"
  "  border-radius: 6px;"
  "  padding: 6px 8px;"
  "  min-width: 140px;"
  "}"

  /* DEBUG: to visually confirm selector matches, uncomment the line below */
  /* "eventbox.label-box { background-color: red; }" */

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
  "  border: 2px solid rgba(255, 255, 255, 0.06);"
  "  padding: 8px;"
  "  font-size: 18px;"
  "  color: #00ff66;"
  "}"

  /* selected value */
  ".button.value-rect.selected {"
  "  background-color: #ffffff;"
  "  color: #333333;"
  "  border: 2px solid #e6b04d;"
  "}";

/* Load CSS once with APPLICATION priority (so theme won't override) */
static void load_menu_css(void)
{
    static gboolean loaded = FALSE;
    if (loaded) return;

    GtkCssProvider *provider = gtk_css_provider_new();
    GError *err = NULL;
    if (!gtk_css_provider_load_from_data(provider, css, -1, &err)) {
        g_printerr("Failed to load menu CSS: %s\n", err ? err->message : "(unknown)");
        g_clear_error(&err);
    } else {
        /* Use APPLICATION priority to override theme defaults for these rules */
        gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                                 GTK_STYLE_PROVIDER(provider),
                                                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        g_print("Menu CSS loaded OK\n");
        loaded = TRUE;
    }
    g_object_unref(provider);
}

/* Global pointer to currently selected value button */
static GtkWidget *g_selected_value_btn = NULL;

/* Value button clicked handler */
static void on_value_clicked(GtkButton *btn, gpointer user_data)
{
    (void)user_data;

    /* Unselect previous */
    if (g_selected_value_btn && GTK_IS_WIDGET(g_selected_value_btn)) {
        GtkStyleContext *ctx_prev = gtk_widget_get_style_context(g_selected_value_btn);
        gtk_style_context_remove_class(ctx_prev, "selected");
    }

    /* Select this one */
    g_selected_value_btn = GTK_WIDGET(btn);
    gtk_style_context_add_class(gtk_widget_get_style_context(g_selected_value_btn), "selected");

    /* Example behavior: if the label is numeric, increment it */
    const char *lbl = gtk_button_get_label(btn);
    if (lbl) {
        /* try parse integer at start */
        char *endp = NULL;
        long val = strtol(lbl, &endp, 10);
        if (endp != lbl) {
            /* succeeded parsing an integer - update label */
            char buf[64];
            g_snprintf(buf, sizeof(buf), "%ld", val + 1);
            gtk_button_set_label(btn, buf);
        }
    }

    /* print debug info */
    const char *menu_name = g_object_get_data(G_OBJECT(btn), "menu-name");
    g_print("Clicked value for %s -> %s\n", menu_name ? menu_name : "(unknown)",
            gtk_button_get_label(btn));
}

/* Create single stacked row: EventBox(label) above Button(value) */
GtkWidget * create_value_row(const char *menu_text, const char *value_text)
{
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);

    /* EventBox to draw rectangle behind label */
    GtkWidget *menu_box = gtk_event_box_new();

    /* Force EventBox to draw its background on all themes */
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(menu_box), TRUE);

    /* Set a widget name as a fallback if needed */
    gtk_widget_set_name(menu_box, "label-box");

    /* Add class so CSS selector matches */
    gtk_style_context_add_class(gtk_widget_get_style_context(menu_box), "label-box");

    /* Label inside the eventbox */
    GtkWidget *menu_label = gtk_label_new(menu_text);
    gtk_label_set_xalign(GTK_LABEL(menu_label), 0.5);
    gtk_container_add(GTK_CONTAINER(menu_box), menu_label);

    /* Value button */
    GtkWidget *value_btn = gtk_button_new_with_label(value_text);
    gtk_style_context_add_class(gtk_widget_get_style_context(value_btn), "value-rect");

    /* Store menu_name for callback, freed automatically when button is destroyed */
    g_object_set_data_full(G_OBJECT(value_btn), "menu-name", g_strdup(menu_text), g_free);

    /* Connect click handler */
    g_signal_connect(value_btn, "clicked", G_CALLBACK(on_value_clicked), NULL);

    /* Ensure visible sizes (tweak to taste) */
    gtk_widget_set_size_request(menu_box, 150, 36);
    gtk_widget_set_size_request(value_btn, 150, 44);

    /* Pack */
    gtk_box_pack_start(GTK_BOX(vbox), menu_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), value_btn, FALSE, FALSE, 0);

    gtk_widget_show_all(vbox);
    return vbox;
}

/* Example menu_panel_new() function - integrate into your app */
GtkWidget * menu_panel_new(void)
{
    /* Create frame and vbox like your original layout */
    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_widget_set_size_request(frame, 170, -1);
    gtk_widget_set_vexpand(frame, TRUE);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
    gtk_container_set_border_width(GTK_CONTAINER(frame), 0);

    GtkStyleContext *fctx = gtk_widget_get_style_context(frame);
    gtk_style_context_add_class(fctx, "app-frame");
    gtk_style_context_add_class(fctx, "menu-panel");

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_add(GTK_CONTAINER(frame), vbox);

    /* Load CSS before creating rows */
    load_menu_css();

    /* Create rows (replace labels/values as needed) */
    gtk_box_pack_start(GTK_BOX(vbox), create_value_row("ZERO", "7.81 us"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_value_row("RANGE", "100"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_value_row("MTL VEL", "5920 M/S"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_value_row("DELAY", "0.00 mm"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), create_value_row("GAIN 1", "25.0 dB"), FALSE, FALSE, 0);

    /* Make first button selected by default */
    GList *rows = gtk_container_get_children(GTK_CONTAINER(vbox));
    if (rows) {
        GtkWidget *first_row = GTK_WIDGET(rows->data);
        if (GTK_IS_WIDGET(first_row)) {
            GList *inner = gtk_container_get_children(GTK_CONTAINER(first_row));
            if (inner && inner->next) {
                GtkWidget *btn = GTK_WIDGET(inner->next->data);
                g_selected_value_btn = btn;
                gtk_style_context_add_class(gtk_widget_get_style_context(btn), "selected");
            }
            if (inner) g_list_free(inner);
        }
        g_list_free(rows);
    }

    gtk_widget_show_all(frame);
    return frame;
}

