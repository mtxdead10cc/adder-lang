import gdb
from gdb import Value

def srcref_to_string(srcref:Value) -> str:
    idx_start: Value = srcref['idx_start']
    idx_end: Value = srcref['idx_end']
    source:Value = srcref['src']
    if source.address == 0:
        return "<srcref-null>"
    buffer = (source["buff"] + idx_start)
    result = buffer.string(length=idx_end-idx_start)
    return f"{result}"


class SrcPrinter:
    def __init__(self, val):
        self.val = val

    def children(self):
        yield "path_length", self.val['path_length']
        yield "buff_length", self.val['buff_length']
        yield "path", self.val['path'].string(length=self.val['path_length'])
        yield "buff", self.val['buff'].string(length=self.val['buff_length'])

    def display_hint(self):
        return 'array'

    def to_string(self):
        buffer = self.val["buff"]
        result = buffer.string(length=self.val["buff_length"])
        return f"{result}"

class SrcrefPrinter:
    def __init__(self, val):
        self.val = val

    def children(self):
        yield "idx_start", self.val['idx_start']
        yield "idx_end", self.val['idx_end']
        yield "src", self.val['src']

    def display_hint(self):
        return 'string'

    def to_string(self):
        return srcref_to_string(self.val)

class AstPrinter:

    def __init__(self, val):
        self.val:Value = val
    
    def children(self):
        
        count = int(self.val["size"])
        if count == 0:
            tag_name = str(self.val['tag'])
            if tag_name == "AST_INT":
                yield "value_int", self.val["as"]["value_int"]
            elif tag_name == "AST_FLOAT":
                yield "value_float", self.val["as"]["value_float"]
            elif tag_name == "AST_CHAR":
                yield "value_char", self.val["as"]["value_char"]
            elif tag_name == "AST_BOOL":
                yield "value_bool", self.val["as"]["value_bool"]
            elif tag_name == "AST_STRING" or tag_name == "AST_SYMBOL":
                yield "srcref", self.val["as"]["srcref"]
            else:
                yield "tag", self.val['tag']
                yield "size", self.val["size"]
        else:
            ptr: Value = self.val["as"]["items"]
            for i in range(count):
                yield f"item[{i}]", (ptr + i).dereference()

    def to_string(self):

        tag_name = str(self.val['tag'])
        count = int(self.val["size"])
        if int(self.val["size"]) > 0:
            return tag_name + f" ({count})"
        
        value = None
        if tag_name == "AST_INT":
            value = str(int(self.val["as"]["value_int"]))
        elif tag_name == "AST_FLOAT":
            value = str(float(self.val["as"]["value_float"]))
        elif tag_name == "AST_CHAR":
            value = str(self.val["as"]["value_char"])
        elif tag_name == "AST_BOOL":
            value = str(bool(self.val["as"]["value_bool"]))
        elif tag_name == "AST_STRING" or tag_name == "AST_SYMBOL":
            value = srcref_to_string(self.val["as"]["srcref"])

        if value != None:
            return tag_name + f" ({value})"

        return tag_name + " (empty)"
            
    def display_hint(self):
        return 'array'

def json_string_to_string(jstr:Value) -> str:
    textptr: Value = jstr['text']
    length: Value = jstr['length']
    if textptr.address == 0:
        return "<null>"
    result = textptr.string(length=int(length))
    return str(result)

class JsonObjectPrinter:

    def __init__(self, val):
        self.val:Value = val
    
    def children(self):
        count = int(self.val["size"])
        valptr: Value = self.val["values"]
        keyptr: Value = self.val["keys"]
        yield "size", self.val["size"]
        yield "capacity", self.val["capacity"]
        for i in range(count):
            k = (keyptr + i).dereference()
            v = (valptr + i).dereference()
            kstr = json_string_to_string(k['as']['string'])
            yield "'" + kstr + "'", v

    def to_string(self):
        return f"json_object ({self.val['size']})"
            
    def display_hint(self):
        return 'array'
    
