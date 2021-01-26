import sys
import re

import pygments
from pygments.lexer import RegexLexer, include, bygroups, using
from pygments.token import Error, Punctuation, Literal, Token, \
     Text, Comment, Operator, Keyword, Name, String, Number, Generic, \
     Whitespace
from pygments.lexers.haskell import CryptolLexer

__all__ = ['SAWScriptLexer']

class SAWScriptLexer(RegexLexer):
      name = "SAWScript"
      aliases = ["sawscript", "saw-script"]
      filenames = ["*.saw"]
      tokens = {
        'root': [
            (r'\s+', Text),
            (r'do', Keyword),
            (r'(let)(\s+)([a-zA-Z_][a-zA-Z_0-9]*)',
             bygroups(Keyword, Text, Name.Function)),
            (r'\b(crucible_[a-zA-Z_0-9]+|import|include|llvm_load_module|llvm_int|return)\b', Name.Builtin),
            (r'true|false', Keyword.Constant),
            (r'([a-zA-Z_][a-zA-Z0-9]*)(\s*)(<-)',
             bygroups(Name.Variable, Text, Operator.Word)),
            (r'(\()\s*([a-zA-Z_][a-zA-Z_0-9]*)(\s*\:\s*)([a-zA-Z_][a-zA-Z_0-9]*)\s*(\))',bygroups(Text,Name.Function, Text, Name.Function, Text)),
            (r'[a-zA-Z_][a-zA-Z_0-9]*', Name),
            (r'[0-9]+', Number),
            (r'\{\{', Literal, 'cryptol'),
            (r'<-', Operator.Word),
            (r'=', Operator.Word),
            (r'[\[\]{}\(\);,]', Punctuation),
            (r'/\*', Comment.Multiline, 'comment'),
            (r'//.*?$', Comment.Singleline),
            (r'"', String, 'string')
        ],
        'comment': [
            (r'[^*/]', Comment.Multiline),
            (r'/\*', Comment.Multiline, '#push'),
            (r'\*/', Comment.Multiline, '#pop'),
            (r'[*/]', Comment.Multiline)
        ],
        'string': [
            ('[^"]+', String),
            ('"', String, '#pop'),
        ],
        'cryptol': [
            (r'(.+?)(\}\})',
             bygroups(using(CryptolLexer), Literal),
             '#pop')
        ]
    }

def test(file):
    f = open(file)
    for t in pygments.lex(f.read(), SAWScriptLexer()):
        print(t)
