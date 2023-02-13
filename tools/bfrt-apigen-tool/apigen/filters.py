import re
import functools


apigen_filters = dict()
apigen_public_prefix = 'BFAPI'

def apigen_filter(func):
    global apigen_filters
    apigen_filters[func.__name__] = func
    return func

@apigen_filter
def normalize_field_name(name):
    # Replace $number with _number_ ($5name -> _5_name)
    name = re.sub(r'\$(\d+)', r'_\1_', name)

    # Replace $ with _
    name = re.sub(r'\$', r'_', name)

    # Remove prefix
    name = re.sub(r'^pipe\.', r'', name)

    # Replace SwitchIngress with 'i'
    name = re.sub(r'SwitchIngress', r'i', name)

    # Replace SwitchEngress with 'e'
    name = re.sub(r'SwitchEgress', r'e', name)

    # Replace Parse with 'p'
    name = re.sub(r'Parser', r'p', name)

    return re.sub('[.\[\]\:]', '_', name)

@apigen_filter
def camelize(phrase):
    words = re.findall(r'[\w]+', phrase)
    return ''.join([ word.capitalize() for word in words])

@apigen_filter
def acronymize(field_name):
    tokens = field_name.split('.')
    acronyms = []
    for token in tokens:
        words =token.split('_')
        if len(words) == 1:
            acronyms.append(words[0].upper())
        else:
            acronyms.append(''.join([w[0].upper() for w in words if w]))
    return '_'.join(acronyms)

@apigen_filter
def normalize(name):
    # Replace $number with _number_ ($5name -> _5_name)
    name = re.sub(r'\$(\d+)', r'_\1_', name)
    # Replace $ with _
    name = re.sub(r'\$', r'_', name)
    return re.sub('[.\[\]]', '_', name)


@apigen_filter
def abbreviate(full_name):
    abbr = re.sub(r'^pipe\.', '', full_name)
    abbr = re.sub(r'SwitchIngress', r'i', abbr)
    abbr = re.sub(r'SwitchEngress', r'e', abbr)
    return abbr

@apigen_filter
def mk_c_type(bf_data_type):
    if not isinstance(bf_data_type, dict):
        return bf_data_type
    else:
        if bf_data_type['type'] == 'bytes':
            bytes_width = bf_data_type['width']
            if bytes_width == 1:
                return 'bool'
            elif bytes_width <= 8:
                return 'uint8_t'
            elif bytes_width <= 16:
                return 'uint16_t'
            elif bytes_width <= 32:
                return 'uint32_t'
            elif bytes_width <= 64:
                return 'uint64_t'
            else:
                return 'uint8_t *'
        elif bf_data_type['type'] == 'uint32':
            return 'uint32_t'
        elif bf_data_type['type'] == 'string':
            return 'char *'

@apigen_filter
def type_prefix(c_type_str):
    if c_type_str == 'bool':
        return 'b'
    elif c_type_str == 'uint8_t':
        return 'uc'
    elif c_type_str == 'uint16_t':
        return 'us'
    elif c_type_str == 'uint32_t':
        return 'ui'
    elif c_type_str == 'uint64_t':
        return 'ul'
    else:
        return 'puc'
    
@apigen_filter
def last_part(name, splitor='.'):
    return name.split(splitor)[-1] 


@apigen_filter
def mk_public_func_prefix(table_name):
    global apigen_public_prefix
    name = camelize(last_part(table_name))
    
    return ''.join([apigen_public_prefix, name])
    
@apigen_filter
def mk_type_prefix(table_name):
    global apigen_public_prefix

    name = last_part(table_name).replace('_', '').upper()
    return '_'.join([apigen_public_prefix, name])
    
@apigen_filter
def clip_tablename(tablename):
    
    # translate non-alphabetic characters into None
    tablename = tablename.translate({ord(c): None for c in '$'}).lower()

    # replace '.' to '_'
    tablename = tablename.replace('.', '_')

    # cut prefix
    prefixes = ['pipe_']
    for prefix in prefixes:
        if tablename.startswith(prefix):
            tablename = tablename[len(prefix):]

    # Add suffix and return
    return tablename + '_tbl'

@apigen_filter
def clip_tablename2(tablename):
    # translate non-alphabetic characters into None
    tablename = tablename.translate({ord(c): None for c in '$'})

    # split the table name and get the last one as cliped name
    return tablename.split('.')[-1].lower()
    

@apigen_filter
def clip_actionname(action_name):
    return action_name.split('.')[-1].lower()

@apigen_filter
def clip_keyname(key_name):
    cliped_name = key_name.split('.')[-1].lower()
    return cliped_name.replace('$', '')

@apigen_filter
def mk_tbname(tbname):
    '''
    Simplify the table name that read from bf-rt.json file.
    '''

    # Remove 'pipe.' prefix
    tbname = re.sub(r'^pipe\.', '', tbname)

    # Abbrivate long words
    abbrs = {
        'SwitchIngress': 'i', 
        'SwitchEngress': 'e',
        'SwitchIgressParser': 'ip'
    }
    for word, abbr in abbrs.items():
        tbname = tbname.replace(word, abbr)

    # Acronymize field names and re-concatenated with '_'
    field_names = tbname.split('.')

    return '_'.join([acronymize(n) for n in field_names])



if __name__ == '__main__':
    print(apigen_filters)
