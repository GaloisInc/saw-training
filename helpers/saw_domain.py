from collections import defaultdict
from docutils.parsers.rst import directives
from sphinx import addnodes
from sphinx.directives import ObjectDescription
from sphinx.domains import Domain, Index
from sphinx.domains.std import StandardDomain
from sphinx.roles import XRefRole
from sphinx.util.nodes import make_refnode

class SAWDocDirective(ObjectDescription):
    """An abstract node that describes SAWScript names."""
    has_content = True
    required_arguments = 1
    option_spec = {
        'type': directives.unchanged_required
    }

    extra_desc = None

    def handle_signature(self, sig, signode):
        signode += addnodes.desc_name(text=sig)
        t = self.options.get('type')
        if t is not None:
            signode += addnodes.desc_type(raw_source='&nbsp;:&nbsp;', text=' : ')
            signode += addnodes.desc_type(text=t)
        if self.extra_desc is not None:
            signode += addnodes.desc_type(raw_source='&nbsp;', text=' ')
            signode += addnodes.desc_annotation(text=f'({self.extra_desc})')
        return sig

    def add_target_and_index(self, name_cls, sig, signode):
        if self.extra_desc is not None:
            signode['ids'].append('saw-' + self.extra_desc + '-' + sig)
            if 'noindex' not in self.options:
                saw_domain = self.env.get_domain('saw')
                saw_domain.add_term(sig, self.extra_desc)


class SAWCommandDirective(SAWDocDirective):
    """A node that describes SAW commands."""

    extra_desc = 'command'

class SAWFunctionDirective(SAWDocDirective):
    """A node that describes SAW commands."""

    extra_desc = 'function'

class SAWValueDirective(SAWDocDirective):
    """A node that describes SAW values."""

    extra_desc = 'value'

class SAWTypeDirective(ObjectDescription):
    """A node that descsribes SAWScript types."""
    has_content = True
    required_arguments = 1
    option_spec = {'constructors': directives.unchanged}

    def handle_signature(self, sig, signode):
        signode += addnodes.desc_name(text=sig)
        signode += addnodes.desc_type(raw_source='&nbsp;', text=' ')
        signode += addnodes.desc_annotation(text='(type)')

        return sig

    def add_target_and_index(self, name_cls, sig, signode):
        signode['ids'].append('saw-type' + '-' + sig)
        if 'noindex' not in self.options:
            saw_domain = self.env.get_domain('saw')
            saw_domain.add_term(sig, 'type')

        ctors = self.options.get('constructors')
        if ctors is not None:
            for c in ctors.split():
                signode['ids'].append('saw-constructor' + '-' + c)
                if 'noindex' not in self.options:
                    saw_domain = self.env.get_domain('saw')
                    saw_domain.add_term(c, 'constructor')

class SAWSyntaxDirective(ObjectDescription):
    """A node that descsribes SAWScript syntactic constructs."""
    has_content = True
    required_arguments = 1
    option_spec = {}

    def handle_signature(self, sig, signode):
        signode += addnodes.desc_name(text=sig)
        signode += addnodes.desc_type(raw_source='&nbsp;', text=' ')
        signode += addnodes.desc_annotation(text='(syntax)')

        return sig

    def add_target_and_index(self, name_cls, sig, signode):
        signode['ids'].append('saw-syntax' + '-' + sig)
        if 'noindex' not in self.options:
            saw_domain = self.env.get_domain('saw')
            saw_domain.add_term(sig, 'syntax')


class SAWOperatorIndex(Index):
    """An index over all the documented SAW operators."""

    name = 'saw-operators'
    localname = 'SAW Operator Index'
    shortname = 'Operators'

    def generate(self, docnames=None):
        content = defaultdict(list)

        # sort the list of recipes in alphabetical order
        operators = self.domain.get_objects()
        operators = sorted(operators, key=lambda op: op[0])

        # name, subtype, docname, anchor, extra, qualifier, description
        for name, dispname, typ, docname, anchor, _ in operators:
            content[dispname[0].lower()].append(
                (dispname, 0, docname, anchor, docname, '', typ)
            )

        # convert the dict to the sorted list of tuples expected
        content = sorted(content.items())

        return content, True



class SAWDomain(Domain):
    name = 'saw'
    label = 'SAW'
    roles = {'ref': XRefRole()}
    directives = {'command': SAWCommandDirective,
                  'function': SAWFunctionDirective,
                  'type': SAWTypeDirective,
                  'syntax': SAWSyntaxDirective,
                  'value': SAWValueDirective}
    indices = {SAWOperatorIndex}
    initial_data = {'documented': defaultdict(list)}

    def resolve_xref(self, env, fromdocname, builder, typ, target, node,
                     contnode):
        match = [(docname, anchor)
                 for name, sig, typ, docname, anchor, prio
                 in self.get_objects() if sig == target]

        if len(match) > 0:
            todocname = match[0][0]
            targ = match[0][1]

            return make_refnode(builder, fromdocname, todocname, targ,
                                contnode, targ)
        else:
            print(f'Found no target for {target}')
            return None

    def add_term(self, signature, desc):
        """Add a new SAW command to the domain."""
        name = f'saw.{desc}.{signature}'
        anchor = f'saw-{desc}-{signature}'
        self.data['documented'][desc].append(
            (name, signature, f'SAW {desc}', self.env.docname, anchor, 0)
        )

    def get_objects(self):
        for category in self.data['documented']:
            for obj in self.data['documented'][category]:
                yield obj

def setup(sphinx):
    sphinx.add_domain(SAWDomain)
    return {
        'version': '0.1',
        'parallel_read_safe': True,
        'parallel_write_safe': True,
    }
