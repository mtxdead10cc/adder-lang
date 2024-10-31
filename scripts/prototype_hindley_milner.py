
class MonoType:
    def find(self) -> 'MonoType':
        return self

class TyVar(MonoType):
    def __init__(self, name: str) -> None:
        self.forwarded: MonoType = None
        self.name: str = name

    def find(self) -> MonoType:
        # Exercise for the reader: path compression
        result: MonoType = self
        while isinstance(result, TyVar):
            it = result.forwarded
            if it is None:
                return result
            result = it
        return result

    def make_equal_to(self, other: MonoType) -> None:
        chain_end = self.find()
        assert isinstance(chain_end, TyVar), f"already resolved to {chain_end}"
        chain_end.forwarded = other
        
    def __str__(self) -> str:
        target = self.find()
        if target != self:
            return f"[var '{self.name}' >> {target}]"
        else:
            return f"[var '{self.name}']"

class TyCon(MonoType):
    def __init__(self, name:str, args:list[MonoType]) -> None:
        self.name = name
        self.args = args
        
    def __str__(self) -> str:
        return f"[con '{self.name}' [{','.join([str(s) for s in self.args])}]]"
    
IntType = TyCon("int", [])
BoolType = TyCon("bool", [])

def list_type(ty: MonoType) -> MonoType:
    return TyCon("list", [ty])

def func_type(arg: MonoType, ret: MonoType) -> MonoType:
    return TyCon("->", [arg, ret])

class Forall:
    def __init__(self, tyvars: list[TyVar], ty: MonoType) -> None:
        self.tyvars = tyvars
        self.ty = ty

fresh_var_counter = 0

def fresh_tyvar(prefix: str = "t") -> TyVar:
    global fresh_var_counter
    result = f"{prefix}{fresh_var_counter}"
    fresh_var_counter += 1
    return TyVar(result)

class Node:
    pass

class Var(Node):
    def __init__(self, name:str) -> None:
        self.name:str = name

class Int(Node):
    pass

class Bool(Node):
    pass

class Function(Node):
    def __init__(self, arg:Var, body:Node) -> None:
        assert isinstance(arg, Var)
        self.arg = arg
        self.body = body

class Apply(Node):
    def __init__(self, func:Node, arg:Node) -> None:
        self.func = func
        self.arg = arg
    
class Assign(Node):
    def __init__(self,  name: Var, value: Node) -> None:
        assert isinstance(name, Var)
        self.name = name
        self.value = value

class Where(Node):
    def __init__(self, binding:Assign, body:Node) -> None:
        assert isinstance(binding, Assign)
        self.binding = binding
        self.body = body
    
def unify(ty1: MonoType, ty2: MonoType) -> None:
    ty1 = ty1.find()
    ty2 = ty2.find()
    if isinstance(ty1, TyVar):
        ty1.make_equal_to(ty2)
        return
    if isinstance(ty2, TyVar):  # Mirror
        return unify(ty2, ty1)
    if isinstance(ty1, TyCon) and isinstance(ty2, TyCon):
        if ty1.name != ty2.name:
            raise TypeError(f"Failed to unify {type(ty1)} and {type(ty2)}")
        if len(ty1.args) != len(ty2.args):
            raise TypeError(f"Failed to unify args of {type(ty1)} and {type(ty2)}")
        for l, r in zip(ty1.args, ty2.args):
            unify(l, r)
        return
    raise TypeError(f"Unexpected type {type(ty1)}")


def apply_ty(ty: MonoType, subst: dict[str, MonoType]) -> MonoType:
    if isinstance(ty, TyVar):
        return subst.get(ty.name, ty)
    if isinstance(ty, TyCon):
        return TyCon(ty.name, [apply_ty(arg, subst) for arg in ty.args])
    raise TypeError(f"Unknown type: {ty}")


def instantiate(scheme: Forall) -> MonoType:
    fresh = {tyvar.name: fresh_tyvar() for tyvar in scheme.tyvars}
    return apply_ty(scheme.ty, fresh)


def ftv_ty(ty: MonoType) -> set[str]:
    if isinstance(ty, TyVar):
        return {ty.name}
    if isinstance(ty, TyCon):
        return set().union(*map(ftv_ty, ty.args))
    raise TypeError(f"Unknown type: {ty}")


def generalize(ty: MonoType, ctx: dict[str, Forall]) -> Forall:
    def ftv_scheme(ty: Forall) -> set[str]:
        return ftv_ty(ty.ty) - set(tyvar.name for tyvar in ty.tyvars)

    def ftv_ctx(ctx: dict[str, Forall]) -> set[str]:
        return set().union(*(ftv_scheme(scheme) for scheme in ctx.values()))

    # TODO(max): Freshen?
    # TODO(max): Test with free type variable in the context
    tyvars = ftv_ty(ty) - ftv_ctx(ctx)
    return Forall([TyVar(name) for name in sorted(tyvars)], ty)

def recursive_find(ty: MonoType) -> MonoType:
    if isinstance(ty, TyVar):
        found = ty.find()
        if ty is found:
            return found
        return recursive_find(found)
    if isinstance(ty, TyCon):
        return TyCon(ty.name, [recursive_find(arg) for arg in ty.args])
    raise TypeError(type(ty))

def infer(expr: Node, ctx: dict[str, Forall]) -> MonoType:
    result = fresh_tyvar()
    if isinstance(expr, Var):
        scheme = ctx.get(expr.name)
        if scheme is None:
            raise TypeError(f"Unbound variable {expr.name}")
        unify(result, instantiate(scheme))
        return result
    if isinstance(expr, Int):
        unify(result, IntType)
        return result
    if isinstance(expr, Bool):
        unify(result, BoolType)
        return result
    if isinstance(expr, Function):
        arg_tyvar = fresh_tyvar("a")
        assert isinstance(expr.arg, Var)
        body_ctx = {**ctx, expr.arg.name: Forall([], arg_tyvar)}
        body_ty = infer(expr.body, body_ctx)
        unify(result, TyCon("->", [arg_tyvar, body_ty]))
        return result
    if isinstance(expr, Apply):
        func_ty = infer(expr.func, ctx)
        arg_ty = infer(expr.arg, ctx)
        unify(func_ty, TyCon("->", [arg_ty, result]))
        return result
    if isinstance(expr, Where):
        name, value, body = expr.binding.name.name, expr.binding.value, expr.body
        value_ty = infer(value, ctx)
        value_scheme = generalize(recursive_find(value_ty), ctx)
        body_ty = infer(body, {**ctx, name: value_scheme})
        unify(result, body_ty)
        return result
    raise TypeError(f"Unexpected type {type(expr)}")


if __name__ == "__main__":
    
    # create fake function f: T -> T
    ctx = {"f": Forall([], func_type(TyVar("T"), TyVar("T")))}
    
    expr = Where(
        Assign(
            Var("y"),
            Apply(Var("f"), Int())),
        Where(
            Assign(Var("x"), Apply(Var("f"), Int())),
            Var("x"))
    )
    
    res = infer(expr, ctx)
    print(res)
