
import tkinter as tk
from tkinter import messagebox, scrolledtext
from datetime import datetime
import threading
import os

class AgendaApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Agenda")
        self.root.configure(bg='yellow')

        self.events = []
        self.load_events()

        self.label_date = tk.Label(root, text="Date and Time (YYYY/MM/DD HH:MM):", bg='yellow')
        self.label_date.grid(row=0, column=0, padx=10, pady=5, sticky=tk.W)
        self.entry_date = tk.Entry(root)
        self.entry_date.grid(row=0, column=1, padx=10, pady=5)

        self.label_event = tk.Label(root, text="Event:", bg='yellow')
        self.label_event.grid(row=1, column=0, padx=10, pady=5, sticky=tk.W)
        self.entry_event = tk.Entry(root)
        self.entry_event.grid(row=1, column=1, padx=10, pady=5)

        self.button_add = tk.Button(root, text="Add Event", command=self.add_event)
        self.button_add.grid(row=0, column=2, rowspan=2, padx=10, pady=5, sticky=tk.W+tk.E+tk.N+tk.S)

        self.text_events = scrolledtext.ScrolledText(root, width=50, height=10, state=tk.NORMAL)
        self.text_events.grid(row=2, column=0, columnspan=3, padx=10, pady=5)

        self.button_notepad = tk.Button(root, text="Edit Events", command=self.edit_events)
        self.button_notepad.grid(row=3, column=0, padx=10, pady=5, sticky=tk.W+tk.E)

        self.button_show_time = tk.Button(root, text="Show Current Time", command=self.show_current_time)
        self.button_show_time.grid(row=3, column=1, padx=10, pady=5, sticky=tk.W+tk.E)

        self.update_event_list()
        self.start_checking_events()

    def load_events(self):
        if os.path.exists("agenda.ini"):
            with open("agenda.ini", "r") as file:
                lines = file.readlines()
                for line in lines:
                    parts = line.strip().split("=")
                    if len(parts) == 2:
                        self.events.append((parts[0], parts[1]))

    def save_events(self):
        with open("agenda.ini", "w") as file:
            for event in self.events:
                file.write(f"{event[0]}={event[1]}\n")

    def add_event(self):
        date_str = self.entry_date.get()
        event_str = self.entry_event.get()
        try:
            datetime.strptime(date_str, "%Y/%m/%d %H:%M")
            self.events.append((date_str, event_str))
            self.save_events()
            self.update_event_list()
            self.entry_date.delete(0, tk.END)
            self.entry_event.delete(0, tk.END)
        except ValueError:
            messagebox.showerror("Error", "Invalid date format. Please use YYYY/MM/DD HH:MM.")

    def update_event_list(self):
        self.text_events.config(state=tk.NORMAL)
        self.text_events.delete(1.0, tk.END)
        for event in self.events:
            self.text_events.insert(tk.END, f"{event[0]}: {event[1]}\n")
        self.text_events.config(state=tk.DISABLED)

    def edit_events(self):
        os.system("notepad agenda.ini")
        self.events.clear()
        self.load_events()
        self.update_event_list()

    def show_current_time(self):
        now = datetime.now().strftime("%Y/%m/%d %H:%M")
        messagebox.showinfo("Current Time", now)

    def start_checking_events(self):
        self.check_events()
        self.root.after(60000, self.start_checking_events)  # Check every minute

    def check_events(self):
        now = datetime.now().strftime("%Y/%m/%d %H:%M")
        for event in self.events:
            if event[0] == now:
                messagebox.showinfo("Event Reminder", f"Event: {event[1]}")

if __name__ == "__main__":
    root = tk.Tk()
    app = AgendaApp(root)
    root.mainloop()
