import tkinter as tk
from tkinter import filedialog, messagebox
import subprocess
import os

def run_yrrd():
    # 1. Get the filename from the entry box
    filename = file_entry.get()
    #-w flag target words
    target_words = words_entry.get().strip()
    
    if not filename or not os.path.exists(filename):
        messagebox.showerror("Error", "Please select a valid file first.")
        return
       
    cmd = ["./yrrd", filename]
   
    # If the user typed words then implement -w flag
    if target_words:
        cmd.append("-w")
        cmd.extend(target_words.replace(',', ' ').split())
  
    try:
        # 2. Execute yrrd.c
        # (AFTER runnning 'make yrrd'
        subprocess.run(cmd, check=True)

        # 3. Read the output file
        if os.path.exists("results.txt"):
            with open("results.txt", "r") as f:
                output_text.delete(1.0, tk.END)  # Clear old results
                output_text.insert(tk.END, f.read())
        else:
            messagebox.showwarning("Warning", "Analysis finished, but results.txt was not found.")
            
    except subprocess.CalledProcessError:
        messagebox.showerror("Error", "The C program failed to run. Make sure it is compiled.")

def browse_file():
    file_path = filedialog.askopenfilename(filetypes=[("Text files", "*.txt"), ("All files", "*.*")])
    file_entry.delete(0, tk.END)
    file_entry.insert(0, file_path)

# --- UI Setup ---
root = tk.Tk()
root.title("yrrd Word Frequency Tool")
root.geometry("500x450")
root.configure(bg='#ccccff');

label_style = {"bg": "#2C3E50", "fg": "white", "font": ("Verdana", 10, "bold")}

# File selection row
frame = tk.Frame(root)
frame.pack(pady=20)

file_entry = tk.Entry(frame, width=40)
file_entry.pack(side=tk.LEFT, padx=5)

browse_btn = tk.Button(frame, text="Browse", command=browse_file)
browse_btn.pack(side=tk.LEFT)

# Word filtering (-w flag)
tk.Label(root, text="Step 2: Words to Count (Optional, space-separated)", **label_style).pack(pady=(20, 5))
words_entry = tk.Entry(root, width=52)
words_entry.pack(pady=5)

# Action button
run_btn = tk.Button(root, text="Analyze File", command=run_yrrd, bg="#6666CC", fg="white", font=('Verdana', 12, 'bold'))
run_btn.pack(pady=10)

# Results display
output_text = tk.Text(root, height=15, width=55, font=("Verdana", 10))
output_text.pack(padx=20, pady=10)

root.mainloop()
