class AutoWindow(object):
    def __init__(self, tui):
        self.tui = tui
        self.list = []
        self.title = ""
        self.start = 0
        self.prev = {}
    
    def close(self):
        pass

    def vscroll(self, num):
        pass
    
    def render(self):
        if not self.tui.is_valid():
            return
        self.tui.title = self.title
        self.tui.erase()

        for item in self.list[self.start:]:
            self.tui.write(item)

    def create_auto(self):
        self.list = []
        try:
            frame = gdb.selected_frame()
        except gdb.error:
            self.title = "No Frame"
            self.list.append("No frame currently selected.\n\n")
            self.render()
            return
        self.title = frame.name()
        block = frame.block()
        while block:
            for symbol in block:
                if not symbol.is_variable and not symbol.is_argument:
                    continue
                value = frame.read_var(symbol, block)
                self.prev[symbol.name] = value
                self.list.append(f'{"A" if symbol.is_argument else " "}{symbol.line:<6}{str(symbol.type):<32}{symbol.name:<32}: {value}\n\n')
            block = block.superblock
        self.render()

def AutoWinFactory(tui):
    win = AutoWindow(tui)
    gdb.events.before_prompt.connect(win.create_auto)
    return win

    def close(self):
        gdb.events.before_prompt.disconnect(self.create_auto)

gdb.register_window_type("vars", AutoWinFactory)
