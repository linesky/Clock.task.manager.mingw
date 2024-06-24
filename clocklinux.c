
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FILENAME "clock.ini"
#define TIMER_INTERVAL 60000

typedef struct {
    struct tm time;
    char message[256];
} Event;

Event events[100];
int event_count = 0;

GtkWidget *entry_datetime, *entry_message;

void load_events() {
    FILE *file = fopen(FILENAME, "r");
    if (!file) {
        file = fopen(FILENAME, "w");
        if (!file) {
            g_print("Could not create clock.ini file\n");
            return;
        }
        fclose(file);
        return;
    }

    event_count = 0;
    char line[512];
    while (fgets(line, sizeof(line), file) && event_count < 100) {
        int year, month, day, hour, minute;
        char message[256];
        if (sscanf(line, "%d\\%d\\%d %d:%d=%255[^\n]", &year, &month, &day, &hour, &minute, message) == 6) {
            events[event_count].time.tm_year = year - 1900;
            events[event_count].time.tm_mon = month - 1;
            events[event_count].time.tm_mday = day;
            events[event_count].time.tm_hour = hour;
            events[event_count].time.tm_min = minute;
            events[event_count].time.tm_sec = 0;
            strncpy(events[event_count].message, message, sizeof(events[event_count].message));
            event_count++;
        }
    }

    fclose(file);
}

void save_event(const char *datetime, const char *message) {
    FILE *file = fopen(FILENAME, "a");
    if (file) {
        fprintf(file, "%s=%s\n", datetime, message);
        fclose(file);
    }
}

gboolean check_events(gpointer data) {
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);

    for (int i = 0; i < event_count; i++) {
        if (difftime(mktime(&events[i].time), now) <= 0) {
            GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", events[i].message);
            gtk_window_set_title(GTK_WINDOW(dialog), "Event Notification");
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);

            // Remove the event from the list
            for (int j = i; j < event_count - 1; j++) {
                events[j] = events[j + 1];
            }
            event_count--;
            i--; // Check the new event at this position
        }
    }

    return TRUE; // Keep the timer running
}

void show_current_time(GtkWidget *widget, gpointer data) {
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);

    char buffer[256];
    strftime(buffer, sizeof(buffer), "Current Time: %Y\\%m\\%d %H:%M:%S", current_time);

    GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", buffer);
    gtk_window_set_title(GTK_WINDOW(dialog), "Current Time");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void add_event(GtkWidget *widget, gpointer data) {
    const char *datetime = gtk_entry_get_text(GTK_ENTRY(entry_datetime));
    const char *message = gtk_entry_get_text(GTK_ENTRY(entry_message));
    save_event(datetime, message);
    load_events(); // Recarrega os eventos
}

void open_notepad(GtkWidget *widget, gpointer data) {
    system("gedit " FILENAME);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Agenda");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 300);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    entry_datetime = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_datetime), "YYYY\\MM\\DD HH:MM");
    gtk_box_pack_start(GTK_BOX(vbox), entry_datetime, FALSE, FALSE, 0);

    entry_message = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_message), "Event message");
    gtk_box_pack_start(GTK_BOX(vbox), entry_message, FALSE, FALSE, 0);

    GtkWidget *button_notepad = gtk_button_new_with_label("Open Notepad");
    g_signal_connect(button_notepad, "clicked", G_CALLBACK(open_notepad), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button_notepad, FALSE, FALSE, 0);

    GtkWidget *button_showtime = gtk_button_new_with_label("Show Current Time");
    g_signal_connect(button_showtime, "clicked", G_CALLBACK(show_current_time), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button_showtime, FALSE, FALSE, 0);

    GtkWidget *button_add = gtk_button_new_with_label("Add Event");
    g_signal_connect(button_add, "clicked", G_CALLBACK(add_event), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button_add, FALSE, FALSE, 0);

    GdkRGBA yellow;
    gdk_rgba_parse(&yellow, "yellow");
    gtk_widget_override_background_color(window, GTK_STATE_FLAG_NORMAL, &yellow);

    load_events();
    g_timeout_add(TIMER_INTERVAL, check_events, NULL); // Verifica os eventos a cada minuto

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
