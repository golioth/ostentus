try:
    open("main.py", "r")
except OSError:
    with open("main.py", "w") as f:
        f.write("from ostentus import ostentus\n\n")
        f.write("o = ostentus()\n")
        f.write("o.listen()\n")
        f.flush()

