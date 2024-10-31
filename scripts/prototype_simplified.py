
class Ast:
    def __init__(self, name:str) -> None:
        self.name = name

class Value(Ast):
    def __init__(self, name:str, value) -> None:
        super().__init__(name)
        self.value = value
        
class Int(Value):
    def __init__(self, value) -> None:
        super().__init__("Int", value)

class Float(Value):
    def __init__(self, value) -> None:
        super().__init__("Float", value)
        
class Bool(Value):
    def __init__(self, value) -> None:
        super().__init__("Bool", value)
        
class Char(Value):
    def __init__(self, value) -> None:
        super().__init__("Char", value)
        
class Array(Ast):
    def __init__(self, inner:Ast) -> None:
        super().__init__("Array")
        self.inner = inner
        
class Tuple(Ast):
    def __init__(self, args:list[Ast]) -> None:
        super().__init__("Tuple")
        self.args = args

class Function(Ast):
    def __init__(self, args:list[Ast], body:Ast) -> None:
        super().__init__("Function")
        