class JsonArrayPrinter:

    def __init__(self, val):
        self.val:Value = val
    
    def children(self):
        count = int(self.val["size"])
        valptr: Value = self.val["values"]
        yield "size", self.val["size"]
        yield "capacity", self.val["capacity"]
        for i in range(count):
            yield f"#{i}", (valptr + i).dereference()

    def to_string(self):
        return f"json_array ({self.val['size']})"
            
    def display_hint(self):
        return 'array'

class JsonStringPrinter:

    def __init__(self, val):
        self.val:Value = val
    
    def children(self):
        yield "text", self.val["text"]
        yield "length", self.val["length"]

    def to_string(self):
        return json_string_to_string(self.val)
            
    def display_hint(self):
        return 'string'
    
class JsonErrorPrinter:

    def __init__(self, val):
        self.val:Value = val
    
    def children(self):
        yield "expected", self.val["expected"]
        yield "got.text", self.val["got"]

    def to_string(self):
        return f"json_error (expected: {self.val["expected"]} got:\"{self.val['got']}\")"
            
    def display_hint(self):
        return 'array'
    
class JsonValuePrinter:

    def __init__(self, val):
        self.val:Value = val
    
    def children(self):
        yield "type", self.val["type"]

        tstr = str(self.val["type"])
        
        if tstr == "JSON_VALUE_NULL":
            yield "value", None
        elif tstr == "JSON_VALUE_STRING":
            yield "string", self.val['as']['string']
        elif tstr == "JSON_VALUE_NUMBER_DOUBLE":
            yield "number_double", self.val['as']['number_double']
        elif tstr == "JSON_VALUE_NUMBER_INTEGER":
            yield "number_integer", self.val['as']['number_integer']
        elif tstr == "JSON_VALUE_BOOLEAN":
            yield "boolean", self.val['as']['boolean']
        elif tstr == "JSON_VALUE_ARRAY":
            yield "array", self.val['as']['array']
        elif tstr == "JSON_VALUE_OBJECT":
            yield "object", self.val['as']['object']
        elif tstr == "JSON_VALUE_ERROR":
            yield "error", self.val['as']['error']

    def to_string(self):
        tstr = str(self.val["type"])
        if tstr == "JSON_VALUE_NULL":
            return "null"
        elif tstr == "JSON_VALUE_STRING":
            return "'" + json_string_to_string(self.val['as']['string']) + "'"
        elif tstr == "JSON_VALUE_NUMBER_DOUBLE":
            return str(float(self.val['as']['number_double']))
        elif tstr == "JSON_VALUE_NUMBER_INTEGER":
            return str(int(self.val['as']['number_integer']))
        elif tstr == "JSON_VALUE_BOOLEAN":
            return str(int(self.val['as']['boolean']))
        elif tstr == "JSON_VALUE_ARRAY":
            return f"json_array ({self.val['as']['array']['size']})"
        elif tstr == "JSON_VALUE_OBJECT":
            return f"json_object ({self.val['as']['object']['size']})"
        elif tstr == "JSON_VALUE_ERROR":
            return f"json_error ('{self.val['as']['error']['text']}')"
            
    def display_hint(self):
        return 'array'

def lookup_type(val: Value):
    tstr = str(val.type)
    if tstr == 'ast_t *':
        return AstPrinter(val)
    elif tstr == 'src_t *':
        return SrcPrinter(val)
    elif tstr == 'srcref_t *':
        return SrcrefPrinter(val)
    elif tstr == 'srcref_t':
        return SrcrefPrinter(val)
    elif tstr == 'json_object_t':
        return JsonObjectPrinter(val)
    elif tstr == 'json_array_t':
        return JsonArrayPrinter(val)
    elif tstr == 'json_string_t':
        return JsonStringPrinter(val)
    elif tstr == 'json_error_t':
        return JsonErrorPrinter(val)
    elif tstr == 'json_value_t *':
        return JsonValuePrinter(val)
    return None

gdb.pretty_printers.append(lookup_type)